#pragma once
#include "SDK.h"
#include <algorithm>
#include <cmath>
// Bound corrections and wait for camera feedback instead of replaying an entire
// angular error for every Present call while the game has not consumed input.
struct SnapMotion {
    Vec3 sentCamera{};uint64_t sentAt=0;bool waiting=false,stalled=false;
    void Reset(){*this={};}
    Vec3 Step(Vec3 error,Vec3 camera,float smooth,uint64_t now){
        if(!std::isfinite(error.x)||!std::isfinite(error.y)||!std::isfinite(camera.x)||!std::isfinite(camera.y)||!std::isfinite(smooth))return {};
        if(waiting){
            if(now<sentAt){Reset();return {};}
            if(now-sentAt<12)return {};
            const float changed=std::hypot(std::remainder(camera.x-sentCamera.x,360.f),std::remainder(camera.y-sentCamera.y,360.f));
            if(changed<.0001f){stalled=now-sentAt>=150;return {};}
            waiting=false;
        }
        smooth=std::clamp(smooth,1.f,30.f);
        const float length=std::hypot(error.x,error.y);
        if(length<=0)return {};
        const float fraction=1.f/(2.f+smooth*.25f);
        const float amount=std::min(length*fraction,3.f/(1.f+smooth));
        return {error.x*amount/length,error.y*amount/length,0};
    }
    void Sent(Vec3 camera,uint64_t now){sentCamera=camera;sentAt=now;waiting=true;}
};
