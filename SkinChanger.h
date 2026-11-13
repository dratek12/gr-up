#pragma once
#include <cstdint>
namespace SkinChanger {
    bool ApplyWeapon(uintptr_t weapon,uint32_t handle,int weaponId,int paint,int seed,int statTrak,float wear);
    void Frame();
    void Reset();
}
