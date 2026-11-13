#pragma once
#include "SDK.h"
#include <span>
namespace AimChecks {
struct Cloud { Vec3 center; float radius; };
inline bool Finite(Vec3 p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
inline bool SegmentHitsCloud(Vec3 a,Vec3 b,const Cloud& cloud) {
    if(!Finite(a)||!Finite(b)||!Finite(cloud.center)||!std::isfinite(cloud.radius)||cloud.radius<=0)return true;
    const auto d=b-a,v=cloud.center-a;
    const float length=d.x*d.x+d.y*d.y+d.z*d.z;
    if(!std::isfinite(length))return true;
    const float t=length>.0001f?std::clamp((v.x*d.x+v.y*d.y+v.z*d.z)/length,0.f,1.f):0.f;
    const Vec3 distance{a.x+t*d.x-cloud.center.x,a.y+t*d.y-cloud.center.y,a.z+t*d.z-cloud.center.z};
    return distance.x*distance.x+distance.y*distance.y+distance.z*distance.z<=cloud.radius*cloud.radius;
}
inline bool FlashClear(float overlay,float screenshot) {
    return std::isfinite(overlay)&&std::isfinite(screenshot)&&overlay>=0&&screenshot>=0&&overlay<=.001f&&screenshot<=.001f;
}
inline bool TraceClear(uintptr_t wanted,uintptr_t hit,float fraction,bool solid) {
    return wanted>=0x10000 && !solid && std::isfinite(fraction) && fraction>=0 && fraction<=1 &&
        (hit==wanted || (!hit && fraction>=.9999f));
}
bool Begin(uintptr_t local,uintptr_t list,Vec3 eye,bool flash=true,bool smoke=true,bool wall=true);
bool Clear(uintptr_t local,uintptr_t target,Vec3 eye,Vec3 point,bool wall=true);
// Independent world ray for visualization; never changes aim-check readiness.
bool Visible(uintptr_t skip,uintptr_t target,Vec3 start,Vec3 end);
bool RayEnd(uintptr_t skip,Vec3 start,Vec3 end,Vec3& hit);
const char* Status();
}
