#pragma once
#include "SDK.h"
#include <span>
#include <cstdint>
namespace TriggerLogic {
using Config = getrekt::VisualReplica::AimCfg;
inline bool Enabled(const Config& cfg, bool bindHeld) {
    return cfg.trig && (!cfg.trigKey || (cfg.triggerBind > 0 && cfg.triggerBind < 256 && bindHeld));
}
inline const CPlayer* Target(const Config& cfg, std::span<const CPlayer> players, int localTeam,
    Vec3 eye, const float matrix[16], ImVec2 size) {
    const CPlayer* best=nullptr;
    float nearest=25.f*25.f;
    for (const auto& player : players) {
        if (!player.valid || player.health<=0 || !player.entity) continue;
        if (!cfg.trigFf && localTeam && player.team==localTeam) continue;
        float dx=player.origin.x-eye.x, dy=player.origin.y-eye.y;
        float len=std::hypot(dx,dy);
        float rx=len>0.001f ? -dy/len : 1.f, ry=len>0.001f ? dx/len : 0.f;
        // Representative points: this SDK has origins, not skeletal hitboxes.
        Vec3 points[7]{ player.head,
            {player.origin.x,player.origin.y,player.origin.z+55.f},
            {player.origin.x,player.origin.y,player.origin.z+42.f},
            {player.origin.x+rx*12.f,player.origin.y+ry*12.f,player.origin.z+52.f},
            {player.origin.x-rx*12.f,player.origin.y-ry*12.f,player.origin.z+52.f},
            {player.origin.x+rx*5.f,player.origin.y+ry*5.f,player.origin.z+18.f},
            {player.origin.x-rx*5.f,player.origin.y-ry*5.f,player.origin.z+18.f} };
        bool selected[7]{cfg.head,cfg.chest,cfg.stomach,cfg.arms,cfg.arms,cfg.legs,cfg.legs};
        for (int i=0;i<7;++i) {
            if (!selected[i]) continue;
            ImVec2 screen;
            if (!Sdk::W2S(matrix,points[i],screen,size)) continue;
            float x=screen.x-size.x*.5f, y=screen.y-size.y*.5f;
            float distance=x*x+y*y;
            if (distance<=nearest) { nearest=distance; best=&player; }
        }
    }
    return best;
}
struct Delay {
    uintptr_t target=0;
    int weapon=0;
    uint64_t since=0;
    float delay=-1.f;
    void Reset() { target=0; weapon=0; since=0; delay=-1.f; }
    bool Ready(uintptr_t candidate, int weaponId, const Config& cfg, uint64_t now) {
        if (!candidate) { Reset(); return false; }
        float milliseconds=cfg.delayOn && std::isfinite(cfg.trigDelay) ? std::clamp(cfg.trigDelay,0.f,1000.f) : 0.f;
        if (target!=candidate || weapon!=weaponId || delay!=milliseconds) {
            target=candidate; weapon=weaponId; delay=milliseconds; since=now;
        }
        return now>=since && double(now-since)>=milliseconds;
    }
};
inline bool Chance(const Config& cfg, float roll) {
    return !cfg.hitChanceOn || (std::isfinite(cfg.hitChance) && roll < std::clamp(cfg.hitChance,0.f,100.f));
}
}
