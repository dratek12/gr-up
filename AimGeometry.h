#pragma once
#include "SDK.h"
#include <cmath>

namespace AimGeometry {
inline bool Angles(const Vec3& eye, const Vec3& point, Vec3& angles) {
    const Vec3 delta = point - eye;
    const float horizontal = std::hypot(delta.x, delta.y);
    if (!std::isfinite(horizontal) || !std::isfinite(delta.z) ||
        (horizontal < .0001f && std::fabs(delta.z) < .0001f)) return false;
    constexpr float degrees = 57.295779513f;
    angles = Vec3(-std::atan2(delta.z, horizontal) * degrees,
        std::atan2(delta.y, delta.x) * degrees, 0);
    return true;
}
inline Vec3 Difference(const Vec3& target, const Vec3& current) {
    return Vec3(std::remainder(target.x - current.x, 360.f),
        std::remainder(target.y - current.y, 360.f), 0);
}
// Recover the world-space sight ray from the same projection used to draw the
// target. This includes the rendered camera position/bob during player movement.
inline bool ScreenDifference(const float* m,ImVec2 size,ImVec2 point,Vec3& result){
    if(!m||size.x<=0||size.y<=0||!std::isfinite(size.x)||!std::isfinite(size.y)||!std::isfinite(point.x)||!std::isfinite(point.y))return false;
    Vec3 right{m[0],m[1],m[2]},up{m[4],m[5],m[6]},forward{m[12],m[13],m[14]};
    const float sx=std::hypot(std::hypot(right.x,right.y),right.z),sy=std::hypot(std::hypot(up.x,up.y),up.z),sf=std::hypot(std::hypot(forward.x,forward.y),forward.z);
    if(!std::isfinite(sx)||!std::isfinite(sy)||!std::isfinite(sf)||sx<.001f||sy<.001f||sf<.001f)return false;
    const float x=(point.x*2/size.x-1)*sf/(sx*sx),y=(1-point.y*2/size.y)*sf/(sy*sy);
    Vec3 ray{forward.x+right.x*x+up.x*y,forward.y+right.y*x+up.y*y,forward.z+right.z*x+up.z*y};
    Vec3 target{},camera{};
    if(!Angles({},ray,target)||!Angles({},forward,camera))return false;
    result=Difference(target,camera);return true;
}
inline ImVec2 Mouse(const Vec3& difference, float sensitivity, float smoothing) {
    if (!std::isfinite(sensitivity) || sensitivity <= .001f || !std::isfinite(smoothing) ||
        !std::isfinite(difference.x) || !std::isfinite(difference.y)) return {};
    const float divisor = sensitivity * .022f * std::max(smoothing, 1.f);
    // Positive mouse X decreases yaw; positive mouse Y increases pitch.
    return ImVec2(-difference.y / divisor, difference.x / divisor);
}
// Mask bits: Head, Chest, Stomach, Legs (both knees).
inline bool Selected(int mask,int bone) {
    return bone>=1 && bone<=5 && (mask & (bone>=4?8:1<<(bone-1)))!=0;
}
inline bool Valid(int bone,const CPlayer& p) {
    switch(bone){case 1:return p.headValid;case 2:return p.chestValid;
        case 3:return p.stomachValid;case 4:return p.leftLegValid;case 5:return p.rightLegValid;default:return false;}
}
inline Vec3 Point(int bone, const CPlayer& player) {
    if (bone == 2) return player.chest;
    if (bone == 3) return player.stomach;
    if (bone == 1) return player.head;
    if (bone == 4) return player.leftLeg;
    if (bone == 5) return player.rightLeg;
    return {};
}
}
