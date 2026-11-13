#pragma once
#include <imgui.h>
#include <cmath>
#include <algorithm>
struct SpreadGeometry {
    ImVec2 center{};
    float radius=0;
};
inline bool MakeSpreadGeometry(const float* matrix, ImVec2 size, float cone, SpreadGeometry& result) {
    result={};
    if(!matrix || !std::isfinite(size.x) || !std::isfinite(size.y) || size.x<=0 || size.y<=0 ||
        !std::isfinite(cone) || cone<0 || cone>2)return false;
    const float scale=std::sqrt(matrix[4]*matrix[4]+matrix[5]*matrix[5]+matrix[6]*matrix[6])*size.y*.5f;
    if(!std::isfinite(scale) || scale<=0)return false;
    result.center={size.x*.5f,size.y*.5f};
    result.radius=std::clamp(cone*scale,2.f,std::max(2.f,size.y*.4f));
    return true;
}
