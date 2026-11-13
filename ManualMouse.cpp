#include "Global.h"
#include "ManualMouse.h"
namespace ManualMouse {
namespace {
HANDLE thread=nullptr,ready=nullptr;DWORD threadId=0;
std::atomic<bool> installed{false},suspended{false};
std::atomic<bool> shotIntercept{false};
std::atomic<bool> keyboardReady{false};
std::array<std::atomic<bool>,256> keys{};
std::atomic<int> rawX{0};
std::array<std::atomic<uint64_t>,256> presses{};
std::atomic<uint64_t> shotPresses{0};
std::array<bool,256> physicallyDown{}; // Owned by the hook worker; not cleared by menu suspension.
void PhysicalEvent(int key,bool down){
    const bool edge=down&&!physicallyDown[key];physicallyDown[key]=down;
    const bool focused=Sdk::Foreground();keys[key]=down&&focused;
    if(edge&&focused)++presses[key];
}
LRESULT CALLBACK Keyboard(int code,WPARAM w,LPARAM l) {
    if(code==HC_ACTION && l) {
        const auto& e=*reinterpret_cast<const KBDLLHOOKSTRUCT*>(l);
        if(!(e.flags&(LLKHF_INJECTED|LLKHF_LOWER_IL_INJECTED)) && e.vkCode<256) {
            if(!Sdk::Foreground())for(auto& k:keys)k=false;
            PhysicalEvent(int(e.vkCode),w==WM_KEYDOWN||w==WM_SYSKEYDOWN);
        }
    }
    return CallNextHookEx(nullptr,code,w,l);
}
LRESULT CALLBACK Mouse(int code,WPARAM w,LPARAM l) {
    if(code==HC_ACTION && l && !G::Unload.load()) {
        const auto& event=*reinterpret_cast<const MSLLHOOKSTRUCT*>(l);
        const bool intercept=InterceptShotClick(unsigned(w),event.dwExtraInfo,shotIntercept.load(),Sdk::Foreground(),suspended.load());
        const bool shotEdge=intercept&&w==WM_LBUTTONDOWN&&!physicallyDown[VK_LBUTTON];
        if(!(event.flags&3u)||intercept){
            if(w==WM_LBUTTONDOWN||w==WM_LBUTTONUP)PhysicalEvent(VK_LBUTTON,w==WM_LBUTTONDOWN);
            if(w==WM_RBUTTONDOWN||w==WM_RBUTTONUP)PhysicalEvent(VK_RBUTTON,w==WM_RBUTTONDOWN);
            if(w==WM_MBUTTONDOWN||w==WM_MBUTTONUP)PhysicalEvent(VK_MBUTTON,w==WM_MBUTTONDOWN);
            if(w==WM_XBUTTONDOWN||w==WM_XBUTTONUP)PhysicalEvent(HIWORD(event.mouseData)==XBUTTON1?VK_XBUTTON1:VK_XBUTTON2,w==WM_XBUTTONDOWN);
        }
        if(shotEdge)++shotPresses;
        if(HardwareMove(unsigned(w),event.flags,Sdk::Foreground(),suspended.load()))
            G::LastManualMouse=GetTickCount64();
        if(intercept)return 1;
    }
    return CallNextHookEx(nullptr,code,w,l);
}
DWORD WINAPI Worker(void*) {
    for(int key=1;key<256;++key)physicallyDown[key]=(GetAsyncKeyState(key)&0x8000)!=0;
    MSG msg{};PeekMessageW(&msg,nullptr,WM_USER,WM_USER,PM_NOREMOVE);
    HHOOK hook=SetWindowsHookExW(WH_MOUSE_LL,Mouse,G::SelfModule,0);
    HHOOK keyboard=SetWindowsHookExW(WH_KEYBOARD_LL,Keyboard,G::SelfModule,0);
    keyboardReady=keyboard!=nullptr;
    installed=hook!=nullptr;SetEvent(ready);
    if(hook || keyboard) {
        while(GetMessageW(&msg,nullptr,0,0)>0) {TranslateMessage(&msg);DispatchMessageW(&msg);}
        if(hook)UnhookWindowsHookEx(hook);
        if(keyboard)UnhookWindowsHookEx(keyboard);
    }
    installed=false;keyboardReady=false;for(auto& k:keys)k=false;return 0;
}
}
bool Start() {
    if(thread)return installed.load();
    ready=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!ready)return false;
    suspended=false;shotIntercept=false;G::LastManualMouse=0;
    thread=CreateThread(nullptr,0,Worker,nullptr,0,&threadId);
    if(!thread){CloseHandle(ready);ready=nullptr;return false;}
    if(WaitForSingleObject(ready,2000)!=WAIT_OBJECT_0 || !installed.load()){Stop();return false;}
    return true;
}
bool Stop() {
    suspended=true;shotIntercept=false;G::LastManualMouse=0;
    if(thread) {
        PostThreadMessageW(threadId,WM_QUIT,0,0);
        if(WaitForSingleObject(thread,2000)!=WAIT_OBJECT_0)return false;
        CloseHandle(thread);thread=nullptr;
    }
    if(ready){CloseHandle(ready);ready=nullptr;}
    return true;
}
void Suspend(bool value){suspended=value;if(value)shotIntercept=false;rawX=0;G::LastManualMouse=0;for(auto& k:keys)k=false;}
void SetShotIntercept(bool enabled){shotIntercept=enabled&&installed.load()&&!suspended.load();}
bool ShotInterceptEnabled(){return shotIntercept.load()&&!suspended.load();}
bool PhysicalKey(int key){return key>0&&key<256&&keyboardReady.load()&&Sdk::Foreground()&&keys[key].load();}
void RecordRawX(int x){if(!suspended.load())rawX.fetch_add(std::clamp(x,-10000,10000));}
int ConsumeRawX(){return rawX.exchange(0);}
uint64_t PressSerial(int key){return key>0&&key<256?presses[key].load():0;}
uint64_t ShotPressSerial(){return shotPresses.load();}
bool KeyboardReady(){return keyboardReady.load();}
}
