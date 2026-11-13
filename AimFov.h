#pragma once
#include "SDK.h"
namespace AimFov {
enum Mode { Crosshair=0, OnTarget=1, FollowLocked=2 };
// Target circles represent a fixed world-space reach calibrated at 500 game units, reduced by 30%.
inline float TargetRadius(int mode,float baseRadius,float worldDistance,bool scaleByDistance=true){
    if(!std::isfinite(baseRadius)||baseRadius<=0)return 0;
    if(mode!=OnTarget||!scaleByDistance)return baseRadius;
    if(!std::isfinite(worldDistance)||worldDistance<=0)return 0;
    return baseRadius*(350.f/std::max(worldDistance,64.f));
}
struct Circle {
    ImVec2 center{};float radius=0;
    bool Set(const float* matrix,ImVec2 size,float degrees) {
        *this={};
        if(!matrix||!std::isfinite(size.x)||!std::isfinite(size.y)||size.x<=0||size.y<=0||
           !std::isfinite(degrees)||degrees<=0||degrees>30)return false;
        const float scale=std::hypot(std::hypot(matrix[0],matrix[1]),matrix[2]);
        radius=size.x*.5f*scale*std::tan(degrees*.01745329252f);
        center={size.x*.5f,size.y*.5f};
        return std::isfinite(radius)&&radius>0;
    }
    float Distance(ImVec2 point)const{return std::hypot(point.x-center.x,point.y-center.y);}
    bool Contains(ImVec2 point,float actualRadius)const{
        const float distance=Distance(point);return std::isfinite(actualRadius)&&actualRadius>0&&std::isfinite(distance)&&distance<=actualRadius;
    }
    bool Contains(ImVec2 point)const{return Contains(point,radius);}
};
// Retention is bounded by the same visible circle; follow never ignores the FOV boundary.
inline bool Prefer(int mode,uint32_t locked,uint32_t candidate,float distance,uint32_t best,float bestDistance) {
    if(mode==FollowLocked&&locked!=0xFFFFFFFFu){
        if(candidate==locked&&best!=locked)return true;
        if(best==locked&&candidate!=locked)return false;
    }
    return distance<bestDistance;
}
}
