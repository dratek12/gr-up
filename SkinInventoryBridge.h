#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace SkinInventoryBridge {
struct Config {
    int paint=0, seed=0, statTrak=-1;
    float wear=.0001f;
    bool operator==(const Config&) const = default;
};
struct PaintOption { int id{}; std::string name; float wearMin{}, wearMax{1}; };
struct ItemOption { int id{}; std::string name; bool knife{}; std::vector<PaintOption> paints; };

bool Start();
void Stop();
bool Ready();
bool Sync(const std::unordered_map<int,Config>& desired);
std::vector<ItemOption> Catalog();
}
