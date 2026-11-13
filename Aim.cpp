#include "Aim.h"
#include "Global.h"
#include "AimGeometry.h"
#include "RcsLogic.h"
#include "AimChecks.h"
#include "NonStickyAim.h"
#include "AimFov.h"
#include "AimDeviation.h"
#include "AimMotion.h"
#include "ClickAssist.h"
#include "AimCadence.h"
#include "ManualMouse.h"
#include "MousePollGate.h"
#include "AimTrace.h"
static ClickAssist::Edge click;
static ClickAssist::KeyEdge shotKey;
static ClickAssist::ShotSequence shotSequence;
static AimCadence cadence;
static int inputWeapon=-1;
static RcsLogic::State recoilState;
static NonSticky::State assist;
static int lastMode=-1,lastBone=-1;static float lastFov=-1;
static int lastLegitMode=-1;
static AimMotion::Curve motion;static AimMotion::Counts aimCounts;
static uint32_t motionTarget=0xFFFFFFFFu;static int motionBone=0;
static bool lastTimed=false;
static void ResetMotion(){cadence.Reset();motion.Reset();aimCounts.Reset();motionTarget=0xFFFFFFFFu;motionBone=0;}
static void ResetShot(){
    if(shotSequence.Cancel()){
        INPUT up{};up.type=INPUT_MOUSE;up.mi.dwFlags=MOUSEEVENTF_LEFTUP;up.mi.dwExtraInfo=ManualMouse::ShotInjectionTag;
        SendInput(1,&up,sizeof(up));
    }
    MousePollGate::AllowShot(false);
}
static int previousShots=0; static uint64_t lastShot=0;
void ResetAim() { ManualMouse::SetShotIntercept(false);ResetShot();shotKey.Reset();click.Reset(); recoilState.Reset();assist.Reset();ResetMotion();previousShots=0;lastShot=0; }

