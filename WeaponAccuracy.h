#pragma once
#include "SDK.h"

struct WeaponAccuracy {
    int id = 0;
    float inaccuracy = 0, spread = 0;
};
inline bool ReadWeaponAccuracy(uintptr_t pawn, uintptr_t list, WeaponAccuracy& result) {
    result = {};
    const int id = Sdk::ActiveWeaponId(pawn,list);
    if (!getrekt::Weapons::Find(id)) return false;
    const auto services=Sdk::R<uintptr_t>(pawn+Off::WeaponServices);
    const auto weapon=Sdk::ResolveHandle(list,Sdk::R<uint32_t>(services+Off::ActiveWeapon));
    const auto data=Sdk::R<uintptr_t>(weapon+Off::WeaponVData);
    const int mode=Sdk::R<int>(weapon+Off::WeaponMode);
    float inaccuracy{},spread{};
    if(mode<0 || mode>1 || data<0x10000 ||
        !Sdk::Read(pawn+Off::InterpolatedInaccuracy,&inaccuracy,sizeof(inaccuracy)) ||
        !Sdk::Read(data+Off::WeaponSpread+4*mode,&spread,sizeof(spread)) ||
        !std::isfinite(inaccuracy) || !std::isfinite(spread) || inaccuracy<0 || spread<0 || inaccuracy>1 || spread>1) return false;
    result={id,inaccuracy,spread};
    return true;
}
