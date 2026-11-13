#include "Backtrack.h"
#include "Global.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr size_t MaxTracks = 128;
constexpr size_t MaxSamples = 64;
constexpr uint64_t SamplePeriodMs = 15;
constexpr uint64_t MaxAgeMs = 1000;
constexpr uint64_t TrackLifeMs = 3000;

struct Sample {
    Vec3 origin{}, head{}, chest{}, stomach{}, leftLeg{}, rightLeg{}, eye{};
    bool headValid=false, chestValid=false, stomachValid=false;
    bool leftLegValid=false, rightLegValid=false;
    uint64_t time=0;
};
struct Track {
    uintptr_t entity=0;
    uint32_t handle=0xFFFFFFFFu;
    uint64_t lastSeen=0;
    std::array<Sample, MaxSamples> samples{};
    size_t next=0, count=0;
};
std::array<Track, MaxTracks> tracks;

Track* Find(const CPlayer& player) {
    for(auto& track:tracks)
        if(track.entity==player.entity && track.handle==player.handle)
            return &track;
    return nullptr;
}
Track* Allocate(const CPlayer& player,uint64_t now) {
    if(auto* found=Find(player))return found;
    for(auto& track:tracks) {
        if(!track.entity || now<track.lastSeen || now-track.lastSeen>TrackLifeMs ||
           (track.entity==player.entity && track.handle!=player.handle)) {
            track=Track{};
            track.entity=player.entity;
            track.handle=player.handle;
            return &track;
        }
    }
    return nullptr;
}
bool Finite(Vec3 point) {
    return std::isfinite(point.x)&&std::isfinite(point.y)&&std::isfinite(point.z);
}
}
void BtReset() { tracks={}; }

void BtRecordAt(std::span<const CPlayer> players,uint64_t now) {
    for(const auto& player:players) {
        if(!player.valid || player.health<=0 || player.entity<0x10000 ||
           !Finite(player.origin) || !Finite(player.head))continue;
        auto* track=Allocate(player,now);
        if(!track)continue;
        track->lastSeen=now;
        if(track->count) {
            const auto& latest=track->samples[(track->next+MaxSamples-1)%MaxSamples];
            if(now>=latest.time && now-latest.time<SamplePeriodMs)continue;
            if(now<latest.time){*track=Track{};track->entity=player.entity;track->handle=player.handle;track->lastSeen=now;}
        }
        auto& sample=track->samples[track->next];
        sample={player.origin,player.head,player.chest,player.stomach,player.leftLeg,
                player.rightLeg,player.eye,player.headValid,player.chestValid,
                player.stomachValid,player.leftLegValid,player.rightLegValid,now};
        track->next=(track->next+1)%MaxSamples;
        track->count=std::min(track->count+1,MaxSamples);
    }
}
void BtRecord() { BtRecordAt(G::Players,GetTickCount64()); }

bool BtApplyAt(CPlayer& player,float latencyMs,uint64_t now) {
    if(!std::isfinite(latencyMs) || latencyMs<1 || latencyMs>MaxAgeMs ||
       player.entity<0x10000 || !player.valid || player.health<=0)return false;
    auto* track=Find(player);
    if(!track)return false;
    for(size_t offset=1;offset<=track->count;++offset) {
        const auto& sample=track->samples[(track->next+MaxSamples-offset)%MaxSamples];
        if(now<sample.time)continue;
        const auto age=now-sample.time;
        if(age>MaxAgeMs)break;
        if(age<uint64_t(latencyMs))continue;
        player.origin=sample.origin;
        player.head=sample.head;
        player.chest=sample.chest;
        player.stomach=sample.stomach;
        player.leftLeg=sample.leftLeg;
        player.rightLeg=sample.rightLeg;
        player.eye=sample.eye;
        player.headValid=sample.headValid;
        player.chestValid=sample.chestValid;
        player.stomachValid=sample.stomachValid;
        player.leftLegValid=sample.leftLegValid;
        player.rightLegValid=sample.rightLegValid;
        return true;
    }
    return false;
}
bool BtApply(CPlayer& player,float latencyMs) {
    return BtApplyAt(player,latencyMs,GetTickCount64());
}
