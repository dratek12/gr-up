#pragma once
// Menu state; runtime features consume the typed settings below.
#include "GetRektTheme.h"
#include "../WeaponCatalog.hpp"
#include "../ExtraOptions.hpp"
#include <cstring>
#include <array>
#include <unordered_map>
#include <string>
#include <vector>

namespace getrekt {
    struct VisualReplica {
        int page = 1, legit = 0, legitLeft = 0, legitRight = 0, visuals = 0, esp = 0, misc = 0, skin = 0;
        int weapon = 0, selectedKnifeId = 0, skinChoice = 3, profile = 0;
        struct SkinCfg {
            bool enabled = false;
            int paintKit = 0, seed = 0, statTrak = -1;
            float wear = .0001f;
        };
        struct SkinPaint { int id=0; std::string name; float wearMin=0,wearMax=1; };
        struct SkinItem { int id=0; std::string name; bool knife=false; std::vector<SkinPaint> paints; };
        std::unordered_map<int, SkinCfg> skinConfigs;
        std::vector<SkinItem> skinOptions;
        std::string skinStatus;
        struct AimCfg {
            int key = 0, bone = 0, triggerBind = 0;
            bool overrideEnabled = false;
            bool on = false, always = false, ff = false, silent = false, jump = false;
            float fov = 3.f, speed = 4.f, speedShoot = 3.f, lockMs = 50.f, releaseMs = 100.f, reactMs = 25.f;
            bool rcs = false, rcsAlways = false;
            float rcsXmin = 1.f, rcsXmax = 1.f, rcsYmin = 1.f, rcsYmax = 1.f;
            bool trig = false, trigKey = false, trigFf = false, head = false, chest = false, stomach = false, arms = false, legs = false;
            bool hitChanceOn = false, backtrack = false, delayOn = false;
            float hitChance = 50.f, trigDelay = 250.f, minDamage = 30.f;
            bool drawFov=false, showTarget=false;
            bool wallCheck=true, flashCheck=true, smokeCheck=true;
            ImVec4 fovColor{1.f,.9f,0.f,1.f}, targetColor{0.f,.85f,1.f,1.f};
            int fovMode=0;
            float aimDeviation=0;
            bool timedAssist=false;
            int targetFovSizing=0;
            int aimStyle=0;float snapDelay=25;bool separateShootSmooth=false;
            int aimBones=1; // Head=1, Chest=2, Stomach=4, Legs=8. bone is legacy config data.
        };
        struct WeaponGroup { std::string name; AimCfg config; std::vector<int> weapons; };
        struct Cs2State {
            bool aim = false, rcs = false, ammo = false, bombinfo = false, showinfo = false, team = false, bhop = false, trigger = false;
            bool autoFire = false, fovCircle = false, watermark = false;
            float fov = 6.f, smooth = 3.f, dist = 300.f, hitchance = 70.f, delay = 0.f, randomize = 0.6f, recoilMin = 40.f, recoilMax = 80.f;
            int bone = 0, aimKey = 0, triggerKey = 0;
            AimCfg basic[6];
            std::unordered_map<int, AimCfg> adv;
            std::unordered_map<int, WeaponGroup> groups;
            int nextGroupId=0;
        } cs2;
        ExtraOptions extra;
        bool saveGrenadeSpot=false, clearGrenadeSpots=false;
        std::string extraStatus;
        int normalWeapon = 0;
        int advSel = 0;
        int selectedGroup = -1, groupCategory = 0;
        std::array<char,65> groupName{};
        std::string groupStatus;
        float normalScrollRequest = -1;
        std::array<float, 6> normalFov{ 1,1,1,1,1,1 };
        bool weaponWindow = true, clubWindow = false, spectators = false;
        Theme colors;
        std::unordered_map<std::string, bool> checks;
        std::unordered_map<std::string, float> numbers;
        std::unordered_map<std::string, int> choices;
        std::unordered_map<std::string, int> keybinds;
        std::unordered_map<std::string, std::string> bindingLabels;
        std::unordered_map<std::string, decltype(keybinds)> savedKeybinds;
        std::array<bool, 256> keyDown{};
        std::string capturing;
        bool allowHotkeys = false;
        int reservedMenu = 45, reservedPanic = 46;
        std::unordered_map<std::string, ImVec4> swatches;
        std::unordered_map<std::string, std::array<char, 128>> strings;
        std::vector<std::string> profiles{ "Default" };
        std::unordered_map<std::string, decltype(checks)> savedChecks;
        std::unordered_map<std::string, decltype(numbers)> savedNumbers;
        std::unordered_map<std::string, decltype(choices)> savedChoices;
        ImTextureID playerTexture = 0, knifeTexture = 0, gloveTexture = 0;
        bool disabled = false;
        std::string chamsStatus;
        std::string aimChecksStatus;
        struct PreviewState {
            bool boxes = false, names = false, health = false, armour = false, weapon = false, snaplines = false, arrows = false, sound = false, head = false, crosshair = false;
            bool glow = false, chams = false;
            bool chamsVisibleOnly = false;
            int chamsMaterial = 0;
            ImVec4 chamsOccluded{ 1,0,0,1 };
            ImVec4 color{ 0,.8f,1,1 };
            ImVec4 colBox{ 0.35f,0.69f,1.f,1.f }, colName{ 0.90f,0.90f,0.90f,1.f }, colHealth{ 0.31f,0.86f,0.31f,1.f };
            ImVec4 colArmor{ 0.35f,0.63f,0.94f,1.f }, colHead{ .2f,1.f,.35f,1.f }, colSpline{ 0.35f,0.69f,1.f,1.f };
            ImVec4 colWeapon{ .9f,.9f,.9f,1.f }, colArrows{ .35f,.69f,1.f,1.f };
            ImVec4 colSound{ .2f,1.f,.35f,1.f }, colCrosshair{ .2f,1.f,.35f,1.f };
            float soundDuration = 5.f, arrowRadius = 160.f;
            int spreadMode = 0;
            bool glowVisibleOnly = false;
            int glowMaterial = 0;
            ImVec4 glowVisible{0,.8f,1,1}, glowOccluded{1,0,0,1};
            bool glowC4 = false, eyeRay = false;
            ImVec4 colC4{1,.65f,.05f,1}, colEyeRay{1,.3f,.2f,1};
            float eyeRayLength = 2000.f;
        } preview;
        struct ProfileState {
            ExtraOptions extra;
            Cs2State cs2;
            PreviewState preview;
            getrekt::Config theme;
            std::unordered_map<std::string, bool> checks;
            std::unordered_map<std::string, float> numbers;
            std::unordered_map<std::string, int> choices, keybinds;
            int legitMode=0;
            std::unordered_map<int, SkinCfg> skinConfigs;
        };
        std::unordered_map<std::string, ProfileState> savedProfiles;
        ProfileState CaptureProfile() const {
            return {extra, cs2, preview, colors.config, checks, numbers, choices, keybinds,legit,skinConfigs};
        }
        void RestoreProfile(const ProfileState& state) {
            legit=state.legitMode;
            extra=state.extra; spectators=extra.spectators;
            cs2 = state.cs2; preview = state.preview; colors.config = state.theme; skinConfigs = state.skinConfigs;
            selectedKnifeId = 0;
            checks = state.checks; numbers = state.numbers; choices = state.choices; keybinds = state.keybinds;
            CancelBinding();
        }
        static constexpr size_t MaxProfiles = 128;
        std::string profileStatus;
        bool SaveProfile(const std::string& name) {
            if (name.empty() || name.size() > 4096) { profileStatus = "Invalid profile name"; return false; }
            if (!savedProfiles.contains(name) && savedProfiles.size() >= MaxProfiles) {
                profileStatus = "Maximum 128 saved profiles"; return false;
            }
            savedProfiles[name] = CaptureProfile();
            if (std::find(profiles.begin(), profiles.end(), name) == profiles.end()) profiles.push_back(name);
            profileStatus = "Profile saved";
            return true;
        }
        bool DeleteProfile(std::string name) {
            if (profiles.size() <= 1) return false;
            savedProfiles.erase(name);
            savedChecks.erase(name); savedNumbers.erase(name); savedChoices.erase(name); savedKeybinds.erase(name);
            std::erase(profiles, name);
            if (profiles.empty()) profiles.push_back("Default");
            profile = std::clamp(profile, 0, static_cast<int>(profiles.size()) - 1);
            profileStatus = "Profile deleted";
            return true;
        }
        bool LoadProfile(const std::string& name) {
            auto it = savedProfiles.find(name);
            if (it == savedProfiles.end()) return false;
            RestoreProfile(it->second); return true;
        }
        int CreateWeaponGroup(const std::string& name) {
            if (name.empty() || name.size()>64 || name.find_first_not_of(" \t\r\n")==std::string::npos || cs2.groups.size()>=64) return -1;
            if(cs2.nextGroupId>=1000000)return -1;
            for (const auto& [key,g]:cs2.groups) if(g.name==name) return -1;
            const int id=cs2.nextGroupId++;
            WeaponGroup group;group.name=name;group.config=cs2.basic[groupCategory];group.config.overrideEnabled=true;
            cs2.groups.emplace(id,std::move(group));return id;
        }
        bool RenameWeaponGroup(int id,const std::string& name) {
            if (!cs2.groups.contains(id) || name.empty() || name.size()>64 || name.find_first_not_of(" \t\r\n")==std::string::npos) return false;
            for(const auto& [key,g]:cs2.groups) if(key!=id && g.name==name) return false;
            cs2.groups.at(id).name=name;return true;
        }
        bool AssignWeaponGroup(int id,int weapon,bool member) {
            if(!cs2.groups.contains(id) || !Weapons::Find(weapon)) return false;
            if(member) {
                for(auto& [key,g]:cs2.groups) std::erase(g.weapons,weapon);
                cs2.groups.at(id).weapons.push_back(weapon);
                if(auto it=cs2.adv.find(weapon);it!=cs2.adv.end()) it->second.overrideEnabled=false;
            } else std::erase(cs2.groups.at(id).weapons,weapon);
            return true;
        }
        AimCfg& AimConfig(int id, int category) {
            auto it = cs2.adv.find(id);
            if (it != cs2.adv.end() && it->second.overrideEnabled) return it->second;
            for(auto& [key,g]:cs2.groups) if(g.config.overrideEnabled && std::find(g.weapons.begin(),g.weapons.end(),id)!=g.weapons.end()) return g.config;
            return cs2.basic[std::clamp(category, 0, 5)];
        }
        const AimCfg& ActiveAimConfig(int id,int category) const {
            if(legit==0)return cs2.basic[std::clamp(category,0,5)];
            static const AimCfg disabled{};
            if(legit!=1)return disabled;
            auto it=cs2.adv.find(id);
            if(it!=cs2.adv.end()&&it->second.overrideEnabled)return it->second;
            for(const auto& [key,g]:cs2.groups)if(g.config.overrideEnabled&&
                std::find(g.weapons.begin(),g.weapons.end(),id)!=g.weapons.end())return g.config;
            return disabled;
        }
        bool Light() const { return colors.config.windowBg.x + colors.config.windowBg.y + colors.config.windowBg.z > 1.5f; }
        ImU32 Gray(int dark, int light) const { int v = Light() ? light : dark;return IM_COL32(v, v, v, 255); }
        ImU32 AccentText() const { auto c = colors.config.accent;if (Light()) { c.x *= .52f;c.y *= .52f;c.z *= .52f; }return Color(c); }
        void BoundCheck(const char* label, bool& value) {
            checks[Id(label)] = value;value = Check(label, value);
        }
        void EspCheck(const char* label, bool& value, ImVec4& color) {
            BoundCheck(label, value);
            ImGui::SameLine(238);ImGui::PushID(label);
            ImGui::ColorEdit4("##color", &color.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);
            ImGui::PopID();
        }
        void Bind(const char* label, bool& value) { checks[Id(label)] = value;value = Check(label, value); }
        void BindSlider(const char* label, float& value, float min, float max, const char* format = "%.0f") {
            numbers[Id(label)] = value;Slider(label, value, min, max, format);value = numbers[Id(label)];
        }
        void BindCombo(const char* label, const char* list, int& value) {
            choices[Id(label)] = value;Combo(label, list, value);value = choices[Id(label)];
        }

