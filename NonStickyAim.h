#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
namespace NonSticky {
// Reaction -> bounded assistance -> release; a new target never inherits time.
struct State {
    uint32_t target=0xFFFFFFFFu;
    int weapon=0;
    uint64_t acquired=0, releaseUntil=0;
    void Reset() { *this={}; }
    bool Step(uint32_t candidate,int item,bool active,uint64_t now,float reaction,float lock,float release,bool timed=true) {
        if(!std::isfinite(reaction) || !std::isfinite(lock) || !std::isfinite(release)) {Reset();return false;}
        if(weapon!=item) {Reset();weapon=item;}
        if(!active || candidate==0xFFFFFFFFu) {target=0xFFFFFFFFu;acquired=0;return false;}
        if(now<releaseUntil)return false;
        if(candidate!=target || weapon!=item) {target=candidate;weapon=item;acquired=now;}
        if(now<acquired){Reset();return false;}
        const auto wait=uint64_t(std::clamp(reaction,0.f,500.f));
        const auto duration=uint64_t(std::clamp(lock,0.f,1000.f));
        const auto elapsed=now-acquired;
        if(elapsed<wait)return false;
        if(!timed)return true;
        if(!duration || elapsed-wait>=duration) {
            target=0xFFFFFFFFu;releaseUntil=now+uint64_t(std::clamp(release,0.f,1000.f));return false;
        }
        return true;
    }
};
inline bool ManualRecent(uint64_t sample,uint64_t now){return sample && now>=sample && now-sample<=80;}
inline float Fraction(float smoothing,float dt) {
    if(!std::isfinite(smoothing)||!std::isfinite(dt)||dt<=0)return 0;
    // Exponential response with a time constant, independent of render frequency.
    return 1.f-std::exp(-std::min(dt,.05f)/(std::max(smoothing,1.f)*.025f));
}
}
