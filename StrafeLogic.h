#pragma once
#include <cmath>
namespace StrafeLogic {
inline int Direction(bool enabled,bool alive,bool ground,bool held,bool wasdCheck,bool moving,float velocity,float limit,int dx){
    if(!enabled||!alive||ground||!held||(wasdCheck&&moving)||!std::isfinite(velocity)||!std::isfinite(limit)||velocity<limit)return 0;
    return dx>1?1:dx< -1?-1:0;
}
}
