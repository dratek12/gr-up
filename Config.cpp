#include "Global.h"
#include "Config.h"
#include "ConfigCodec.h"
#include <filesystem>
#include <fstream>
static std::filesystem::path ConfigPath() {
    wchar_t path[32768]{};
    DWORD count = GetModuleFileNameW(G::SelfModule, path, static_cast<DWORD>(std::size(path)));
    if (!count || count >= std::size(path)) return {};
    return std::filesystem::path(path).parent_path() / L"cs2esp.ini";
}
void SaveConfig() {
    auto path = ConfigPath(); if (path.empty()) return;
    auto temporary = path; temporary += L".tmp";
    std::ofstream stream(temporary, std::ios::trunc);
    bool ok = ConfigCodec::Save(stream, G::Ui.state);
    stream.close(); ok = ok && !stream.fail();
    if (ok) ok = MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
    G::Log(ok ? "Config saved" : "Config save failed");
}
void LoadConfig() {
    auto path = ConfigPath(); if (path.empty()) return;
    std::ifstream stream(path); if (!stream) return;
    G::Log(ConfigCodec::Load(stream, G::Ui.state) ? "Config loaded" : "Invalid config; settings unchanged");
}
