#pragma once
#include "SDK.h"
namespace ClickAssist {
struct Edge {
    int key=0;uint64_t serial=0;bool ready=false;
    void Reset(){*this={};}
    bool Consume(bool allowed,int binding,uint64_t count){
        if(!ready||key!=binding){key=binding;serial=count;ready=true;return false;}
        const bool changed=count>serial;serial=count;
        return allowed&&binding>0&&binding<256&&changed;
    }
};
struct KeyEdge {
    int key=0;bool previous=false,ready=false;
    void Reset(){*this={};}
    bool Consume(bool enabled,int binding,bool down){
        if(!enabled||binding<=0||binding>=256){Reset();return false;}
        if(!ready||key!=binding){key=binding;previous=down;ready=true;return false;}
        const bool edge=down&&!previous;previous=down;return edge;
    }
};
struct ShotSequence {
    uintptr_t owner=0;int weapon=0;uint64_t due=0,release=0;
    bool pending=false,pressed=false;
    bool Arm(uintptr_t pawn,int item,uint64_t now){
        if(!pawn||!item||pending||pressed)return false;
        owner=pawn;weapon=item;due=now+20;pending=true;return true;
    }
    // 1: press, 2: release, -1: cancel after owner/weapon change.
    int Poll(uintptr_t pawn,int item,uint64_t now){
        if(!pending&&!pressed)return 0;
        if(pawn!=owner||item!=weapon)return -1;
        if(pending&&now>=due){pending=false;pressed=true;release=now+25;return 1;}
        if(pressed&&now>=release){pressed=false;return 2;}
        return 0;
    }
    bool Cancel(){const bool wasPressed=pressed;*this={};return wasPressed;}
};
inline Vec3 Step(Vec3 error,float fov,float smooth){
    const float length=std::hypot(error.x,error.y);
    if(!std::isfinite(length)||!std::isfinite(fov)||!std::isfinite(smooth)||fov<=0||length<=0)return {};
    smooth=std::clamp(smooth,1.f,30.f);
    const float fraction=1.f/std::pow(smooth,.65f);
    const float amount=std::min(length*fraction,fov/(1.f+.2f*(smooth-1.f)));
    return {error.x*amount/length,error.y*amount/length,0};
}
}
