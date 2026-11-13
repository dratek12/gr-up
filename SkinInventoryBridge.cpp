#include "SkinInventoryBridge.h"
#include "Global.h"
#include "GameLayout.h"
#include "SkinPaintNames.h"
#include "OpenChangerCore/src/features/changer/Changer.h"
#include "OpenChangerCore/src/features/changer/catalog/catalog.h"
#include "OpenChangerCore/src/features/changer/loadout/loadout.h"
#include "OpenChangerCore/src/features/changer/skins/decoded.h"
#include "OpenChangerCore/src/sdk/engine2/CNetworkClientService.h"
#include "OpenChangerCore/src/sdk/engine2/CNetworkGameClient.h"
#include "OpenChangerCore/src/game/hook.h"
#include "OpenChangerCore/src/game/link.h"
#include "OpenChangerCore/src/game/pattern.h"
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <ranges>
#include <string>
#include <utility>

namespace SkinInventoryBridge {
namespace {
using MainCallback=void(*)(void*);
using RunOnMainThread=bool(*)(MainCallback,void*,bool);
std::atomic<bool> ready{false};
std::unordered_map<int,Config> applied;
RunOnMainThread mainThread{};
using RegenerateWeaponSkins=void(*)();
RegenerateWeaponSkins regenerate{};
std::chrono::steady_clock::time_point refreshAt{};
bool refreshPending=false;
bool materialized=false;

void AtRegenerate(void*) {
    if(ready.load()&&regenerate) {
        const auto refreshed=oc::changer::refreshLocalWeapons(!materialized);
        regenerate();
        G::Log("Weapon materials refreshed for %d local weapons",refreshed.Count);
        if(refreshed.MaterializationNeeded) {
            if(auto* service=CNetworkClientService::get()) {
                if(auto* client=service->m_pNetworkGameClient()) {
                    CNetworkGameClient::ForceFullUpdate(client,"skin first materialization");
                    G::Log("Requested appearance update for own weapons");
                }
            }
        }
        if(refreshed.MaterializationNeeded)materialized=true;
    }
}

void AtStart(void* result) {
    auto& ok=*static_cast<bool*>(result);
    if(oc::resolveLinks()!=0)return;
    auto& changer=oc::changer::Changer::get();
    if(!changer.start())return;
    if(!oc::installHooks()) {
        oc::removeHooks();
        changer.stop();
        return;
    }
    // Refresh only weapon materials when the selected finish changes. The
    // game's full network update also rebuilds the HUD and flashes the view.
    if(const auto address=oc::findPattern(oc::GameModule::Client,Sig::RegenerateWeaponSkins))
        regenerate=reinterpret_cast<RegenerateWeaponSkins>(*address);
    ok=true;
}

void AtStop(void*) {
    auto& changer=oc::changer::Changer::get();
    changer.stop();
    oc::removeHooks();
}

void Remove(int id) {
    const auto* item=oc::changer::Changer::get().catalog().item(static_cast<std::uint16_t>(id));
    if(!item)return;
    const auto& ids=oc::changer::Changer::get().catalog().Ids;
    for(const auto team:{ids.Terrorists,ids.CounterTerrorists}) {
        oc::changer::ItemRecord old{};
        old.Team=team;old.Slot=item->Slot;old.Definition=static_cast<std::uint16_t>(id);
        oc::changer::Changer::get().unequip(old);
    }
}

bool Put(int id,const Config& cfg,std::string& reason) {
    auto& changer=oc::changer::Changer::get();
    const auto& catalog=changer.catalog();
    const auto* item=catalog.item(static_cast<std::uint16_t>(id));
    if(!item||(cfg.paint!=0&&!catalog.paint(static_cast<std::uint32_t>(cfg.paint)))) {
        reason="Unknown weapon or paint kit";return false;
    }
    bool accepted=false;
    for(const auto team:{catalog.Ids.Terrorists,catalog.Ids.CounterTerrorists}) {
        oc::changer::ItemRecord record{};
        record.Team=team;record.Slot=item->Slot;
        record.Definition=static_cast<std::uint16_t>(id);
        record.Paint=static_cast<std::uint32_t>(cfg.paint);
        record.Seed=static_cast<std::uint32_t>(cfg.seed);
        record.Wear=cfg.wear;
        record.StatTrak=cfg.statTrak>=0;
        record.StatTrakCount=record.StatTrak?static_cast<std::uint32_t>(cfg.statTrak):0;
        auto outcome=changer.equip(std::move(record));
        if(outcome)accepted=true;
        else reason=outcome.error();
    }
    return accepted;
}

std::string DisplayItemName(const oc::changer::CatalogItem& item) {
    if(!getrekt::Weapons::Find(item.Definition)) {
        std::string name=item.Name;
        if(name.starts_with("weapon_"))name.erase(0,7);
        if(name.starts_with("knife_"))name.erase(0,6);
        if(name=="knife")return "Default Knife";
        bool beginning=true;
        for(char& c:name) {
            if(c=='_') {c=' ';beginning=true;}
            else if(beginning) {c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));beginning=false;}
        }
        return name;
    }
    return getrekt::Weapons::Name(item.Definition);
}
}

