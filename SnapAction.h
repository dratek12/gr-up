#pragma once
#include <cstdint>
struct SnapAction {
    bool down=false,pending=false;uint32_t target=0xFFFFFFFFu;double alignedAt=-1;
    uint32_t locked=0xFFFFFFFFu;int lockedBone=0;
    bool Accept(uint32_t id,int bone)const{return locked==0xFFFFFFFFu||(locked==id&&lockedBone==bone);}
    void Lock(uint32_t id,int bone){if(pending&&locked==0xFFFFFFFFu){locked=id;lockedBone=bone;}}
    void Cancel(){locked=0xFFFFFFFFu;lockedBone=0;pending=false;alignedAt=-1;target=0xFFFFFFFFu;}
    void Suspend(){Cancel();down=true;}
    bool Arm(bool allowed,bool key){
        if(!allowed||!key)Cancel();else if(!down)pending=true;
        down=key;return pending;
    }
    bool Ready(bool aligned,uint32_t id,double now,double delay){
        if(!pending||!aligned){alignedAt=-1;target=id;return false;}
        if(target!=id||alignedAt<0||now<alignedAt){target=id;alignedAt=now;}
        return now-alignedAt>=delay;
    }
};
