#pragma once
#include <imgui.h>
namespace getrekt {
struct ExtraOptions {
    bool noFlash=false, noVisualRecoil=false, noScopeZoom=false;
    bool viewmodelFovOn=false, playerFovOn=false, night=false;
    float viewmodelFov=68, playerFov=90, exposure=.25f;
    bool watermark=false, cornerInfo=false, hitmarker=false, damage=false;
    bool tracer=false, tracerOthers=false, grenadePrediction=false;
    float hitmarkerSize=5, tracerDuration=1;
    int hitSound=0;
    bool autoStrafe=false, edgeJump=false, spectators=false, eventLog=false;
    bool playerInfo=false, ranks=false, infoTeamCheck=false;
    int edgeKey='V';
    bool grenadeHelper=false, locations=false, trajectory=false;
    float helperDistance=1000;
    int grenadeType=0;
    bool hideViewmodel=false, noPostprocessing=false;
    bool thirdPerson=false;
    float thirdPersonDistance=150;
    int thirdPersonKey=0;
    bool strafeWasd=true;
    float strafeLimit=3;
    int strafeKey=32;
    bool soundCircles=false, soundCircleTeam=true;
    float soundCircleDuration=2, soundCircleRadius=48, soundCircleWidth=2;
    ImVec4 soundCircleColor{.2f,1.f,.35f,1.f};
};
}
