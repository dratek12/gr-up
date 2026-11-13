#include "ThirdPerson.h"
#include "SDK.h"
#include "ClientConVars.h"
namespace ThirdPerson {
namespace {
uintptr_t slot=0,object=0,field=0;bool attempted=false,owned=false;unsigned char original=0;
bool Resolve() {
    if(attempted)return field!=0;attempted=true;
    const auto base=Sdk::Client();IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
    if(!Sdk::Read(base,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||
       !Sdk::Read(base+dos.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
       nt.OptionalHeader.SizeOfImage>128*1024*1024)return false;
    std::vector<unsigned char> bytes(nt.OptionalHeader.SizeOfImage);
    if(!Sdk::Read(base,bytes.data(),bytes.size()))return false;
    // Native thirdperson command: set camera flag, initialize yaw/pitch/distance.
    // Both the flag offset and input pointer are derived from this verified instruction sequence.
    const auto& pattern=Sig::ThirdPerson;
    size_t match=0;
    for(size_t i=Off::ThirdPersonSignatureLead;i+sizeof(pattern)/sizeof(int)<=bytes.size();++i){
        size_t j=0;for(;j<std::size(pattern);++j)if(pattern[j]>=0&&bytes[i+j]!=pattern[j])break;
        if(j==std::size(pattern)){if(match)return false;match=i;}
    }
    if(!match||bytes[match-Off::ThirdPersonSignatureLead]!=0x4c||bytes[match-Off::ThirdPersonSignatureLead+1]!=0x8b||bytes[match-Off::ThirdPersonSignatureLead+2]!=5)return false;
    const auto instruction=base+match-Off::ThirdPersonSignatureLead;
    slot=instruction+7+Sdk::R<int32_t>(instruction+3);
    const auto offset=Sdk::R<uint32_t>(base+match+3);
    if(slot<base||slot+8>base+bytes.size()||offset<8||offset>0x1000)return false;
    object=Sdk::R<uintptr_t>(slot);const auto table=Sdk::R<uintptr_t>(object);
    unsigned char value=2;
    if(object<base||object+offset>=base+bytes.size()||table<base||table>=base+bytes.size()||
       !Sdk::Read(object+offset,&value,1)||value>1)return false;
    field=object+offset;return true;
}
bool Valid(){return field&&Sdk::R<uintptr_t>(slot)==object;}
void Restore(){
    if(owned&&Valid()&&Sdk::R<unsigned char>(field)==1)Sdk::Write(field,&original,1);
    owned=false;
}
}
bool Apply(bool enabled,float distance){
    if(!enabled){Reset();return true;}
    if(!std::isfinite(distance)||!Resolve()||!Valid())return false;
    distance=std::clamp(distance,30.f,300.f);
    bool ok=ClientConVars::Float("c_maxdistance",true,300.f);
    ok=ClientConVars::Float("cam_idealdist",true,distance)&&ok;
    ok=ClientConVars::Bool("c_thirdpersonshoulder",true,false)&&ok;
    if(!ok){Reset();return false;}
    unsigned char current=2;if(!Sdk::Read(field,&current,1)||current>1){Reset();return false;}
    if(!owned){original=current;owned=true;}
    const unsigned char on=1;return Sdk::Write(field,&on,1);
}
void Reset(){
    Restore();
    ClientConVars::Float("cam_idealdist",false,150.f);
    ClientConVars::Float("c_maxdistance",false,200.f);
    ClientConVars::Bool("c_thirdpersonshoulder",false,false);
}
}
