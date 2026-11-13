#include "Global.h"
#include "ManualMouse.h"
#include "MousePollGate.h"
#include <array>
#include <cstring>

namespace MousePollGate {
namespace {
struct Patch {void** slot=nullptr;void* original=nullptr;void* replacement=nullptr;};
std::array<Patch,4> patches{};
size_t count=0;
std::atomic<bool> ready{false},passShot{false};

SHORT WINAPI KeyState(int key) {
    if(key==VK_LBUTTON&&ready.load()&&ManualMouse::ShotInterceptEnabled()&&!passShot.load())return 0;
    return GetKeyState(key);
}
SHORT WINAPI AsyncKeyState(int key) {
    if(key==VK_LBUTTON&&ready.load()&&ManualMouse::ShotInterceptEnabled()&&!passShot.load())return 0;
    return GetAsyncKeyState(key);
}
UINT WINAPI RawInputBuffer(PRAWINPUT data, PUINT size, UINT headerSize) {
    const UINT result=GetRawInputBuffer(data,size,headerSize);
    if(result==UINT(-1)||!data||!ready.load()||!ManualMouse::ShotInterceptEnabled()||passShot.load())return result;
    auto raw=data;
    for(UINT i=0;i<result;++i) {
        if(raw->header.dwType==RIM_TYPEMOUSE)
            raw->data.mouse.usButtonFlags &= ~(RI_MOUSE_LEFT_BUTTON_DOWN|RI_MOUSE_LEFT_BUTTON_UP);
        raw=reinterpret_cast<PRAWINPUT>(reinterpret_cast<uintptr_t>(raw)+
            ((raw->header.dwSize+sizeof(void*)-1)&~(sizeof(void*)-1)));
    }
    return result;
}
bool Write(void** slot,void* value) {
    DWORD old=0;
    if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&old))return false;
    InterlockedExchangePointer(slot,value);
    DWORD ignored=0;VirtualProtect(slot,sizeof(void*),old,&ignored);
    return true;
}
bool FindAndPatch(HMODULE module,const char* symbol,void* replacement) {
    if(!module||count>=patches.size())return false;
    auto base=reinterpret_cast<uintptr_t>(module);
    auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>0x1000)return false;
    auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    if(nt->Signature!=IMAGE_NT_SIGNATURE)return false;
    const auto& dir=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if(!dir.VirtualAddress||!dir.Size)return false;
    auto desc=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+dir.VirtualAddress);
    for(;desc->Name;++desc) {
        const auto dll=reinterpret_cast<const char*>(base+desc->Name);
        if(_stricmp(dll,"USER32.dll")!=0||!desc->OriginalFirstThunk||!desc->FirstThunk)continue;
        auto names=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+desc->OriginalFirstThunk);
        auto values=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+desc->FirstThunk);
        for(size_t i=0;names[i].u1.AddressOfData;++i) {
            if(IMAGE_SNAP_BY_ORDINAL64(names[i].u1.Ordinal))continue;
            auto import=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names[i].u1.AddressOfData);
            if(strcmp(reinterpret_cast<const char*>(import->Name),symbol))continue;
            auto slot=reinterpret_cast<void**>(&values[i].u1.Function);
            auto original=*slot;
            if(!original||!Write(slot,replacement))return false;
            patches[count++]={slot,original,replacement};
            return true;
        }
    }
    return false;
}
}
bool Start() {
    if(ready.load())return true;
    count=0;passShot=false;
    auto input=GetModuleHandleA("inputsystem.dll"),sdl=GetModuleHandleA("SDL3.dll");
    if(!FindAndPatch(input,"GetKeyState",reinterpret_cast<void*>(&KeyState))||
       !FindAndPatch(sdl,"GetKeyState",reinterpret_cast<void*>(&KeyState))||
       !FindAndPatch(sdl,"GetAsyncKeyState",reinterpret_cast<void*>(&AsyncKeyState))||
       !FindAndPatch(sdl,"GetRawInputBuffer",reinterpret_cast<void*>(&RawInputBuffer))) {
        Stop();return false;
    }
    ready=true;G::Log("Mouse 1 gate: inputsystem and SDL polling hooks active");return true;
}
bool Stop() {
    ready=false;passShot=false;
    bool ok=true;
    for(size_t i=count;i>0;--i) {
        auto& p=patches[i-1];
        if(p.slot&&*p.slot==p.replacement&&!Write(p.slot,p.original))ok=false;
        p={};
    }
    count=0;return ok;
}
bool Ready(){return ready.load();}
void AllowShot(bool allow){passShot=allow;}
}
