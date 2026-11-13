#pragma once
#include "CommonIncludes.h"
#include "SDK.h"


#include <atomic>
#include <vector>

namespace G {
    inline std::atomic<uint64_t> LastManualMouse{0};
    inline HWND  Window = nullptr;
    inline bool  Ready = false;
    inline float Sens = 2.5f;

    inline bool  LocalAlive = false;
    inline int   LocalTeam = 0;
    inline Vec3  LocalOrigin{};
    inline Vec3  LocalEye{};
    inline Vec3  Punch{};
    inline double FlashBlockedUntil=0;

    inline std::vector<CPlayer> Players;

    inline getrekt::UI Ui;

    inline int  DbgCnt[6]{};
    inline uintptr_t DbgList = 0, DbgChunk = 0;
    inline int  DbgSlots = 0;
    inline float DbgPitch = 0.f, DbgYaw = 0.f;

    inline std::atomic<bool> Unload{ false };
    inline std::atomic<bool> Unhooked{ false };
    inline HMODULE  SelfModule = nullptr;
    inline HANDLE   UnloadEvent = nullptr;
    inline HANDLE DetachedEvent = nullptr;
    inline IDXGISwapChain* SwapChain = nullptr;

    void Log(const char* fmt, ...);
    void CloseLog();
}
