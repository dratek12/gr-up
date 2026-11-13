#include "CommonIncludes.h"
#include "Global.h"
#include "SDK.h"
#include "ESP.h"
#include "OverlayGeometry.h"
#include "WeaponAccuracy.h"
#include "SpreadGeometry.h"
#include "AimChecks.h"
#include "WorldVisuals.h"

#include <imgui.h>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "PlayerSnapshot.h"
#include "SoundEspPolicy.h"

static bool W2S(const Vec3& world, ImVec2& out) {
    float matrix[16]{};
    return Sdk::Read(Sdk::Client() + Off::ViewMatrix, matrix, sizeof(matrix)) &&
        Sdk::W2S(matrix, world, out, ImGui::GetIO().DisplaySize);
}

// ---------------------------------------------------------------- kresba
static void DrawPlayer(ImDrawList* d, const CPlayer& p, const ImVec2& top, const ImVec2& bot) {
    const auto& st = G::Ui.state.preview;
    ImU32 col = ImGui::ColorConvertFloat4ToU32(st.colBox);
    ImU32 txt = ImGui::ColorConvertFloat4ToU32(st.colName);

    ImVec2 boxMin{}, boxMax{};
    if (!PlayerBounds(top, bot, boxMin, boxMax)) return;
    ImVec2 lo(boxMin.x - 1, boxMin.y - 1);
    ImVec2 hi(boxMax.x + 1, boxMax.y + 1);
    float h = hi.y - lo.y;
    float cx = (lo.x + hi.x) * 0.5f;

    if (st.boxes) {
        d->AddRect(lo, hi, IM_COL32(0, 0, 0, 255));
        d->AddRect(boxMin, boxMax, col);
    }

    if (st.health) {
        float f = (p.health > 100 ? 100.f : static_cast<float>(p.health)) / 100.f;
        float barTop = hi.y - h * f;
        d->AddRectFilled(ImVec2(lo.x - 6, lo.y), ImVec2(lo.x - 2, hi.y), IM_COL32(12, 12, 12, 255));
        d->AddRectFilled(ImVec2(lo.x - 5, barTop), ImVec2(lo.x - 3, hi.y), ImGui::ColorConvertFloat4ToU32(st.colHealth));
        char buf[8]{};
        sprintf_s(buf, "%d", p.health);
        d->AddText(ImVec2(lo.x - 24, barTop - 6), ImGui::ColorConvertFloat4ToU32(st.colHealth), buf);
    }

    if (st.armour && p.armour > 0) {
        float f = (p.armour > 100 ? 100.f : static_cast<float>(p.armour)) / 100.f;
        float barTop = hi.y - h * f;
        d->AddRectFilled(ImVec2(hi.x + 2, lo.y), ImVec2(hi.x + 6, hi.y), IM_COL32(12, 12, 12, 255));
        d->AddRectFilled(ImVec2(hi.x + 3, barTop), ImVec2(hi.x + 5, hi.y), ImGui::ColorConvertFloat4ToU32(st.colArmor));
        char buf[8]{};
        sprintf_s(buf, "%d", p.armour);
        d->AddText(ImVec2(hi.x + 8, barTop - 6), ImGui::ColorConvertFloat4ToU32(st.colArmor), buf);
    }

    if (st.names) {
        const char* n = p.name[0] ? p.name : "player";
        ImVec2 sz = ImGui::CalcTextSize(n);
        d->AddText(ImVec2(cx - sz.x * 0.5f, lo.y - sz.y - 2), txt, n);
    }

    if (st.weapon && p.weapon[0]) {
        ImVec2 sz = ImGui::CalcTextSize(p.weapon);
        d->AddText(ImVec2(cx - sz.x * 0.5f, hi.y + 2), ImGui::ColorConvertFloat4ToU32(st.colWeapon), p.weapon);
    }

    ImVec2 head{};
    if (st.head && p.headValid && W2S(p.head, head)) {
        float r = 2.f;
        for (const Vec3 offset : {Vec3(4.5f,0,0),Vec3(0,4.5f,0),Vec3(0,0,4.5f)}) {
            ImVec2 edge{};
            if (W2S(p.head + offset, edge)) r = std::max(r, std::hypot(edge.x-head.x, edge.y-head.y));
        }
        r = std::min(r, 48.f);
        for(int n=3;n>0;--n) d->AddCircle(head,r+n,IM_COL32(50,255,90,int(18*st.colHead.w)),32,2.f);
        d->AddCircle(head, r, ImGui::ColorConvertFloat4ToU32(st.colHead), 32);
    }

    if (st.snaplines) {
        ImVec2 ds = ImGui::GetIO().DisplaySize;
        d->AddLine(ImVec2(ds.x * 0.5f, ds.y), ImVec2(cx, hi.y), ImGui::ColorConvertFloat4ToU32(st.colSpline));
    }
}

