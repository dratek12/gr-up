#pragma once
#include "AimGeometry.h"
namespace RcsLogic {
inline bool Camera(const float* matrix, Vec3& angle) {
    const Vec3 direction(matrix[12], matrix[13], matrix[14]);
    const float length = std::hypot(std::hypot(direction.x, direction.y), direction.z);
    return std::isfinite(length) && length > .5f && length < 2.f && AimGeometry::Angles({}, direction, angle);
}
inline float Strength(float low, float high, float sample) {
    if (!std::isfinite(low) || !std::isfinite(high)) return 0;
    low = std::clamp(low, 0.f, 2.f); high = std::clamp(high, 0.f, 2.f);
    if (low > high) std::swap(low, high);
    return low + (high-low)*std::clamp(sample, 0.f, 1.f);
}
struct State {
    uintptr_t owner=0; int weapon=0; bool active=false;
    Vec3 previous{}; float x=1, y=1;
    void Reset() { *this = {}; }
    Vec3 Step(uintptr_t pawn, int id, bool enabled, bool firing, const Vec3& recoil,
              const getrekt::VisualReplica::AimCfg& cfg, float randomX, float randomY) {
        if (owner!=pawn || weapon!=id) { Reset(); owner=pawn; weapon=id; previous=recoil; return {}; }
        if (!std::isfinite(recoil.x) || !std::isfinite(recoil.y) ||
            std::fabs(recoil.x)>30 || std::fabs(recoil.y)>30) { Reset(); return {}; }
        if (!enabled || !firing) { previous=recoil; active=false; return {}; }
        if (!active) { x=Strength(cfg.rcsXmin,cfg.rcsXmax,randomX); y=Strength(cfg.rcsYmin,cfg.rcsYmax,randomY); active=true; }
        auto delta=AimGeometry::Difference(recoil,previous); previous=recoil;
        return Vec3(-delta.x*x,-delta.y*y,0);
    }
};
}
