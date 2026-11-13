#include "AimChecks.h"
#include "Global.h"
#include <vector>
#include <cstring>
namespace AimChecks {
namespace {
using FilterFn=void*(__fastcall*)(void*,void*,uint64_t,int,int);
using ShapeFn=bool(__fastcall*)(void*,void*,const Vec3*,const Vec3*,void*,void*);
FilterFn initFilter=nullptr;ShapeFn traceShape=nullptr;uintptr_t managerSlot=0;
bool attempted=false,frameReady=false;const char* status="Checks not initialized";
std::vector<Cloud> clouds;
std::vector<uintptr_t> Find(const std::vector<unsigned char>& bytes,const char* pattern,uintptr_t base) {
    std::vector<int> tokens;std::vector<uintptr_t> result;
    while(*pattern) {
        if(*pattern==' '){++pattern;continue;}
        if(*pattern=='?'){tokens.push_back(-1);while(*pattern=='?')++pattern;}
        else {char* end=nullptr;tokens.push_back(int(strtoul(pattern,&end,16)));if(end==pattern)return {};pattern=end;}
    }
    for(size_t i=0;i+tokens.size()<=bytes.size();++i) {
        size_t j=0;for(;j<tokens.size();++j)if(tokens[j]>=0&&bytes[i+j]!=tokens[j])break;
        if(j==tokens.size())result.push_back(base+i);
    }
    return result;
}
bool Initialize() {
    if(attempted)return initFilter&&traceShape&&managerSlot;
    attempted=true;const auto base=Sdk::Client();IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
    if(!Sdk::Read(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||
       !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
       !nt.OptionalHeader.SizeOfImage||nt.OptionalHeader.SizeOfImage>128*1024*1024)return false;
    std::vector<unsigned char> bytes(nt.OptionalHeader.SizeOfImage);
    if(!Sdk::Read(base,bytes.data(),bytes.size()))return false;
    auto shape=Find(bytes,Sig::TraceShape,base);
    auto filter=Find(bytes,Sig::TraceFilter,base);
    auto managers=Find(bytes,Sig::TraceManager,base);
    uintptr_t slot=0;
    for(auto address:managers) {
        const auto resolved=address+7+Sdk::R<int32_t>(address+3);
        if(resolved<base||resolved+8>base+bytes.size()||(slot&&slot!=resolved))return false;
        slot=resolved;
    }
    if(shape.size()!=1||filter.size()!=1||!slot)return false;
    traceShape=(ShapeFn)shape[0];initFilter=(FilterFn)filter[0];managerSlot=slot;
    G::Log("Aim checks: trace +%llX, filter +%llX, manager +%llX",shape[0]-base,filter[0]-base,slot-base);
    return true;
}
bool NativeTrace(uintptr_t local,Vec3 a,Vec3 b,uintptr_t& hit,float& fraction,bool& solid) {
    alignas(16) unsigned char ray[64]{},filter[128]{},result[512]{};
    const auto manager=Sdk::R<uintptr_t>(managerSlot);if(manager<0x10000)return false;
    __try {
        initFilter(filter,(void*)local,0x1C1043,4,15);
        traceShape((void*)manager,ray,&a,&b,filter,result);
        memcpy(&hit,result+Off::TraceHit,8);memcpy(&fraction,result+Off::TraceFraction,4);
        solid=result[Off::TraceSolid]!=0; // native trace treats this as a hit even with fraction == 1
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {return false;}
}
bool ReadClouds(uintptr_t list) {
    clouds.clear();if(list<0x10000)return false;
    for(int chunkIndex=0;chunkIndex<64;++chunkIndex) {
        uintptr_t chunk=0;if(!Sdk::Read(list+Off::EntityChunkStart+8*chunkIndex,&chunk,sizeof(chunk)))return false;
        if(chunk<0x10000)continue;
        for(int i=0;i<512;++i) {
            const auto entity=Sdk::R<uintptr_t>(chunk+Off::EntityStride*i);if(entity<0x10000)continue;
            const auto identity=Sdk::R<uintptr_t>(entity+Off::EntityIdentity),name=Sdk::R<uintptr_t>(identity+Off::EntityClassName);
            char text[24]{};if(!Sdk::Read(name,text,sizeof(text))||memcmp(text,"smokegrenade_projectile",23))continue;
            bool active=false;Vec3 origin{};
            if(!Sdk::Read(entity+Off::SmokeActive,&active,sizeof(active)))return false;
            if(!active)continue;
            if(!Sdk::Read(entity+Off::SmokeOrigin,&origin,sizeof(origin))||!Finite(origin))return false;
            // Conservative cloud envelope, not the voxel density/hole simulation.
            origin.z+=45.f;clouds.push_back({origin,185.f});
        }
    }
    return true;
}
}
bool Begin(uintptr_t local,uintptr_t list,Vec3 eye,bool flash,bool smoke,bool wall) {
    frameReady=false;
    status="Checks unavailable";
    clouds.clear();
    if(!Finite(eye) || local<0x10000 || (wall && !Initialize()))return false;
    if(flash){
        float overlay=0,screenshot=0;
        if(GetTickCount64()/1000.<G::FlashBlockedUntil ||
           !Sdk::Read(local+Off::FlashDuration,&overlay,4) || !Sdk::Read(local+Off::FlashAlpha,&screenshot,4) ||
           !FlashClear(overlay,screenshot)){status="Flash";return false;}
    }
    if(smoke){
        float density=0;
        if(!Sdk::Read(local+Off::FlashEndTime,&density,4) || !std::isfinite(density) ||
           density<0 || density>.001f || !ReadClouds(list)){status="Smoke";return false;}
    }
    frameReady=true;
    status="Ready";
    return true;
}
bool Clear(uintptr_t local,uintptr_t target,Vec3 eye,Vec3 point,bool wall) {
    if(!frameReady||!Finite(eye)||!Finite(point))return false;
    for(const auto& cloud:clouds)if(SegmentHitsCloud(eye,point,cloud))return false;
    if(!wall)return true;
    uintptr_t hit=0;float fraction=0;bool solid=true;
    return NativeTrace(local,eye,point,hit,fraction,solid) && TraceClear(target,hit,fraction,solid);
}
bool Visible(uintptr_t skip,uintptr_t target,Vec3 start,Vec3 end) {
    if(skip<0x10000 || !Finite(start) || !Finite(end) || !Initialize())return false;
    uintptr_t hit=0;float fraction=0;bool solid=true;
    return NativeTrace(skip,start,end,hit,fraction,solid) && TraceClear(target,hit,fraction,solid);
}
bool RayEnd(uintptr_t skip,Vec3 start,Vec3 end,Vec3& result) {
    if(skip<0x10000 || !Finite(start) || !Finite(end) || !Initialize())return false;
    uintptr_t hit=0;float fraction=0;bool solid=true;
    if(!NativeTrace(skip,start,end,hit,fraction,solid) || solid || !std::isfinite(fraction) || fraction<0 || fraction>1)return false;
    const auto d=end-start;result=start+Vec3(d.x*fraction,d.y*fraction,d.z*fraction);return true;
}
const char* Status(){return status;}
}