static void DrawArrow(ImDrawList* d, const CPlayer& p) {
    const auto& cfg = G::Ui.state.preview;
    if (!cfg.arrows || !G::LocalAlive) return;
    ImVec2 point{}, size=ImGui::GetIO().DisplaySize;
    if (W2S(p.head,point) && point.x>=0 && point.y>=0 && point.x<size.x && point.y<size.y) return;
    const auto view=Sdk::R<Vec3>(Sdk::Client()+Off::ViewAngles);
    const auto delta=p.origin-G::LocalOrigin;
    const float angle=std::atan2(delta.y,delta.x)-view.y*.01745329252f;
    if(!std::isfinite(angle))return;
    ImVec2 direction(-std::sin(angle),-std::cos(angle));
    const float radius=std::min(cfg.arrowRadius,std::min(size.x,size.y)*.45f);
    ImVec2 c(size.x*.5f+direction.x*radius,size.y*.5f+direction.y*radius);
    ImVec2 side(-direction.y*5.f,direction.x*5.f);
    d->AddTriangleFilled(ImVec2(c.x+direction.x*9,c.y+direction.y*9),
        ImVec2(c.x-direction.x*5+side.x,c.y-direction.y*5+side.y),
        ImVec2(c.x-direction.x*5-side.x,c.y-direction.y*5-side.y),ImGui::ColorConvertFloat4ToU32(cfg.colArrows));
}

static void DrawCrosshair(ImDrawList* d) {
    const auto& cfg=G::Ui.state.preview;
    if(!G::LocalAlive || (!cfg.crosshair && !cfg.spreadMode))return;
    const auto size=ImGui::GetIO().DisplaySize;
    const ImVec2 center(size.x*.5f,size.y*.5f);
    auto cross=[&](ImVec2 c,ImU32 color) {
        d->AddLine(ImVec2(c.x-4,c.y),ImVec2(c.x+4,c.y),color,1.5f);
        d->AddLine(ImVec2(c.x,c.y-4),ImVec2(c.x,c.y+4),color,1.5f);
    };
    if(cfg.crosshair)cross(center,ImGui::ColorConvertFloat4ToU32(cfg.colCrosshair));
    if(!cfg.spreadMode)return;
    const auto client=Sdk::Client();
    const auto pawn=Sdk::R<uintptr_t>(client+Off::LocalPlayer);
    WeaponAccuracy accuracy;
    if(!ReadWeaponAccuracy(pawn,Sdk::EntityList(),accuracy))return;
    float matrix[16]{};
    if(!Sdk::Read(client+Off::ViewMatrix,matrix,sizeof(matrix)))return;
    // PredictableBaseAngle is an impulse, not the current recoil angle. Landing
    // updates it and leaves it unchanged after camera recovery. Never use it to
    // displace a spread indicator: spread is a cone around the current view.
    // Radius is the estimated angular cone, not a prediction of a random bullet seed.
    SpreadGeometry geometry;
    if(!MakeSpreadGeometry(matrix,size,accuracy.inaccuracy+accuracy.spread,geometry))return;
    const auto aim=geometry.center;
    d->AddCircle(aim,geometry.radius,IM_COL32(50,255,90,65),48);
    if(cfg.spreadMode==1)cross(aim,IM_COL32(50,255,90,230));
    else d->AddCircle(aim,3.f,IM_COL32(50,255,90,230),24,1.5f);
}