        VisualReplica() {
            colors.config.accent = ImVec4(1, .48f, .045f, 1);
            colors.config.windowBg = ImVec4(.105f, .10f, .095f, 1);
            colors.config.panelBg = ImVec4(.175f, .175f, .175f, 1);
            colors.config.text = ImVec4(.84f, .84f, .84f, 1);
            colors.SetChangelog("GetRekt visual reference", std::vector<Release>{{"1.2", { { ChangeKind::Updated,"Visual reconstruction from the supplied showcase" },{ ChangeKind::Added,"Local demonstration controls and reference previews" } }}});
            savedProfiles.emplace("Default", CaptureProfile());
        }
        ImU32 Color(ImVec4 c) const { return ImGui::ColorConvertFloat4ToU32(c); }
        ImU32 Accent(float alpha = 1) const { auto c = colors.config.accent;c.w = alpha;return Color(c); }
        std::string Id(const char* label) const { return std::to_string(ImGui::GetID(label)); }
        void Label(const char* label) { ImGui::TextUnformatted(label, strstr(label, "##")); }
        static int DefaultBinding(const std::string& label) {
            if (label == "c" || label == "C")return 'C';if (label == "v" || label == "V")return 'V';
            if (label == "TAB")return 9;
            if (label == "MOUSE4" || label == "MOUSE 4")return 5;
            if (label == "MOUSE5" || label == "MOUSE 5")return 6;
            return 0;
        }
        static std::string BindingName(int key) {
            if (!key)return "key not set";
            if (key == 1)return "MOUSE 1";if (key == 2)return "MOUSE 2";
            if (key == 4)return "MOUSE 3";if (key == 5)return "MOUSE 4";if (key == 6)return "MOUSE 5";
            if (key == 9)return "TAB";if (key >= 'A' && key <= 'Z')return std::string(1, static_cast<char>(key));
            if (key >= '0' && key <= '9')return std::string(1, static_cast<char>(key));
            if (key >= 112 && key <= 135)return "F" + std::to_string(key - 111);
        switch (key) { case 8:return "BACKSPACE";case 13:return "ENTER";case 16:return "SHIFT";case 17:return "CTRL";case 18:return "ALT";case 32:return "SPACE";case 37:return "LEFT";case 38:return "UP";case 39:return "RIGHT";case 40:return "DOWN";default:return "KEY " + std::to_string(key); }
        }
        void CancelBinding() { capturing.clear();keyDown.fill(false); }
        bool HandleBindingKey(int key, bool down) {
            if (key <= 0 || key >= 256)return false;
            bool edge = down && !keyDown[key];keyDown[key] = down;
            if (!capturing.empty()) {
                if (!edge)return true;
                if (key == 27) { keybinds[capturing] = 0;capturing.clear();return true; }
                if (key == reservedMenu || key == reservedPanic || key == 121 || key == 91 || key == 92 || key == 93)return true;
                if (key == 1 || key == 2 || key == 4 || key == 5 || key == 6 || key >= 8) { keybinds[capturing] = key;capturing.clear(); }
                return true;
            }
            if (edge && allowHotkeys)for (const auto& binding : keybinds)if (binding.second == key)checks[binding.first] = !checks[binding.first];
            return false;
        }
        void Section(const char* label) {
            auto p = ImGui::GetCursorScreenPos(); auto* d = ImGui::GetWindowDrawList();
            const float right = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 20;
            d->AddLine(ImVec2(p.x + 2, p.y + 6), ImVec2(p.x + 20, p.y + 6), Accent(.55f));
            d->AddText(ImVec2(p.x + 30, p.y), AccentText(), label);
            float end = p.x + 38 + ImGui::CalcTextSize(label).x;
            if (end < right)d->AddLine(ImVec2(end, p.y + 6), ImVec2(right, p.y + 6), Accent(.55f));
            ImGui::Dummy(ImVec2(0, 14));
        }
        bool Check(const char* label, bool initial = false) {
            auto key = Id(label);auto [it, inserted] = checks.try_emplace(key, initial);(void)inserted;
            bool& value = it->second;auto p = ImGui::GetCursorScreenPos();
            std::string text = label; if (auto suffix = text.find("##"); suffix != std::string::npos) text.resize(suffix);const auto bracket = text.find('[');bool hasBinding = bracket != std::string::npos;
            if (hasBinding) { auto close = text.find(']', bracket);keybinds.try_emplace(key, DefaultBinding(text.substr(bracket + 1, close - bracket - 1)));text.resize(bracket);while (!text.empty() && text.back() == ' ')text.pop_back();bindingLabels[key] = text; }
            ImGui::PushID(label);bool clicked = ImGui::InvisibleButton("check", ImVec2(hasBinding ? 20 + ImGui::CalcTextSize(text.c_str()).x : ImGui::GetContentRegionAvail().x, 14));
            if (clicked && !disabled)value = !value;
            auto* d = ImGui::GetWindowDrawList();
            d->AddRectFilledMultiColor(ImVec2(p.x, p.y + 1), ImVec2(p.x + 12, p.y + 13), Gray(24, 220), Gray(24, 220), Gray(5, 165), Gray(5, 165));
            if (value)d->AddRectFilled(ImVec2(p.x + 2, p.y + 3), ImVec2(p.x + 10, p.y + 11), Accent());
            d->AddText(ImVec2(p.x + 20, p.y + 1), Color(colors.config.text), text.c_str());
            if (hasBinding) {
                ImGui::SameLine(0, 5);auto pos = ImGui::GetCursorScreenPos();
                std::string badge = "[" + (capturing == key ? std::string("press a key...") : BindingName(keybinds[key])) + "]";
                if (ImGui::InvisibleButton("binding", ImVec2(ImGui::CalcTextSize(badge.c_str()).x + 2, 14)) && !disabled) { capturing = key;keyDown.fill(false); }
                d->AddText(ImVec2(pos.x, pos.y + 1), capturing == key ? AccentText() : Color(colors.config.text), badge.c_str());
                if (ImGui::IsItemHovered())ImGui::SetTooltip("Click to bind a key or mouse button. Escape clears. Menu, panic and F10 are reserved.");
            }
            ImGui::PopID();return value;
        }
        void Checks(std::initializer_list<const char*> labels) { for (auto label : labels)Check(label); }
        void Slider(const char* label, float initial, float min, float max, const char* format = "%.0f") {
            auto [it, inserted] = numbers.try_emplace(Id(label), initial);(void)inserted;
            ImGui::PushID(label);auto p = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##value", ImVec2(174, 16));
            if (ImGui::IsItemActive() && !disabled)it->second = min + std::clamp((ImGui::GetIO().MousePos.x - p.x - 5) / 164.f, 0.f, 1.f) * (max - min);
            auto* d = ImGui::GetWindowDrawList();
            d->AddRectFilledMultiColor(p, ImVec2(p.x + 174, p.y + 16), Gray(26, 205), Gray(26, 205), Gray(5, 165), Gray(5, 165));
            float x = p.x + 2 + std::clamp((it->second - min) / (max - min), 0.f, 1.f) * 163;
            d->AddRectFilled(ImVec2(x, p.y + 2), ImVec2(x + 9, p.y + 14), Accent(), 2);
            char number[64]{};snprintf(number, sizeof(number), format, static_cast<double>(it->second));auto size = ImGui::CalcTextSize(number);
            d->AddText(ImVec2(p.x + (174 - size.x) / 2, p.y + 1), Color(colors.config.text), number);
            ImGui::SameLine();Label(label);ImGui::PopID();
        }
        void Combo(const char* label, const char* list, int initial = 0) {
            auto [it, inserted] = choices.try_emplace(Id(label), initial);(void)inserted;
            ImGui::PushID(label);ImGui::PushItemWidth(174);ImGui::Combo("##choice", &it->second, list);
            auto end = ImGui::GetItemRectMax();auto start = ImGui::GetItemRectMin();auto* d = ImGui::GetWindowDrawList();
            d->AddTriangleFilled(ImVec2(end.x - 12, start.y + 4), ImVec2(end.x - 4, start.y + 4), ImVec2(end.x - 8, start.y + 11), Color(colors.config.text));
            ImGui::PopItemWidth();ImGui::SameLine();Label(label);ImGui::PopID();
        }
        void Input(const char* label) {
            auto& value = strings[Id(label)];ImGui::PushID(label);ImGui::PushItemWidth(174);
            ImGui::InputText("##input", value.data(), value.size());ImGui::PopItemWidth();
            ImGui::SameLine();Label(label);ImGui::PopID();
        }
        void Swatch(const char* label, ImVec4 initial = ImVec4(0, 1, 1, 1)) {
            auto [it, inserted] = swatches.try_emplace(Id(label), initial);(void)inserted;
            colors.ColorRow(label, it->second);
        }
        void Tabs(const char* id, std::initializer_list<const char*> labels, int& selected, float width, bool main = false) {
            bool childTabs = !main && width < 400;
            if (childTabs) { width = ImGui::GetWindowWidth();ImGui::SetCursorPosX(0); }
            ImGui::PushID(id);int i = 0;float each = width / static_cast<float>(labels.size());
            for (auto label : labels) {
                if (i)ImGui::SameLine(0, 0);auto p = ImGui::GetCursorScreenPos();
                ImGui::PushID(i);bool click = ImGui::InvisibleButton("tab", ImVec2(each, 32));
                if (click && !disabled)selected = i;auto* d = ImGui::GetWindowDrawList();
                d->AddRectFilledMultiColor(p, ImVec2(p.x + each, p.y + 32), Gray(39, 225), Gray(39, 225), Gray(26, 205), Gray(26, 205));
                if (ImGui::IsItemHovered())d->AddRectFilled(p, ImVec2(p.x + each, p.y + 32), Gray(60, 185));
                auto size = ImGui::CalcTextSize(label);
                d->AddText(ImVec2(p.x + (each - size.x) / 2, p.y + 10), selected == i ? AccentText() : Color(colors.config.text), label);
                if (!main)d->AddLine(ImVec2(p.x, p.y + 31), ImVec2(p.x + each, p.y + 31), selected == i ? Accent() : IM_COL32(95, 95, 95, 255), 2);
                ImGui::PopID();++i;
            }ImGui::PopID();if (childTabs)ImGui::SetCursorPosX(20);
        }
        void Panel(const char* id, float x, float y, float w = 300, float h = 370) {
            ImGui::SetCursorPos(ImVec2(x, y));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 2));
            ImGui::BeginChild(id, ImVec2(w, h), ImGuiChildFlags_AlwaysUseWindowPadding, (disabled ? ImGuiWindowFlags_NoInputs : 0));
            ImGui::PopStyleVar();
        }
        void EndPanel() { ImGui::EndChild(); }
        void List(const char* id, const std::vector<std::string>& names, int& value, float height, float width = 0) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, (Light() ? ImVec4(.83f, .83f, .83f, 1) : ImVec4(.10f, .095f, .09f, 1)));
            ImGui::BeginChild(id, ImVec2(width, height), false, disabled ? ImGuiWindowFlags_NoInputs : 0);
            for (size_t i = 0;i < names.size();++i) {
                ImGui::PushID(static_cast<int>(i));auto p = ImGui::GetCursorScreenPos();float w = ImGui::GetContentRegionAvail().x;
                if (ImGui::InvisibleButton("row", ImVec2(w, 14)))value = static_cast<int>(i);
                auto* d = ImGui::GetWindowDrawList();if (value == static_cast<int>(i) || ImGui::IsItemHovered())d->AddRectFilledMultiColor(p, ImVec2(p.x + w, p.y + 16), Accent(.3f), Accent(.3f), Accent(), Accent());
                d->AddText(p, Color(colors.config.text), names[i].c_str());ImGui::PopID();
            }
            ImGui::EndChild();ImGui::PopStyleColor();
        }
        void AntiAim() {
            Panel("anti-left", 30, 120, 300, 270);
            Section("Anti-aim:");Combo("Pitch", "Off\0Up\0Down\0Random\0");Combo("Yaw", "Off\0Left\0Right\0Back\0");Combo("Yaw fake", "Off\0Left\0Right\0Back\0");Check("LBY breaker");EndPanel();
            Panel("anti-spread", 30, 412, 300, 78);Section("Spread resolver:");Check("Resolve all");EndPanel();
            Panel("anti-right", 363, 120, 297, 370);
            Section("Nospread resolver:");Check("Nospread resolver enable");Combo("Pitch", "Up\0Down\0Off\0");Combo("Yaw", "Self\0Left\0Right\0");Section("Self settings:");Combo("Base angle", "Fake body\0Real body\0");Combo("Resolve type", "Bruteforce multiply\0Off\0");Check("Jitter");Section("Bruteforce multiply:");Combo("Using", "Missed shots\0Always\0");Slider("Angle", 0, -180, 180);Section("");Check("Untrust", false);
            Section("CS2 aimbot:");BoundCheck("Enable [key not set]", cs2.aim);Slider("FOV##cs2", 6, 0, 40, "%.1f");Slider("Smooth##cs2", 3, 1, 20, "%.1f");Slider("Hit chance##cs2", 70, 0, 100);Slider("Randomize##cs2", 0.6f, 0, 5, "%.1f");BoundCheck("RCS##cs2", cs2.rcs);Slider("Recoil min##cs2", 40, 0, 100);Slider("Recoil max##cs2", 80, 0, 100);ImGui::TextUnformatted("Trigger settings: LEGIT > weapon");BoundCheck("Auto fire##cs2", cs2.autoFire);BoundCheck("FOV circle##cs2", cs2.fovCircle);BoundCheck("Watermark##cs2", cs2.watermark);
            cs2.aimKey = keybinds[Id("Enable [key not set]")];cs2.triggerKey = keybinds[Id("Triggerbot [key not set]")];cs2.fov = numbers[Id("FOV##cs2")];cs2.smooth = numbers[Id("Smooth##cs2")];cs2.hitchance = numbers[Id("Hit chance##cs2")];cs2.randomize = numbers[Id("Randomize##cs2")];cs2.recoilMin = numbers[Id("Recoil min##cs2")];cs2.recoilMax = numbers[Id("Recoil max##cs2")];
            EndPanel();
        }
        void WeaponIcon(int index, ImVec2 p, ImU32 color) {
            // Local vector silhouettes; no game assets or game integration.
            auto* draw = ImGui::GetWindowDrawList();
            auto line = [&](float x, float y, float x2, float y2, float width) {draw->AddLine(ImVec2(p.x + x, p.y + y), ImVec2(p.x + x2, p.y + y2), color, width);};
            auto rect = [&](float x, float y, float w, float h) {draw->AddRectFilled(ImVec2(p.x + x, p.y + y), ImVec2(p.x + x + w, p.y + y + h), color);};
            if (index == 0) { rect(6, 4, 29, 4);rect(5, 7, 18, 3);line(10, 9, 7, 17, 5);rect(34, 5, 7, 2); }
            else if (index == 1) { rect(15, 7, 20, 4);line(2, 13, 16, 9, 5);line(20, 10, 18, 17, 4);line(27, 11, 30, 18, 4);line(33, 8, 48, 8, 2);rect(37, 5, 2, 4); }
            else if (index == 2) { rect(14, 7, 22, 4);line(2, 12, 15, 9, 4);line(21, 9, 19, 16, 3);line(34, 8, 51, 8, 2);rect(21, 2, 13, 3);rect(24, 5, 2, 3);line(39, 10, 41, 17, 1); }
            else if (index == 3) { rect(10, 6, 27, 7);line(0, 12, 13, 11, 4);rect(20, 3, 13, 2);line(25, 12, 25, 17, 4);rect(37, 8, 11, 2); }
            else if (index == 4) { rect(11, 5, 21, 7);line(0, 9, 13, 8, 4);rect(15, 2, 13, 2);line(16, 11, 14, 18, 3);line(27, 11, 28, 19, 4);line(31, 7, 47, 7, 2);rect(38, 4, 2, 5); }
            else { line(1, 13, 13, 8, 5);rect(12, 7, 34, 3);rect(16, 10, 13, 3);rect(44, 6, 7, 2); }
        }
        void AimPanels(AimCfg& cfg, bool group = false) {
            Panel("normal-aim", 30, 206, 300, 138);
            if (normalScrollRequest >= 0) { ImGui::SetScrollY(normalScrollRequest);normalScrollRequest = -1; }
            if (legit == 1) ImGui::Checkbox(group ? "Enable shared group" : "Use weapon override", &cfg.overrideEnabled);
            if(legit==1)ImGui::TextWrapped("Advanced only: enable this weapon or group. Basic is not used.");
            keybinds.try_emplace(Id("Aimbot  [key not set]"), cfg.key);
            Section("Aimbot:");Bind("Aimbot  [key not set]", cfg.on);Bind("Always on", cfg.always);Bind("Friendlyfire", cfg.ff);Bind("Jump check", cfg.jump);
            cfg.key = keybinds[Id("Aimbot  [key not set]")];
            Section("Custom settings:");
            EspCheck("Draw FOV",cfg.drawFov,cfg.fovColor);
            EspCheck("Show Target",cfg.showTarget,cfg.targetColor);
            BindCombo("FOV mode","On crosshair\0On target\0Follow locked target\0",cfg.fovMode);
            if(cfg.fovMode==1){
                BindCombo("Target FOV size","Static\0Distance\0",cfg.targetFovSizing);
            }
            BindSlider("FOV", cfg.fov, 0, 30, "%.2f");BindCombo("Aim style","Curve (smooth)\0Click assist (one move)\0Key aim + shot\0",cfg.aimStyle);
            if(cfg.aimStyle==0){
                BindSlider("Smooth",cfg.speed,1,30,"%.2f");
                Bind("Separate smooth while firing",cfg.separateShootSmooth);
                if(cfg.separateShootSmooth)BindSlider("Firing smooth",cfg.speedShoot,1,30,"%.2f");
            }else{
                BindSlider(cfg.aimStyle==2?"Key aim smooth":"Click smooth",cfg.speed,1,30,"%.2f");
            }
            Section("Aim points:");
            ImGui::PushID("aim-points");
            const char* pointLabels[]={"Head","Chest","Stomach","Legs"};
            for(int i=0;i<4;++i){bool selected=(cfg.aimBones&(1<<i))!=0;
                if(ImGui::Checkbox(pointLabels[i],&selected)){
                    if(selected)cfg.aimBones|=1<<i;else cfg.aimBones&=~(1<<i);
                }
            }
            ImGui::PopID();
            if(!cfg.aimBones)ImGui::TextWrapped("Select at least one point to aim.");
            BindSlider("Aim deviation (units)",cfg.aimDeviation,0,4,"%.2f");
            Section("Aim timing:");Bind("Timed release",cfg.timedAssist);
            ImGui::BeginDisabled(!cfg.timedAssist);BindSlider("Lock for X ms", cfg.lockMs, 0, 1000);BindSlider("Release after X ms", cfg.releaseMs, 0, 1000);ImGui::EndDisabled();
            BindSlider("Reaction time (ms)", cfg.reactMs, 0, 500);
            EndPanel();
            Panel("normal-rcs", 30, 356, 300, 134);
            Section("RCS (1 = 100%):");Bind("RCS", cfg.rcs);Bind("RCS enabled always", cfg.rcsAlways);
            BindSlider("X min", cfg.rcsXmin, 0, 2, "%.2f");BindSlider("X max", cfg.rcsXmax, 0, 2, "%.2f");
            BindSlider("Y min", cfg.rcsYmin, 0, 2, "%.2f");BindSlider("Y max", cfg.rcsYmax, 0, 2, "%.2f");EndPanel();
            Panel("normal-trigger", 363, 206, 297, 284);
            keybinds.try_emplace(Id("On key  [key not set]"), cfg.triggerBind);
            Section("Triggerbot:");Bind("Triggerbot", cfg.trig);Bind("On key  [key not set]", cfg.trigKey);
            cfg.triggerBind = keybinds[Id("On key  [key not set]")];
            Bind("Friendlyfire", cfg.trigFf);
            Bind("Head", cfg.head);Bind("Chest", cfg.chest);Bind("Stomach", cfg.stomach);Bind("Arms", cfg.arms);Bind("Legs", cfg.legs);Bind("Hit chance", cfg.hitChanceOn);
            BindSlider("Hit chance estimate", cfg.hitChance, 0, 100, "%.2f");Bind("Delay", cfg.delayOn);BindSlider("Delay", cfg.trigDelay, 0, 1000);
            BindSlider("Damage estimate", cfg.minDamage, 0, 100, "%.2f");
            EndPanel();
        }
        void AdvancedAimPanels(AimCfg& cfg, bool group=false) {
            Panel("advanced-left",30,206,300,284);
            Tabs("advanced-left-tabs",{"Global","Triggerbot"},legitLeft,300);
            if (legitLeft == 0) {
                Section("Global settings");
                ImGui::Checkbox(group ? "Enable group profile" : "Enable weapon profile", &cfg.overrideEnabled);
                const char* bindLabel = "Enable  [key not set]";
                keybinds.try_emplace(Id(bindLabel), cfg.key);
                Bind(bindLabel, cfg.on);
                cfg.key = keybinds[Id(bindLabel)];
                Bind("Always on", cfg.always);
                Bind("Aim at teammates", cfg.ff);
                Bind("Flash check", cfg.flashCheck);
                Bind("Jump check", cfg.jump);
                Bind("Smoke check", cfg.smokeCheck);
                Bind("Wall check", cfg.wallCheck);
                ImGui::Separator();
                EspCheck("Draw FOV", cfg.drawFov, cfg.fovColor);
                EspCheck("Show Target", cfg.showTarget, cfg.targetColor);
            } else {
                keybinds.try_emplace(Id("On key  [key not set]"), cfg.triggerBind);
                Section("Triggerbot:");Bind("Triggerbot", cfg.trig);Bind("On key  [key not set]", cfg.trigKey);
                cfg.triggerBind = keybinds[Id("On key  [key not set]")];
                Bind("Friendlyfire", cfg.trigFf);
                Bind("Head", cfg.head);Bind("Chest", cfg.chest);Bind("Stomach", cfg.stomach);Bind("Arms", cfg.arms);Bind("Legs", cfg.legs);Bind("Hit chance", cfg.hitChanceOn);
                BindSlider("Hit chance estimate", cfg.hitChance, 0, 100, "%.2f");
                BindSlider("Damage estimate", cfg.minDamage, 0, 100, "%.2f");
            }
            EndPanel();
            Panel("advanced-right",363,206,297,284);
            Tabs("advanced-right-tabs",{"Basic","RCS","Delays"},legitRight,297);
            if (legitRight == 0) {
                BindCombo("Aim style","Curve\0Click assist\0Key aim + shot\0",cfg.aimStyle);
                BindSlider("Smooth",cfg.speed,1,30,"%.2f");
                if (cfg.aimStyle == 0) {
                    Bind("Separate firing smooth",cfg.separateShootSmooth);
                    if (cfg.separateShootSmooth) BindSlider("Firing smooth",cfg.speedShoot,1,30,"%.2f");
                }
                BindSlider("FOV",cfg.fov,0,30,"%.2f");
                BindCombo("FOV position","On crosshair\0On target\0Follow locked target\0",cfg.fovMode);
                if (cfg.fovMode == 1) BindCombo("Target FOV size","Static\0Distance\0",cfg.targetFovSizing);
                Section("Aim points:");
                ImGui::PushID("aim-points");
                const char* pointLabels[]={"Head","Chest","Stomach","Legs"};
                for(int i=0;i<4;++i){bool selected=(cfg.aimBones&(1<<i))!=0;
                    if(ImGui::Checkbox(pointLabels[i],&selected)){
                        if(selected)cfg.aimBones|=1<<i;else cfg.aimBones&=~(1<<i);
                    }
                }
                ImGui::PopID();
                BindSlider("Aim deviation",cfg.aimDeviation,0,4,"%.2f");
            } else if (legitRight == 1) {
                Section("RCS (1 = 100%):");Bind("RCS", cfg.rcs);Bind("RCS enabled always", cfg.rcsAlways);
                BindSlider("X min", cfg.rcsXmin, 0, 2, "%.2f");BindSlider("X max", cfg.rcsXmax, 0, 2, "%.2f");
                BindSlider("Y min", cfg.rcsYmin, 0, 2, "%.2f");BindSlider("Y max", cfg.rcsYmax, 0, 2, "%.2f");
            } else {
                Section("Aim delays");
                ImGui::BeginDisabled(cfg.aimStyle != 0);
                BindSlider("Reaction (ms)",cfg.reactMs,0,500);
                Bind("Timed release",cfg.timedAssist);
                ImGui::BeginDisabled(!cfg.timedAssist);
                BindSlider("Lock (ms)",cfg.lockMs,0,1000);
                BindSlider("Release (ms)",cfg.releaseMs,0,1000);
                ImGui::EndDisabled();
                ImGui::EndDisabled();
                Section("Trigger delay");
                Bind("Delay shot",cfg.delayOn);
                ImGui::BeginDisabled(!cfg.delayOn);
                BindSlider("Delay (ms)",cfg.trigDelay,0,1000);
                ImGui::EndDisabled();
            }
            EndPanel();
        }
        void NormalLegit() {
            ImGui::SetCursorPos(ImVec2(30, 160));
            const char* labels[]{ "Pistols","Rifles","Sniper rifles","SMGs","Machine guns","Shotguns" };
            for (int i = 0;i < 6;++i) {
                if (i)ImGui::SameLine(0, 0);auto pos = ImGui::GetCursorScreenPos();
                ImGui::PushID(i);if (ImGui::InvisibleButton("weapon-category", ImVec2(105, 34)) && !disabled)normalWeapon = i;
                WeaponIcon(i, ImVec2(pos.x + 28, pos.y + 7), i == normalWeapon ? Accent() : Color(colors.config.text));
                if (ImGui::IsItemHovered())ImGui::SetTooltip("%s", labels[i]);ImGui::PopID();
            }

            ImGui::PushID(normalWeapon);
            AimPanels(cs2.basic[normalWeapon]);
            ImGui::PopID();
        }
        static const int* AdvIds() {
            static const auto ids = [] { std::array<int, Weapons::All.size()> result{};
                for (size_t i=0; i<result.size(); ++i) result[i] = Weapons::All[i].id; return result; }();
            return ids.data();
        }
        static std::vector<std::string> AdvNames() {
            std::vector<std::string> names;
            for (const auto& w : Weapons::All) names.emplace_back(w.name);
            return names;
        }
        void AdvancedLegit() {
            if(selectedGroup>=0 && !cs2.groups.contains(selectedGroup)) selectedGroup=-1;
            Panel("adv-selector", 30, 160, 630, 38);
            advSel = std::clamp(advSel, 0, static_cast<int>(Weapons::All.size()) - 1);
            ImGui::SetNextItemWidth(260);
            const char* profileName = selectedGroup >= 0 ? cs2.groups.at(selectedGroup).name.c_str() : Weapons::All[advSel].name;
            if (ImGui::BeginCombo("##weapon-profile", profileName)) {
                for (int i=0;i<static_cast<int>(Weapons::All.size());++i) {
                    if(ImGui::Selectable(Weapons::All[i].name,advSel==i)) { advSel=i;selectedGroup=-1; }
                    if(advSel==i) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();if(ImGui::Button("Weapon config"))weaponWindow=!weaponWindow;
            EndPanel();
            ImGui::PushID("adv-cfg");
            if(selectedGroup>=0) {
                ImGui::PushID("shared-group");ImGui::PushID(selectedGroup);
                AdvancedAimPanels(cs2.groups.at(selectedGroup).config,true);
                ImGui::PopID();ImGui::PopID();
            } else {
                const auto& selected=Weapons::All[advSel];ImGui::PushID(selected.id);
                auto [it,inserted]=cs2.adv.try_emplace(selected.id);
                if(inserted) { it->second=AimConfig(selected.id,Weapons::Category(selected.id));it->second.overrideEnabled=false; }
                AdvancedAimPanels(it->second);ImGui::PopID();
            }
            ImGui::PopID();
        }
        void WeaponConfigWindow(ImVec2 menuPosition) {
            const auto display=ImGui::GetIO().DisplaySize;
            const float width=std::clamp(display.x-menuPosition.x-704.f,350.f,530.f);
            ImGui::SetNextWindowPos(ImVec2(std::max(0.f,std::min(menuPosition.x+704,display.x-width)),std::max(0.f,std::min(menuPosition.y+92,display.y-428))),ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(width,428),ImGuiCond_Always);
            if(ImGui::Begin("Weapon config",&weaponWindow,ImGuiWindowFlags_NoResize)) {
                if(selectedGroup>=0&&!cs2.groups.contains(selectedGroup))selectedGroup=-1;
                if(ImGui::RadioButton("Individual weapons",selectedGroup<0))selectedGroup=-1;
                ImGui::SetNextItemWidth(width-110);ImGui::InputTextWithHint("##group-name","Group name",groupName.data(),groupName.size());
                ImGui::SameLine();if(ImGui::Button("Add")){const int id=CreateWeaponGroup(groupName.data());if(id>=0){selectedGroup=id;groupStatus.clear();}else groupStatus="Use a unique group name.";}
                ImGui::BeginDisabled(selectedGroup<0);
                if(ImGui::Button("Rename"))groupStatus=RenameWeaponGroup(selectedGroup,groupName.data())?"":"Invalid group name.";
                ImGui::SameLine();if(ImGui::Button("Remove")){cs2.groups.erase(selectedGroup);selectedGroup=-1;groupName.fill(0);}
                ImGui::EndDisabled();
                if(ImGui::BeginTable("weapon-columns",3,ImGuiTableFlags_SizingStretchSame)){
                    ImGui::TableNextColumn();ImGui::TextUnformatted("Categories / groups");
                    ImGui::BeginChild("categories",ImVec2(0,270),true);
                    const char* cats[]={"Pistols","Assault rifles","Sniper rifles","SMGs","Machine guns","Shotguns"};
                    for(int i=0;i<6;++i)if(ImGui::Selectable(cats[i],selectedGroup<0&&groupCategory==i)){groupCategory=i;selectedGroup=-1;}
                    ImGui::Separator();
                    std::vector<int> ids;for(const auto& [id,g]:cs2.groups)ids.push_back(id);std::sort(ids.begin(),ids.end());
                    for(int id:ids)if(ImGui::Selectable(cs2.groups.at(id).name.c_str(),selectedGroup==id)){selectedGroup=id;snprintf(groupName.data(),groupName.size(),"%s",cs2.groups.at(id).name.c_str());}
                    ImGui::EndChild();ImGui::TableNextColumn();
                    ImGui::TextUnformatted(selectedGroup>=0?"Members (-)":"Weapons");
                    ImGui::BeginChild("members",ImVec2(0,270),true);
                    for(int i=0;i<int(Weapons::All.size());++i){const auto& w=Weapons::All[i];
                        if(selectedGroup>=0){const auto members=cs2.groups.at(selectedGroup).weapons;
                            if(std::find(members.begin(),members.end(),w.id)!=members.end()&&ImGui::Selectable(w.name,true))AssignWeaponGroup(selectedGroup,w.id,false);
                        }else if(Weapons::Category(w.id)==groupCategory&&ImGui::Selectable(w.name,advSel==i))advSel=i;
                    }
                    ImGui::EndChild();ImGui::TableNextColumn();ImGui::TextUnformatted("Available (+)");
                    ImGui::BeginChild("available",ImVec2(0,270),true);
                    if(selectedGroup>=0)for(const auto& w:Weapons::All){const auto members=cs2.groups.at(selectedGroup).weapons;
                        if(std::find(members.begin(),members.end(),w.id)==members.end()&&ImGui::Selectable(w.name,false))AssignWeaponGroup(selectedGroup,w.id,true);
                    }
                    ImGui::EndChild();ImGui::EndTable();
                }
                if(!groupStatus.empty())ImGui::TextWrapped("%s",groupStatus.c_str());
            }ImGui::End();
        }
        void Legit() {
            ImGui::SetCursorPos(ImVec2(30, 120));Tabs("legit", { "Basic","Advanced" }, legit, 630);
            if (!legit) NormalLegit(); else AdvancedLegit();
        }
        void DrawPlayerPreview() {
            auto origin = ImGui::GetWindowPos();auto* d = ImGui::GetWindowDrawList();
            ImVec2 a(origin.x + 70, origin.y + 40), b(a.x + 183, a.y + 244);
            if (playerTexture)d->AddImage(playerTexture, a, b, ImVec2(0, 0), ImVec2(1, 1), preview.chams ? Color(preview.color) : IM_COL32(255, 255, 255, 255));
            ImVec2 lo(a.x + 16, a.y + 4), hi(b.x - 12, b.y - 4);
            if (preview.glow)for (int n = 5;n > 0;--n) { auto c = preview.glowVisible;c.w = .08f;d->AddRect(ImVec2(lo.x - n, lo.y - n), ImVec2(hi.x + n, hi.y + n), Color(c), 0, 3.f, ImDrawFlags_RoundCornersAll); }
            if (preview.boxes) { d->AddRect(ImVec2(lo.x - 1, lo.y - 1), ImVec2(hi.x + 1, hi.y + 1), IM_COL32(0, 0, 0, 255));d->AddRect(lo, hi, Color(preview.colBox)); }
            if (preview.health) { d->AddRectFilled(ImVec2(lo.x - 9, lo.y), ImVec2(lo.x - 4, hi.y), IM_COL32(12, 12, 12, 255));d->AddRectFilled(ImVec2(lo.x - 8, hi.y - (hi.y - lo.y) * .75f), ImVec2(lo.x - 5, hi.y), Color(preview.colHealth));d->AddText(ImVec2(lo.x - 29, lo.y + 48), Color(colors.config.text), "75"); }
            if (preview.armour) { d->AddRectFilled(ImVec2(hi.x + 4, lo.y), ImVec2(hi.x + 9, hi.y), IM_COL32(12, 12, 12, 255));d->AddRectFilled(ImVec2(hi.x + 5, lo.y + 20), ImVec2(hi.x + 8, hi.y), Color(preview.colArmor)); }
            if (preview.names)d->AddText(ImVec2(lo.x + 38, lo.y - 17), Color(preview.colName), "Preview player");
            if (preview.weapon)d->AddText(ImVec2(lo.x + 53, hi.y + 7), Color(preview.colWeapon), "AK-47");
            if (preview.head) {
                ImVec2 head(a.x+100,a.y+20);
                for(int n=3;n>0;--n) d->AddCircle(head,17.f+n,IM_COL32(50,255,90,18),32,2.f);
                d->AddCircle(head,17.f,Color(preview.colHead),32);
            }
            if (preview.snaplines)d->AddLine(ImVec2(origin.x + 148, origin.y + 306), ImVec2((lo.x + hi.x) / 2, hi.y), Color(preview.colSpline));
            if (preview.arrows)d->AddTriangleFilled(ImVec2(lo.x - 38, lo.y + 95), ImVec2(lo.x - 24, lo.y + 88), ImVec2(lo.x - 24, lo.y + 102), Color(preview.colArrows));
            if (preview.sound)d->AddCircle(ImVec2((lo.x + hi.x) / 2, hi.y - 8), 48, Color(preview.colSound), 48);
            if (preview.crosshair) { ImVec2 c((lo.x + hi.x) / 2, (lo.y + hi.y) / 2);d->AddLine(ImVec2(c.x - 7, c.y), ImVec2(c.x + 7, c.y), Color(preview.colCrosshair));d->AddLine(ImVec2(c.x, c.y - 7), ImVec2(c.x, c.y + 7), Color(preview.colCrosshair)); }
        }
        void Visuals() {
            ImGui::SetCursorPos(ImVec2(30, 120));Tabs("visuals", { "ESP","Other" }, visuals, 630);
            Panel("visuals-left", 30, 180, 300, 310);
            if (!visuals) {
                Tabs("esp", { "ESP","GLOW","CHAMS" }, esp, 260);Section("Players:");BoundCheck("Team check", cs2.team);
                if (esp == 0) {
                    EspCheck("Boxes", preview.boxes, preview.colBox);
                    EspCheck("Names", preview.names, preview.colName);
                    EspCheck("Health", preview.health, preview.colHealth);
                    EspCheck("Armour", preview.armour, preview.colArmor);
                    EspCheck("Weapon", preview.weapon, preview.colWeapon);
                    EspCheck("Snaplines", preview.snaplines, preview.colSpline);
                    EspCheck("Enemy aim direction", preview.eyeRay, preview.colEyeRay);
                    if(preview.eyeRay) BindSlider("Ray length", preview.eyeRayLength, 100, 8192);
                    EspCheck("POV arrows", preview.arrows, preview.colArrows);
                    if (preview.arrows) BindSlider("Arrow distance", preview.arrowRadius, 40, 400);
                    EspCheck("Sound circles",extra.soundCircles,extra.soundCircleColor);
                    if(extra.soundCircles){
                        Bind("Sound circle team check",extra.soundCircleTeam);
                        BindSlider("Duration (s)##circles",extra.soundCircleDuration,.5f,5.f,"%.1f");
                        BindSlider("Radius##circles",extra.soundCircleRadius,8,150,"%.0f");
                        BindSlider("Thickness##circles",extra.soundCircleWidth,1,5,"%.1f");
                    }
                    EspCheck("Sound ESP", preview.sound, preview.colSound);
                    if (preview.sound) BindSlider("Duration (s)", preview.soundDuration, .5f, 15.f, "%.1f");
                    EspCheck("Head circle", preview.head, preview.colHead);
                    Section("Other:");
                    ImGui::TextUnformatted("Spread crosshair");
                    BindCombo("##spread", "Off\0Cross\0Circle\0", preview.spreadMode);
                    if (preview.spreadMode) ImGui::TextWrapped("Current weapon spread estimate");
                    EspCheck("Crosshair", preview.crosshair, preview.colCrosshair);
                    BindSlider("Max distance", cs2.dist, 50, 20000);

                }
                else if (esp == 2) {
                    BoundCheck("Enable", preview.chams);
                    BindCombo("Material##chams", "Flat\0Textured\0Metallic\0", preview.chamsMaterial);
                    BoundCheck("Visible only##chams", preview.chamsVisibleOnly);
                    colors.ColorRow("Visible", preview.color);
                    colors.ColorRow("Occluded", preview.chamsOccluded);
                    if (!chamsStatus.empty()) ImGui::TextWrapped("%s", chamsStatus.c_str());
                }
                else {
                    BoundCheck("Enable", preview.glow);
                    BindCombo("Material##glow", "Flat\0Textured\0Metallic\0", preview.glowMaterial);
                    BoundCheck("Visible only", preview.glowVisibleOnly);
                    colors.ColorRow("Visible", preview.glowVisible);
                    colors.ColorRow("Occluded", preview.glowOccluded);
                    Section("Bomb:"); EspCheck("Glow C4", preview.glowC4, preview.colC4);
                    if (!chamsStatus.empty()) ImGui::TextWrapped("%s", chamsStatus.c_str());
                }
            }
            else {
                Section("Hit feedback:");
                BoundCheck("Hitmarker", extra.hitmarker);BindSlider("Hitmarker size",extra.hitmarkerSize,1,20);
                BindCombo("Hit sound", "Off\0Click\0Double click\0",extra.hitSound);
                BoundCheck("Damage indicator",extra.damage);BoundCheck("Bullet tracer",extra.tracer);
                BoundCheck("Other players' tracers",extra.tracerOthers);BindSlider("Tracer duration",extra.tracerDuration,.1f,5,"%.1f s");
                ImGui::TextWrapped("Tracers show shot direction, not server-confirmed impacts.");
                Section("Grenades:");BoundCheck("Grenade prediction",extra.grenadePrediction);
                ImGui::TextWrapped("Estimated flight to first impact; no bounce or detonation prediction.");
                Section("Information:");BoundCheck("Watermark",extra.watermark);BoundCheck("Corner info",extra.cornerInfo);

            }
            EndPanel();Panel("visuals-right", 363, 180, 297, 310);
            if (!visuals) { Section("Preview:");DrawPlayerPreview(); }
            else {
                Section("Camera:");BoundCheck("No visual recoil",extra.noVisualRecoil);
                BoundCheck("No flash",extra.noFlash);BoundCheck("No scope zoom",extra.noScopeZoom);
                BoundCheck("Hide hands and weapon",extra.hideViewmodel);BoundCheck("No postprocessing",extra.noPostprocessing);
                BoundCheck("Override viewmodel FOV",extra.viewmodelFovOn);BindSlider("Viewmodel FOV",extra.viewmodelFov,54,120);
                BoundCheck("Override player FOV",extra.playerFovOn);BindSlider("Player FOV",extra.playerFov,60,140);
                Section("Lighting:");BoundCheck("Nightmode",extra.night);BindSlider("Exposure",extra.exposure,.05f,1,"%.2f");
                ImGui::TextWrapped("%s",extraStatus.c_str());
            }
            EndPanel();
        }
        void Misc() {
            Panel("misc-left",30,120);Section("Movement:");
            BoundCheck("Bunnyhop",cs2.bhop);keybinds.try_emplace(Id("Legit auto strafe [Space]"),extra.strafeKey);BoundCheck("Legit auto strafe [Space]",extra.autoStrafe);extra.strafeKey=keybinds[Id("Legit auto strafe [Space]")];
            if(extra.autoStrafe){
                Bind("WASD check",extra.strafeWasd);
                BindSlider("Velocity limit",extra.strafeLimit,1,255,"%.0f");

                ImGui::TextWrapped("Airborne only. Hold key (Space by default) and move mouse left/right. 1-3: standing, 3-150: walking, 150+: running.");
            }
            BoundCheck("Edge jump [v]",extra.edgeJump);extra.edgeKey=keybinds[Id("Edge jump [v]")];
            ImGui::TextWrapped("Hold Space for bunnyhop. Click the key badge to change the strafe hold key. Hold the bound key for edge jump.");
            Section("Information:");BoundCheck("Spectator list",extra.spectators);spectators=extra.spectators;
            BoundCheck("Event log",extra.eventLog);BoundCheck("Player info box",extra.playerInfo);
            BoundCheck("Show competitive ranks",extra.ranks);BoundCheck("Team check",extra.infoTeamCheck);
            ImGui::TextWrapped("Ranks are shown only when provided by the server. Events: health, death and your round damage.");
            BoundCheck("Show match info",cs2.showinfo);EndPanel();
            Panel("misc-right",363,120,297);Section("Camera:");
            keybinds.try_emplace(Id("Third person [key not set]"),extra.thirdPersonKey);
            BoundCheck("Third person [key not set]",extra.thirdPerson);
            extra.thirdPersonKey=keybinds[Id("Third person [key not set]")];
            BindSlider("Camera distance",extra.thirdPersonDistance,30,300);
            ImGui::TextWrapped("No key: always on. Bound key: hold to use. Returns to first person when scoped.");
            Section("Grenade helper:");
            BoundCheck("Enable",extra.grenadeHelper);BindCombo("Grenade","Smoke\0Flash\0HE\0Molotov\0",extra.grenadeType);
            BoundCheck("Show locations",extra.locations);BoundCheck("Show trajectory",extra.trajectory);
            BindSlider("Distance",extra.helperDistance,100,5000);
            if(ImGui::Button("Save current position"))saveGrenadeSpot=true;
            if(ImGui::Button("Clear saved positions"))clearGrenadeSpots=true;
            ImGui::TextWrapped("Personal markers for this session (up to 32). Records position, aim and grenade type; no built-in lineups. Flight estimate stops at first impact.");
            ImGui::TextWrapped("%s",extraStatus.c_str());EndPanel();
        }
        void Skins() {
            Panel("skin-left", 30, 120);Section("Weapon:");
            std::vector<std::string> names;
            std::vector<const SkinItem*> guns, knives;
            names.reserve(skinOptions.size() + 1);
            for (const auto& item : skinOptions) (item.knife ? knives : guns).push_back(&item);
            if (!knives.empty()) names.emplace_back("Knife");
            for (const auto* item : guns) names.emplace_back(item->name);
            weapon = names.empty() ? 0 : std::clamp(weapon, 0, static_cast<int>(names.size()) - 1);
            if (!names.empty()) List("skin-weapons", names, weapon, 320);
            EndPanel();
            const bool selectingKnife = !knives.empty() && weapon == 0;
            Panel("skin-right", 363, 120, 297);Section(selectingKnife ? "Knife:" : "Weapon finish:");
            if (skinOptions.empty()) { ImGui::TextUnformatted("Loading finish catalog..."); EndPanel(); return; }
            const SkinItem* selected = nullptr;
            if (selectingKnife) {
                for (const auto* item : knives) if (item->id == selectedKnifeId) selected = item;
                if (selected == nullptr) {
                    for (const auto* item : knives) if (skinConfigs.contains(item->id) && skinConfigs.at(item->id).enabled) { selected = item; break; }
                    if (selected == nullptr) {
                        for (const auto* item : knives) if (item->name == "Default Knife") { selected = item; break; }
                    }
                    if (selected == nullptr) selected = knives.front();
                    selectedKnifeId = selected->id;
                }
                ImGui::TextUnformatted("Knife type");
                ImGui::SetNextItemWidth(240);
                if (ImGui::BeginCombo("##knife-type", selected->name.c_str())) {
                    for (const auto* item : knives) {
                        ImGui::PushID(item->id);
                        if (ImGui::Selectable(item->name.c_str(), selectedKnifeId == item->id)) {
                            selectedKnifeId = item->id;
                            selected = item;
                            for (const auto* other : knives) skinConfigs[other->id].enabled = other->id == item->id;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            } else if (!guns.empty()) {
                selected = guns[weapon - (knives.empty() ? 0 : 1)];
            }
            if (selected == nullptr) { EndPanel(); return; }
            const int id = selected->id;
            auto& cfg = skinConfigs[id];
            ImGui::PushID(id);
            if (ImGui::Checkbox("Enable##skin", &cfg.enabled) && cfg.enabled && selected->knife)
                for (const auto& other : skinOptions) if (other.knife && other.id != id) skinConfigs[other.id].enabled = false;
            const SkinPaint* active = nullptr;
            for (const auto& paint : selected->paints) if (paint.id == cfg.paintKit) { active = &paint; break; }
            const char* current = active ? active->name.c_str() : (cfg.paintKit == 0 && selected->knife ? "Default finish" : "Choose finish");
            ImGui::TextUnformatted("Finish");
            ImGui::SetNextItemWidth(240);
            if (ImGui::BeginCombo("##finish", current)) {
                if (selected->knife && ImGui::Selectable("Default finish", cfg.paintKit == 0)) {
                    cfg.paintKit = 0;
                    cfg.enabled = true;
                    for (const auto& other : skinOptions)
                        if (other.knife && other.id != id) skinConfigs[other.id].enabled = false;
                }
                for (const auto& paint : selected->paints) {
                    if (ImGui::Selectable(paint.name.c_str(), cfg.paintKit == paint.id)) {
                        cfg.paintKit = paint.id;
                        cfg.enabled = true;
                        if (selected->knife)
                            for (const auto& other : skinOptions)
                                if (other.knife && other.id != id) skinConfigs[other.id].enabled = false;
                        cfg.wear = std::clamp(cfg.wear, std::max(.000001f, paint.wearMin), paint.wearMax);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted("Seed");ImGui::SetNextItemWidth(240);ImGui::InputInt("##seed", &cfg.seed);
            ImGui::TextUnformatted("Wear");ImGui::SetNextItemWidth(240);ImGui::InputFloat("##wear", &cfg.wear, .001f, .01f, "%.6f");
            ImGui::TextUnformatted("StatTrak (-1 off)");ImGui::SetNextItemWidth(240);ImGui::InputInt("##stattrak", &cfg.statTrak);
            cfg.seed = std::clamp(cfg.seed, 0, 1000);
            cfg.wear = std::clamp(cfg.wear, active ? std::max(.000001f, active->wearMin) : .000001f, active ? active->wearMax : 1.f);
            cfg.statTrak = std::clamp(cfg.statTrak, -1, 999999);
            ImGui::PopID();
            ImGui::TextWrapped("%s", skinStatus.c_str());
            EndPanel();
        }
        void Config() {
            Panel("config-left", 30, 120);Section("Config Manager:");
            auto& name = strings["profile-name"];ImGui::PushItemWidth(132);ImGui::InputText("##profile-name", name.data(), name.size());ImGui::PopItemWidth();
            ImGui::SameLine();if (ImGui::Button("Refresh", ImVec2(130, 16)))profile = std::clamp(profile, 0, static_cast<int>(profiles.size()) - 1);
            if (ImGui::Button("Add & Save", ImVec2(132, 16)) && name[0]) { if (SaveProfile(name.data())) profile = static_cast<int>(std::find(profiles.begin(), profiles.end(), name.data()) - profiles.begin()); }
            if (ImGui::Button("Delete selected config", ImVec2(132, 16)) && profiles.size() > 1) { DeleteProfile(profiles[profile]); }
            ImGui::SetCursorPos(ImVec2(160, 43));List("profiles", profiles, profile, 156, 130);
            ImGui::SetCursorPos(ImVec2(160, 202));
            if (ImGui::Button("Save", ImVec2(62, 16))) { SaveProfile(profiles[profile]); }
            ImGui::SameLine();if (ImGui::Button("Load", ImVec2(62, 16)) ) { LoadProfile(profiles[profile]); }
            ImGui::TextWrapped("%s", profileStatus.c_str());
            EndPanel();Panel("config-right", 363, 120, 297);Section("Global configs:");EndPanel();
        }
        struct Spectator { std::string name; bool thirdPerson = false; ImTextureID avatar = 0; };
        std::vector<Spectator> spectatorRows;
        // Compact reference: avatar left, nickname above viewing mode; no column headings.
        void Spectators() {
            const float width = 224, rowHeight = 46, headerHeight = 22;
            const float height = headerHeight + 4 + rowHeight * static_cast<float>((std::max)(size_t{ 1 }, spectatorRows.size()));
            ImGui::SetNextWindowPos(ImVec2(740, 36), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.f);
            ImGui::Begin("##spectator-list", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            auto p = ImGui::GetWindowPos();auto* d = ImGui::GetWindowDrawList();
            d->PushClipRect(p, ImVec2(p.x + width, p.y + height), false);
            d->AddRectFilled(p, ImVec2(p.x + width, p.y + height), Color(colors.config.windowBg));
            auto header = colors.config.accent;
            d->AddRectFilled(p, ImVec2(p.x + width, p.y + headerHeight), Color(header));
            d->AddRect(p, ImVec2(p.x + width, p.y + height), Gray(65, 145));
            d->AddLine(ImVec2(p.x, p.y + headerHeight), ImVec2(p.x + width, p.y + headerHeight), Gray(65, 145));
            const char* title = "Spectator list";const ImU32 text = Color(colors.config.text);
            const auto titleSize = ImGui::CalcTextSize(title);
            d->AddText(ImVec2(p.x + (width - titleSize.x) / 2, p.y + 4), text, title);
            for (size_t i = 0;i < spectatorRows.size();++i) {
                const auto& row = spectatorRows[i];const float y = p.y + headerHeight + 6 + rowHeight * static_cast<float>(i);
                const ImVec2 avatar(p.x + 16, y + 2), avatarEnd(p.x + 48, y + 34);
                if (row.avatar)d->AddImage(row.avatar, avatar, avatarEnd);
                else { d->AddRectFilled(avatar, avatarEnd, Color(colors.config.panelBg), 3.f);d->AddText(ImVec2(avatar.x + 12, avatar.y + 8), text, "?"); }
                d->PushClipRect(ImVec2(p.x + 60, y), ImVec2(p.x + width - 8, y + rowHeight), true);
                d->AddText(ImVec2(p.x + 64, y + 1), text, row.name.c_str());
                d->AddText(ImVec2(p.x + 64, y + 16), text, row.thirdPerson ? "3rd person" : "1st person");d->PopClipRect();
                if (i + 1 < spectatorRows.size())d->AddLine(ImVec2(p.x + 10, y + 39), ImVec2(p.x + width - 10, y + 39), Gray(65, 180));
            }
            d->PopClipRect();ImGui::End();
        }
        void Club() {
            ImGui::SetNextWindowSize(ImVec2(640, 550), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(100, 60), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_TitleBg, colors.config.accent);ImGui::PushStyleColor(ImGuiCol_TitleBgActive, colors.config.accent);
            ImGui::Begin("##members-club", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            auto origin = ImGui::GetWindowPos();auto* d = ImGui::GetWindowDrawList();
            d->AddRectFilled(origin, ImVec2(origin.x + 640, origin.y + 50), Accent(.55f));
            const char* title = "GETREKT MEMBERS' CLUB";
            d->AddText(ImVec2(origin.x + (640 - ImGui::CalcTextSize(title).x) / 2, origin.y + 19), Color(colors.config.text), title);
            ImGui::SetCursorPos(ImVec2(28, 80));
            ImGui::BeginChild("club-content", ImVec2(584, 442), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::TextUnformatted("Users online: demo");
            ImGui::TextUnformatted("Content will refresh in: 00:04");
            if (ImGui::Button("Force update")) {};
            Check("Replace friends with GETREKT' MEMBERS");
            ImGui::TextColored(ImVec4(.8f, .18f, .15f, 1), "More features for GETREKT CLUB like: messages, info about");
            ImGui::TextColored(ImVec4(.8f, .18f, .15f, 1), "current connected server, profile picture and much more coming soon!");
            Section("Change log:");
            ImGui::BeginChild("historical-changelog", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            struct Row { const char* version;const char* type;const char* text; };
            const Row rows[] = {
                { "Version 1.4.6","ADD","Player info box" },{ "","ADD","Grief box" },{ "","ADD","Run boost" },{ "","ADD","Overwatch revealer" },{ "","ADD","Clantag stealer" },{ "","ADD","Event log" },{ "","ADD","Spread cross" },{ "","FIX","Name stealer \"GOTV\" ignored" },
                { "Version 1.4.5","UPDATE","CSGO update" },{ "","ADD","Name stealer" },{ "","ADD","98 Nade" },{ "","ADD","Name/Clan changer ADD gamesense animation style" },{ "","CHANGED","Name/Clan changer synced" },{ "","CHANGED","Clantag animation \"___\" chars changed" },{ "","FIX","Skinchanger filter out correctly M4A4 skins" } };
            ImGui::Columns(3, "change-columns", false);ImGui::SetColumnOffset(1, 100);ImGui::SetColumnOffset(2, 180);
            for (const auto& row : rows) {
                ImGui::TextUnformatted(row.version);ImGui::NextColumn();auto p = ImGui::GetCursorScreenPos();
                ImU32 color = strcmp(row.type, "ADD") == 0 ? IM_COL32(35, 125, 40, 255) : strcmp(row.type, "FIX") == 0 ? IM_COL32(35, 100, 235, 255) : strcmp(row.type, "UPDATE") == 0 ? IM_COL32(155, 45, 200, 255) : IM_COL32(255, 125, 55, 255);
                ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 76, p.y + 19), color);
                ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + 5, p.y + 2), IM_COL32(255, 255, 255, 255), row.type);ImGui::Dummy(ImVec2(76, 19));ImGui::NextColumn();ImGui::TextUnformatted(row.text);ImGui::NextColumn();
            }
            ImGui::Columns(1);ImGui::EndChild();ImGui::EndChild();ImGui::End();ImGui::PopStyleColor(2);
        }
        void Draw(bool panic) {
            disabled = panic;colors.Apply();auto& style = ImGui::GetStyle();style.ItemSpacing = ImVec2(4, 2);style.FramePadding = ImVec2(2, 1);style.GrabMinSize = 9;style.ScrollbarSize = 6;
            style.Colors[ImGuiCol_FrameBg] = Light() ? ImVec4(.78f, .78f, .78f, 1) : ImVec4(.06f, .06f, .06f, 1);
            ImGui::SetNextWindowSize(ImVec2(690, 520), ImGuiCond_Always);ImGui::SetNextWindowPos(ImVec2(24, 36), ImGuiCond_FirstUseEver);
            colors.BeginMenu("##video-reference", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | (panic ? ImGuiWindowFlags_NoInputs : 0));
            auto p = ImGui::GetWindowPos();auto* d = ImGui::GetWindowDrawList();d->PushClipRect(p, ImVec2(p.x + 690, p.y + 520), false);
            d->AddRectFilled(ImVec2(p.x, p.y + 92), ImVec2(p.x + 690, p.y + 520), Color(colors.config.windowBg));
            d->AddRect(ImVec2(p.x + 4, p.y + 95), ImVec2(p.x + 686, p.y + 516), Gray(5, 165));
            d->AddRect(ImVec2(p.x + 1, p.y + 92), ImVec2(p.x + 689, p.y + 519), IM_COL32(65, 63, 59, 255));
            d->AddRectFilled(ImVec2(p.x, p.y + 16), ImVec2(p.x + 690, p.y + 58), Accent(.55f));
            d->AddRectFilledMultiColor(ImVec2(p.x, p.y + 58), ImVec2(p.x + 690, p.y + 92), Gray(47, 225), Gray(47, 225), Gray(30, 205), Gray(30, 205));
            const float captionWidth = ImGui::CalcTextSize("GetRekt Club").x + 6;
            d->AddRectFilled(p, ImVec2(p.x + captionWidth, p.y + 16), Accent());d->AddText(ImVec2(p.x + 3, p.y + 2), IM_COL32(255, 255, 255, 255), "GetRekt Club");
            auto title = ImGui::CalcTextSize("GetRekt");d->AddText(ImVec2(p.x + (690 - title.x) / 2, p.y + 31), IM_COL32(255, 255, 255, 255), "GetRekt");d->PopClipRect();
            ImGui::SetCursorPos(ImVec2(0, 0));if (ImGui::InvisibleButton("caption", ImVec2(captionWidth, 16)))clubWindow = !clubWindow;
            ImGui::SetCursorPos(ImVec2(0, 16));ImGui::InvisibleButton("drag", ImVec2(690, 42));if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) { auto delta = ImGui::GetIO().MouseDelta;ImGui::SetWindowPos(ImVec2(std::clamp(p.x + delta.x, 0.f, (std::max)(0.f, ImGui::GetIO().DisplaySize.x - 690)), std::clamp(p.y + delta.y, 0.f, (std::max)(0.f, ImGui::GetIO().DisplaySize.y - 520)))); }
            ImGui::SetCursorPos(ImVec2(0, 58));Tabs("main", { "ANTI-AIM","LEGIT","VISUALS","MISC","SKIN-CHANGER","CONFIG" }, page, 690, true);
            if (page == 0)AntiAim();else if (page == 1)Legit();else if (page == 2)Visuals();else if (page == 3)Misc();else if (page == 4)Skins();else Config();

            ImGui::End();
            if (clubWindow && !panic)Club();
            if (spectators && !panic)Spectators();
            if (page == 1 && legit == 1 && weaponWindow && !panic) WeaponConfigWindow(p);
        }
    };
}
