#include "ClientConVars.h"
#include "SDK.h"
#include <unordered_map>
#include <array>
namespace ClientConVars {
namespace {
struct Image {
    uintptr_t base;std::vector<unsigned char> data;
    Image(const wchar_t* name):base((uintptr_t)GetModuleHandleW(name)) {
        IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
        if(!Sdk::Read(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||
           !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
           !nt.OptionalHeader.SizeOfImage||nt.OptionalHeader.SizeOfImage>128*1024*1024)return;
        data.resize(nt.OptionalHeader.SizeOfImage);if(!Sdk::Read(base,data.data(),data.size()))data.clear();
    }
    uintptr_t Find(const char* pattern)const {
        std::vector<int> bytes;while(*pattern){if(*pattern==' '){++pattern;continue;}if(*pattern=='?'){bytes.push_back(-1);++pattern;}else{char* end;bytes.push_back((int)strtoul(pattern,&end,16));if(end==pattern)return 0;pattern=end;}}
        uintptr_t found=0;for(size_t i=0;i+bytes.size()<=data.size();++i){size_t j=0;for(;j<bytes.size();++j)if(bytes[j]>=0&&bytes[j]!=data[i+j])break;if(j==bytes.size()){if(found)return 0;found=base+i;}}
        return found;
    }
};
struct Variable {uintptr_t object=0,node=0,namePointer=0;bool changed=false;size_t size=0;std::array<unsigned char,8> original{},last{};};
std::unordered_map<std::string,Variable> vars;
int typeOffset=0,valueOffset=0;bool attempted=false,ready=false;
bool Initialize() {
    if(attempted)return ready;attempted=true;
    Image client(L"client.dll"),tier(L"tier0.dll");
    // Layout patterns independently checked in the current client. References:
    // Osiris MemoryPatterns/Windows/{CvarPatternsWindows,ConVarPatternsWindows}.h
    auto sys=client.Find(Sig::ConVarSystem);
    auto list=tier.Find(Sig::ConVarList);
    auto type=tier.Find(Sig::ConVarType),value=tier.Find(Sig::ConVarValue);
    if(!sys||!list||!type||!value)return false;
    const uintptr_t slot=sys+11+Sdk::R<int32_t>(sys+7);
    if(slot<client.base||slot+8>client.base+client.data.size())return false;
    typeOffset=Sdk::R<uint8_t>(type+7);valueOffset=Sdk::R<uint8_t>(value+7);
    const int memOffset=Sdk::R<uint8_t>(list+3);
    if(typeOffset<8||typeOffset>128||valueOffset<8||valueOffset>128||memOffset<8||memOffset>240)return false;
    const auto system=Sdk::R<uintptr_t>(slot),memory=Sdk::R<uintptr_t>(system+memOffset);
    const auto allocation=Sdk::R<uint16_t>(system+memOffset-Off::ConVarAllocationBeforeMemory);
    uint16_t index=Sdk::R<uint16_t>(system+memOffset+8);
    std::vector<bool> visited(65536);
    for(size_t count=0;index!=0xFFFF && count<65535;++count) {
        if(index>=allocation||visited[index])return false;visited[index]=true;
        const auto node=memory+Off::ConVarNodeStride*index,object=Sdk::R<uintptr_t>(node),namePointer=Sdk::R<uintptr_t>(object);
        char name[128]{};
        if(!Sdk::ReadChars(namePointer,name,sizeof(name)))return false;
        for(const char* wanted:{"fov_cs_debug","viewmodel_fov","mat_tonemap_force_scale","r_drawviewmodel","cl_disable_postprocessing","cam_idealdist","c_maxdistance","c_thirdpersonshoulder"})
            if(!strcmp(name,wanted))vars.emplace(name,Variable{object,node,namePointer});
        index=Sdk::R<uint16_t>(node+Off::ConVarNodeNext);
    }
    ready=!vars.empty();return ready;
}
bool Valid(const Variable& v,int type) {
    return v.object>=0x10000 && Sdk::R<uintptr_t>(v.node)==v.object &&
        Sdk::R<uintptr_t>(v.object)==v.namePointer && Sdk::R<uint16_t>(v.object+typeOffset)==type;
}
void Restore(Variable& v) {
    std::array<unsigned char,8> current{};
    if(v.changed && Sdk::R<uintptr_t>(v.node)==v.object && Sdk::R<uintptr_t>(v.object)==v.namePointer &&
       Sdk::Read(v.object+valueOffset,current.data(),v.size) && !memcmp(current.data(),v.last.data(),v.size))
        Sdk::Write(v.object+valueOffset,v.original.data(),v.size);
    v.changed=false;
}
template<class T>bool Apply(const char* name,bool enabled,T value,int type) {
    if(!attempted && !enabled)return true;
    if(!Initialize())return !enabled;
    auto it=vars.find(name);if(it==vars.end())return !enabled;
    auto& v=it->second;if(!enabled){Restore(v);return true;}
    if(!Valid(v,type))return false;
    std::array<unsigned char,8> current{};if(!Sdk::Read(v.object+valueOffset,current.data(),sizeof(T)))return false;
    if(!v.changed || memcmp(current.data(),v.last.data(),sizeof(T)))v.original=current;
    v.size=sizeof(T);memcpy(v.last.data(),&value,sizeof(T));v.changed=true;
    return Sdk::Write(v.object+valueOffset,&value,sizeof(T));
}
}
bool Float(const char* name,bool enabled,float value){return std::isfinite(value)&&Apply(name,enabled,value,7);}
bool Bool(const char* name,bool enabled,bool value){return Apply(name,enabled,value,0);}
void Reset(){for(auto& [name,v]:vars)Restore(v);}
}
