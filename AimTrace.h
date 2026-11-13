#pragma once
#include "Global.h"
#include <cstdio>
#include <filesystem>

// Opt-in live diagnostics. No file is opened unless aim-trace.enabled is present
// beside the DLL. Buffered output avoids a disk flush on every render frame.
namespace AimTrace {
inline int inputQuery=0;
inline FILE* File() {
    static FILE* file=[]()->FILE* {
        wchar_t name[32768]{};
        if(!GetModuleFileNameW(G::SelfModule,name,DWORD(std::size(name))))return nullptr;
        const auto folder=std::filesystem::path(name).parent_path();
        if(!std::filesystem::exists(folder/L"aim-trace.enabled"))return nullptr;
        auto f=_wfopen((folder/L"aim-trace.csv").c_str(),L"w");
        if(f)std::fputs("ms,stage,allow,query,target,errorX,errorY,dt,dx,dy\n",f);
        return f;
    }();
    return file;
}
struct Frame {
    const char* stage="initial"; bool allow=false; unsigned target=0;
    float x=0,y=0,dt=0;int dx=0,dy=0;
    ~Frame(){
        if(auto f=File()){
            const auto now=GetTickCount64();
            std::fprintf(f,"%llu,%s,%d,%d,%u,%.6f,%.6f,%.6f,%d,%d\n",now,stage,allow,inputQuery,target,x,y,dt,dx,dy);
            static uint64_t flushed=0;
            if(now-flushed>=1000){std::fflush(f);flushed=now;}
        }
    }
};
}
