#include "AimChecks.h"
#include "Global.h"
#include "Hooks.h"
#include "ESP.h"
#include "Aim.h"
#include "LegitBot.h"
#include "Misc.h"
#include "ExtraRuntime.h"
#include "NativeJump.h"
#include "Config.h"
#include "Chams.h"
#include "MenuMouse.h"
#include "ManualMouse.h"
#include "MousePollGate.h"
#include "BombGlow.h"
#include "SkinChanger.h"
#include "AimTrace.h"
#include "GameplayState.h"
#include <mutex>
#include <atomic>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
using Present1Fn = HRESULT(WINAPI*)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
using ResizeFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
using Resize1Fn = HRESULT(WINAPI*)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*);
namespace {
    std::recursive_mutex renderMutex;
    std::mutex inputMutex;
    struct Message { HWND window; UINT msg; WPARAM w; LPARAM l; };
    std::vector<Message> messages;
    struct Patch { void** slot; void* original; void* replacement; bool installed; };
    std::vector<Patch> patches;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGISwapChain> gameSwap;
    ImGuiContext* guiContext = nullptr;
    bool win32Ready = false, dx11Ready = false;
    std::atomic<WNDPROC> originalWnd{nullptr};
    std::atomic<bool> menuOpen{false}, inputActive{false};
    MenuMouse menuMouse;
    UINT_PTR mouseTimer = 0;
    GameplayState gameplayState;
    std::atomic<UINT_PTR> gameplayTimer{0};
    const UINT restoreMouseMessage = RegisterWindowMessageW(L"cs2esp.restore-menu-mouse.20261107");
    const UINT gameplayQuery = RegisterWindowMessageW(L"cs2esp.gameplay-input.20261108");
    bool GameplayInput(HWND window) {
        if(menuOpen.load() || GetForegroundWindow()!=window)return false;
        auto sdl=GetModuleHandleW(L"SDL3.dll");
        auto focus=reinterpret_cast<void*(*)()>(GetProcAddress(sdl,"SDL_GetMouseFocus"));
        auto relative=reinterpret_cast<bool(*)(void*)>(GetProcAddress(sdl,"SDL_GetWindowRelativeMouseMode"));
        auto focused=focus?focus():nullptr;
        return MenuMouse::AllowsAutomation(menuOpen.load(),focused!=nullptr,focused&&relative&&relative(focused));
    }
    bool CanAutomate() {
        const bool allowed=!G::Unload.load() && !menuOpen.load() && G::Window &&
            GetForegroundWindow()==G::Window && gameplayState.Allows(GetTickCount64());
        AimTrace::inputQuery=allowed?1:0;
        return allowed;
    }
    void CloseMenu(HWND window) {
        menuOpen = false;
        ManualMouse::Suspend(false);
        if (mouseTimer) { KillTimer(window, mouseTimer); mouseTimer = 0; }
        menuMouse.Close();
    }
    void OpenMenu(HWND window) {
        gameplayState.Block();
        NativeJump::Suspend();
        HMODULE sdl = GetModuleHandleW(L"SDL3.dll");
        if (sdl) {
            auto& a = menuMouse.api;
            a.focus = reinterpret_cast<decltype(a.focus)>(GetProcAddress(sdl, "SDL_GetMouseFocus"));
            a.relative = reinterpret_cast<decltype(a.relative)>(GetProcAddress(sdl, "SDL_GetWindowRelativeMouseMode"));
            a.setRelative = reinterpret_cast<decltype(a.setRelative)>(GetProcAddress(sdl, "SDL_SetWindowRelativeMouseMode"));
            a.grab = reinterpret_cast<decltype(a.grab)>(GetProcAddress(sdl, "SDL_GetWindowMouseGrab"));
            a.setGrab = reinterpret_cast<decltype(a.setGrab)>(GetProcAddress(sdl, "SDL_SetWindowMouseGrab"));
            a.drain = reinterpret_cast<decltype(a.drain)>(GetProcAddress(sdl, "SDL_GetRelativeMouseState"));
            if (!menuMouse.Open()) G::Log("Menu: SDL mouse release failed");
        }
        menuOpen = true;
        ManualMouse::Suspend(true);
        if (GetCapture() == window) ReleaseCapture();
        ClipCursor(nullptr);
        // Reapply on the owning thread if the host re-enables mouse capture.
        mouseTimer = SetTimer(window, 0, 16, nullptr);
    }
    bool configLoaded = false;
    // Used only on the window thread: remember which presses reached the host.
    std::array<bool,256> hostDown{}, menuDown{};
    thread_local unsigned presentDepth = 0;
    struct PresentScope { PresentScope() { ++presentDepth; } ~PresentScope() { --presentDepth; } };
    struct ContextScope {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ContextScope() { ImGui::SetCurrentContext(guiContext); }
        ~ContextScope() { ImGui::SetCurrentContext(previous); }
    };
    bool WriteSlot(void** slot, void* value) {
        DWORD previous{};
        if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &previous)) return false;
        InterlockedExchangePointer(reinterpret_cast<void* volatile*>(slot), value);
        DWORD ignored{};
        const bool restored = VirtualProtect(slot, sizeof(void*), previous, &ignored) != FALSE;
        if (!restored) G::Log("Could not restore vtable page protection");
        return true;
    }
    bool Install(void* object, size_t index, void* replacement) {
        void** slot = *reinterpret_cast<void***>(object) + index;
        for (const auto& patch : patches) if (patch.slot == slot) return true;
        patches.push_back({slot, *slot, replacement, false});
        if (!WriteSlot(slot, replacement)) { patches.pop_back(); return false; }
        patches.back().installed = true;
        return true;
    }
    template<class F> F Original(void* object, size_t index) {
        void** slot = *reinterpret_cast<void***>(object) + index;
        for (const auto& patch : patches) if (patch.slot == slot) return reinterpret_cast<F>(patch.original);
        return nullptr;
    }
    bool InputMessage(UINT m) {
        return (m >= WM_KEYFIRST && m <= WM_KEYLAST) || (m >= WM_MOUSEFIRST && m <= WM_MOUSELAST)
            || m == WM_INPUT;
    }
    bool BlockInput(UINT msg, WPARAM w, bool menu) {
        int key = 0; bool down = false, up = false;
        switch (msg) {
        case WM_KEYDOWN: case WM_SYSKEYDOWN: key = static_cast<int>(w); down = true; break;
        case WM_KEYUP: case WM_SYSKEYUP: key = static_cast<int>(w); up = true; break;
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: key=VK_LBUTTON; down=true; break;
        case WM_LBUTTONUP: key=VK_LBUTTON; up=true; break;
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: key=VK_RBUTTON; down=true; break;
        case WM_RBUTTONUP: key=VK_RBUTTON; up=true; break;
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: key=VK_MBUTTON; down=true; break;
        case WM_MBUTTONUP: key=VK_MBUTTON; up=true; break;
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: key=HIWORD(w)==XBUTTON1?VK_XBUTTON1:VK_XBUTTON2; down=true; break;
        case WM_XBUTTONUP: key=HIWORD(w)==XBUTTON1?VK_XBUTTON1:VK_XBUTTON2; up=true; break;
        }
        if (key>0 && key<256) {
            if (down) {
                if (menu || menuDown[key]) { if (!hostDown[key]) menuDown[key]=true; return true; }
                hostDown[key]=true;
                return false;
            }
            if (up) {
                bool forwarded=hostDown[key], captured=menuDown[key];
                hostDown[key]=menuDown[key]=false;
                return !forwarded && (captured || menu);
            }
        }
        return menu && InputMessage(msg);
    }
    LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM w, LPARAM l) {
        if (gameplayQuery && msg == gameplayQuery) return GameplayInput(window)?1:0;
        const auto timer=gameplayTimer.load();
        if(timer && msg==WM_TIMER && w==timer){
            gameplayState.Publish(!G::Unload.load() && GameplayInput(window),GetTickCount64());
            return 0;
        }
        // Block before forwarding the event which opens an overlay or loses focus.
        if(msg==WM_KILLFOCUS || (msg==WM_ACTIVATEAPP&&!w) ||
           ((msg==WM_KEYDOWN||msg==WM_SYSKEYDOWN)&&w==VK_ESCAPE))gameplayState.Block();
        if(msg==WM_INPUT && inputActive.load() && !menuOpen.load()) {
            RAWINPUT raw{};UINT bytes=sizeof(raw);
            if(GetRawInputData(reinterpret_cast<HRAWINPUT>(l),RID_INPUT,&raw,&bytes,sizeof(RAWINPUTHEADER))!=UINT(-1)
                && raw.header.dwType==RIM_TYPEMOUSE) {
                if(!(raw.data.mouse.usFlags&MOUSE_MOVE_ABSOLUTE))ManualMouse::RecordRawX(raw.data.mouse.lLastX);
                if(ManualMouse::ShotInterceptEnabled() &&
                   (raw.data.mouse.usButtonFlags&(RI_MOUSE_LEFT_BUTTON_DOWN|RI_MOUSE_LEFT_BUTTON_UP)) &&
                   GetMessageExtraInfo()!=static_cast<LPARAM>(ManualMouse::ShotInjectionTag)) {
                    DefWindowProcW(window,msg,w,l);
                    return 0;
                }
            }
        }
        if (restoreMouseMessage && msg == restoreMouseMessage) { CloseMenu(window); return 0; }
        if (mouseTimer && msg == WM_TIMER && w == mouseTimer) {
            if (G::Unload.load() || GetForegroundWindow() != window) CloseMenu(window);
            else { menuMouse.Enforce(); ClipCursor(nullptr); }
            return 0;
        }
        if (inputActive.load() && !G::Unload.load()) {
            if (msg == WM_KEYDOWN && !(l & (1LL << 30))) {
                if (w == VK_ESCAPE) NativeJump::Suspend();
                if (w == VK_END) { CloseMenu(window); G::Unload = true; if (G::UnloadEvent) SetEvent(G::UnloadEvent); return 0; }
                if (w == VK_INSERT) { if (menuOpen.load()) CloseMenu(window); else OpenMenu(window); return 0; }
            }
            if (msg != WM_INPUT && (InputMessage(msg) || msg == WM_SETFOCUS || msg == WM_KILLFOCUS)) {
                std::lock_guard lock(inputMutex);
                // Preserve key releases and ordering even while rendering is paused.
                messages.push_back({window, msg, w, l});
            }
            if (msg == WM_KILLFOCUS) { CloseMenu(window); hostDown.fill(false); menuDown.fill(false); }
            if (msg == WM_SETCURSOR && menuOpen.load() && LOWORD(l) == HTCLIENT) {
                SetCursor(nullptr); // ImGui draws the menu cursor.
                return TRUE;
            }
            if (BlockInput(msg, w, menuOpen.load())) {
                if (msg == WM_INPUT) DefWindowProcW(window, msg, w, l); // raw-input cleanup
                return 0;
            }
        }
        auto original = originalWnd.load();
        return original ? CallWindowProcW(original, window, msg, w, l) : DefWindowProcW(window, msg, w, l);
    }
    bool CreateRtv() {
        rtv.Reset();
        ComPtr<ID3D11Texture2D> buffer;
        return gameSwap && device && SUCCEEDED(gameSwap->GetBuffer(0, IID_PPV_ARGS(&buffer)))
            && SUCCEEDED(device->CreateRenderTargetView(buffer.Get(), nullptr, &rtv));
    }
    void ReleaseGraphics() {
        inputActive = false;
        if (guiContext) {
            ImGuiContext* previous = ImGui::GetCurrentContext();
            ImGui::SetCurrentContext(guiContext);
            if (dx11Ready) ImGui_ImplDX11_Shutdown();
            if (win32Ready) ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext(guiContext);
            ImGui::SetCurrentContext(previous == guiContext ? nullptr : previous);
            guiContext = nullptr;
        }
        dx11Ready = win32Ready = false;
        rtv.Reset(); context.Reset(); device.Reset(); gameSwap.Reset();
        G::SwapChain = nullptr; G::Ready = false;
    }
    bool Setup(IDXGISwapChain* sc) {
        if (G::Ready && gameSwap.Get() == sc) return true;
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(sc->GetDesc(&desc)) || desc.OutputWindow != G::Window) return false;
        // Only a chain for our window may replace the previous rendering resources.
        // UI configuration lives in G::Ui and is not reloaded during this transition.
        if (guiContext || gameSwap) ReleaseGraphics();
        if (FAILED(sc->GetDevice(IID_PPV_ARGS(&device)))) return false;
        device->GetImmediateContext(&context);
        gameSwap = sc; G::SwapChain = sc;
        if (!CreateRtv()) { ReleaseGraphics(); return false; }
        auto previous = ImGui::GetCurrentContext();
        guiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(guiContext);
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        win32Ready = ImGui_ImplWin32_Init(G::Window);
        if (win32Ready) dx11Ready = ImGui_ImplDX11_Init(device.Get(), context.Get());
        ImGui::SetCurrentContext(previous);
        if (!dx11Ready) { ReleaseGraphics(); return false; }
        G::Ui.setVisible(menuOpen.load());
        if (!configLoaded) { LoadConfig(); configLoaded = true; }
        G::Ready = true; inputActive = true;
        return true;
    }
    void Render(IDXGISwapChain* sc, UINT flags) {
        if (G::Unload.load() || presentDepth != 1 || (flags & DXGI_PRESENT_TEST) || !Setup(sc)) return;
        if (!rtv && !CreateRtv()) return;
        ContextScope contextScope;
        G::Ui.setVisible(menuOpen.load());
        G::Ui.setFocused(Sdk::Foreground());
        std::vector<Message> pending;
        { std::lock_guard lock(inputMutex); pending.swap(messages); }
        for (const auto& m : pending) {
            bool consumed = getrekt::Win32Input(G::Ui, m.msg, m.w, m.l);
            if (!consumed && m.msg == WM_KEYDOWN && !(m.l & (1LL << 30))) {
                if (m.w == VK_F5) SaveConfig();
                if (m.w == VK_F9) LoadConfig();
            }
            ImGui_ImplWin32_WndProcHandler(m.window, m.msg, m.w, m.l);
        }
        ImGui::GetIO().MouseDrawCursor = menuOpen.load();
        ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
        G::Ui.state.chamsStatus=Chams::Status();
        G::Ui.state.aimChecksStatus=AimChecks::Status();
        DrawESP(); ExtraRuntime::Frame(); G::Ui.draw(); SkinChanger::Frame(); WorldVisuals::UpdateBombGlow(); Chams::Publish();
        const bool gameplay=CanAutomate();
        RunAim(gameplay);
        if (gameplay) { RunTrigger(); RunMisc(); }
        else { ResetTrigger(); ResetMisc(); }
        ImGui::Render();
        ID3D11RenderTargetView* previous[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
        ID3D11DepthStencilView* depth = nullptr;
        context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, previous, &depth);
        ID3D11RenderTargetView* target = rtv.Get();
        context->OMSetRenderTargets(1, &target, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, previous, depth);
        for (auto view : previous) if (view) view->Release();
        if (depth) depth->Release();
    }
    HRESULT WINAPI Present(IDXGISwapChain* sc, UINT sync, UINT flags) {
        PresentScope scope;
        PresentFn original;
        { std::lock_guard lock(renderMutex); original = Original<PresentFn>(sc, 8);
            try { Render(sc, flags); } catch (...) { G::Unload = true; G::Log("Rendering failed; deactivating"); } }
        return original ? original(sc, sync, flags) : E_FAIL;
    }
    HRESULT WINAPI Present1(IDXGISwapChain1* sc, UINT sync, UINT flags, const DXGI_PRESENT_PARAMETERS* params) {
        PresentScope scope;
        Present1Fn original;
        { std::lock_guard lock(renderMutex); original = Original<Present1Fn>(sc, 22);
            try { Render(sc, flags); } catch (...) { G::Unload = true; G::Log("Rendering failed; deactivating"); } }
        return original ? original(sc, sync, flags, params) : E_FAIL;
    }
    HRESULT WINAPI Resize(IDXGISwapChain* sc, UINT n, UINT w, UINT h, DXGI_FORMAT fmt, UINT flags) {
        std::lock_guard lock(renderMutex);
        auto original = Original<ResizeFn>(sc, 13);
        const bool ours = gameSwap.Get() == sc;
        if (ours) rtv.Reset();
        HRESULT result = original ? original(sc, n, w, h, fmt, flags) : E_FAIL;
        if (ours) CreateRtv();
        return result;
    }
    HRESULT WINAPI Resize1(IDXGISwapChain3* sc, UINT n, UINT w, UINT h, DXGI_FORMAT fmt, UINT flags, const UINT* mask, IUnknown* const* queues) {
        std::lock_guard lock(renderMutex);
        auto original = Original<Resize1Fn>(sc, 39);
        const bool ours = gameSwap.Get() == sc;
        if (ours) rtv.Reset();
        HRESULT result = original ? original(sc, n, w, h, fmt, flags, mask, queues) : E_FAIL;
        if (ours) CreateRtv();
        return result;
    }
}
bool Hooks::UnloadAll() {
    // Restore SDL on its owning thread before acquiring renderMutex: the host
    // may be in Present while processing messages on that same thread.
    if (menuOpen.load() && originalWnd.load() && G::Window && IsWindow(G::Window) && restoreMouseMessage) {
        DWORD_PTR result{};
        if (!SendMessageTimeoutW(G::Window, restoreMouseMessage, 0, 0,
            SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &result)) return false;
    }
    std::lock_guard lock(renderMutex);
    G::Unload = true; inputActive = false;
    gameplayState.Block();
    if(const auto timer=gameplayTimer.exchange(0))KillTimer(G::Window,timer);
    ResetTrigger(); ResetAim(); ResetMisc(); ExtraRuntime::Reset(); SkinChanger::Reset(); WorldVisuals::ResetBombGlow();
    bool restored = NativeJump::Stop();
    restored = MousePollGate::Stop() && restored;
    restored = ManualMouse::Stop() && restored;
    restored = Chams::Shutdown() && restored;
    auto original = originalWnd.load();
    if (original && G::Window && IsWindow(G::Window)) {
        auto current = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(G::Window, GWLP_WNDPROC));
        if (current == &WindowProc) {
            SetLastError(0);
            if (!SetWindowLongPtrW(G::Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original)) && GetLastError()) restored = false;
        }
        // A later subclass may chain to us; keep the resident forwarding stub.
    }
    for (auto& patch : patches) if (patch.installed) {
        if (*patch.slot == patch.replacement) {
            if (WriteSlot(patch.slot, patch.original)) patch.installed = false;
            else restored = false;
        } else patch.installed = false; // do not overwrite another component's hook
    }
    ReleaseGraphics();
    { std::lock_guard inputLock(inputMutex); messages.clear(); }
    if (restored) { G::Unhooked = true; if (G::DetachedEvent) SetEvent(G::DetachedEvent); }
    return restored;
}
bool Hooks::Init() {
    std::lock_guard lock(renderMutex);
    if (!G::Window) G::Window = FindWindowW(nullptr, L"Counter-Strike 2");
    DWORD pid{}; if (G::Window) GetWindowThreadProcessId(G::Window, &pid);
    if (pid != GetCurrentProcessId()) return false;
    HWND dummy = CreateWindowExW(0, L"STATIC", L"cs2esp setup", WS_OVERLAPPEDWINDOW, 0, 0, 32, 32, nullptr, nullptr, G::SelfModule, nullptr);
    if (!dummy) return false;
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 1; desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; desc.OutputWindow = dummy;
    desc.SampleDesc.Count = 1; desc.Windowed = TRUE; desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    ComPtr<IDXGISwapChain> sc; ComPtr<ID3D11Device> dev; ComPtr<ID3D11DeviceContext> ctx;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &desc, &sc, &dev, nullptr, &ctx);
    bool ok = SUCCEEDED(hr);
    if (ok) ok = Install(sc.Get(), 8, reinterpret_cast<void*>(&Present)) && Install(sc.Get(), 13, reinterpret_cast<void*>(&Resize));
    ComPtr<IDXGISwapChain1> sc1; ComPtr<IDXGISwapChain3> sc3;
    if (ok && SUCCEEDED(sc.As(&sc1))) ok = Install(sc1.Get(), 22, reinterpret_cast<void*>(&Present1));
    if (ok && SUCCEEDED(sc.As(&sc3))) ok = Install(sc3.Get(), 39, reinterpret_cast<void*>(&Resize1));
    sc3.Reset(); sc1.Reset(); sc.Reset(); ctx.Reset(); dev.Reset(); DestroyWindow(dummy);
    if (!ok) { UnloadAll(); return false; }
    originalWnd = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(G::Window, GWLP_WNDPROC));
    SetLastError(0);
    auto previous = SetWindowLongPtrW(G::Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowProc));
    if (!previous && GetLastError()) { UnloadAll(); return false; }
    originalWnd = reinterpret_cast<WNDPROC>(previous);
    gameplayState.Block();
    gameplayTimer=SetTimer(G::Window,reinterpret_cast<UINT_PTR>(&gameplayState),16,nullptr);
    if(!gameplayTimer.load()){G::Log("Input ownership timer unavailable");UnloadAll();return false;}
    if(!ManualMouse::Start())G::Log("Manual mouse detection unavailable; Non sticky remains inactive");
    if(!MousePollGate::Start())G::Log("Mouse 1 gate unavailable");
    if(!NativeJump::Start())G::Log("Native jump unavailable; no keyboard fallback");
    Chams::Initialize(); // Optional: ESP/menu stay usable when the game layout is unsupported.
    return true;
}
