#pragma once
#include "TriggerLogic.h"
#include "WeaponAccuracy.h"
namespace TriggerEstimate {
struct Region { ImVec2 center{}; float radius=0; int group=0; };
inline bool Project(const CPlayer& p, const float* m, ImVec2 size, std::array<Region,7>& regions) {
    if (!p.headValid) return false;
    ImVec2 head{},feet{};
    if (!Sdk::W2S(m,p.head,head,size) || !Sdk::W2S(m,p.origin,feet,size)) return false;
    const float height=feet.y-head.y;
    if (!std::isfinite(height) || height<=1) return false;
    // Conservative screen-space regions, scaled with the current head/stance.
    regions={Region{head,height*.065f,0},
        Region{{head.x,head.y+height*.24f},height*.12f,1},
        Region{{head.x,head.y+height*.43f},height*.115f,2},
        Region{{head.x-height*.18f,head.y+height*.30f},height*.07f,3},
        Region{{head.x+height*.18f,head.y+height*.30f},height*.07f,3},
        Region{{head.x-height*.07f,head.y+height*.75f},height*.09f,4},
        Region{{head.x+height*.07f,head.y+height*.75f},height*.09f,4}};
    return true;
}
inline int Hit(const TriggerLogic::Config& cfg, const std::array<Region,7>& regions, ImVec2 point) {
    const bool selected[]{cfg.head,cfg.chest,cfg.stomach,cfg.arms,cfg.legs};
    for (const auto& r: regions) if (selected[r.group] && r.radius>0 &&
        std::hypot(point.x-r.center.x,point.y-r.center.y)<=r.radius) return r.group;
    return -1;
}
inline float Probability(const TriggerLogic::Config& cfg,const std::array<Region,7>& regions,
                         ImVec2 center,float inaccuracy,float spread,float projection) {
    if (!std::isfinite(inaccuracy) || !std::isfinite(spread) || !std::isfinite(projection) ||
        inaccuracy<0 || spread<0 || projection<=0) return 0;
    unsigned hits=0; constexpr unsigned samples=256;
    // Deterministic integration of two independent radial spread components.
    for(unsigned i=0;i<samples;++i) {
        float a=i*2.39996323f,b=i*3.88322208f;
        float r1=(float((i*73)%samples)+.5f)/samples*inaccuracy*projection;
        float r2=(float((i*151+47)%samples)+.5f)/samples*spread*projection;
        ImVec2 point(center.x+std::cos(a)*r1+std::cos(b)*r2,center.y+std::sin(a)*r1+std::sin(b)*r2);
        hits+=Hit(cfg,regions,point)>=0;
    }
    return 100.f*hits/samples;
}
inline float Damage(float base,float headMultiplier,float rangeModifier,float armorRatio,
                    float distance,int group,int armor,bool helmet) {
    if (!std::isfinite(base) || !std::isfinite(rangeModifier) || !std::isfinite(distance) ||
        !std::isfinite(headMultiplier) || !std::isfinite(armorRatio) || base<=0 || base>1000 ||
        rangeModifier<=0 || rangeModifier>1 || distance<0 || headMultiplier<=0 || headMultiplier>10 ||
        armorRatio<0 || armorRatio>5 || group<0 || group>4) return 0;
    float damage=base*std::pow(rangeModifier,distance/500.f);
    damage*=group==0 ? headMultiplier : group==2 ? 1.25f : group==4 ? .75f : 1.f;
    if(armor>0 && group!=4 && (group!=0 || helmet)) {
        float reduced=damage*std::clamp(armorRatio*.5f,0.f,1.f);
        damage=(damage-reduced)*.5f>armor ? damage-armor*2.f : reduced;
    }
    return std::max(0.f,damage);
}
}
