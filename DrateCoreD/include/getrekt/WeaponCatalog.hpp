#pragma once
#include <array>
namespace getrekt::Weapons {
struct Entry { int id; const char* name; int category; };
inline constexpr std::array<Entry,34> All{{
    {1,"Deagle",0},
    {2,"Elite",0},
    {3,"Five-SeveN",0},
    {4,"Glock",0},
    {30,"Tec-9",0},
    {32,"P2000",0},
    {36,"P250",0},
    {61,"USP-S",0},
    {63,"CZ75",0},
    {64,"R8",0},
    {7,"AK-47",1},
    {8,"AUG",1},
    {10,"FAMAS",1},
    {13,"Galil",1},
    {16,"M4A4",1},
    {39,"SG 553",1},
    {60,"M4A1-S",1},
    {9,"AWP",2},
    {11,"G3SG1",2},
    {38,"SCAR-20",2},
    {40,"SSG 08",2},
    {17,"MAC-10",3},
    {19,"P90",3},
    {23,"MP5-SD",3},
    {24,"UMP-45",3},
    {26,"Bizon",3},
    {33,"MP7",3},
    {34,"MP9",3},
    {14,"M249",4},
    {28,"Negev",4},
    {25,"XM1014",5},
    {27,"MAG-7",5},
    {29,"Sawed-Off",5},
    {35,"Nova",5},
}};
inline const Entry* Find(int id) { for (const auto& w : All) if (w.id == id) return &w; return nullptr; }
inline int Category(int id) { auto w = Find(id); return w ? w->category : 1; }
inline const char* Name(int id) { auto w = Find(id); return w ? w->name : ""; }
}
