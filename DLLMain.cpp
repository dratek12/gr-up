#include "Global.h"
#include "Hooks.h"
#include "SkinInventoryBridge.h"
static DWORD WINAPI MainThread(LPVOID) {
    wchar_t name[80]{};
    swprintf_s(name, L"Local\\cs2esp_unload_%lu", GetCurrentProcessId());
    G::UnloadEvent = CreateEventW(nullptr, TRUE, FALSE, name);
    swprintf_s(name, L"Local\\cs2esp_detached_%lu", GetCurrentProcessId());
    G::DetachedEvent = CreateEventW(nullptr, TRUE, FALSE, name);
    if (!G::UnloadEvent || !G::DetachedEvent) { G::Log("Cannot create control events"); return 0; }
    while (!GetModuleHandleW(L"client.dll")) {
        if (WaitForSingleObject(G::UnloadEvent, 200) == WAIT_OBJECT_0) {
            G::Unhooked = true; SetEvent(G::DetachedEvent); return 0;
        }
    }
    // Keep callback code mapped even if another thread has fetched its address
    // immediately before the vtable / window procedure is restored.
    HMODULE pinned{};
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
        reinterpret_cast<LPCWSTR>(&MainThread), &pinned)) return 0;
    if (!Hooks::Init()) {
        G::Log("Hook initialization failed");
        Hooks::UnloadAll();
        return 0;
    }
    if (!SkinInventoryBridge::Start()) G::Log("Skin inventory changer initialization failed");
    G::Log("Hooks initialized; Insert: menu, End: deactivate");
    while (!G::Unhooked.load()) {
        if (WaitForSingleObject(G::UnloadEvent, 50) == WAIT_OBJECT_0) G::Unload = true;
        if (G::Unload.load()) Hooks::UnloadAll();
    }
    G::Log("Deactivated. Module remains resident until process exit.");
    G::CloseLog();
    return 0;
}
BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        G::SelfModule = mod;
        DisableThreadLibraryCalls(mod);
        HANDLE worker = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (worker) CloseHandle(worker);
    }
    return TRUE;
}
