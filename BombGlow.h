#pragma once
#include "SDK.h"
namespace WorldVisuals {
// Only these fields are owned; other native glow state is left untouched.
struct GlowPatch {
    uintptr_t entity=0;
    int originalType=0;
    uint32_t originalColor=0;
    uint8_t originalEligible=0,originalGlowing=0;
    bool Capture(uintptr_t address) {
        entity=address;const auto glow=entity+Off::GlowComponent;
        return Sdk::Read(glow+Off::GlowType,&originalType,4) && Sdk::Read(glow+Off::GlowColor,&originalColor,4) &&
            Sdk::Read(glow+Off::GlowEnabled,&originalEligible,1) && Sdk::Read(glow+Off::GlowThroughWalls,&originalGlowing,1);
    }
    bool Apply(uint32_t color) const {
        const auto glow=entity+Off::GlowComponent;const int type=3;const uint8_t yes=1;
        return Sdk::Write(glow+Off::GlowType,&type,4) && Sdk::Write(glow+Off::GlowColor,&color,4) &&
            Sdk::Write(glow+Off::GlowEnabled,&yes,1) && Sdk::Write(glow+Off::GlowThroughWalls,&yes,1);
    }
    void Restore() const {
        const auto glow=entity+Off::GlowComponent;
        Sdk::Write(glow+Off::GlowThroughWalls,&originalGlowing,1);Sdk::Write(glow+Off::GlowEnabled,&originalEligible,1);
        Sdk::Write(glow+Off::GlowColor,&originalColor,4);Sdk::Write(glow+Off::GlowType,&originalType,4);
    }
};
void UpdateBombGlow();
void ResetBombGlow();
}
