#pragma once
namespace ClientConVars {
bool Float(const char* name,bool enabled,float value);
bool Bool(const char* name,bool enabled,bool value);
void Reset();
}
