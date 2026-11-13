#pragma once
#include <atomic>
#include <cstdint>

// The window thread samples SDL input ownership; rendering only reads this
// snapshot. Never synchronously wait for the window thread from Present.
class GameplayState {
    std::atomic<uint64_t> sample{0};
public:
    void Publish(bool allowed,uint64_t now){sample.store((now<<1)|uint64_t(allowed),std::memory_order_release);}
    void Block(){sample.store(0,std::memory_order_release);}
    bool Allows(uint64_t now) const {
        const auto value=sample.load(std::memory_order_acquire),when=value>>1;
        return (value&1) && now>=when && now-when<=100;
    }
};
