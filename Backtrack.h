#pragma once
#include "SDK.h"
#include <span>

void BtReset();
void BtRecord();
void BtRecordAt(std::span<const CPlayer> players, uint64_t now);
bool BtApply(CPlayer& player, float latencyMs);
bool BtApplyAt(CPlayer& player, float latencyMs, uint64_t now);
