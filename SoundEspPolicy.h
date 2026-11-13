#pragma once

namespace SoundEspPolicy {
inline bool ShowPlayer(bool enabled, bool lineOfSight, bool heardRecently) {
    return !enabled || lineOfSight || heardRecently;
}
}
