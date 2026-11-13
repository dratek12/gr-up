#pragma once
#include <cstdint>
// Kept separate from drawing so the same reader can be checked without a renderer.
void UpdatePlayers(uintptr_t list, uintptr_t localPawn, uintptr_t localController);
void UpdatePlayers();
