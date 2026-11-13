#include "Misc.h"
#include "Global.h"
#include "ManualMouse.h"
#include "NativeJump.h"
#include "StrafeLogic.h"
namespace {
int strafe=0;uintptr_t previousPawn=0;
bool Key(int key,bool down) {
    INPUT input{};input.type=INPUT_KEYBOARD;input.ki.wVk=WORD(key);
    input.ki.dwFlags=down?0:KEYEVENTF_KEYUP;
    return SendInput(1,&input,sizeof(input))==1;
}
void ReleaseStrafe() {
    if(strafe && !ManualMouse::PhysicalKey(strafe))Key(strafe,false);
    strafe=0;
}
}
void ResetMisc() {
    NativeJump::Suspend();ManualMouse::ConsumeRawX();ReleaseStrafe();previousPawn=0;
}
void RunMisc() {
    if(!G::LocalAlive || !Sdk::Foreground() || !ManualMouse::KeyboardReady()){ResetMisc();return;}
    const auto pawn=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalPlayer);
    if(previousPawn!=pawn){ResetMisc();previousPawn=pawn;}
    const auto& c=G::Ui.state.extra;
    NativeJump::Publish(G::Ui.state.cs2.bhop,c.edgeJump,c.edgeKey);
    uint32_t flags=0;if(!Sdk::Read(pawn+Off::Flags,&flags,4)){ResetMisc();return;}
    const bool ground=flags&1,held=ManualMouse::PhysicalKey(c.strafeKey);
    Vec3 velocity{};const bool valid=Sdk::Read(pawn+Off::PawnVelocity,&velocity,sizeof(velocity));
    const bool moving=ManualMouse::PhysicalKey('W')||ManualMouse::PhysicalKey('A')||ManualMouse::PhysicalKey('S')||ManualMouse::PhysicalKey('D');
    const int direction=StrafeLogic::Direction(c.autoStrafe,valid,ground,held,c.strafeWasd,moving,
        std::hypot(velocity.x,velocity.y),c.strafeLimit,ManualMouse::ConsumeRawX());
    if(!direction){ReleaseStrafe();return;}
    const int wanted=direction<0?'A':'D';
    if(strafe!=wanted){ReleaseStrafe();if(Key(wanted,true))strafe=wanted;}
}
