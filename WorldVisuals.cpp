#include "WorldVisuals.h"
#include "BombGlow.h"
#include "Global.h"
#include "AimChecks.h"
#include "ChamsPass.h"
#include <unordered_map>
namespace WorldVisuals {
namespace {
uintptr_t PlantedSlot() {
    static uintptr_t slot=0;static bool attempted=false;
    if(attempted)return slot;attempted=true;
    const auto base=Sdk::Client();IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
    if(!Sdk::Read(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||
       !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
       !nt.OptionalHeader.SizeOfImage||nt.OptionalHeader.SizeOfImage>128*1024*1024)return 0;
    std::vector<unsigned char> bytes(nt.OptionalHeader.SizeOfImage);if(!Sdk::Read(base,bytes.data(),bytes.size()))return 0;
    // Upstream cs2-dumper signature, independently verified against the live C4.
    // This build stores a direct entity pointer, not a pointer-to-pointer.
    for(size_t i=0;i+sizeof(Sig::PlantedBomb)<=bytes.size();++i) {
        if(bytes[i]!=Sig::PlantedBomb[0]||bytes[i+1]!=Sig::PlantedBomb[1]||bytes[i+2]!=Sig::PlantedBomb[2]||bytes[i+7]!=Sig::PlantedBomb[7]||bytes[i+8]!=Sig::PlantedBomb[8]||bytes[i+9]!=Sig::PlantedBomb[9])continue;
        if(slot){slot=0;return 0;}
        int32_t delta=0;memcpy(&delta,bytes.data()+i+3,4);
        const auto resolved=base+i+7+delta;
        if(resolved<base || resolved+8>base+bytes.size())return 0;
        slot=resolved;
    }
    if(slot)G::Log("C4: planted entity pointer +%llX",slot-base);
    return slot;
}
}
std::vector<uint32_t> Bombs(uintptr_t list) {
    std::vector<uint32_t> bombs;
    if(list<0x10000)return bombs;
    auto add=[&](uintptr_t entity,bool planted) {
        if(entity<0x10000)return;
        uint32_t owner=0;bool ticking=false,defused=true,exploded=true;
        if(planted) {
            if(!Sdk::Read(entity+Off::BombTicking,&ticking,1)||!Sdk::Read(entity+Off::BombDefused,&defused,1)||!Sdk::Read(entity+Off::BombExploded,&exploded,1))return;
        } else if(!Sdk::Read(entity+Off::BombOwner,&owner,4))return;
        if(!BombEligible(planted,owner,ticking,defused,exploded))return;
        auto scene=Sdk::R<uintptr_t>(entity+Off::Scene);uint8_t dormant=1;Vec3 origin{};
        if(!Sdk::Read(scene+Off::Dormant,&dormant,1)||dormant||!Sdk::Origin(scene,origin))return;
        const auto identity=Sdk::R<uintptr_t>(entity+Off::EntityIdentity);uint32_t handle=0xFFFFFFFFu;
        if(Sdk::Read(identity+Off::EntityHandle,&handle,4) && Sdk::ResolveHandle(list,handle)==entity &&
           std::find(bombs.begin(),bombs.end(),handle)==bombs.end())bombs.push_back(handle);
    };
    const auto slot=PlantedSlot();if(slot)add(Sdk::R<uintptr_t>(slot),true);
    for(int c=0;c<64;++c) {
        auto chunk=Sdk::R<uintptr_t>(list+Off::EntityChunkStart+8*c);if(chunk<0x10000)continue;
        for(int i=0;i<512;++i) {
            auto entity=Sdk::R<uintptr_t>(chunk+Off::EntityStride*i);if(entity<0x10000)continue;
            auto identity=Sdk::R<uintptr_t>(entity+Off::EntityIdentity);char name[40]{};
            if(!Sdk::ReadStr(identity+Off::EntityClassName,name,sizeof(name)))continue;
            if(!strcmp(name,"planted_c4"))add(entity,true);
            else if(!strcmp(name,"weapon_c4"))add(entity,false);
        }
    }
    return bombs;
}

namespace {
std::unordered_map<uint32_t,GlowPatch> patches;
uintptr_t previousList=0;
void Restore(uintptr_t list,uint32_t handle,const GlowPatch& patch) {
    if(Sdk::ResolveHandle(list,handle)==patch.entity)patch.Restore();
}
}
void ResetBombGlow() {
    const auto list=Sdk::EntityList();
    if(list==previousList)for(const auto& [handle,patch]:patches)Restore(list,handle,patch);
    patches.clear();previousList=0;
}
void UpdateBombGlow() {
    const auto list=Sdk::EntityList();const auto& cfg=G::Ui.state.preview;
    if(list!=previousList){patches.clear();previousList=list;}
    if(!cfg.glowC4 || G::Unload.load() || !G::LocalAlive){ResetBombGlow();return;}
    auto handles=Bombs(list);
    for(auto it=patches.begin();it!=patches.end();) {
        if(std::find(handles.begin(),handles.end(),it->first)==handles.end()) {
            Restore(list,it->first,it->second);it=patches.erase(it);
        } else ++it;
    }
    const auto local=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalPlayer);
    const auto color=Chams::Color(cfg.colC4.x,cfg.colC4.y,cfg.colC4.z,cfg.colC4.w);
    for(auto handle:handles) {
        const auto entity=Sdk::ResolveHandle(list,handle);if(!entity)continue;
        Vec3 origin{};
        const bool visible=!cfg.glowVisibleOnly || (Sdk::Origin(Sdk::R<uintptr_t>(entity+Off::Scene),origin) &&
            AimChecks::Visible(local,entity,G::LocalEye,origin+Vec3(0,0,4)));
        if(!visible) {if(auto it=patches.find(handle);it!=patches.end()){Restore(list,handle,it->second);patches.erase(it);}continue;}
        auto it=patches.find(handle);
        if(it==patches.end()) {
            GlowPatch patch;if(!patch.Capture(entity))continue;
            it=patches.emplace(handle,patch).first;
        }
        if(!it->second.Apply(color)){Restore(list,handle,it->second);patches.erase(it);}
    }
}

}
