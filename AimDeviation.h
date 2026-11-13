#pragma once
#include "SDK.h"
namespace AimDeviation {
inline Vec3 Offset(uint32_t handle,int weapon,int bone,float amount){
    if(!std::isfinite(amount)||amount<=0)return {};
    amount=std::min(amount,4.f);
    uint32_t seed=handle^(uint32_t(weapon)*0x9e3779b9u)^(uint32_t(bone)*0x85ebca6bu);
    auto random=[&](){seed+=0x9e3779b9u;uint32_t x=seed;x=(x^(x>>16))*0x21f0aaadu;x=(x^(x>>15))*0x735a2d97u;return float((x^(x>>15))&0xffffff)/float(0xffffff);};
    Vec3 direction{random()*2-1,random()*2-1,random()*2-1};
    const float length=std::hypot(std::hypot(direction.x,direction.y),direction.z);
    if(length<.0001f)return {};
    const float scale=amount*(.35f+.65f*random())/length;
    return {direction.x*scale,direction.y*scale,direction.z*scale};
}
inline bool Activated(bool allowInput,bool enabled,bool always,bool bound,bool held){
    return allowInput&&(always||(enabled&&bound&&held));
}
}
