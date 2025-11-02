#ifndef ZCAMERA_H
#define ZCAMERA_H

#include "zCamSB.h"

enum WallJumpViewState
{
    WJVS_DISABLED,
    WJVS_DISABLING,
    WJVS_ENABLED,
    WJVS_ENABLING
};

enum camera_owner_enum
{
    CO_BOULDER = 0x1,
    CO_CRUISE_BUBBLE,
    CO_BUNGEE = 0x4,
    CO_BOSS = 0x8,
    CO_OOB = 0x10,
    CO_ZIPLINE = 0x20,
    CO_TURRET = 0x40,
    CO_REWARDANIM = 0x80
};

struct zFlyKey
{
    S32 frame;
    F32 matrix[12];
    F32 aperture[2];
    F32 focal;
};

struct zCamTweakLook
{
    F32 h;
    F32 dist;
    F32 pitch;
};

// unknown if these are even used?
extern xCamera zcam_backupcam;
extern xCamera zcam_backupconvers;
extern xQuat zcam_quat;

// .sbss
F32 zcam_overrot_tmr;
S32 zcam_near;
U8 zcam_centering;
U8 zcam_lastcentering;
S32 zcam_mode;
S32 zcam_bbounce;
S32 zcam_lbbounce;
S32 zcam_convers;
S32 zcam_lconvers;
S32 zcam_longbounce;
S32 zcam_highbounce;
S32 zcam_cutscene;
S32 zcam_reward;
xVec3* zcam_playervel;
S32 zcam_fly;
S32 zcam_flypaused;
void* zcam_flydata;
U32 zcam_flysize;
F32 zcam_flytime;
F32 zcam_flyrate;
S32 zcam_flyJustStopped;
xCamAsset* zcam_dest;
F32 zcam_tmr;
F32 zcam_ttm;
F32 zcam_fovcurr;
F32 zcam_fovdest;
U32 zcam_flyasset_current;
S32 sCamTweakCount;
// .sdata

F32 zcam_pad_pyaw_scale = 0.18124573f; // unknown math
F32 zcam_pad_pitch_scale = 1.0f / 52;
F32 zcam_near_d = 3.0f;
F32 zcam_near_h = 1.8f;
F32 zcam_near_pitch = DEG2RAD(10);
F32 zcam_far_d = 5.0f;
F32 zcam_far_h = 3.0f;
F32 zcam_far_pitch = DEG2RAD(15);
F32 zcam_wall_d = 7.5f;
F32 zcam_wall_h = 2.0f;
F32 zcam_wall_pitch = DEG2RAD(18);
F32 zcam_above_d = 0.2f;
F32 zcam_above_h = 2.2f;
F32 zcam_above_pitch = DEG2RAD(70);
F32 zcam_below_d = 0.6f;
F32 zcam_below_h = 0.2f;
F32 zcam_below_pitch = DEG2RAD(-70);
F32 zcam_highbounce_d = 0.2f;
F32 zcam_highbounce_h = 5.0f;
F32 zcam_highbounce_pitch = DEG2RAD(89);
F32 zcam_overrot_min = DEG2RAD(25);
F32 zcam_overrot_mid = DEG2RAD(90);
F32 zcam_overrot_max = DEG2RAD(170);
F32 zcam_overrot_rate = 0.1f;
F32 zcam_overrot_tstart = 1.5f;
F32 zcam_overrot_tend = 2.5f;
F32 zcam_overrot_velmin = 3.0f;
F32 zcam_overrot_velmax = 5.0f;
F32 zcam_overrot_tmanual = 1.5f;
F32 zcam_mintgtheight = FLOAT_MIN;

bool sNearToggleEnabled;
F32 sCamTweakLerp;
F32 sCamTweakTime;
F32 sCamTweakPitchCur;
F32 sCamTweakPitch[2];
F32 sCamTweakDistMultCur;
F32 sCamTweakDistMult[2];
static zCamTweakLook zcam_neartweak;
static zCamTweakLook zcam_fartweak;
F32 sCamD;
F32 sCamH;
F32 sCamPitch;

void zCameraReset(xCamera* cam);
void zCameraSetBbounce(S32 bbouncing);
void zCameraSetHighbounce(S32 hbounce);
void zCameraSetLongbounce(S32 hbounce);
void zCameraSetPlayerVel(xVec3* vel);
void zCameraSetConvers(int on);
void zCameraDoTrans(xCamAsset* asset, float ttime);
U32 zCamera_FlyOnly();
void zCameraTranslate(xCamera* camera, F32 x, F32 y, F32 z);
void zCameraReset(xCamera* camera);
void zCameraUpdate(xCamera* camera, F32 dt);

void zCameraEnableInput();
void zCameraDisableInput();
U32 zCameraIsTrackingDisabled();
void zCameraEnableTracking(camera_owner_enum owner);
void zCameraDisableTracking(camera_owner_enum owner);
void zCameraDisableLassoCam();
void zCameraEnableLassoCam();
void zCameraSetLassoCamFactor(F32 new_factor);
F32 zCameraGetLassoCamFactor();
void zCameraEnableWallJump(xCamera* cam, const xVec3& collNormal);
S32 zCameraGetConvers();
void zCameraTranslate(xCamera* cam, F32 x, F32 y, F32 z);
void zCameraDisableWallJump(xCamera* cam);
void zCameraSetReward(S32 reward);
void zCameraMinTargetHeightSet(F32 min_height);
void zCameraMinTargetHeightClear();

#endif
