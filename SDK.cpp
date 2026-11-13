#include "Global.h"

#include <cmath>
#include <cstring>

static uintptr_t g_client = 0;

uintptr_t Sdk::Client() {
    if (!g_client) g_client = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    return g_client;
}

bool Sdk::Read(uintptr_t addr, void* out, size_t size) {
    if (addr < 0x10000 || !out || !size) return false;
    __try {
        memcpy(out, reinterpret_cast<void*>(addr), size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Sdk::Write(uintptr_t addr,const void* value,size_t size) {
    if(addr<0x10000 || !value || !size)return false;
    __try { memcpy(reinterpret_cast<void*>(addr),value,size);return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) {return false;}
}

bool Sdk::Foreground() {
    return G::Window && GetForegroundWindow() == G::Window;
}

void Sdk::SendMouseCounts(int dx,int dy) {
    if(!dx&&!dy)return;
    INPUT in{};in.type=INPUT_MOUSE;in.mi.dx=dx;in.mi.dy=dy;in.mi.dwFlags=MOUSEEVENTF_MOVE;
    SendInput(1,&in,sizeof(in));
}
void Sdk::SendMouse(float dx, float dy) {
    if (!std::isfinite(dx) || !std::isfinite(dy)) return;
    static float residualX = 0, residualY = 0;
    residualX += std::clamp(dx, -10000.f, 10000.f);
    residualY += std::clamp(dy, -10000.f, 10000.f);
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dx = static_cast<LONG>(residualX);
    in.mi.dy = static_cast<LONG>(residualY);
    if (!in.mi.dx && !in.mi.dy) return;
    residualX -= in.mi.dx;
    residualY -= in.mi.dy;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &in, sizeof(in));
}

bool Sdk::Origin(uintptr_t scene, Vec3& out) {
    if (scene < 0x10000) return false;
    Vec3 v{};
    if (!Read(scene + Off::AbsOrigin, &v, sizeof(v)) ||
        !std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z)) return false;
    out = v; // (0,0,0) is a valid world position too.
    return true;
}

bool Sdk::ReadChars(uintptr_t addr, char* out, size_t max) {
    if (!out || max == 0) return false;
    out[0] = 0;
    if (max < 2) return false;
    size_t n = 0;
    while (n < max - 1) {
        unsigned char bytes[4]{};
        if (!Read(addr + n, bytes, 1)) { out[0] = 0; return false; }
        if (!bytes[0]) break;
        unsigned count = bytes[0] < 0x80 ? 1 : bytes[0] >= 0xC2 && bytes[0] <= 0xDF ? 2
            : bytes[0] >= 0xE0 && bytes[0] <= 0xEF ? 3 : bytes[0] >= 0xF0 && bytes[0] <= 0xF4 ? 4 : 0;
        if (!count || bytes[0] < 32 || bytes[0] == 127) { out[0] = 0; return false; }
        if (n + count >= max) break; // never leave half a UTF-8 codepoint
        for (unsigned i=1; i<count; ++i)
            if (!Read(addr+n+i, &bytes[i], 1) || (bytes[i]&0xC0)!=0x80) { out[0]=0; return false; }
        if ((bytes[0]==0xE0 && bytes[1]<0xA0) || (bytes[0]==0xED && bytes[1]>=0xA0)
            || (bytes[0]==0xF0 && bytes[1]<0x90) || (bytes[0]==0xF4 && bytes[1]>=0x90)) { out[0]=0; return false; }
        memcpy(out+n, bytes, count); n+=count; out[n]=0;
    }
    return n > 0;
}

bool Sdk::ReadStr(uintptr_t addr, char* out, size_t max) {
    uintptr_t ptr = R<uintptr_t>(addr);
    if (ptr < 0x10000) return false;
    return ReadChars(ptr, out, max);
}

uintptr_t Sdk::EntityList() {
    auto client = Client();
    return client ? R<uintptr_t>(client + Off::EntityList) : 0;
}

uintptr_t Sdk::EntAt(uintptr_t list, int index) {
    if (list < 0x10000 || index < 0 || index >= 0x7FFF) return 0;
    auto chunk = R<uintptr_t>(list + Off::EntityChunkStart + sizeof(uintptr_t) * (index >> 9));
    if (chunk < 0x10000) return 0;
    uintptr_t e = R<uintptr_t>(chunk + static_cast<uintptr_t>(index & 0x1FF) * Off::EntityStride);
    return e >= 0x10000 ? e : 0;
}

uintptr_t Sdk::ResolveHandle(uintptr_t list, uint32_t handle) {
    if (handle == 0xFFFFFFFFu) return 0;
    auto entity = EntAt(list, static_cast<int>(handle & 0x7FFF));
    if (!entity) return 0;
    auto identity = R<uintptr_t>(entity + Off::EntityIdentity);
    uint32_t actual{};
    // Check the serial too: an old handle must not select a recycled slot.
    if (identity < 0x10000 || !Read(identity + Off::EntityHandle, &actual, sizeof(actual)) || actual != handle) return 0;
    return entity;
}
float Sdk::RandRange(float a, float b) {
    if (!std::isfinite(a) || !std::isfinite(b)) return 0.f;
    if (a > b) std::swap(a, b);
    static thread_local std::mt19937 engine{std::random_device{}()};
    return std::uniform_real_distribution<float>(a, b)(engine);
}
bool Sdk::W2S(const float m[16], const Vec3& w, ImVec2& out, ImVec2 size) {
    float x = m[0]*w.x + m[1]*w.y + m[2]*w.z + m[3];
    float y = m[4]*w.x + m[5]*w.y + m[6]*w.z + m[7];
    float z = m[12]*w.x + m[13]*w.y + m[14]*w.z + m[15];
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || z < .01f) return false;
    out = ImVec2(size.x*.5f*(1+x/z), size.y*.5f*(1-y/z));
    return std::isfinite(out.x) && std::isfinite(out.y);
}

int Sdk::ActiveWeaponId(uintptr_t pawn, uintptr_t list) {
    if (pawn < 0x10000) return 0;
    uintptr_t ws = Sdk::R<uintptr_t>(pawn + Off::WeaponServices);
    if (ws < 0x10000) return 0;
    uint32_t h = 0xFFFFFFFFu;
    if (!Read(ws + Off::ActiveWeapon, &h, sizeof(h))) return 0;
    uintptr_t wp = ResolveHandle(list ? list : EntityList(), h);
    if (wp < 0x10000) return 0;
    return static_cast<int>(Sdk::R<uint16_t>(wp + Off::AttributeManager + Off::Item + Off::ItemDefIndex));
}

