#pragma once
#include <unordered_map>
#include <cmath>
#include <cstdint>
#include <algorithm>

class SoundReveal {
    struct Entry {
        float server = 0, client = 0, shot = 0;
        double heard = -1, seen = 0;
        uint32_t weapon = 0xFFFFFFFFu;
    };
    std::unordered_map<uint32_t, Entry> entries;
public:
    void Clear() { entries.clear(); }
    double HeardAt(uint32_t handle) const {auto it=entries.find(handle);return it==entries.end()?-1:it->second.heard;}
    bool Update(uint32_t handle, float server, float client, float shot, double now, float duration, uint32_t weapon = 0xFFFFFFFFu) {
        if (handle == 0xFFFFFFFFu || !std::isfinite(now) || !std::isfinite(duration)) return false;
        auto [it, fresh] = entries.try_emplace(handle);
        auto& e = it->second;
        bool heard = false;
        auto sample = [&](float value, float& previous) {
            if (!std::isfinite(value) || value < 0) return;
            if (!fresh && value > previous) heard = true;
            previous = value;
        };
        sample(server,e.server); sample(client,e.client);
        if (e.weapon != weapon) { e.shot = std::isfinite(shot) && shot >= 0 ? shot : 0; e.weapon = weapon; }
        else sample(shot,e.shot);
        if (heard) e.heard = now;
        e.seen = now;
        return e.heard >= 0 && now >= e.heard && now - e.heard < std::clamp(duration,.5f,15.f);
    }
    void Prune(double now) {
        std::erase_if(entries, [now](const auto& pair) { return now < pair.second.seen || now - pair.second.seen > 1.; });
    }
};
