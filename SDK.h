#pragma once
#include "CommonIncludes.h"
#include "GameLayout.h"

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float a, float b, float c) : x(a), y(b), z(c) {}
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
};

struct CPlayer {
    uintptr_t entity = 0;
    bool  valid = false;
    int   health = 0;
    int   team = 0;
    int   armour = 0;
    Vec3  origin{};
    Vec3  head{};
    bool headValid = false;
    Vec3 chest{}, stomach{}, eye{}, eyeAngles{}, leftLeg{}, rightLeg{};
    bool leftLegValid=false, rightLegValid=false;
    bool chestValid=false, stomachValid=false, eyeAnglesValid=false;
    uint32_t handle = 0xFFFFFFFFu;
    bool soundVisible = false;
    bool lineOfSight = false;
    double soundHeardAt=-1;
    float soundTime = 0.f, clientSoundTime = 0.f, shotTime = 0.f;
    char  name[64]{};
    char  weapon[48]{};
};

namespace Sdk {
    uintptr_t Client();
    int ActiveWeaponId(uintptr_t pawn, uintptr_t list = 0);
    bool Read(uintptr_t addr, void* out, size_t size);
    bool Write(uintptr_t addr,const void* value,size_t size);
    template <class T> T R(uintptr_t addr) { T v{}; Read(addr, &v, sizeof(T)); return v; }

    float RandRange(float a, float b);
    bool W2S(const float m[16], const Vec3& world, ImVec2& out, ImVec2 size);
    bool Foreground();
    void SendMouse(float dx, float dy);
    void SendMouseCounts(int dx,int dy);
    bool Origin(uintptr_t scene, Vec3& out);
    bool ReadStr(uintptr_t addr, char* out, size_t max);
    bool ReadChars(uintptr_t addr, char* out, size_t max);
    uintptr_t EntityList();
    uintptr_t EntAt(uintptr_t list, int index);
    uintptr_t ResolveHandle(uintptr_t list, uint32_t handle);
}