static void DrawSoundCircles(ImDrawList* d) {
    struct Pulse {double time;Vec3 origin;};
    static std::unordered_map<uint32_t,Pulse> pulses;
    const auto& cfg=G::Ui.state.extra;
    if(!cfg.soundCircles || !G::LocalAlive){pulses.clear();return;}
    const double now=GetTickCount64()/1000.;
    std::erase_if(pulses,[&](const auto& pair){return now<pair.second.time || now-pair.second.time>=cfg.soundCircleDuration ||
        std::none_of(G::Players.begin(),G::Players.end(),[&](const CPlayer& p){return p.handle==pair.first&&p.valid&&p.health>0&&(!cfg.soundCircleTeam||p.team!=G::LocalTeam);});});
    for(const auto& p:G::Players) {
        if(!p.valid||p.health<=0||(cfg.soundCircleTeam&&p.team==G::LocalTeam)||p.soundHeardAt<0||now-p.soundHeardAt>=cfg.soundCircleDuration)continue;
        auto it=pulses.find(p.handle);
        if(it==pulses.end()||it->second.time!=p.soundHeardAt)pulses[p.handle]={p.soundHeardAt,p.origin};
    }
    for(const auto& [id,p]:pulses){
        const float t=float((now-p.time)/cfg.soundCircleDuration);
        if(t<0||t>=1)continue;
        const float radius=cfg.soundCircleRadius*(.15f+.85f*t);
        auto color=cfg.soundCircleColor;color.w*=1-t;
        for(int i=0;i<64;++i){
            const float a=i*6.2831853f/64,b=(i+1)*6.2831853f/64;ImVec2 start,end;
            if(W2S(p.origin+Vec3(std::cos(a)*radius,std::sin(a)*radius,2),start)&&
               W2S(p.origin+Vec3(std::cos(b)*radius,std::sin(b)*radius,2),end))
                d->AddLine(start,end,ImGui::ColorConvertFloat4ToU32(color),cfg.soundCircleWidth);
        }
    }
}
// ---------------------------------------------------------------- vstup
void DrawESP() {
    if (!G::Ready) return;

    __try {
        UpdatePlayers();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        G::Players.clear();
        return;
    }

    ImDrawList* d = ImGui::GetForegroundDrawList();
    if (!d) return;
    DrawSoundCircles(d);
    DrawCrosshair(d);

    for (auto& p : G::Players) {
        if (!p.valid) continue;
        if (G::Ui.state.cs2.team && G::LocalTeam && p.team == G::LocalTeam) continue;
        if (!SoundEspPolicy::ShowPlayer(G::Ui.state.preview.sound,p.lineOfSight,p.soundVisible)) continue;

        if (!WithinEspDistance(p.origin, G::LocalOrigin, G::Ui.state.cs2.dist)) continue;
        DrawArrow(d,p);
        if(G::Ui.state.preview.eyeRay && p.eyeAnglesValid && p.team!=G::LocalTeam) {
            Vec3 direction{},end{};
            if(WorldVisuals::Direction(p.eyeAngles,G::Ui.state.preview.eyeRayLength,direction) &&
               AimChecks::RayEnd(p.entity,p.eye,p.eye+direction,end)) {
                // Sample short segments so a ray crossing the near plane remains drawable.
                Vec3 previous=p.eye;ImVec2 a{},b{};
                for(int j=1;j<=64;++j) {
                    const auto diff=end-p.eye;const float t=j/64.f;
                    Vec3 point=p.eye+Vec3(diff.x*t,diff.y*t,diff.z*t);
                    if(W2S(previous,a)&&W2S(point,b))d->AddLine(a,b,ImGui::ColorConvertFloat4ToU32(G::Ui.state.preview.colEyeRay),1.5f);
                    previous=point;
                }
            }
        }

        ImVec2 top{}, bot{};
        if (!W2S(p.head + Vec3(0, 0, 8.f), top)) continue;
        if (!W2S(p.origin, bot)) continue;
        if (top.y > bot.y) continue;

        DrawPlayer(d, p, top, bot);
    }
}
