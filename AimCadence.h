#pragma once
#include "SDK.h"
struct AimCadence {
    uint64_t sentAt=0,stepAt=0;Vec3 sentCamera{};bool waiting=false,started=false;
    void Reset(){*this={};}
    float Next(Vec3 camera,uint64_t now){
        if(!started){started=true;stepAt=now;return .008f;}
        if(now<stepAt){Reset();return 0;}
        const auto elapsed=now-stepAt;
        if(elapsed<8)return 0;
        if(waiting){
            const float change=std::hypot(std::remainder(camera.x-sentCamera.x,360.f),std::remainder(camera.y-sentCamera.y,360.f));
            if(!std::isfinite(change))return 0;
            // A dropped/sub-pixel input must not freeze a held activation.
            if(change<.0001f&&now-sentAt<32)return 0;
            waiting=false;
        }
        stepAt=now; // Every integration step consumes time, even if it emits zero counts.
        return std::min(float(elapsed)/1000.f,.032f);
    }
    void Sent(Vec3 camera,uint64_t now){sentCamera=camera;sentAt=now;waiting=true;}
};
inline float AimStableDistance(float distance,bool current){return current?std::max(0.f,distance-2.f):distance;}
