#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include "GameLayout.h"

namespace Chams {
template<class Hidden,class Base,class Glow> void BatchPasses(int count,Hidden&& hidden,Base&& base,Glow&& glow) {
    for(int i=0;i<count;++i)hidden(i);
    for(int i=0;i<count;++i)base(i);
    for(int i=0;i<count;++i)glow(i);
}

// Verified in the running renderer: the +0x28 field is a key, NOT a second material pointer.
struct Mesh {
    std::byte prefix[Off::MeshScene];
    uintptr_t scene;
    void* material;
    std::byte middle[Off::MeshColor - Off::MeshMaterial - sizeof(void*)];
    uint32_t color;
    std::byte suffix[Off::MeshStride - Off::MeshColor - sizeof(uint32_t)];
};
static_assert(offsetof(Mesh, color) == Off::MeshColor && sizeof(Mesh) == Off::MeshStride);
inline uint32_t Color(float r, float g, float b, float a) {
    auto channel=[](float x) { return uint32_t(std::isfinite(x) ? std::clamp(x,0.f,1.f)*255.f+.5f : 0.f); };
    return channel(r) | channel(g)<<8 | channel(b)<<16 | channel(a)<<24;
}
template<class Draw> void Passes(Mesh& mesh, bool visibleOnly, void* visibleMaterial, void* hiddenMaterial,
    uint32_t visible, uint32_t hidden, Draw&& draw) {
    struct Restore {
        Mesh& mesh; void* material; uint32_t color;
        ~Restore() { mesh.material=material; mesh.color=color; }
    } restore{mesh,mesh.material,mesh.color};
    if (!visibleOnly && hiddenMaterial) {
        mesh.material=hiddenMaterial;mesh.color=hidden;draw();
    }
    mesh.material=visibleMaterial ? visibleMaterial : restore.material;
    mesh.color=visible;draw();
}

template<class Base, class Draw> void GlowPasses(Mesh& mesh, bool visibleOnly, void* visibleRim, void* hiddenRim,
    uint32_t visible, uint32_t hidden, Base&& base, Draw&& draw) {
    struct Restore {
        Mesh& mesh; void* material; uint32_t color;
        ~Restore() { mesh.material=material;mesh.color=color; }
    } restore{mesh,mesh.material,mesh.color};
    if(!visibleOnly && hiddenRim) { mesh.material=hiddenRim;mesh.color=hidden;draw(); }
    // Opaque visible surfaces cover the hidden rim before the visible rim is drawn.
    mesh.material=restore.material;mesh.color=restore.color;base();
    if(visibleRim) { mesh.material=visibleRim;mesh.color=visible;draw(); }
}
}
