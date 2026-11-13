#pragma once
#include "SDK.h"
namespace AimMotion {
struct Curve {
    Vec3 velocity{};
    void Reset(){velocity={};}
    Vec3 Step(Vec3 error,float smooth,float fov,float dt,float tolerance){
        const float distance=std::hypot(error.x,error.y);
        if(!std::isfinite(distance)||!std::isfinite(smooth)||!std::isfinite(fov)||!std::isfinite(dt)||
           !std::isfinite(tolerance)||dt<=0||fov<=0||tolerance<0||distance<=tolerance){Reset();return {};}
        dt=std::min(dt,.05f);smooth=std::clamp(smooth,1.f,30.f);
        const float t=std::clamp(distance/fov,0.f,1.f);
        const float gain=.4f+.6f*t*t*(3-2*t);
        const float fraction=1-std::exp(-dt*gain/(smooth*.035f));
        const float maxSpeed=fov/(.12f+smooth*.045f);
        Vec3 desired{error.x*fraction/dt,error.y*fraction/dt,0};
        const float speed=std::hypot(desired.x,desired.y);
        if(speed>maxSpeed){desired.x*=maxSpeed/speed;desired.y*=maxSpeed/speed;}
        if(velocity.x*error.x+velocity.y*error.y<0)Reset();
        Vec3 change=desired-velocity;const float changeLength=std::hypot(change.x,change.y);
        const float limit=maxSpeed/.08f*dt;
        if(changeLength>limit){change.x*=limit/changeLength;change.y*=limit/changeLength;}
        velocity=velocity+change;
        Vec3 move{std::clamp(velocity.x*dt,std::min(0.f,error.x),std::max(0.f,error.x)),
                  std::clamp(velocity.y*dt,std::min(0.f,error.y),std::max(0.f,error.y)),0};
        velocity={move.x/dt,move.y/dt,0};return move;
    }
};
struct Counts {
    float x=0,y=0;
    void Reset(){x=y=0;}
    ImVec2 Step(ImVec2 move){
        if(!std::isfinite(move.x)||!std::isfinite(move.y)){Reset();return {};}
        if(move.x*x<0)x=0;if(move.y*y<0)y=0;
        x+=std::clamp(move.x,-10000.f,10000.f);y+=std::clamp(move.y,-10000.f,10000.f);
        const ImVec2 whole{std::trunc(x),std::trunc(y)};x-=whole.x;y-=whole.y;return whole;
    }
};
}
