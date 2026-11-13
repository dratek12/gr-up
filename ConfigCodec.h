#pragma once
#include <getrekt/GetRektUI.hpp>
#include <tuple>
#include <limits>
#include <type_traits>
#include <iomanip>
#include <istream>
#include <ostream>
namespace ConfigCodec {
using State = getrekt::VisualReplica;
inline auto Fields(getrekt::ExtraOptions& x) {
    return std::tie(x.noFlash,x.noVisualRecoil,x.noScopeZoom,x.viewmodelFovOn,x.playerFovOn,x.night,
        x.viewmodelFov,x.playerFov,x.exposure,x.watermark,x.cornerInfo,x.hitmarker,x.damage,x.tracer,x.tracerOthers,
        x.grenadePrediction,x.hitmarkerSize,x.tracerDuration,x.hitSound,x.autoStrafe,x.edgeJump,x.spectators,x.eventLog,
        x.playerInfo,x.ranks,x.infoTeamCheck,x.edgeKey,x.grenadeHelper,x.locations,x.trajectory,x.helperDistance,x.grenadeType,
        x.hideViewmodel,x.noPostprocessing,x.thirdPerson,x.thirdPersonDistance,x.thirdPersonKey,x.strafeWasd,x.strafeLimit,x.strafeKey,x.soundCircles,x.soundCircleTeam,x.soundCircleDuration,x.soundCircleRadius,x.soundCircleWidth,x.soundCircleColor);
}
inline bool ValidExtra(const getrekt::ExtraOptions& x) {
    return x.viewmodelFov>=54 && x.viewmodelFov<=120 && x.playerFov>=60 && x.playerFov<=140 &&
        x.exposure>=.05f && x.exposure<=1 && x.hitmarkerSize>=1 && x.hitmarkerSize<=20 &&
        x.tracerDuration>=.1f && x.tracerDuration<=5 && x.hitSound>=0 && x.hitSound<=2 &&
        x.edgeKey>=0 && x.edgeKey<256 && x.helperDistance>=100 && x.helperDistance<=5000 &&
        x.grenadeType>=0 && x.grenadeType<4 && x.thirdPersonDistance>=30 && x.thirdPersonDistance<=300 &&
        x.thirdPersonKey>=0 && x.thirdPersonKey<256 && x.strafeLimit>=1 && x.strafeLimit<=255 && x.strafeKey>0 && x.strafeKey<256 &&
        x.soundCircleDuration>=.5f && x.soundCircleDuration<=5 && x.soundCircleRadius>=8 && x.soundCircleRadius<=150 &&
        x.soundCircleWidth>=1 && x.soundCircleWidth<=5 &&
        x.soundCircleColor.x>=0&&x.soundCircleColor.x<=1&&x.soundCircleColor.y>=0&&x.soundCircleColor.y<=1&&
        x.soundCircleColor.z>=0&&x.soundCircleColor.z<=1&&x.soundCircleColor.w>=0&&x.soundCircleColor.w<=1;
}
inline auto Fields(State::AimCfg& x) {
    return std::tie(x.key,x.bone,x.overrideEnabled,x.on,x.always,x.ff,x.silent,x.jump,
        x.fov,x.speed,x.speedShoot,x.lockMs,x.releaseMs,x.reactMs,x.rcs,x.rcsAlways,
        x.rcsXmin,x.rcsXmax,x.rcsYmin,x.rcsYmax,x.trig,x.trigKey,x.trigFf,x.head,x.chest,
        x.stomach,x.arms,x.legs,x.hitChanceOn,x.backtrack,x.delayOn,x.hitChance,x.trigDelay,x.minDamage,x.triggerBind,x.drawFov,x.fovMode,x.aimDeviation,x.timedAssist,x.targetFovSizing,x.aimBones,x.aimStyle,x.snapDelay,x.separateShootSmooth,
        x.showTarget,x.wallCheck,x.flashCheck,x.smokeCheck,x.fovColor,x.targetColor);
}
inline auto Fields(State::Cs2State& x) {
    return std::tie(x.aim,x.rcs,x.ammo,x.bombinfo,x.showinfo,x.team,x.bhop,x.trigger,
        x.autoFire,x.fovCircle,x.watermark,x.fov,x.smooth,x.dist,x.hitchance,x.delay,
        x.randomize,x.recoilMin,x.recoilMax,x.bone,x.aimKey,x.triggerKey);
}
inline auto Fields(State::PreviewState& x) {
    return std::tie(x.boxes,x.names,x.health,x.armour,x.weapon,x.snaplines,x.arrows,x.sound,
        x.head,x.crosshair,x.glow,x.chams,x.color,x.colBox,x.colName,x.colHealth,x.colArmor,x.colHead,x.colSpline,
        x.chamsVisibleOnly,x.chamsMaterial,x.chamsOccluded,
        x.colWeapon,x.colArrows,x.colSound,x.colCrosshair,x.soundDuration,x.arrowRadius,x.spreadMode,
        x.glowVisibleOnly,x.glowMaterial,x.glowVisible,x.glowOccluded,x.glowC4,x.eyeRay,x.colC4,x.colEyeRay,x.eyeRayLength);
}
inline auto Fields(State::SkinCfg& x) {
    return std::tie(x.enabled,x.paintKit,x.seed,x.statTrak,x.wear);
}
inline bool ValidSkins(const std::unordered_map<int,State::SkinCfg>& skins) {
    if(skins.size()>128)return false;
    for(const auto& [id,c]:skins)
        if(id<1||id>65535||c.paintKit<0||c.paintKit>20000||c.seed<0||c.seed>1000||
           c.statTrak< -1||c.statTrak>999999||!std::isfinite(c.wear)||c.wear<.000001f||c.wear>1.f)return false;
    return true;
}
inline auto Fields(getrekt::Config& x) {
    return std::tie(x.accent,x.windowBg,x.panelBg,x.text,x.caption,x.title,x.windowOpacity,x.hoverSeconds);
}
inline auto Fields(ImVec4& x) { return std::tie(x.x,x.y,x.z,x.w); }
template<class T> bool Write(std::ostream& out, const T& value) {
    if constexpr (std::is_same_v<T,std::string>) out << std::quoted(value) << ' ';
    else if constexpr (std::is_arithmetic_v<T>) out << value << ' ';
    else { auto copy = value; return std::apply([&](auto&... v) { return (Write(out,v) && ...); }, Fields(copy)); }
    return bool(out);
}
template<class T> bool Read(std::istream& in, T& value) {
    if constexpr (std::is_same_v<T,std::string>) {
        in >> std::quoted(value); return bool(in) && value.size() <= 4096;
    } else if constexpr (std::is_arithmetic_v<T>) {
        in >> value;
        if constexpr (std::is_floating_point_v<T>) return bool(in) && std::isfinite(value);
        return bool(in);
    } else return std::apply([&](auto&... v) { return (Read(in,v) && ...); }, Fields(value));
}
template<class M> bool WriteMap(std::ostream& out, const M& map) {
    if (!Write(out,map.size())) return false;
    for (const auto& [key,value] : map) if (!Write(out,key) || !Write(out,value)) return false;
    return true;
}
template<class M> bool ReadMap(std::istream& in, M& map) {
    size_t count{}; if (!Read(in,count) || count > 4096) return false;
    map.clear();
    for(size_t i=0;i<count;++i) {
        typename M::key_type key{}; typename M::mapped_type value{};
        if (!Read(in,key) || !Read(in,value)) return false;
        if (!map.emplace(std::move(key),std::move(value)).second) return false;
    }
    return true;
}
inline bool ValidGroups(const State::Cs2State& state) {
    if(state.groups.size()>64 || state.nextGroupId<0 || state.nextGroupId>1000000)return false;
    std::vector<int> assigned;std::vector<std::string> names;
    for(const auto& [id,g]:state.groups) {
        if(id<0 || id>=state.nextGroupId || g.name.empty() || g.name.size()>64 || g.name.find_first_not_of(" \t\r\n")==std::string::npos ||
            std::find(names.begin(),names.end(),g.name)!=names.end() || g.weapons.size()>getrekt::Weapons::All.size())return false;
        names.push_back(g.name);
        for(int weapon:g.weapons) {
            if(!getrekt::Weapons::Find(weapon) || std::find(assigned.begin(),assigned.end(),weapon)!=assigned.end())return false;
            assigned.push_back(weapon);
        }
    }
    return true;
}
inline bool WriteGroups(std::ostream& out,const State::Cs2State& state) {
    if(!ValidGroups(state) || !Write(out,state.groups.size()) || !Write(out,state.nextGroupId))return false;
    for(const auto& [id,g]:state.groups) {
        if(!Write(out,id) || !Write(out,g.name) || !Write(out,g.config) || !Write(out,g.weapons.size()))return false;
        for(int weapon:g.weapons)if(!Write(out,weapon))return false;
    }
    return true;
}
inline bool ReadAim(std::istream& in, State::AimCfg& cfg, int version);
inline bool ReadGroups(std::istream& in,State::Cs2State& state,int version) {
    size_t count=0;if(!Read(in,count) || count>64 || !Read(in,state.nextGroupId))return false;
    for(size_t i=0;i<count;++i) {
        int id=0;State::WeaponGroup g;size_t members=0;
        if(!Read(in,id) || !Read(in,g.name) || !ReadAim(in,g.config,version) || !Read(in,members) || members>getrekt::Weapons::All.size())return false;
        for(size_t j=0;j<members;++j){int weapon=0;if(!Read(in,weapon))return false;g.weapons.push_back(weapon);}
        if(!state.groups.emplace(id,std::move(g)).second)return false;
    }
    return ValidGroups(state);
}
inline bool WriteProfile(std::ostream& out, const State::ProfileState& s) {
    if(s.legitMode<0||s.legitMode>1)return false;
    auto valid=[](const State::AimCfg& c){return c.fovMode>=0&&c.fovMode<=2&&std::isfinite(c.aimDeviation)&&c.aimDeviation>=0&&c.aimDeviation<=4&&c.targetFovSizing>=0&&c.targetFovSizing<=1&&c.aimBones>=0&&c.aimBones<=15&&c.aimStyle>=0&&c.aimStyle<=2&&c.snapDelay>=1&&c.snapDelay<=500&&c.speed>=0&&c.speed<=30&&c.speedShoot>=0&&c.speedShoot<=30;};
    for(const auto& c:s.cs2.basic)if(!valid(c))return false;
    for(const auto& [id,c]:s.cs2.adv)if(!valid(c))return false;
    for(const auto& [id,g]:s.cs2.groups)if(!valid(g.config))return false;
    if (!Write(out,s.cs2)) return false;
    for (const auto& cfg : s.cs2.basic) if (!Write(out,cfg)) return false;
    return WriteMap(out,s.cs2.adv) && Write(out,s.preview) && Write(out,s.theme)
        && WriteMap(out,s.checks) && WriteMap(out,s.numbers) && WriteMap(out,s.choices) && WriteMap(out,s.keybinds) && WriteGroups(out,s.cs2) && ValidExtra(s.extra) && Write(out,s.extra) && Write(out,s.legitMode)
        && ValidSkins(s.skinConfigs) && WriteMap(out,s.skinConfigs);
}
inline bool ReadAim(std::istream& in, State::AimCfg& cfg, int version) {
    auto fields = Fields(cfg);
    auto prefix=[&]<size_t... I>(std::index_sequence<I...>){return (Read(in,std::get<I>(fields))&&...);};
    const bool ok=version>=17?Read(in,cfg):version==16?prefix(std::make_index_sequence<44>{}):version==15?prefix(std::make_index_sequence<41>{}):version==14?prefix(std::make_index_sequence<40>{}):version==13?prefix(std::make_index_sequence<39>{}):version==12?prefix(std::make_index_sequence<38>{}):version==11?prefix(std::make_index_sequence<37>{}):version>=3?prefix(std::make_index_sequence<35>{}):prefix(std::make_index_sequence<34>{});
    if(ok&&version<15)cfg.aimBones=cfg.bone==0?7:cfg.bone>=1&&cfg.bone<=3?1<<(cfg.bone-1):0;
    if(ok&&version<16)cfg.separateShootSmooth=true;
    if(ok && version < 17) {
        cfg.fovColor=cfg.targetColor=ImVec4(65.f/255,235.f/255,110.f/255,1);
        if(cfg.fovMode==1){cfg.showTarget=cfg.drawFov;cfg.drawFov=false;}
    }
    return ok&&cfg.fovMode>=0&&cfg.fovMode<=2&&std::isfinite(cfg.aimDeviation)&&cfg.aimDeviation>=0&&cfg.aimDeviation<=4&&cfg.targetFovSizing>=0&&cfg.targetFovSizing<=1&&cfg.aimBones>=0&&cfg.aimBones<=15&&cfg.aimStyle>=0&&cfg.aimStyle<=2&&cfg.snapDelay>=1&&cfg.snapDelay<=500&&cfg.speed>=0&&cfg.speed<=30&&cfg.speedShoot>=0&&cfg.speedShoot<=30;
}
inline bool ReadProfile(std::istream& in, State::ProfileState& s, int version) {
    if (!Read(in,s.cs2)) return false;
    for (auto& cfg : s.cs2.basic) if (!ReadAim(in,cfg,version)) return false;
    size_t count{}; if (!Read(in,count) || count > 4096) return false;
    for (size_t i=0; i<count; ++i) {
        int id{}; State::AimCfg cfg;
        if (!Read(in,id) || !ReadAim(in,cfg,version) || !s.cs2.adv.emplace(id,cfg).second) return false;
    }
    bool previewOk = false;
    if (version >= 8) previewOk = Read(in,s.preview);
    else {
        auto fields = Fields(s.preview);
        auto prefix = [&]<size_t... I>(std::index_sequence<I...>) { return (Read(in,std::get<I>(fields)) && ...); };
        if (version >= 6) previewOk = prefix(std::make_index_sequence<33>{});
        else if (version == 5) previewOk = prefix(std::make_index_sequence<29>{});
        else if (version == 4) previewOk = prefix(std::make_index_sequence<22>{});
        else previewOk = prefix(std::make_index_sequence<19>{});
        if (s.preview.colHead.x == .35f && s.preview.colHead.y == .69f && s.preview.colHead.z == 1.f)
            s.preview.colHead = {.2f,1.f,.35f,1.f};
    }
    if (!previewOk || !Read(in,s.theme) || !ReadMap(in,s.checks) || !ReadMap(in,s.numbers)
        || !ReadMap(in,s.choices) || !ReadMap(in,s.keybinds)) return false;
    if(version>=7 && !ReadGroups(in,s.cs2,version))return false;
    if(version>=9) {
        auto fields=Fields(s.extra);
        auto legacy=[&]<size_t... I>(std::index_sequence<I...>){return (Read(in,std::get<I>(fields))&&...);};
        if(!(version>=16?Read(in,s.extra):version>=10?legacy(std::make_index_sequence<37>{}):legacy(std::make_index_sequence<34>{}))||!ValidExtra(s.extra))return false;
    }
    if(version>=12&&(!Read(in,s.legitMode)||s.legitMode<0||s.legitMode>1))return false;
    if(version>=18&&(!ReadMap(in,s.skinConfigs)||!ValidSkins(s.skinConfigs)))return false;
    // Older files stored these controls only as UI hashes, not usable feature settings.
    // Their new typed fields retain the defaults when loading v2/v3.
    return s.preview.chamsMaterial >= 0 && s.preview.chamsMaterial <= 2
        && s.preview.glowMaterial >= 0 && s.preview.glowMaterial <= 2
        && s.preview.spreadMode >= 0 && s.preview.spreadMode <= 2
        && s.preview.soundDuration >= .5f && s.preview.soundDuration <= 15.f
        && s.preview.eyeRayLength >= 100.f && s.preview.eyeRayLength <= 8192.f
        && s.preview.arrowRadius >= 40.f && s.preview.arrowRadius <= 400.f;
}
inline bool Save(std::ostream& out, const State& state) {
    if (state.savedProfiles.size() > State::MaxProfiles) return false;
    out << "CS2ESP_CONFIG 18\n" << std::setprecision(std::numeric_limits<float>::max_digits10);
    if (!WriteProfile(out,state.CaptureProfile()) || !Write(out,state.savedProfiles.size())) return false;
    for (const auto& [name,profile] : state.savedProfiles)
        if (!Write(out,name) || !WriteProfile(out,profile)) return false;
    if (!Write(out,state.profiles.size())) return false;
    for (const auto& name : state.profiles) if (!Write(out,name)) return false;
    return Write(out,state.profile);
}
inline bool Load(std::istream& in, State& state) {
    std::string magic; int version{};
    if (!(in >> magic >> version) || magic != "CS2ESP_CONFIG" || (version < 2 || version > 18)) return false;
    State::ProfileState current;
    if (!ReadProfile(in,current,version)) return false;
    size_t count{}; if (!Read(in,count) || count > State::MaxProfiles) return false;
    decltype(state.savedProfiles) profiles;
    for(size_t i=0;i<count;++i) {
        std::string name; State::ProfileState profile;
        if (!Read(in,name) || !ReadProfile(in,profile,version) || !profiles.emplace(name,std::move(profile)).second) return false;
    }
    auto names = state.profiles;
    int selected = state.profile;
    if (version >= 3) {
        size_t size{}; if (!Read(in,size) || size == 0 || size > 4096) return false;
        names.clear();
        for (size_t i=0; i<size; ++i) {
            std::string name;
            if (!Read(in,name) || std::find(names.begin(),names.end(),name)!=names.end()) return false;
            names.push_back(std::move(name));
        }
        if (!Read(in,selected)) return false;
    }
    // Commit only after the complete file has been parsed.
    state.RestoreProfile(current); state.savedProfiles = std::move(profiles);
    state.profiles = std::move(names);
    state.profile = std::clamp(selected,0,static_cast<int>(state.profiles.size())-1);
    for(const auto& [name,profile] : state.savedProfiles)
        if(std::find(state.profiles.begin(),state.profiles.end(),name)==state.profiles.end()) state.profiles.push_back(name);
    return true;
}
}
