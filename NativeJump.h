#pragma once
#include <cstdint>
namespace NativeJump {
inline constexpr uint32_t Press=65537,Release=256;
struct Command {bool write=false;uint32_t value=Release;};
struct Controller {
    bool owned=false,initialized=false,groundBefore=false;
    uint32_t identity=0;uint64_t edgeUntil=0;
    Command Step(bool allowed,bool bhopHeld,bool edgeHeld,bool ground,uint32_t pawnIdentity,uint64_t now) {
        if(!allowed) {const bool release=owned;*this={};return {release,Release};}
        if(initialized && identity!=pawnIdentity) {
            const bool release=owned;*this={};
            if(release)return {true,Release};
        }
        if(initialized && groundBefore && !ground && edgeHeld)edgeUntil=now+20;
        initialized=true;identity=pawnIdentity;groundBefore=ground;
        if(!edgeHeld)edgeUntil=0;
        const bool edgeActive=now<edgeUntil;
        if(bhopHeld||edgeActive){owned=true;return {true,(bhopHeld&&ground)||edgeActive?Press:Release};}
        const bool release=owned;owned=false;return {release,Release};
    }
};
bool Start();
bool Stop();
void Publish(bool bhop,bool edge,int edgeKey);
void Suspend();
bool Ready();
}
