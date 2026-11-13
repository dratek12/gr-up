#include "LegitBot.h"
#include "Global.h"
#include "TriggerEstimate.h"
#include "AimChecks.h"
static TriggerLogic::Delay triggerDelay;
static bool pressed=false;
static uint64_t releaseAt=0, nextClick=0;
static void Release() {
    if (!pressed) return;
    INPUT in{};in.type=INPUT_MOUSE;in.mi.dwFlags=MOUSEEVENTF_LEFTUP;
    SendInput(1,&in,sizeof(in));pressed=false;
}
void ResetTrigger() { Release();triggerDelay.Reset();nextClick=0; }
void RunTrigger() {
    static int previousMode=-1;
    if(previousMode!=G::Ui.state.legit){ResetTrigger();previousMode=G::Ui.state.legit;}
    const auto now=GetTickCount64();
    if (!G::LocalAlive || G::Players.empty() || !Sdk::Foreground()) { ResetTrigger(); return; }
    if (pressed) { if(now>=releaseAt) Release(); else return; }
    uintptr_t client=Sdk::Client(),list=Sdk::EntityList();
    uintptr_t pawn=Sdk::R<uintptr_t>(client+Off::LocalPlayer);
    int weapon=Sdk::ActiveWeaponId(pawn,list);
    if (!getrekt::Weapons::Find(weapon)) { ResetTrigger(); return; }
    const auto& cfg=G::Ui.state.ActiveAimConfig(weapon,getrekt::Weapons::Category(weapon));
    if(cfg.aimStyle!=0){ResetTrigger();return;}
    bool held=cfg.triggerBind>0 && cfg.triggerBind<256 && (GetAsyncKeyState(cfg.triggerBind)&0x8000);
    if (!TriggerLogic::Enabled(cfg,held)) { ResetTrigger(); return; }
    // Require the game's crosshair entity as well as the selected screen region.
    int index=-1;
    if (!Sdk::Read(pawn+Off::CrosshairEntity,&index,sizeof(index)) || index<=0 || index>=0x7FFF) { triggerDelay.Reset(); return; }
    const auto entity=Sdk::EntAt(list,index);
    const CPlayer* target=nullptr;
    for(const auto& player:G::Players) if(player.entity==entity && player.valid && player.health>0 &&
        (cfg.trigFf || !G::LocalTeam || player.team!=G::LocalTeam)) { target=&player;break; }
    float matrix[16]{};std::array<TriggerEstimate::Region,7> regions{};
    const auto size=ImGui::GetIO().DisplaySize;const ImVec2 center(size.x*.5f,size.y*.5f);
    if (!target || !Sdk::Read(client+Off::ViewMatrix,matrix,sizeof(matrix)) ||
        !TriggerEstimate::Project(*target,matrix,size,regions)) { triggerDelay.Reset(); return; }
    const auto toTarget=target->head-G::LocalEye;
    const float rayLength=std::sqrt(toTarget.x*toTarget.x+toTarget.y*toTarget.y+toTarget.z*toTarget.z)+32.f;
    const float forwardLength=std::sqrt(matrix[12]*matrix[12]+matrix[13]*matrix[13]+matrix[14]*matrix[14]);
    if(!std::isfinite(forwardLength)||forwardLength<.5f) {ResetTrigger();return;}
    const Vec3 rayPoint=G::LocalEye+Vec3(matrix[12]*rayLength/forwardLength,matrix[13]*rayLength/forwardLength,matrix[14]*rayLength/forwardLength);
    if (!AimChecks::Begin(pawn,list,G::LocalEye) || !AimChecks::Clear(pawn,target->entity,G::LocalEye,rayPoint)) { ResetTrigger(); return; }
    const int group=TriggerEstimate::Hit(cfg,regions,center);
    if(group<0) { triggerDelay.Reset();return; }
    const auto services=Sdk::R<uintptr_t>(pawn+Off::WeaponServices);
    const auto weaponEntity=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(services+Off::ActiveWeapon));
    const auto data=Sdk::R<uintptr_t>(weaponEntity+Off::WeaponVData);
    int clip=0;
    if(!weaponEntity || !data || !Sdk::Read(weaponEntity+Off::Clip1,&clip,sizeof(clip)) || clip<=0) { triggerDelay.Reset();return; }
    WeaponAccuracy accuracy{};
    float projection=std::sqrt(matrix[4]*matrix[4]+matrix[5]*matrix[5]+matrix[6]*matrix[6])*size.y*.5f;
    if(cfg.hitChanceOn && (!ReadWeaponAccuracy(pawn,list,accuracy) || !std::isfinite(cfg.hitChance) ||
        TriggerEstimate::Probability(cfg,regions,center,accuracy.inaccuracy,accuracy.spread,projection)<std::clamp(cfg.hitChance,0.f,100.f))) { triggerDelay.Reset();return; }
    const auto delta=target->head-G::LocalEye;
    const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y+delta.z*delta.z);
    const auto itemServices=Sdk::R<uintptr_t>(target->entity+Off::ItemServices);
    const bool helmet=Sdk::R<bool>(itemServices+Off::HasHelmet);
    const auto damage=TriggerEstimate::Damage(float(Sdk::R<int>(data+Off::WeaponDamage)),
        Sdk::R<float>(data+Off::HeadshotMultiplier),Sdk::R<float>(data+Off::RangeModifier),
        Sdk::R<float>(data+Off::ArmorRatio),distance,group,target->armour,helmet);
    if(!std::isfinite(cfg.minDamage) || damage<std::clamp(cfg.minDamage,0.f,100.f)) { triggerDelay.Reset();return; }
    if(!triggerDelay.Ready(entity,weapon,cfg,now) || now<nextClick || (GetAsyncKeyState(VK_LBUTTON)&0x8000)) return;
    INPUT in{};in.type=INPUT_MOUSE;in.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
    if(SendInput(1,&in,sizeof(in))==1) { pressed=true;releaseAt=now+25;nextClick=now+100;triggerDelay.Reset(); }
}