void RunAim(bool allowInput) {
    AimTrace::Frame trace;trace.allow=allowInput;
    if(lastLegitMode!=G::Ui.state.legit){ResetAim();lastLegitMode=G::Ui.state.legit;}
    if(!allowInput)ResetAim();
    if (!G::LocalAlive || !Sdk::Foreground()) { ResetAim(); return; }
    auto client = Sdk::Client();
    auto local = Sdk::R<uintptr_t>(client + Off::LocalPlayer);
    int weapon = Sdk::ActiveWeaponId(local);
    if (!getrekt::Weapons::Find(weapon)) { ResetAim(); return; }
    const auto& cfg = G::Ui.state.ActiveAimConfig(weapon, getrekt::Weapons::Category(weapon));
    const bool onShot=cfg.aimStyle==2;
    const bool mouseShot=allowInput&&cfg.on&&onShot&&cfg.key==VK_LBUTTON&&MousePollGate::Ready();
    if(!mouseShot)ManualMouse::SetShotIntercept(false);
    if(assist.weapon!=weapon||lastMode!=cfg.fovMode||lastBone!=cfg.aimBones||lastFov!=cfg.fov||lastTimed!=cfg.timedAssist){assist.Reset();ResetMotion();}
    lastTimed=cfg.timedAssist;
    lastMode=cfg.fovMode;lastBone=cfg.aimBones;lastFov=cfg.fov;
    if(inputWeapon!=weapon){click.Reset();cadence.Reset();inputWeapon=weapon;}
    const bool bound=cfg.key>0&&cfg.key<256;
    const bool keyHeld=bound&&(GetAsyncKeyState(cfg.key)&0x8000);
    const bool clicked=click.Consume(allowInput&&cfg.on&&bound&&(cfg.aimStyle==1||mouseShot),cfg.key,
        mouseShot?ManualMouse::ShotPressSerial():ManualMouse::PressSerial(cfg.key));
    const bool keyboardShot=shotKey.Consume(allowInput&&cfg.on&&onShot&&bound&&cfg.key!=VK_LBUTTON,cfg.key,keyHeld);
    const bool shotPressed=onShot&&(cfg.key==VK_LBUTTON?clicked:keyboardShot);
    Vec3 current{}, camera{}; float matrix[16]{};
    if (!Sdk::Read(client+Off::ViewAngles,&current,sizeof(current)) ||
        !Sdk::Read(client+Off::ViewMatrix,matrix,sizeof(matrix)) || !RcsLogic::Camera(matrix,camera) ||
        !std::isfinite(current.x) || !std::isfinite(current.y)) { ResetAim(); return; }
    AimFov::Circle circle;
    const auto size=ImGui::GetIO().DisplaySize;
    const bool circleValid=circle.Set(matrix,size,cfg.fov);
    auto draw=[&](ImVec2 center, const ImVec4& color, float radius) {
        if(!circleValid || !std::isfinite(radius) || radius<=0)return;
        ImGui::GetBackgroundDrawList()->AddCircle(center,radius,ImGui::ColorConvertFloat4ToU32(color),96,1.f);
    };
    if(cfg.drawFov && cfg.fovMode!=AimFov::FollowLocked)draw(circle.center,cfg.fovColor,circle.radius);
    const auto now=GetTickCount64();
    auto fallbackShot=[&](){
        if(onShot&&cfg.key==VK_LBUTTON&&shotPressed&&shotSequence.Arm(local,weapon,now))
            G::Log("key-shot fallback: weapon=%d",weapon);
    };
    if(!onShot||!cfg.on)ResetShot();
    else {
        const int action=shotSequence.Poll(local,weapon,now);
        if(action<0)ResetShot();
        else if(action==1){
            if(cfg.key!=VK_LBUTTON&&(GetAsyncKeyState(VK_LBUTTON)&0x8000))ResetShot();
            else {
                INPUT down{};down.type=INPUT_MOUSE;down.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;down.mi.dwExtraInfo=ManualMouse::ShotInjectionTag;
                MousePollGate::AllowShot(true);
                if(SendInput(1,&down,sizeof(down))!=1)ResetShot();
                else G::Log("aim shot fired: weapon=%d",weapon);
            }
        }else if(action==2){
            INPUT up{};up.type=INPUT_MOUSE;up.mi.dwFlags=MOUSEEVENTF_LEFTUP;up.mi.dwExtraInfo=ManualMouse::ShotInjectionTag;
            SendInput(1,&up,sizeof(up));
            MousePollGate::AllowShot(false);
        }
    }
    trace.stage="checks";
    if (!AimChecks::Begin(local,Sdk::EntityList(),G::LocalEye,cfg.flashCheck,cfg.smokeCheck,cfg.wallCheck)) {
        ManualMouse::SetShotIntercept(false);fallbackShot();assist.Reset();ResetMotion();return;
    }
    if(mouseShot&&circleValid)ManualMouse::SetShotIntercept(true);
    const bool grounded = (Sdk::R<uint32_t>(local+Off::Flags)&1)!=0;
    int shots=0;
    Sdk::Read(local+Off::ShotsFired,&shots,sizeof(shots));
    const bool discrete=cfg.aimStyle==1 || onShot;
    bool aimHeld=AimDeviation::Activated(allowInput,cfg.on,cfg.always,bound,keyHeld);
    if (discrete) aimHeld=onShot ? shotPressed : clicked;
    if(recoilState.owner!=local || recoilState.weapon!=weapon) { previousShots=shots;lastShot=0; }
    if(shots>previousShots) lastShot=now;
    previousShots=shots;
    const bool shooting = shots>0 && ((GetAsyncKeyState(VK_LBUTTON)&0x8000) || (lastShot && now-lastShot<150));
    const auto correction = recoilState.Step(local,weapon,allowInput && !discrete && cfg.rcs && (cfg.rcsAlways || aimHeld) && grounded,
        shooting,AimGeometry::Difference(camera,current),cfg,Sdk::RandRange(0,1),Sdk::RandRange(0,1));
    auto recoilMouse = AimGeometry::Mouse(correction,G::Sens,1);
    auto applyRecoil=[&](){if(allowInput)Sdk::SendMouse(recoilMouse.x,recoilMouse.y);};
    if (!circleValid || cfg.fovMode<0 || cfg.fovMode>2) { fallbackShot();ManualMouse::SetShotIntercept(false);applyRecoil();assist.Reset();ResetMotion();return; }
    if(!aimHeld || (cfg.jump&&!grounded)){assist.Reset();ResetMotion();}
    if(!aimHeld && !cfg.showTarget){
        trace.stage="inactive";
        applyRecoil();
        if(cfg.drawFov && cfg.fovMode==AimFov::FollowLocked)draw(circle.center,cfg.fovColor,circle.radius);
        return;
    }

    struct Candidate {const CPlayer* player;ImVec2 screen;Vec3 delta,world;float distance;int bone;float radius;};
    std::vector<Candidate> candidates;
    const Candidate* best=nullptr;
    // Auto picks one eligible point per player, so each target gets one circle.
    auto consider=[&](const CPlayer& player) {
        if(!player.valid || player.health<=0 || (!cfg.ff && G::LocalTeam && player.team==G::LocalTeam))return;
        std::vector<Candidate> points;
        for(int bone=1;bone<=5;++bone) {
            if(!AimGeometry::Selected(cfg.aimBones,bone))continue;
            if(!AimGeometry::Valid(bone,player))continue;
            Vec3 angle{};const auto point=AimGeometry::Point(bone,player);
            if(!AimGeometry::Angles(G::LocalEye,point,angle))continue;
            ImVec2 screen{};if(!Sdk::W2S(matrix,point,screen,size))continue;
            if(screen.x<0||screen.x>size.x||screen.y<0||screen.y>size.y)continue;
            const float distance=circle.Distance(screen);
            if(!std::isfinite(distance))continue;
            const Vec3 relative=point-G::LocalEye;
            const float radius=AimFov::TargetRadius(cfg.fovMode,circle.radius,std::hypot(std::hypot(relative.x,relative.y),relative.z),cfg.targetFovSizing==1);
            if(!std::isfinite(radius)||radius<=0)continue;
            // Outside-FOV traces are needed only for visible target-circle previews.
            if(distance>radius && !cfg.showTarget)continue;
            Vec3 difference=AimGeometry::Difference(angle,camera);
            if(cfg.aimStyle==0&&!AimGeometry::ScreenDifference(matrix,size,screen,difference))continue;
            points.push_back({&player,screen,difference,point,distance,bone,radius});
        }
        std::sort(points.begin(),points.end(),[&](const Candidate& a,const Candidate& b){
            const bool aInside=a.distance<=a.radius,bInside=b.distance<=b.radius;
            if(aInside!=bInside)return aInside;
            return AimStableDistance(a.distance,cfg.aimStyle==0&&a.player->handle==motionTarget&&a.bone==motionBone)<AimStableDistance(b.distance,cfg.aimStyle==0&&b.player->handle==motionTarget&&b.bone==motionBone);
        });
        for(const auto& point:points){
            if(AimChecks::Clear(local,player.entity,G::LocalEye,point.world,cfg.wallCheck)){candidates.push_back(point);break;}
        }
    };
    for(const auto& player:G::Players)consider(player);
    for(const auto& candidate:candidates)if(circle.Contains(candidate.screen,candidate.radius) && (!best ||
        AimFov::Prefer(cfg.fovMode,assist.target,candidate.player->handle,candidate.distance,best->player->handle,best->distance)))best=&candidate;
    if(cfg.aimStyle==0&&best&&cfg.fovMode!=AimFov::FollowLocked){
        for(const auto& c:candidates)if(c.player->handle==motionTarget&&circle.Contains(c.screen,c.radius)&&AimStableDistance(c.distance,true)<=best->distance){best=&c;break;}
    }
    const bool helping=aimHeld&&best&&!(cfg.jump&&!grounded)&&(discrete||assist.Step(best->player->handle,weapon,aimHeld,now,cfg.reactMs,cfg.lockMs,cfg.releaseMs,cfg.timedAssist));
    if(cfg.showTarget)for(const auto& candidate:candidates)draw(candidate.screen,cfg.targetColor,candidate.radius);
    if(cfg.drawFov && cfg.fovMode==AimFov::FollowLocked){
        const auto locked=std::find_if(candidates.begin(),candidates.end(),[&](const Candidate& c){return c.player->handle==assist.target&&circle.Contains(c.screen);});
        draw(locked==candidates.end()?circle.center:locked->screen,cfg.fovColor,circle.radius);
    }
    static int previousStatus=-1;
    const int status=(aimHeld?1:0)|(best?2:0)|(helping?4:0);
    if(status!=previousStatus) { G::Log("Legit aim: active=%d target=%d assist=%d weapon=%d mode=%d",aimHeld,best!=nullptr,helping,weapon,G::Ui.state.legit);previousStatus=status; }
    trace.stage=best?"nonsticky":"no-target";
    if(!helping){fallbackShot();applyRecoil();ResetMotion();return;}
    trace.target=best->player->handle;
    if(motionTarget!=best->player->handle||motionBone!=best->bone){ResetMotion();motionTarget=best->player->handle;motionBone=best->bone;}
    Vec3 delta=best->delta;
    trace.stage="deviation";
    if(cfg.aimDeviation>0){
        const auto point=best->world+AimDeviation::Offset(best->player->handle,weapon,best->bone,cfg.aimDeviation);
        ImVec2 screen{};Vec3 angle{};
        if(!Sdk::W2S(matrix,point,screen,size)||!circle.Contains(screen,best->radius)||
           !AimGeometry::Angles(G::LocalEye,point,angle)||!AimChecks::Clear(local,best->player->entity,G::LocalEye,point,cfg.wallCheck)){
            fallbackShot();assist.Reset();ResetMotion();return;
        }
        delta=AimGeometry::Difference(angle,camera);
        if(cfg.aimStyle==0&&!AimGeometry::ScreenDifference(matrix,size,screen,delta)){ResetMotion();return;}
    }
    const float smoothing=cfg.aimStyle==0&&cfg.separateShootSmooth&&shooting?cfg.speedShoot:cfg.speed;
    trace.x=delta.x;trace.y=delta.y;
    const float tolerance=std::max(.01f,G::Sens*.022f*.75f);
    if(discrete){
        trace.stage=onShot ? "key-shot" : "click";
        const auto move=AimGeometry::Mouse(ClickAssist::Step(delta,cfg.fov,smoothing),G::Sens,1);
        // One key press owns one correction and, after game input applies it, one shot.
        G::Log("%s: weapon=%d target=%u bone=%d dx=%d dy=%d",trace.stage,weapon,best->player->handle,best->bone,int(std::trunc(move.x)),int(std::trunc(move.y)));
        Sdk::SendMouseCounts(int(std::trunc(move.x)),int(std::trunc(move.y)));
        if(onShot)shotSequence.Arm(local,weapon,now);
        ResetMotion();return;
    }
    trace.stage="tolerance";
    if(std::hypot(delta.x,delta.y)<=tolerance){cadence.Reset();motion.Reset();aimCounts.Reset();return;}
    const float dt=cadence.Next(camera,now);
    trace.dt=dt;trace.stage="cadence";
    if(dt<=0)return;
    const auto step=motion.Step(delta,smoothing,cfg.fov,dt,tolerance);
    const auto counts=aimCounts.Step(AimGeometry::Mouse(step,G::Sens,1));
    trace.stage=(counts.x||counts.y)?"send":"fractional";trace.dx=int(counts.x);trace.dy=int(counts.y);
    if(counts.x||counts.y){Sdk::SendMouseCounts(int(counts.x),int(counts.y));cadence.Sent(camera,now);}
}
