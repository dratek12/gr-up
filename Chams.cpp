#include "Global.h"
#include "Chams.h"
#include "ChamsPass.h"
#include "WorldVisuals.h"
#include "AimChecks.h"
#include "SoundEspPolicy.h"
#include <memory>
#include <string>
#include <array>

namespace Chams {
namespace {
using DrawFn=uintptr_t(__fastcall*)(void*,void*,Mesh*,int,void*,void*,void*,void*);
using LoadFn=bool(__fastcall*)(void*,void*,const char*,const void*,const char*,unsigned);
using MakeFn=void(__fastcall*)(void*,void**,const char*,void*,unsigned,unsigned);
DrawFn original{};
void** slot{};
std::atomic<bool> active{false};
std::atomic<int> status{0}; // 0: not initialized, 1: ready, 2: unsupported, 3: material failure
struct Frame {
    bool enabled=false, visibleOnly=false;
    bool chams=false, glow=false, glowVisibleOnly=false;
    int glowMaterial=0;
    uint32_t glowVisible=0, glowHidden=0;
    int material=0;
    uint32_t visible=0, hidden=0;
    std::array<uint32_t,64> handles{};
    std::array<bool,64> occluded{};
    size_t count=0;

};
std::atomic<std::shared_ptr<const Frame>> frame;
struct Materials { void* visible{}; void* hidden{}; } materials[3];
Materials glowRim;
// Materials and their creation data remain valid for late callbacks until process exit.
std::vector<std::unique_ptr<std::byte[]>> storage;
std::atomic<uint64_t> batches{0}, colored{0}, hiddenPasses{0};

struct Image {
    uintptr_t base{};std::vector<unsigned char> bytes;
    explicit Image(const wchar_t* module) {
        base=(uintptr_t)GetModuleHandleW(module);
        IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
        if (!Sdk::Read(base,&dos,sizeof(dos)) || dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<0 ||
            !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt)) || nt.Signature!=IMAGE_NT_SIGNATURE ||
            nt.OptionalHeader.SizeOfImage>128*1024*1024) { base=0;return; }
        bytes.resize(nt.OptionalHeader.SizeOfImage);
        if (!Sdk::Read(base,bytes.data(),bytes.size())) { bytes.clear();base=0; }
    }
    template<class T>T At(size_t offset) const {
        T result{};if(offset<=bytes.size() && sizeof(T)<=bytes.size()-offset)memcpy(&result,bytes.data()+offset,sizeof(T));return result;
    }
    uintptr_t Unique(const char* pattern) const {
        std::vector<int> wanted;
        while(*pattern) {
            if(*pattern==' '){++pattern;continue;}
            if(*pattern=='?'){wanted.push_back(-1);while(*pattern=='?')++pattern;}
            else {char* end;wanted.push_back(int(strtoul(pattern,&end,16)));if(end==pattern)return 0;pattern=end;}
        }
        uintptr_t found=0;
        for(size_t i=0;i+wanted.size()<=bytes.size();++i) {
            size_t j=0;for(;j<wanted.size();++j)if(wanted[j]>=0 && bytes[i+j]!=wanted[j])break;
            if(j==wanted.size()){if(found)return 0;found=base+i;}
        }
        return found;
    }
    void** RendererSlot() const {
        const char name[]=".?AVCAnimatableSceneObjectDesc@@";
        auto type=std::search(bytes.begin(),bytes.end(),std::begin(name),std::end(name));
        if(type==bytes.end() || type-bytes.begin()<16)return nullptr;
        uint32_t typeRva=uint32_t(type-bytes.begin()-16);
        void** found=nullptr;
        for(size_t col=0;col+24<=bytes.size();col+=4) {
            if(At<uint32_t>(col)!=1 || At<uint32_t>(col+12)!=typeRva || At<uint32_t>(col+20)!=col)continue;
            for(size_t ref=0;ref+24<=bytes.size();ref+=8) {
                if(At<uintptr_t>(ref)!=base+col)continue;
                auto function=At<uintptr_t>(ref+16); // descriptor vtable slot 1
                unsigned char actual[sizeof(Sig::RendererPrefix)]{};
                if(function<base || function>=base+bytes.size() || !Sdk::Read(function,actual,sizeof(actual)) || memcmp(actual,Sig::RendererPrefix,sizeof(actual)))continue;
                // The current renderer advances each mesh by 0x70; old 0x68 layouts are incompatible.
                if(At<uint32_t>(function-base+Off::RendererStrideCheck)!=Off::RendererStrideOpcode)continue;
                if(found)return nullptr;
                found=(void**)(base+ref+16);
            }
        }
        return found;
    }
};
bool Patch(void* replacement) {
    if(!slot)return false;DWORD old{},ignored{};
    if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&old))return false;
    InterlockedExchangePointer(slot,replacement);
    if(!VirtualProtect(slot,sizeof(void*),old,&ignored))G::Log("Chams: could not restore vtable protection");
    return true;
}
bool MakeSafe(LoadFn load,MakeFn make,void* kv,const char* text,const char* name,void** output) {
    struct Id {const char* name;uint64_t low,high;} id{"generic",0x469806E97412167Cull,0xE73790B53EE6F2AFull};
    __try {
        if(!load(kv,nullptr,text,&id,nullptr,0))return false;
        void* binding=nullptr;make(nullptr,&binding,name,kv,0,1);
        *output=(void*)Sdk::R<uintptr_t>((uintptr_t)binding);
        return *output && Sdk::R<uintptr_t>((uintptr_t)*output)>=0x10000;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
void* MakeMaterial(LoadFn load,MakeFn make,int kind,bool hidden) {
    auto buffer=std::make_unique<std::byte[]>(4096);
    std::string text="<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->\n{\n";
    text += kind==3 ? "shader = \"csgo_effects.vfx\"\n" : kind==2 ? "shader = \"csgo_character.vfx\"\n" : "shader = \"csgo_unlitgeneric.vfx\"\n";
    text += "g_tColor = resource:\"materials/dev/primary_white_color_tga_21186c76.vtex\"\n";
    text += "g_tNormal = resource:\"materials/default/default_normal_tga_1b833b2a.vtex\"\ng_vColorTint = [1.0,1.0,1.0,1.0]\n";
    if(kind==2)text += "g_tAmbientOcclusion = resource:\"materials/default/default_ao_tga_79a2e0d0.vtex\"\ng_tMetalness = resource:\"materials/default/default_metal_tga_8fbc2820.vtex\"\ng_flMetalness = 1.0\ng_flRoughness = 0.15\n";
    if(kind==3)for(const char* mask : {"g_tMask1","g_tMask2","g_tMask3","g_tTintMask"})
        text += std::string(mask)+" = resource:\"materials/dev/primary_white_color_tga_21186c76.vtex\"\n";
    if(kind==3)text += "F_TRANSLUCENT = 1\nF_ADDITIVE_BLEND = 1\ng_flFresnelExponent = 5.0\ng_flFresnelFalloff = 1.0\ng_flFresnelMin = 0.0\ng_flFresnelMax = 1.0\ng_flOpacityScale = 0.55\ng_flColorBoost = 1.5\n";
    text += hidden ? "F_IGNOREZ = 1\nF_DISABLE_Z_BUFFERING = 1\nF_DISABLE_Z_PREPASS = 1\n" : "F_IGNOREZ = 0\nF_DISABLE_Z_BUFFERING = 0\n";
    text += (hidden || kind==3) ? "F_DISABLE_Z_WRITE = 1\n}\n" : "F_DISABLE_Z_WRITE = 0\n}\n";
    char name[96];sprintf_s(name,"materials/cs2esp/effect_v4_%d_%s.vmat",kind,hidden?"hidden":"visible");
    void* result=nullptr;
    bool ok=MakeSafe(load,make,buffer.get()+256,text.c_str(),name,&result);
    storage.push_back(std::move(buffer));
    G::Log("Chams material %s: %s",name,ok?"created":"failed");
    return ok?result:nullptr;
}
int Target(const Frame& state,const Mesh& mesh) {
    if(mesh.scene<0x10000)return 0;
    uint32_t owner=0xFFFFFFFFu;
    if(!Sdk::Read(mesh.scene+Off::MeshSceneOwner,&owner,sizeof(owner)))return 0;
    for(size_t i=0;i<state.count;++i)if(state.handles[i]==owner)return state.occluded[i]?3:1;
    return 0;
}
uintptr_t __fastcall Draw(void* a,void* b,Mesh* meshes,int count,void* e,void* f,void* g,void* h) {
    ++batches;
    auto state=frame.load();
    if(!active.load() || !state || !state->enabled || !meshes || count<1 || count>4096)
        return original(a,b,meshes,count,e,f,g,h);
    uintptr_t result=0;
    std::vector<int> targets(count);
    for(int i=0;i<count;++i)targets[i]=Target(*state,meshes[i]);
    auto pass=[&](int i,void* material,uint32_t tint) {
        auto& mesh=meshes[i];
        Passes(mesh,true,material,nullptr,tint,0,[&]{result=original(a,b,&mesh,1,e,f,g,h);});
    };
    BatchPasses(count,[&](int i) {
        if(targets[i]==3) {
            ++colored;
            // One hidden layer, then all opaque parts, then visible glow.
            if(state->glow && !state->glowVisibleOnly)pass(i,glowRim.hidden,state->glowHidden);
            else if(state->chams && !state->visibleOnly) {
                ++hiddenPasses;pass(i,materials[state->material].hidden,state->hidden);
            }
        }
    },[&](int i) {
        if((targets[i]==1 || targets[i]==3) && state->chams)pass(i,materials[state->material].visible,state->visible);
        else if((targets[i]==1 || targets[i]==3) && state->glow && state->glowMaterial!=1)
            pass(i,materials[state->glowMaterial].visible,state->glowVisible);
        else result=original(a,b,&meshes[i],1,e,f,g,h);
    },[&](int i) {
        if((targets[i]==1 || targets[i]==3) && state->glow)pass(i,glowRim.visible,state->glowVisible);
    });
    return result;
}
}

bool Initialize() {
    if(status.load()!=0)return active.load();
    Image scene(L"scenesystem.dll"), material(L"materialsystem2.dll");
    slot=scene.RendererSlot();
    auto make=(MakeFn)material.Unique(Sig::MakeMaterial);
    auto load=(LoadFn)GetProcAddress(GetModuleHandleW(L"tier0.dll"),"?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2I@Z");
    if(!slot || !make || !load){status=2;G::Log("Chams: unsupported rendering interfaces");return false;}
    // Textured preserves the model's own visible material. Its hidden silhouette is unlit.
    materials[0]={MakeMaterial(load,make,0,false),MakeMaterial(load,make,0,true)};
    materials[1]={nullptr,materials[0].hidden};
    materials[2]={MakeMaterial(load,make,2,false),MakeMaterial(load,make,2,true)};
    glowRim={MakeMaterial(load,make,3,false),MakeMaterial(load,make,3,true)};
    if(!materials[0].visible || !materials[0].hidden || !materials[2].visible || !materials[2].hidden){status=3;return false;}
    original=(DrawFn)*slot;
    active=true;
    if(!Patch((void*)&Draw)){active=false;status=2;return false;}
    status=glowRim.visible && glowRim.hidden ? 1 : 4;G::Log("Chams initialized: model renderer, mesh stride 0x70, owner +0xC0");return true;
}
void Publish() {
    if(!active.load())return;
    auto next=std::make_shared<Frame>();
    const auto& cfg=G::Ui.state.preview;
    next->chams=cfg.chams || (cfg.sound && !cfg.glow);
    next->glow=cfg.glow && glowRim.visible && glowRim.hidden;
    next->glowVisibleOnly=cfg.glowVisibleOnly;
    next->glowMaterial=std::clamp(cfg.glowMaterial,0,2);
    next->glowVisible=Color(cfg.glowVisible.x,cfg.glowVisible.y,cfg.glowVisible.z,cfg.glowVisible.w);
    next->glowHidden=Color(cfg.glowOccluded.x,cfg.glowOccluded.y,cfg.glowOccluded.z,cfg.glowOccluded.w);
    next->enabled=(next->chams || next->glow) && !G::Unload.load();
    next->visibleOnly=cfg.chams && cfg.chamsVisibleOnly;
    next->material=std::clamp(cfg.chamsMaterial,0,2);
    next->visible=Color(cfg.color.x,cfg.color.y,cfg.color.z,cfg.color.w);
    next->hidden=Color(cfg.chamsOccluded.x,cfg.chamsOccluded.y,cfg.chamsOccluded.z,cfg.chamsOccluded.w);
    if(cfg.sound && !cfg.chams && !cfg.glow) {
        next->material=0;
        next->visible=next->hidden=Color(cfg.colSound.x,cfg.colSound.y,cfg.colSound.z,cfg.colSound.w);
    }
    if(next->enabled && (next->chams || next->glow))for(const auto& player:G::Players) {
        if(!player.valid || player.health<=0 || (G::Ui.state.cs2.team && player.team==G::LocalTeam))continue;
        auto identity=Sdk::R<uintptr_t>(player.entity+Off::EntityIdentity);uint32_t handle=0xFFFFFFFFu;
        if(identity>=0x10000 && Sdk::Read(identity+Off::EntityHandle,&handle,sizeof(handle)) && handle!=0xFFFFFFFFu && next->count<next->handles.size())
        {
            const auto local=Sdk::R<uintptr_t>(Sdk::Client()+Off::LocalPlayer);
            const bool visible=cfg.sound ? player.lineOfSight : (player.headValid && AimChecks::Visible(local,player.entity,G::LocalEye,player.head)) ||
                (player.chestValid && AimChecks::Visible(local,player.entity,G::LocalEye,player.chest)) ||
                (player.stomachValid && AimChecks::Visible(local,player.entity,G::LocalEye,player.stomach));
            if(!SoundEspPolicy::ShowPlayer(cfg.sound,visible,player.soundVisible))continue;
            if(cfg.sound && !cfg.chams && !cfg.glow && visible)continue;
            next->occluded[next->count]=!visible;
            next->handles[next->count++]=handle;
        }
    }
    frame.store(std::move(next));
}
bool Shutdown() {
    active=false;frame.store(nullptr);
    bool ok=!slot || !original || *slot!=(void*)&Draw || Patch((void*)original);
    if(original)G::Log("Chams deactivated: batches=%llu colored=%llu hidden=%llu",batches.load(),colored.load(),hiddenPasses.load());
    return ok;
}
const char* Status() {
    switch(status.load()) {
    case 2:return "Chams unavailable for this game build.";
    case 3:return "Could not load chams materials.";
    case 4:return "Glow materials unavailable; chams remain available.";
    default:return "";
    }
}
}
