#pragma once
#include "SDK.h"
#include <vector>
namespace WorldVisuals {
std::vector<uint32_t> Bombs(uintptr_t list);
inline bool BombEligible(bool planted,uint32_t owner,bool ticking,bool defused,bool exploded) {
    return planted ? ticking && !defused && !exploded : owner==0xFFFFFFFFu;
}
inline bool Direction(Vec3 angles,float length,Vec3& delta) {
    if(!std::isfinite(angles.x)||!std::isfinite(angles.y)||std::fabs(angles.x)>90 ||
       std::fabs(angles.y)>360||!std::isfinite(length)||length<100||length>8192)return false;
    const float pitch=angles.x*.01745329252f,yaw=angles.y*.01745329252f;
    delta={std::cos(pitch)*std::cos(yaw)*length,std::cos(pitch)*std::sin(yaw)*length,-std::sin(pitch)*length};return true;
}
}
