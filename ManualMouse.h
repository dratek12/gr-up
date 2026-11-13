#pragma once
#include <cstdint>
namespace ManualMouse {
inline constexpr uintptr_t ShotInjectionTag = 0x43533253484F54ull;
bool Start();
bool Stop();
void Suspend(bool suspended);
void SetShotIntercept(bool enabled);
bool ShotInterceptEnabled();
bool PhysicalKey(int key);
bool KeyboardReady();
void RecordRawX(int x);
int ConsumeRawX();
uint64_t PressSerial(int key);
uint64_t ShotPressSerial();
inline bool InterceptShotClick(unsigned message,uintptr_t extra,bool enabled,bool focused,bool suspended) {
    return enabled&&focused&&!suspended&&extra!=ShotInjectionTag&&(message==0x201||message==0x202);
}
inline bool HardwareMove(unsigned message,unsigned flags,bool focused,bool suspended) {
    return message==0x200 && !(flags&3u) && focused && !suspended;
}
}
