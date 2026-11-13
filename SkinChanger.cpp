#include "SkinChanger.h"
#include "SkinInventoryBridge.h"
#include "Global.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace {
struct Values {
    uint32_t itemHigh{};
    int paint{}, seed{}, statTrak{};
    float wear{};
};
struct Applied { uint32_t handle{}; int weaponId{}; Values original{}, current{}; };
std::unordered_map<uintptr_t,Applied> applied;

bool ReadValues(uintptr_t weapon, Values& v) {
    const auto item=weapon+Off::AttributeManager+Off::Item;
    return Sdk::Read(item+Off::ItemIDHigh,&v.itemHigh,sizeof(v.itemHigh)) &&
        Sdk::Read(weapon+Off::FallbackPaintKit,&v.paint,sizeof(v.paint)) &&
        Sdk::Read(weapon+Off::FallbackSeed,&v.seed,sizeof(v.seed)) &&
        Sdk::Read(weapon+Off::FallbackWear,&v.wear,sizeof(v.wear)) &&
        Sdk::Read(weapon+Off::FallbackStatTrak,&v.statTrak,sizeof(v.statTrak));
}
bool WriteValues(uintptr_t weapon,const Values& v) {
    const auto item=weapon+Off::AttributeManager+Off::Item;
    return Sdk::Write(item+Off::ItemIDHigh,&v.itemHigh,sizeof(v.itemHigh)) &&
        Sdk::Write(weapon+Off::FallbackPaintKit,&v.paint,sizeof(v.paint)) &&
        Sdk::Write(weapon+Off::FallbackSeed,&v.seed,sizeof(v.seed)) &&
        Sdk::Write(weapon+Off::FallbackWear,&v.wear,sizeof(v.wear)) &&
        Sdk::Write(weapon+Off::FallbackStatTrak,&v.statTrak,sizeof(v.statTrak));
}
uint32_t Handle(uintptr_t weapon) {
    const auto identity=Sdk::R<uintptr_t>(weapon+Off::EntityIdentity);
    return identity>=0x10000?Sdk::R<uint32_t>(identity+Off::EntityHandle):0xFFFFFFFFu;
}
void Restore(uintptr_t weapon,const Applied& value) {
    if(Handle(weapon)==value.handle)WriteValues(weapon,value.original);
}
}

void SkinChanger::Reset() {
    SkinInventoryBridge::Stop();
    for(const auto& [weapon,value]:applied)Restore(weapon,value);
    applied.clear();
}
bool SkinChanger::ApplyWeapon(uintptr_t weapon,uint32_t handle,int weaponId,int paint,int seed,int statTrak,float wear) {
    if(weapon<0x10000||handle==0xFFFFFFFFu||Handle(weapon)!=handle||
       !getrekt::Weapons::Find(weaponId)||paint<1||paint>20000||seed<0||seed>1000||
       statTrak< -1||statTrak>999999||!std::isfinite(wear)||wear<.000001f||wear>1.f)return false;
    const Values desired{0xFFFFFFFFu,paint,seed,statTrak,wear};
    auto it=applied.find(weapon);
    if(it!=applied.end()&&it->second.handle!=handle) {
        applied.erase(it);it=applied.end();
    }
    if(it!=applied.end()&&it->second.current.itemHigh==desired.itemHigh&&
       it->second.current.paint==desired.paint&&it->second.current.seed==desired.seed&&
       it->second.current.statTrak==desired.statTrak&&it->second.current.wear==desired.wear) {
        Values actual{};
        if(!ReadValues(weapon,actual))return false;
        if(actual.itemHigh==desired.itemHigh&&actual.paint==desired.paint&&
           actual.seed==desired.seed&&actual.statTrak==desired.statTrak&&
           actual.wear==desired.wear)return true;
    }
    if(it==applied.end()) {
        Values original{};
        if(!ReadValues(weapon,original))return false;
        it=applied.emplace(weapon,Applied{handle,weaponId,original,{}}).first;
    }
    if(!WriteValues(weapon,desired))return false;
    it->second.current=desired;
    return true;
}
void SkinChanger::Frame() {
    auto& ui=G::Ui.state;
    if(!SkinInventoryBridge::Ready()) {
        ui.skinStatus="Skin inventory unavailable";
        return;
    }
    if(ui.skinOptions.empty()) {
        for(auto& item:SkinInventoryBridge::Catalog()) {
            getrekt::VisualReplica::SkinItem shown{};
            shown.id=item.id;shown.name=std::move(item.name);shown.knife=item.knife;
            for(auto& paint:item.paints)
                shown.paints.push_back({paint.id,std::move(paint.name),paint.wearMin,paint.wearMax});
            ui.skinOptions.push_back(std::move(shown));
        }
    }
    std::unordered_map<int,SkinInventoryBridge::Config> wanted;
    for(const auto& [id,cfg]:ui.skinConfigs)if(cfg.enabled) {
        const auto option=std::find_if(ui.skinOptions.begin(),ui.skinOptions.end(),
            [id](const auto& item){return item.id==id;});
        if(option!=ui.skinOptions.end()&&(cfg.paintKit>0||option->knife))
            wanted.emplace(id,SkinInventoryBridge::Config{cfg.paintKit,cfg.seed,cfg.statTrak,cfg.wear});
    }
    ui.skinStatus=SkinInventoryBridge::Sync(wanted)
        ? "Skin inventory synchronized" : "Skin config rejected; check paint and wear";
}
