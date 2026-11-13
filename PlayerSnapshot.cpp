#include "Global.h"
#include "PlayerSnapshot.h"
#include "SoundReveal.h"
#include "Backtrack.h"
#include "AimChecks.h"

namespace {
SoundReveal sounds;
uintptr_t previousList = 0, previousLocal = 0;
bool previousSoundEnabled = false;
bool Bone(uintptr_t scene, const Vec3& origin, int index, Vec3& result, bool leg=false) {
    const auto bones = Sdk::R<uintptr_t>(scene + Off::BoneArray);
    Vec3 point{};
    if (!Sdk::Read(bones + index * Off::BoneStride, &point, sizeof(point)) ||
        !std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) return false;
    const auto delta = point - origin;
    if ((point.x==0 && point.y==0 && point.z==0) || delta.z < (leg?-5.f:10.f) || delta.z > (leg?60.f:100.f) || std::hypot(delta.x, delta.y) > 80.f) return false;
    result = point;
    return true;
}
uintptr_t FromField(uintptr_t list, uintptr_t entity, uintptr_t offset) {
    uint32_t handle = 0xFFFFFFFFu;
    return entity >= 0x10000 && Sdk::Read(entity + offset, &handle, sizeof(handle))
        ? Sdk::ResolveHandle(list, handle) : 0;
}
bool Alive(uintptr_t pawn) {
    if (pawn < 0x10000) return false;
    int health = Sdk::R<int>(pawn + Off::Health);
    if (health <= 0 || health > 1000) return false;
    auto scene = Sdk::R<uintptr_t>(pawn + Off::Scene);
    uint8_t dormant = 1;
    return scene >= 0x10000 && Sdk::Read(scene + Off::Dormant, &dormant, sizeof(dormant)) && !dormant;
}
bool Eye(uintptr_t pawn, const Vec3& origin, Vec3& result) {
    Vec3 offset{};
    if (!Sdk::Read(pawn + Off::ViewOffset, &offset, sizeof(offset)) ||
        !std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(offset.z) ||
        std::fabs(offset.x) > 32.f || std::fabs(offset.y) > 32.f || offset.z < 10.f || offset.z > 80.f) return false;
    result = origin + offset;
    return true;
}
}

