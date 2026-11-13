#pragma once
#include "SDK.h"
#include "WorldVisuals.h"
namespace ExtraRuntime {
void Frame();
void Reset();
struct DamageCounter {
    bool ready=false;float previous=0;
    float Update(float value) {
        if(!std::isfinite(value)||value<0||value>1000000){ready=false;return 0;}
        const float delta=ready && value>=previous?value-previous:0;
        previous=value;ready=true;return delta;
    }
};
inline Vec3 Ballistic(Vec3 start,Vec3 velocity,float t,float gravity=320.f) {
    return {start.x+velocity.x*t,start.y+velocity.y*t,start.z+velocity.z*t-.5f*gravity*t*t};
}
inline bool ThrowDirection(Vec3 angles,Vec3& direction) {
    if(!WorldVisuals::Direction(angles,100,direction))return false;
    direction={direction.x*.01f,direction.y*.01f,direction.z*.01f};return true;
}
struct FlashPeriod {
    float previous=0;bool initialized=false,wasSuppressed=false;double until=0;
    void Observe(float end,float duration,bool suppress,double now) {
        if(!std::isfinite(end)||!std::isfinite(duration)||end<0||duration<0||duration>30)return;
        if(end>0 && suppress && ((!wasSuppressed) || (initialized&&end!=previous)))until=now+duration;
        previous=end;initialized=true;wasSuppressed=suppress;
    }
};
}

