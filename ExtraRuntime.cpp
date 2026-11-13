#include "ExtraRuntime.h"
#include "MemoryOverride.h"
#include "ClientConVars.h"
#include "NativeJump.h"
#include "ThirdPerson.h"
#include "ManualMouse.h"
#include "Global.h"
#include "AimChecks.h"
#include "WorldVisuals.h"
#include <deque>
#include <unordered_map>
#include <mmsystem.h>
namespace ExtraRuntime {
namespace {
std::array<MemoryOverride,8> overrides;
uintptr_t lastPawn=0,lastList=0;
uint32_t lastHandle=0xFFFFFFFFu;
DamageCounter damageCounter;
FlashPeriod flashPeriod;
float lastDamage=0;double lastHit=-100;
struct Event {double time;std::string text;};
struct Line {double until;Vec3 a,b;bool local;};
struct Shot {uint32_t weapon;float time;};
struct Spot {Vec3 position,angles;int type;};
std::deque<Event> events;
std::deque<Line> lines;
std::vector<Spot> spots;
std::unordered_map<uint32_t,int> health;
std::unordered_map<uint32_t,Shot> shots;
double nextInfo=0,nextPath=0;
std::vector<Vec3> path;
bool damageAvailable=false;
struct Info {std::string name;int health,team,rank,wins;};
std::vector<Info> info;
uint32_t Handle(uintptr_t e){return Sdk::R<uint32_t>(Sdk::R<uintptr_t>(e+Off::EntityIdentity)+Off::EntityHandle);}
void EventAt(double now,std::string text) {
    events.push_back({now,std::move(text)});while(events.size()>8)events.pop_front();
}
bool Screen(Vec3 p,ImVec2& out) {
    float matrix[16]{};return Sdk::Read(Sdk::Client()+Off::ViewMatrix,matrix,sizeof(matrix)) &&
        Sdk::W2S(matrix,p,out,ImGui::GetIO().DisplaySize);
}
void WorldLine(ImDrawList* d,Vec3 a,Vec3 b,ImU32 color) {
    const Vec3 delta=b-a;
    for(int n=0;n<32;++n) {
        float t=n/32.f,u=(n+1)/32.f;ImVec2 x{},y{};
        if(Screen(a+Vec3(delta.x*t,delta.y*t,delta.z*t),x) && Screen(a+Vec3(delta.x*u,delta.y*u,delta.z*u),y))
            d->AddLine(x,y,color,1.5f);
    }
}
void Camera(uintptr_t pawn,uintptr_t list) {
    const auto& c=G::Ui.state.extra;
    const auto camera=Sdk::R<uintptr_t>(pawn+Off::CameraServices);
    const bool alive=G::LocalAlive;
    flashPeriod.Observe(Sdk::R<float>(pawn+Off::FlashMaxAlpha),Sdk::R<float>(pawn+Off::FlashOverlay),alive&&c.noFlash,GetTickCount64()/1000.);
    G::FlashBlockedUntil=flashPeriod.until;
    auto apply=[&](int n,bool enabled,uintptr_t owner,uintptr_t field,auto value,uintptr_t ref=0,uintptr_t exp=0) {
        if(enabled && alive) {
            if(!overrides[n].Set(list,owner,field,value,ref,exp))G::Ui.state.extraStatus="Camera field unavailable in this build";
        } else overrides[n].Restore(list);
    };
    // Leave overlay/screenshot alpha intact: mandatory aim flash check still sees blindness.
    apply(0,c.noFlash,pawn,pawn+Off::FlashOverlayEnd,0.f);
    const bool scoped=Sdk::R<bool>(pawn+Off::Scoped);
    const bool cameraKey=!c.thirdPersonKey || (Sdk::Foreground()&&ManualMouse::PhysicalKey(c.thirdPersonKey));
    if(!ThirdPerson::Apply(c.thirdPerson&&alive&&!scoped&&cameraKey,c.thirdPersonDistance))
        G::Ui.state.extraStatus="Third person unavailable for this game build";
    const bool fov=(c.playerFovOn&&!scoped) || (c.noScopeZoom&&scoped);
    const float value=c.playerFovOn?std::clamp(c.playerFov,60.f,140.f):90;
    bool supported=ClientConVars::Float("fov_cs_debug",alive&&fov,value);
    supported=ClientConVars::Float("viewmodel_fov",alive&&c.viewmodelFovOn,std::clamp(c.viewmodelFov,54.f,120.f))&&supported;
    supported=ClientConVars::Float("mat_tonemap_force_scale",alive&&c.night,std::clamp(c.exposure,.05f,1.f))&&supported;
    supported=ClientConVars::Bool("r_drawviewmodel",alive&&c.hideViewmodel,false)&&supported;
    supported=ClientConVars::Bool("cl_disable_postprocessing",alive&&c.noPostprocessing,true)&&supported;
    if(!supported)G::Ui.state.extraStatus="Required client setting unavailable in this build";
    apply(4,c.noVisualRecoil&&camera>=0x10000,pawn,camera+Off::CameraRecoil,Vec3{},pawn+Off::CameraServices,camera);
}
void PlayerInfo(uintptr_t pawn,uintptr_t list,double now) {
    if(now<nextInfo)return;nextInfo=now+.2;
    info.clear();G::Ui.state.spectatorRows.clear();
    uintptr_t observing=pawn;
    if(!G::LocalAlive) {
        const auto services=Sdk::R<uintptr_t>(pawn+Off::ObserverServices);
        const auto target=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(services+Off::ObserverTarget));
        if(target)observing=target;
    }
    for(int i=1;i<=64;++i) {
        const auto controller=Sdk::EntAt(list,i);if(!controller)continue;
        auto player=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(controller+Off::Pawn));
        auto observer=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(controller+Off::ControllerObserverPawn));
        if(!player)player=observer;
        if(!player || Sdk::ResolveHandle(list,Sdk::R<uint32_t>(player+Off::Controller))!=controller)continue;
        char name[64]{};if(!Sdk::ReadChars(controller+Off::PlayerName,name,sizeof(name)))continue;
        const bool pawnAlive=Sdk::R<bool>(controller+Off::ControllerPawnAlive);
        const int hp=pawnAlive?Sdk::R<int>(player+Off::Health):0,team=Sdk::R<uint8_t>(player+Off::Team);
        if(team<1||team>3||hp<0||hp>1000)continue;
        info.push_back({name,hp,team,Sdk::R<int>(controller+Off::ControllerCompetitiveRank),Sdk::R<int>(controller+Off::ControllerCompetitiveWins)});
        if(player==pawn || pawnAlive)continue;
        if(!observer)observer=player;
        const auto services=Sdk::R<uintptr_t>(observer+Off::ObserverServices);
        const auto mode=Sdk::R<uint8_t>(services+Off::ObserverMode);
        if((mode==4||mode==5) && Sdk::ResolveHandle(list,Sdk::R<uint32_t>(services+Off::ObserverTarget))==observing)
            G::Ui.state.spectatorRows.push_back({name,mode==5,0});
    }
}
void TrackShot(uintptr_t entity,uint32_t handle,Vec3 eye,Vec3 angles,bool local,uintptr_t list,double now) {
    const auto services=Sdk::R<uintptr_t>(entity+Off::WeaponServices);
    uint32_t weaponHandle=Sdk::R<uint32_t>(services+Off::ActiveWeapon);
    auto weapon=Sdk::ResolveHandle(list,weaponHandle);float t=Sdk::R<float>(weapon+Off::LastShotTime);
    if(!weapon || !std::isfinite(t)||t<0)return;
    const auto it=shots.find(handle);const bool fired=it!=shots.end() && it->second.weapon==weaponHandle && t>it->second.time;
    shots[handle]={weaponHandle,t};
    if(!fired)return;
    const auto& c=G::Ui.state.extra;if(!(local?c.tracer:c.tracerOthers))return;
    Vec3 direction{},end{};
    if(WorldVisuals::Direction(angles,8192,direction) && AimChecks::RayEnd(entity,eye,eye+direction,end)) {
        lines.push_back({now+c.tracerDuration,eye,end,local});while(lines.size()>128)lines.pop_front();
    }
}
void Feedback(uintptr_t pawn,uintptr_t list,double now) {
    const auto controller=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalController);
    const auto actions=Sdk::R<uintptr_t>(controller+Off::ControllerDamageActions);float dealt=0;
    damageAvailable=actions>=0x10000 && Sdk::Read(actions+Off::DamageDealt,&dealt,4) && std::isfinite(dealt)&&dealt>=0&&dealt<1000000;
    if(damageAvailable) {
        const float delta=damageCounter.Update(dealt);
        if(delta>0) {
            lastHit=now;lastDamage=delta;
            char text[64];snprintf(text,sizeof(text),"Damage dealt: %.0f",delta);EventAt(now,text);
            // Built-in short waveforms; no worker outliving the DLL and no blocking Beep.
            static std::array<unsigned char,44+800> wave{};
            if(G::Ui.state.extra.hitSound) {
                PlaySoundA(nullptr,nullptr,0);
                const unsigned char header[44]={'R','I','F','F',0,0,0,0,'W','A','V','E','f','m','t',' ',16,0,0,0,1,0,1,0,0x40,0x1f,0,0,0x40,0x1f,0,0,1,0,8,0,'d','a','t','a',0,0,0,0};
                memcpy(wave.data(),header,44);uint32_t riff=836,data=800;memcpy(wave.data()+4,&riff,4);memcpy(wave.data()+40,&data,4);
                for(int i=0;i<800;++i){float envelope=std::max(0.f,1-i/400.f);if(G::Ui.state.extra.hitSound==2 && i>=400)envelope=std::max(0.f,1-(i-400)/400.f);wave[44+i]=uint8_t(128+50*envelope*std::sin(i*.9f));}
                PlaySoundA((LPCSTR)wave.data(),nullptr,SND_MEMORY|SND_ASYNC|SND_NODEFAULT);
            }
        }
    } else damageCounter.ready=false;
    std::unordered_map<uint32_t,int> current;
    for(const auto& p:G::Players) {
        current[p.handle]=p.health;auto it=health.find(p.handle);
        if(it!=health.end() && p.health<it->second)EventAt(now,std::string(p.name)+" HP: "+std::to_string(p.health));
        if(p.eyeAnglesValid)TrackShot(p.entity,p.handle,p.eye,p.eyeAngles,false,list,now);
    }
    for(const auto& [handle,hp]:health)if(!current.contains(handle)) {
        const auto entity=Sdk::ResolveHandle(list,handle);
        if(entity && Sdk::R<int>(entity+Off::Health)<=0)EventAt(now,"Player died");
    }
    health=std::move(current);
    if(G::LocalAlive)TrackShot(pawn,Handle(pawn),G::LocalEye,Sdk::R<Vec3>(Sdk::Client()+Off::ViewAngles),true,list,now);
    if(shots.size()>256)shots.clear();
    while(!events.empty()&&now-events.front().time>6)events.pop_front();
    while(!lines.empty()&&lines.front().until<now)lines.pop_front();
}
bool Grenade(int id){return id==43||id==44||id==45||id==46||id==47||id==48;}
void Grenades(uintptr_t pawn,uintptr_t list,double now,ImDrawList* d) {
    auto& state=G::Ui.state;auto& c=state.extra;
    if(state.clearGrenadeSpots){spots.clear();state.clearGrenadeSpots=false;}
    if(state.saveGrenadeSpot) {
        state.saveGrenadeSpot=false;
        if(G::LocalAlive && spots.size()<32)spots.push_back({G::LocalOrigin,Sdk::R<Vec3>(Sdk::Client()+Off::ViewAngles),c.grenadeType});
    }
    if(!G::LocalAlive){path.clear();return;}
    if(c.grenadeHelper&&c.locations)for(const auto& s:spots) {
        if(s.type!=c.grenadeType)continue;
        const auto diff=s.position-G::LocalOrigin;
        if(std::sqrt(diff.x*diff.x+diff.y*diff.y+diff.z*diff.z)>c.helperDistance)continue;
        ImVec2 p{};if(Screen(s.position,p)) {
            d->AddCircle(p,6,IM_COL32(90,255,100,255));
            char text[80];snprintf(text,sizeof(text),"Lineup  pitch %.1f  yaw %.1f",s.angles.x,s.angles.y);
            d->AddText(ImVec2(p.x+8,p.y),IM_COL32(90,255,100,255),text);
        }
    }
    if(!(c.grenadePrediction || (c.grenadeHelper&&c.trajectory)) || !Grenade(Sdk::ActiveWeaponId(pawn,list))){path.clear();return;}
    if(now>=nextPath) {
        nextPath=now+.05;path.clear();
        const auto services=Sdk::R<uintptr_t>(pawn+Off::WeaponServices);
        const auto weapon=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(services+Off::ActiveWeapon));
        float strength=Sdk::R<float>(weapon+Off::GrenadeThrowStrength);if(!std::isfinite(strength))return;strength=std::clamp(strength,0.f,1.f);
        Vec3 angles=Sdk::R<Vec3>(Sdk::Client()+Off::ViewAngles),dir{};
        angles.x-=(90-std::fabs(angles.x))*10/90;
        if(!ThrowDirection(angles,dir))return;
        Vec3 velocity=Sdk::R<Vec3>(pawn+Off::GrenadeVelocity);if(!AimChecks::Finite(velocity))return;
        const float speed=675.f*(.3f+.7f*strength);
        velocity={dir.x*speed+velocity.x*1.25f,dir.y*speed+velocity.y*1.25f,dir.z*speed+velocity.z*1.25f};
        const auto start=G::LocalEye+Vec3(dir.x*16,dir.y*16,dir.z*16+12*strength-12);
        path.push_back(start);
        for(int i=1;i<=96;++i) {
            Vec3 desired=Ballistic(start,velocity,i/32.f),end{};
            if(!AimChecks::RayEnd(pawn,path.back(),desired,end))break;
            path.push_back(end);auto delta=desired-end;
            if(delta.x*delta.x+delta.y*delta.y+delta.z*delta.z>.01f)break;
        }
    }
    for(size_t i=1;i<path.size();++i)WorldLine(d,path[i-1],path[i],IM_COL32(90,255,100,220));
    if(!path.empty()){ImVec2 p{};if(Screen(path.back(),p))d->AddCircle(p,5,IM_COL32(255,180,30,255));}
}
}
void Reset() {
    ThirdPerson::Reset();
    ClientConVars::Reset();
    const auto list=Sdk::EntityList();for(auto& patch:overrides)patch.Restore(list);
    lastPawn=lastList=0;lastHandle=0xFFFFFFFFu;damageCounter={};lastHit=-100;
    flashPeriod={};G::FlashBlockedUntil=0;
    events.clear();lines.clear();shots.clear();health.clear();path.clear();spots.clear();info.clear();
    G::Ui.state.spectatorRows.clear();PlaySoundA(nullptr,nullptr,0);
}
void Frame() {
    auto& state=G::Ui.state;const auto& c=state.extra;
    const auto list=Sdk::EntityList(),pawn=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalPlayer);
    const auto handle=Handle(pawn);const double now=GetTickCount64()/1000.;
    if(lastList!=list){Reset();lastList=list;}
    if(lastPawn!=pawn || lastHandle!=handle) {
        for(auto& p:overrides)p.Restore(list);damageCounter={};lastHit=-100;health.clear();shots.clear();lines.clear();
        lastPawn=pawn;lastHandle=handle;
        flashPeriod={};G::FlashBlockedUntil=0;
        if(pawn<0x10000)spots.clear();
    }
    state.spectators=c.spectators;state.extraStatus.clear();
    Camera(pawn,list);PlayerInfo(pawn,list,now);Feedback(pawn,list,now);
    if((state.cs2.bhop||c.edgeJump)&&!NativeJump::Ready())state.extraStatus="Native jump unavailable for this game build";
    if((c.hitmarker||c.damage||c.hitSound)&&!damageAvailable)state.extraStatus="Damage counter unavailable on this server";
    auto* d=ImGui::GetForegroundDrawList();const auto size=ImGui::GetIO().DisplaySize;
    if(c.watermark)d->AddText(ImVec2(size.x-180,12),IM_COL32(255,145,30,255),"GetRekt | CS2");
    if(c.cornerInfo || state.cs2.showinfo) {
        char text[180];const Vec3 velocity=Sdk::R<Vec3>(pawn+Off::GrenadeVelocity);
        snprintf(text,sizeof(text),"FPS %.0f | HP %d | Speed %.0f | Players %zu",ImGui::GetIO().Framerate,
            Sdk::R<int>(pawn+Off::Health),AimChecks::Finite(velocity)?std::hypot(velocity.x,velocity.y):0,G::Players.size());
        d->AddText(ImVec2(12,12),IM_COL32(235,235,235,255),text);
    }
    if(now-lastHit<.5 && c.hitmarker) {
        ImVec2 center(size.x*.5f,size.y*.5f);const float r=c.hitmarkerSize;
        for(int x:{-1,1})for(int y:{-1,1})d->AddLine(ImVec2(center.x+x*3,center.y+y*3),ImVec2(center.x+x*(r+3),center.y+y*(r+3)),IM_COL32(230,255,230,255),2);
    }
    if(now-lastHit<1.5 && c.damage) {
        char text[40];snprintf(text,sizeof(text),"-%.0f",lastDamage);
        d->AddText(ImVec2(size.x*.5f+14,size.y*.5f-20-float(now-lastHit)*15),IM_COL32(90,255,100,255),text);
    }
    for(const auto& line:lines)if(now<line.until && (line.local?c.tracer:c.tracerOthers))
        WorldLine(d,line.a,line.b,line.local?IM_COL32(90,255,100,220):IM_COL32(255,130,70,220));
    if(c.eventLog){float y=42;for(const auto& e:events){d->AddText(ImVec2(12,y),IM_COL32(235,235,235,255),e.text.c_str());y+=17;}}
    if(c.playerInfo||c.ranks) {
        ImGui::SetNextWindowSize(ImVec2(350,280),ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Player information",nullptr,ImGuiWindowFlags_NoCollapse)) {
            for(const auto& p:info) {
                if(c.infoTeamCheck&&p.team==G::LocalTeam)continue;
                ImGui::Text("%s | HP %d | team %d",p.name.c_str(),p.health,p.team);
                if(c.ranks){if(p.rank>0&&p.rank<=18)ImGui::Text("Rank %d | wins %d",p.rank,p.wins);else ImGui::TextUnformatted("Rank not provided");}
            }
        }ImGui::End();
    }
    Grenades(pawn,list,now,d);
}
}