void UpdatePlayers(uintptr_t list, uintptr_t local, uintptr_t localController) {
    const bool soundEnabled = G::Ui.state.preview.sound || G::Ui.state.extra.soundCircles;
    if (list != previousList || local != previousLocal || soundEnabled != previousSoundEnabled) sounds.Clear();
    if (list != previousList || local != previousLocal) BtReset();
    previousList = list; previousLocal = local; previousSoundEnabled = soundEnabled;
    const double now = GetTickCount64() / 1000.;
    sounds.Prune(now);
    G::Players.clear();
    G::Players.reserve(64);
    std::fill(std::begin(G::DbgCnt), std::end(G::DbgCnt), 0);
    G::DbgSlots = 0;
    G::DbgList = list;
    G::DbgChunk = list >= 0x10000 ? Sdk::R<uintptr_t>(list + Off::EntityChunkStart) : 0;
    G::LocalAlive = false;
    G::LocalTeam = 0;
    G::LocalOrigin = G::LocalEye = G::Punch = {};
    auto controlledPawn = FromField(list, localController, Off::Pawn);
    if (Alive(local) && Sdk::Origin(Sdk::R<uintptr_t>(local + Off::Scene), G::LocalOrigin)) {
        G::LocalTeam = Sdk::R<uint8_t>(local + Off::Team);
        G::LocalAlive = (G::LocalTeam == 2 || G::LocalTeam == 3) && Eye(local, G::LocalOrigin, G::LocalEye);
        auto services = Sdk::R<uintptr_t>(local + Off::PunchServices);
        Vec3 punch{};
        if (services >= 0x10000 && Sdk::Read(services + Off::PunchAngle, &punch, sizeof(punch)) &&
            std::isfinite(punch.x) && std::isfinite(punch.y) && std::fabs(punch.x) <= 90.f && std::fabs(punch.y) <= 90.f)
            G::Punch = punch;
    }
    for (int i = 1; i <= 64; ++i) {
        auto controller = Sdk::EntAt(list, i);
        if (!controller) continue;
        ++G::DbgCnt[0];
        auto pawn = FromField(list, controller, Off::Pawn);
        if (!pawn || FromField(list, pawn, Off::Controller) != controller || !Alive(pawn)) continue;
        ++G::DbgCnt[1];
        Vec3 origin{}, eye{};
        if (!Sdk::Origin(Sdk::R<uintptr_t>(pawn + Off::Scene), origin) || !Eye(pawn, origin, eye)) continue;
        ++G::DbgCnt[2];
        int team = Sdk::R<uint8_t>(pawn + Off::Team);
        if (team != 2 && team != 3) continue;
        ++G::DbgCnt[3];
        if (pawn == local || pawn == controlledPawn) continue;
        ++G::DbgCnt[4];
        auto& player = G::Players.emplace_back();
        player.entity = pawn;
        player.valid = true;
        player.health = Sdk::R<int>(pawn + Off::Health);
        player.team = team;
        player.armour = Sdk::R<int>(pawn + Off::ArmorValue);
        player.origin = origin;
        player.head = eye;
        const auto model=Sdk::R<uintptr_t>(pawn + Off::Scene);
        player.headValid = Bone(model, origin, Off::HeadBone, player.head);
        player.chestValid = Bone(model, origin, 5, player.chest);
        player.stomachValid = Bone(model, origin, 3, player.stomach);
        player.leftLegValid = Bone(model, origin, Off::LeftKneeBone, player.leftLeg, true);
        player.rightLegValid = Bone(model, origin, Off::RightKneeBone, player.rightLeg, true);
        player.eye=eye;
        player.eyeAnglesValid=Sdk::Read(pawn+Off::EyeAngles,&player.eyeAngles,sizeof(Vec3)) &&
            std::isfinite(player.eyeAngles.x) && std::isfinite(player.eyeAngles.y) &&
            std::fabs(player.eyeAngles.x)<=90 && std::fabs(player.eyeAngles.y)<=360;
        auto identity = Sdk::R<uintptr_t>(pawn + Off::EntityIdentity);
        if (identity >= 0x10000) Sdk::Read(identity + Off::EntityHandle, &player.handle, sizeof(player.handle));
        if (soundEnabled) {
            player.soundTime = Sdk::R<float>(pawn + Off::EmitSoundTime);
            player.clientSoundTime = Sdk::R<float>(pawn + Off::ClientSoundTime);
            const auto weaponServices = Sdk::R<uintptr_t>(pawn + Off::WeaponServices);
            const auto weaponHandle = weaponServices ? Sdk::R<uint32_t>(weaponServices + Off::ActiveWeapon) : 0xFFFFFFFFu;
            auto weaponEntity = Sdk::ResolveHandle(list, weaponHandle);
            player.shotTime = weaponEntity ? Sdk::R<float>(weaponEntity + Off::LastShotTime) : 0.f;
            player.soundVisible = sounds.Update(player.handle, player.soundTime, player.clientSoundTime,
                player.shotTime, now, G::Ui.state.preview.soundDuration, weaponHandle);
            player.soundHeardAt=sounds.HeardAt(player.handle);
        }
        if (G::Ui.state.preview.sound && G::LocalAlive) {
            player.lineOfSight = (player.headValid && AimChecks::Visible(local,pawn,G::LocalEye,player.head)) ||
                (player.chestValid && AimChecks::Visible(local,pawn,G::LocalEye,player.chest)) ||
                (player.stomachValid && AimChecks::Visible(local,pawn,G::LocalEye,player.stomach)) ||
                AimChecks::Visible(local,pawn,G::LocalEye,player.eye);
        }
        Sdk::ReadChars(controller + Off::PlayerName, player.name, sizeof(player.name));
        const char* weapon = getrekt::Weapons::Name(Sdk::ActiveWeaponId(pawn, list));
        if (*weapon) strncpy_s(player.weapon, weapon, _TRUNCATE);
        ++G::DbgCnt[5];
    }
    G::DbgSlots = static_cast<int>(G::Players.size());
    BtRecord();
}

void UpdatePlayers() {
    auto client = Sdk::Client();
    UpdatePlayers(Sdk::EntityList(), Sdk::R<uintptr_t>(client + Off::LocalPlayer),
        Sdk::R<uintptr_t>(client + Off::LocalController));
    auto sensitivity = Sdk::R<uintptr_t>(client + Off::Sensitivity);
    float value = sensitivity >= 0x10000 ? Sdk::R<float>(sensitivity + Off::SensitivityValue) : 0.f;
    G::Sens = std::isfinite(value) && value > .001f && value <= 100.f ? value : 0.f;
}
