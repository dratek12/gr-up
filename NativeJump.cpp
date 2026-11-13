#include "NativeJump.h"
#include "Global.h"
#include "ManualMouse.h"
#include <mmsystem.h>
#include <unordered_set>
namespace NativeJump {
namespace {
HANDLE worker=nullptr,stopEvent=nullptr;
std::atomic<uint32_t> options{0};std::atomic<uint64_t> updated{0};
std::atomic<bool> ready{false};
uintptr_t jumpAddress=0,jumpName=0;
bool Resolve() {
    const auto base=Sdk::Client();IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
    if(!Sdk::Read(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||
       !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
       !nt.OptionalHeader.SizeOfImage||nt.OptionalHeader.SizeOfImage>128*1024*1024)return false;
    std::vector<unsigned char> bytes(nt.OptionalHeader.SizeOfImage);
    if(!Sdk::Read(base,bytes.data(),bytes.size()))return false;
    // Button-list signature/layout from cs2-dumper, verified in the running client.
    const auto& signature=Sig::NativeJump;
    uintptr_t found=0;
    for(size_t i=0;i+std::size(signature)<=bytes.size();++i) {
        size_t n=0;for(;n<std::size(signature);++n)if(signature[n]>=0&&signature[n]!=bytes[i+n])break;
        if(n==std::size(signature)){if(found)return false;found=base+i;}
    }
    if(!found)return false;
    const auto slot=found+7+Sdk::R<int32_t>(found+3);
    if(slot<base||slot+8>base+bytes.size())return false;
    uintptr_t node=Sdk::R<uintptr_t>(slot),candidate=0,namePointer=0;
    std::unordered_set<uintptr_t> visited;
    while(node) {
        if(visited.size()>=256||!visited.insert(node).second||node<base||node+Off::JumpNodeSize>base+bytes.size())return false;
        const auto name=Sdk::R<uintptr_t>(node+Off::JumpNodeName);char text[32]{};
        if(!Sdk::ReadChars(name,text,sizeof(text)))return false;
        if(!strcmp(text,"jump")){if(candidate)return false;candidate=node+Off::JumpNodeButton;namePointer=name;}
        node=Sdk::R<uintptr_t>(node+Off::JumpNodeNext);
    }
    if(!candidate)return false;
    jumpAddress=candidate;jumpName=namePointer;
    G::Log("Native jump: verified button +%llX",jumpAddress-base);return true;
}
bool Write(uint32_t value) {
    if(!jumpAddress || Sdk::R<uintptr_t>(jumpAddress-Off::JumpNodeButton+Off::JumpNodeName)!=jumpName)return false;
    return Sdk::Write(jumpAddress,&value,sizeof(value));
}
DWORD WINAPI Run(void*) {
    const bool timer=timeBeginPeriod(1)==TIMERR_NOERROR;
    Controller state;
    while(WaitForSingleObject(stopEvent,0)!=WAIT_OBJECT_0 && !G::Unload.load()) {
        const auto config=options.load();const uint64_t now=GetTickCount64();
        bool allowed=config && now-updated.load()<250 && Sdk::Foreground() && ManualMouse::KeyboardReady();
        bool ground=false;uint32_t identity=0;
        if(allowed) {
            const auto pawn=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalPlayer);
            const auto list=Sdk::EntityList();identity=Sdk::R<uint32_t>(Sdk::R<uintptr_t>(pawn+Off::EntityIdentity)+Off::EntityHandle);
            uint32_t flags=0;const int hp=Sdk::R<int>(pawn+Off::Health);const auto team=Sdk::R<uint8_t>(pawn+Off::Team);
            allowed=pawn>=0x10000&&Sdk::ResolveHandle(list,identity)==pawn&&hp>0&&hp<=1000&&
                (team==2||team==3)&&Sdk::Read(pawn+Off::Flags,&flags,sizeof(flags));
            ground=(flags&1)!=0;
        }
        const bool bhop=(config&1)&&ManualMouse::PhysicalKey(VK_SPACE);
        const bool edge=(config&2)&&ManualMouse::PhysicalKey(int((config>>8)&255));
        auto command=state.Step(allowed,bhop,edge,ground,identity,now);
        if(command.write&&!Write(command.value)){ready=false;G::Log("Native jump: write failed; stopped");break;}
        WaitForSingleObject(stopEvent,config?1:8);
    }
    auto release=state.Step(false,false,false,false,0,0);if(release.write)Write(release.value);
    if(timer)timeEndPeriod(1);ready=false;return 0;
}
}
bool Start() {
    if(worker)return ready.load();if(!Resolve())return false;
    stopEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!stopEvent)return false;
    options=0;updated=0;ready=true;worker=CreateThread(nullptr,0,Run,nullptr,0,nullptr);
    if(!worker){ready=false;CloseHandle(stopEvent);stopEvent=nullptr;return false;}return true;
}
bool Stop() {
    options=0;if(!worker)return true;
    SetEvent(stopEvent);if(WaitForSingleObject(worker,2000)!=WAIT_OBJECT_0)return false;
    CloseHandle(worker);CloseHandle(stopEvent);worker=stopEvent=nullptr;return true;
}
void Publish(bool bhop,bool edge,int edgeKey) {
    updated=GetTickCount64();options=(bhop?1u:0u)|(edge?2u:0u)|(uint32_t(std::clamp(edgeKey,0,255))<<8);
    // A key binding alone must not keep the worker in its active polling mode.
    if(!bhop&&!edge)options=0;
}
void Suspend(){options=0;}
bool Ready(){return ready.load();}
}
