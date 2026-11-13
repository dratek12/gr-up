#pragma once
#include <cstdint>

// CS2 update surface. Keep game module offsets and entity/schema fields here.
// Values are for the matching client.dll build; verify them after each update.
namespace Off {
    enum : uintptr_t {
        EntityList = 0x271FBB8, LocalPlayer = 0x256A948,
        LocalController = 0x2542548, ViewMatrix = 0x2570250,
        ViewAngles = 0x2581098, Sensitivity = 0x2567C48,
        DefEntityList = EntityList, DefLocalPlayer = LocalPlayer,
        DefLocalController = LocalController, DefViewMatrix = ViewMatrix,
        DefViewAngles = ViewAngles, DefSensitivity = Sensitivity,
        Health = 0x34C, Team = 0x3E7, Scene = 0x330, Dormant = 0x103,
        AbsOrigin = 0xC8, EntityStride = 0x70, EntityIdentity = 0x10,
        EntityHandle = 0x10, EntityClassName = 0x20, EntityChunkStart = 0x10,
        SensitivityValue = 0x58, WeaponServices = 0x12F0,
        MyWeapons = 0x48, ActiveWeapon = 0x60, Controller = 0x14BC,
        PlayerName = 0x6FC, Pawn = 0x92C, ViewOffset = 0xF60,
        AttributeManager = 0x1290, Item = 0x50, ItemDefIndex = 0x1BA,
        ItemIDHigh = 0x1D0, ItemAccountID = 0x1D8,
        FallbackPaintKit = 0x18A8, FallbackSeed = 0x18AC,
        FallbackWear = 0x18B0, FallbackStatTrak = 0x18B4,
        ArmorValue = 0x1ED0, PawnArmor = 0x93C, EyeAngles = 0x3600,
        PunchServices = 0x1598, PunchAngle = 0x50,
        BoneArray = 0x1C0, BoneStride = 0x20, HeadBone = 7,
        LeftKneeBone = 18, RightKneeBone = 21,
        EmitSoundTime = 0x1EA8, ClientSoundTime = 0x1EAC,
        LastShotTime = 0x1B68, InterpolatedInaccuracy = 0x1F58,
        WeaponMode = 0x1A00, WeaponVData = 0x388, WeaponSpread = 0x750,
        ShotsFired = 0x1EB8, Flags = 0x3F4, CrosshairEntity = 0x36DC,
        WeaponDamage = 0x820, HeadshotMultiplier = 0x824,
        ArmorRatio = 0x828, RangeModifier = 0x834, Clip1 = 0x1928,
        ItemServices = 0x12F8, HasHelmet = 0x49,
        // Additional schema fields used outside the core reader.
        ObserverServices = 0x1308, ObserverTarget = 0x4C,
        ObserverMode = 0x48, CameraServices = 0x1328,
        PawnVelocity = 0x3F8, GrenadeVelocity = 0x430,
        Scoped = 0x1EA0, FlashDuration = 0x1504,
        FlashAlpha = 0x1500, FlashEndTime = 0x1530,
        FlashMaxAlpha = 0x14FC, FlashOverlay = 0x1510,
        FlashOverlayEnd = 0x150C,
        SmokeActive = 0x1364, SmokeOrigin = 0x1378,
        BombTicking = 0x1288, BombDefused = 0x12DC,
        BombExploded = 0x12BD, BombOwner = 0x520,
        ControllerObserverPawn = 0x930, ControllerPawnAlive = 0x934,
        ControllerCompetitiveRank = 0x8A0, ControllerCompetitiveWins = 0x8A4,
        ControllerDamageActions = 0x828, DamageDealt = 0x130,
        GrenadeThrowStrength = 0x1F30, CameraRecoil = 0x48,
        GlowComponent = 0xDE8, GlowType = 0x30,
        GlowColor = 0x40, GlowEnabled = 0x50,
        GlowThroughWalls = 0x51,
        MeshSceneOwner = 0xC0, MeshScene = 0x18,
        MeshMaterial = 0x20, MeshColor = 0x50, MeshStride = 0x70,
        TraceHit = 0x08, TraceFraction = 0xAC, TraceSolid = 0xBB,
        JumpNodeName = 0x08, JumpNodeButton = 0x30,
        JumpNodeNext = 0x88, JumpNodeSize = 0x90,
        RendererStrideCheck = 0x210, RendererStrideOpcode = 0x70C38348,
        ThirdPersonSignatureLead = 0x57,
        ConVarNodeStride = 0x10, ConVarNodeNext = 0x0A,
        ConVarAllocationBeforeMemory = 0x06,
    };
}

// AOB patterns are kept beside the offsets so CS2 updates touch one file.
namespace Sig {
    inline constexpr char RegenerateWeaponSkins[] = "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10";
    inline constexpr char TraceShape[] = "48 89 54 24 10 48 89 4C 24 08 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 F0 DB FF FF B8 10 25 00 00";
    inline constexpr char TraceFilter[] = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24";
    inline constexpr char TraceManager[] = "4C 8B 2D ? ? ? ? 24 C9 0C 49";
    inline constexpr char MakeMaterial[] = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 8B F2";
    inline constexpr char ConVarSystem[] = "48 83 EC ? ? 8B ? ? ? ? ? 48 8D 54 ? ? 4C";
    inline constexpr char ConVarList[] = "48 8B ? ? 0F B7 ? 48 03 ? ? 8B ? ? ? 8B 46";
    inline constexpr char ConVarType[] = "E4 74 ? 41 0F B7 ? ?";
    inline constexpr char ConVarValue[] = "AF ? 48 8B CE 8D ? ?";
    inline constexpr int NativeJump[] = {0x48,0x8b,0x15,-1,-1,-1,-1,0x48,0x85,0xd2,0x74,-1,0x48,0x8b,0x02,0x48,0x85,0xc0};
    inline constexpr int ThirdPerson[] = {0x41,0xc6,0x80,-1,-1,-1,-1,1,0xf3,0x41,0x0f,0x11,0x80,-1,-1,-1,-1,0xf3,0x41,0x0f,0x11,0x88,-1,-1,-1,-1,0x41,0xc7,0x80};
    inline constexpr unsigned char RendererPrefix[] = {0x48,0x8B,0xC4,0x48,0x89,0x50,0x10,0x48,0x89,0x48,0x08,0x53,0x56,0x41,0x57,0x48,0x81,0xEC};
    inline constexpr unsigned char PlantedBomb[] = {0x48,0x8B,0x1D,0,0,0,0,0x45,0x32,0xF6};
}
