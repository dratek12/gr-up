#pragma once
#include <imgui.h>
#include <cmath>
inline bool PlayerBounds(ImVec2 head, ImVec2 feet, ImVec2& lo, ImVec2& hi) {
    const float height = feet.y - head.y;
    if (!std::isfinite(height) || height < 2.f || !std::isfinite(head.x) || !std::isfinite(feet.x)) return false;
    const float cx = (head.x + feet.x) * .5f;
    const float halfWidth = height * .225f;
    lo = ImVec2(cx - halfWidth, head.y);
    hi = ImVec2(cx + halfWidth, feet.y);
    return true;
}

// The menu distance is in world units, matching the positions supplied by SDK.
template<class V> inline bool WithinEspDistance(const V& a, const V& b, float maxDistance) {
    if (!std::isfinite(maxDistance) || maxDistance < 0.f) return false;
    const double x=double(a.x)-b.x, y=double(a.y)-b.y, z=double(a.z)-b.z;
    return x*x+y*y+z*z <= double(maxDistance)*maxDistance;
}
