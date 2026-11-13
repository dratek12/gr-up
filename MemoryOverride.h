#pragma once
#include "SDK.h"
#include <array>
// Restore only a still-owned entity and a value last written by this feature.
struct MemoryOverride {
    uintptr_t owner=0,address=0,reference=0,expected=0;
    uint32_t handle=0xFFFFFFFFu;size_t size=0;
    std::array<unsigned char,16> original{},last{};
    bool Valid(uintptr_t list)const {
        return address && Sdk::ResolveHandle(list,handle)==owner &&
            (!reference || Sdk::R<uintptr_t>(reference)==expected);
    }
    void Restore(uintptr_t list) {
        std::array<unsigned char,16> current{};
        if(Valid(list) && Sdk::Read(address,current.data(),size) && !memcmp(current.data(),last.data(),size))
            Sdk::Write(address,original.data(),size);
        *this={};
    }
    template<class T> bool Set(uintptr_t list,uintptr_t entity,uintptr_t field,T value,uintptr_t ref=0,uintptr_t exp=0) {
        static_assert(sizeof(T)<=16);
        uint32_t serial=Sdk::R<uint32_t>(Sdk::R<uintptr_t>(entity+Off::EntityIdentity)+Off::EntityHandle);
        if(address && (owner!=entity||handle!=serial||address!=field||reference!=ref||expected!=exp))Restore(list);
        if(Sdk::ResolveHandle(list,serial)!=entity || (ref && Sdk::R<uintptr_t>(ref)!=exp))return false;
        std::array<unsigned char,16> current{};if(!Sdk::Read(field,current.data(),sizeof(T)))return false;
        if(!address || memcmp(current.data(),last.data(),sizeof(T)))original=current;
        owner=entity;handle=serial;address=field;size=sizeof(T);reference=ref;expected=exp;
        memcpy(last.data(),&value,sizeof(T));return Sdk::Write(address,&value,sizeof(T));
    }
};