bool Start() {
    if(ready.load())return true;
    auto sdl=GetModuleHandleW(L"SDL3.dll");
    if(!sdl)return false;
    mainThread=reinterpret_cast<RunOnMainThread>(GetProcAddress(sdl,"SDL_RunOnMainThread"));
    if(!mainThread)return false;
    bool ok=false;
    if(!mainThread(&AtStart,&ok,true)||!ok)return false;
    ready=true;
    G::Log("Skin inventory changer started");
    return true;
}

void Stop() {
    if(!ready.exchange(false))return;
    refreshPending=false;
    materialized=false;
    applied.clear();
    if(mainThread)mainThread(&AtStop,nullptr,true);
    G::Log("Skin inventory changer stopped");
}

bool Ready() {return ready.load();}

bool Sync(const std::unordered_map<int,Config>& desired) {
    if(!Ready())return false;
    bool changed=false;
    for(const auto& [id,previous]:applied)if(!desired.contains(id)){Remove(id);changed=true;}
    std::unordered_map<int,Config> next;
    bool all=true;
    for(const auto& [id,cfg]:desired) {
        auto it=applied.find(id);
        if(it!=applied.end()&&it->second==cfg) {next.emplace(id,cfg);continue;}
        std::string reason;
        if(Put(id,cfg,reason)){next.emplace(id,cfg);changed=true;}
        else {all=false;G::Log("Skin %d rejected: %s",id,reason.c_str());}
    }
    applied=std::move(next);
    if(changed) {
        refreshPending=true;
        refreshAt=std::chrono::steady_clock::now()+std::chrono::milliseconds(150);
        G::Log("Skin inventory changed; weapon refresh queued");
    } else if(refreshPending&&std::chrono::steady_clock::now()>=refreshAt) {
        refreshPending=false;
        if(mainThread&&regenerate&&mainThread(&AtRegenerate,nullptr,false))
            G::Log("Weapon material refresh scheduled");
        else G::Log("Weapon material refresh unavailable");
    }
    return all;
}

std::vector<ItemOption> Catalog() {
    std::vector<ItemOption> result;
    if(!Ready())return result;
    const auto& catalog=oc::changer::Changer::get().catalog();
    for(const auto& item:catalog.Items) {
        if(item.Kind!=oc::changer::ItemKind::Weapon&&item.Kind!=oc::changer::ItemKind::Knife)continue;
        ItemOption option{item.Definition,DisplayItemName(item),item.Kind==oc::changer::ItemKind::Knife,{}};
        if(!option.knife&&!getrekt::Weapons::Find(item.Definition))continue;
        for(const auto& paint:catalog.Paints) {
            if(std::ranges::contains(paint.Items,item.Definition,&oc::changer::PaintedItem::Definition))
                option.paints.push_back({static_cast<int>(paint.Id),
                    SkinPaintName(static_cast<int>(paint.Id)).empty() ? paint.Name : std::string(SkinPaintName(static_cast<int>(paint.Id))),
                    paint.WearMin,paint.WearMax});
        }
        std::ranges::sort(option.paints,{},&PaintOption::name);
        result.push_back(std::move(option));
    }
    std::ranges::sort(result,[](const ItemOption& a,const ItemOption& b){
        if(a.knife!=b.knife)return !a.knife;
        return a.name<b.name;
    });
    return result;
}
}
