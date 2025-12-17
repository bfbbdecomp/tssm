#include "xpkrsvc.h"
#include "xScrFx.h"
#include "xstransvc.h"
#include "zCamera.h"
#include "zCameraTweak.h"
#include "zGlobals.h"
#include "zMain.h"

namespace
{
    WallJumpViewState wall_jump_enabled;
    bool lassocam_enabled;
    F32 lassocam_factor;
    U32 stop_track;
    bool input_enabled = true;
    F32 dMultiplier;
    F32 dOffset;
    F32 hMultiplier;
    F32 hOffset;
    zCamSB* follow_cam;
    xVec3 wall_jump_view;
} // namespace

void zCameraMinTargetHeightClear()
{
    zcam_mintgtheight = FLOAT_MIN;
}

void zCameraMinTargetHeightSet(F32 min_height)
{
    zcam_mintgtheight = min_height;
}

void zCameraSetReward(S32 on)
{
    if (stop_track != 0)
    {
        zcam_reward = 0;
        return;
    }
    zcam_reward = on;
}

void zCameraDisableWallJump(xCamera* cam)
{
    if (wall_jump_enabled != WJVS_DISABLED)
    {
        wall_jump_enabled = WJVS_DISABLING;
    }
}

void zCameraEnableWallJump(xCamera* cam, const xVec3& collNormal)
{
    if (wall_jump_enabled != WJVS_ENABLED)
    {
        wall_jump_enabled = WJVS_ENABLING;
    }

    xVec3 up = { 0.0f, 0.0f, 0.0f };

    // Missing inline?
    xVec3Cross(&wall_jump_view, &collNormal, &up);
    xVec3Normalize(&wall_jump_view, &wall_jump_view);

    if (xVec3Dot(&wall_jump_view, &globals.oldSkoolCamera.mat.at) < 0.0f)
    {
        // Missing inline?
        xVec3Sub(&wall_jump_view, &g_O3, &wall_jump_view);
    }
}

void zCameraTranslate(xCamera* cam, F32 x, F32 y, F32 z)
{
    cam->mat.pos.x += x;
    cam->mat.pos.y += y;
    cam->mat.pos.z += z;
    cam->tran_accum.x += x;
    cam->tran_accum.y += y;
    cam->tran_accum.z += z;
}

S32 zCameraGetConvers()
{
    return zcam_convers;
}

F32 zCameraGetLassoCamFactor()
{
    return lassocam_factor;
}

void zCameraSetLassoCamFactor(F32 new_factor)
{
    lassocam_factor = new_factor;
}

void zCameraEnableLassoCam()
{
    lassocam_enabled = 1;
}

void zCameraDisableLassoCam()
{
    lassocam_enabled = 0;
}

void zCameraEnableInput()
{
    input_enabled = 1;
}

void zCameraDisableInput()
{
    input_enabled = 0;
}

U32 zCameraIsTrackingDisabled()
{
    return stop_track;
}

void zCameraEnableTracking(camera_owner_enum owner)
{
    stop_track &= ~owner;
}

void zCameraDisableTracking(camera_owner_enum owner)
{
    stop_track |= owner;
}

void zCameraSetPlayerVel(xVec3* vel)
{
    zcam_playervel = vel;
}

void zCameraSetHighbounce(S32 lbounce)
{
    if (zcam_longbounce != 0 || zcam_highbounce != lbounce)
    {
        zcam_lbbounce = 0;
    }

    zcam_highbounce = lbounce;
    zcam_longbounce = 0;
}

void zCameraSetLongbounce(S32 lbounce)
{
    if (zcam_highbounce != 0 || zcam_longbounce != lbounce)
    {
        zcam_lbbounce = 0;
    }

    zcam_longbounce = lbounce;
    zcam_highbounce = 0;
}

void zCameraSetBbounce(S32 bbouncing)
{
    zcam_bbounce = bbouncing;
}

void zCameraUpdate(xCamera* camera, F32 dt)
{
    zcam_near &= 0x1;

    // Missing inline?
    zCameraTweakGlobal_Update(dt);
}

S32 zCameraIsFlyCamRunning()
{
    return zcam_fly;
}

void zCameraFreeLookSetGoals(xCamera* cam, F32 pitch_s, F32& dgoal, F32& hgoal, F32& pitch_goal,
                             F32& lktm, F32 dt)
{
    // from the dwarf
    F32 s;
    // ghidra generated
    F32 fVar2;
    F32 fVar3;
    F32 fVar5;

    if (zcam_bbounce == 0)
    {
        s = zcam_highbounce_d;
        if ((zcam_highbounce == 0) && (s = sCamD, wall_jump_enabled == WJVS_ENABLED))
        {
            s = zcam_wall_d;
        }
        fVar2 = dMultiplier * s + dOffset + 0.0;
        s = zcam_highbounce_h;
        if ((zcam_highbounce == 0) && (s = sCamH, wall_jump_enabled == WJVS_ENABLED))
        {
            s = zcam_wall_h;
        }
        fVar3 = dMultiplier * s + dOffset + 0.0;
        s = sCamPitch;
        if (zcam_highbounce != 0)
        {
            s = zcam_highbounce_pitch;
        }
        if ((lassocam_enabled == '\0') || (stop_track != 0))
        {
            if (0.0 < pitch_s)
            {
                dgoal = pitch_s * (zcam_below_d - fVar2) + fVar2 + 0.0;
                hgoal = pitch_s * (zcam_below_h - fVar3) + fVar3 + 0.0;
                pitch_goal = pitch_s * pitch_s * pitch_s * (zcam_below_pitch - s) + s + 0.0;
            }
            else
            {
                fVar5 = -pitch_s;
                dgoal = fVar5 * (zcam_above_d - fVar2) + fVar2 + 0.0;
                hgoal = fVar5 * (zcam_above_h - fVar3) + fVar3 + 0.0;
                pitch_goal = fVar5 * (zcam_above_pitch - s) + s + 0.0;
            }
            if (0.1 < lktm)
            {
                s = lktm;
                lktm = s - dt;
                if (s - dt < 0.1)
                {
                    lktm = 0.1;
                }
            }
            else
            {
                lktm = 0.1;
            }
        }
        else
        {
            dgoal = lassocam_factor * (fVar2 - zcam_near_d) + zcam_near_d + 0.0;
            hgoal = lassocam_factor * (fVar3 - zcam_near_h) + zcam_near_h + 0.0;
            pitch_goal = lassocam_factor * (s - zcam_near_pitch) + zcam_near_pitch + 0.0;
        }
    }
    else if (zcam_highbounce == 0)
    {
        if (zcam_near == 0)
        {
            s = sCamD;
            if (wall_jump_enabled == WJVS_ENABLED)
            {
                s = zcam_wall_d;
            }
            dgoal = dMultiplier * s + dOffset + 0.0;
        }
        else
        {
            dgoal = 3.5;
        }
        if (zcam_near == 0)
        {
            s = zcam_highbounce_h;
            if ((zcam_highbounce == 0) && (s = sCamH, wall_jump_enabled == WJVS_ENABLED))
            {
                s = zcam_wall_h;
            }
            hgoal = dMultiplier * s + dOffset + 0.0;
        }
        else
        {
            hgoal = 2.4;
        }
        if (zcam_longbounce == 0)
        {
            if (zcam_near == 0)
            {
                pitch_goal = 0.5235988;
            }
            else
            {
                pitch_goal = 0.6981317;
            }
        }
        else
        {
            fVar5 = zcam_playervel->y;
            bool bVar1 = zcam_playervel != 0x0;
            fVar2 = zcam_playervel->x;
            fVar3 = zcam_playervel->z;
            s = sqrt(fVar3 * fVar3 + fVar2 * fVar2 + fVar5 * fVar5);
            if ((bVar1) && (bVar1 = true, s == 0.0))
            {
                bVar1 = false;
            }
            if (bVar1)
            {
                s = ((cam->mat).at.z * fVar3 + (cam->mat).at.x * fVar2 + (cam->mat).at.y * fVar5) /
                    s;
                if (0.0 < s)
                {
                    s = -0.0;
                }
                else
                {
                    s = -s;
                }
            }
            else
            {
                s = 0.0;
            }
            if (zcam_near == 0)
            {
                s = 0.5235988;
            }
            else
            {
                s = (s * 20.0 + 20.0) * 0.017453292;
            }
            pitch_goal = s;
        }
    }
    else
    {
        s = zcam_highbounce_d;
        if ((zcam_highbounce == 0) && (s = sCamD, wall_jump_enabled == WJVS_ENABLED))
        {
            s = zcam_wall_d;
        }
        dgoal = dMultiplier * s + dOffset + 0.0;
        s = zcam_highbounce_h;
        if ((zcam_highbounce == 0) && (s = sCamH, wall_jump_enabled == WJVS_ENABLED))
        {
            s = zcam_wall_h;
        }
        hgoal = dMultiplier * s + dOffset + 0.0;
        s = sCamPitch;
        if (zcam_highbounce != 0)
        {
            s = zcam_highbounce_pitch;
        }
        pitch_goal = s;
    }
    return;
}

// WIP
static void zCameraFlyRestoreBackup(xCamera* backup)
{
    globals.oldSkoolCamera.mat = backup->mat;
    globals.oldSkoolCamera.omat = backup->omat;
    globals.oldSkoolCamera.mbasis = backup->mbasis;
    globals.oldSkoolCamera.bound = backup->bound;
    globals.oldSkoolCamera.focus = backup->focus;

    globals.oldSkoolCamera.flags = backup->flags;
    globals.oldSkoolCamera.tmr = backup->tmr;
    globals.oldSkoolCamera.tm_acc = backup->tm_acc;
    globals.oldSkoolCamera.tm_dec = backup->tm_dec;
    globals.oldSkoolCamera.ltmr = backup->ltmr;
    globals.oldSkoolCamera.ltm_acc = backup->ltm_acc;
    globals.oldSkoolCamera.ltm_dec = backup->ltm_dec;
    globals.oldSkoolCamera.dmin = backup->dmin;
    globals.oldSkoolCamera.dmax = backup->dmax;
    globals.oldSkoolCamera.dcur = backup->dcur;
    globals.oldSkoolCamera.dgoal = backup->dgoal;
    globals.oldSkoolCamera.hmin = backup->hmin;
    globals.oldSkoolCamera.hmax = backup->hmax;
    globals.oldSkoolCamera.hcur = backup->hcur;
    globals.oldSkoolCamera.hgoal = backup->hgoal;
    globals.oldSkoolCamera.pmin = backup->pmin;
    globals.oldSkoolCamera.pmax = backup->pmax;
    globals.oldSkoolCamera.pcur = backup->pcur;
    globals.oldSkoolCamera.pgoal = backup->pgoal;
    globals.oldSkoolCamera.depv = backup->depv;
    globals.oldSkoolCamera.hepv = backup->hepv;
    globals.oldSkoolCamera.pepv = backup->pepv;
    globals.oldSkoolCamera.orn_epv = backup->orn_epv;
    globals.oldSkoolCamera.yaw_epv = backup->yaw_epv;
    globals.oldSkoolCamera.pitch_epv = backup->pitch_epv;
    globals.oldSkoolCamera.roll_epv = backup->roll_epv;
    globals.oldSkoolCamera.orn_cur = backup->orn_cur;
    globals.oldSkoolCamera.orn_goal = backup->orn_goal;
    globals.oldSkoolCamera.orn_diff = backup->orn_diff;
    globals.oldSkoolCamera.yaw_cur = backup->yaw_cur;
    globals.oldSkoolCamera.yaw_goal = backup->yaw_goal;
    globals.oldSkoolCamera.pitch_cur = backup->pitch_cur;
    globals.oldSkoolCamera.pitch_goal = backup->pitch_goal;
    globals.oldSkoolCamera.roll_cur = backup->roll_cur;
    globals.oldSkoolCamera.roll_goal = backup->roll_goal;
    globals.oldSkoolCamera.dct = backup->dct;
    globals.oldSkoolCamera.dcd = backup->dcd;
    globals.oldSkoolCamera.dccv = backup->dccv;
    globals.oldSkoolCamera.dcsv = backup->dcsv;
    globals.oldSkoolCamera.hct = backup->hct;
    globals.oldSkoolCamera.hcd = backup->hcd;
    globals.oldSkoolCamera.hccv = backup->hccv;
    globals.oldSkoolCamera.hcsv = backup->hcsv;
    globals.oldSkoolCamera.pct = backup->pct;
    globals.oldSkoolCamera.pcd = backup->pcd;
    globals.oldSkoolCamera.pccv = backup->pccv;
    globals.oldSkoolCamera.pcsv = backup->pcsv;
    globals.oldSkoolCamera.orn_ct = backup->orn_ct;
    globals.oldSkoolCamera.orn_cd = backup->orn_cd;
    globals.oldSkoolCamera.orn_ccv = backup->orn_ccv;
    globals.oldSkoolCamera.orn_csv = backup->orn_csv;
    globals.oldSkoolCamera.yaw_ct = backup->yaw_ct;
    globals.oldSkoolCamera.yaw_cd = backup->yaw_cd;
    globals.oldSkoolCamera.yaw_ccv = backup->yaw_ccv;
    globals.oldSkoolCamera.yaw_csv = backup->yaw_csv;
    globals.oldSkoolCamera.pitch_ct = backup->pitch_ct;
    globals.oldSkoolCamera.pitch_cd = backup->pitch_cd;
    globals.oldSkoolCamera.pitch_ccv = backup->pitch_ccv;
    globals.oldSkoolCamera.pitch_csv = backup->pitch_csv;
    globals.oldSkoolCamera.roll_ct = backup->roll_ct;
    globals.oldSkoolCamera.roll_cd = backup->roll_cd;
    globals.oldSkoolCamera.roll_ccv = backup->roll_ccv;
    globals.oldSkoolCamera.roll_csv = backup->roll_csv;
}

void zCameraFlyStart(U32 assetID)
{
    PKRAssetTOCInfo info;
    if (xSTGetAssetInfo(assetID, &info) == 0)
    {
        return;
    }

    follow_cam->cfg_common.priority = '\x7f';
    zcam_fly = 1;
    zcam_flypaused = 0;
    zcam_flydata = info.mempos;
    zcam_flysize = info.size;
    zcam_flytime = 0.033333335f;
    zcam_flyasset_current = assetID;
    zcam_flyrate = 1.0f;

    zEntPlayerControlOff(CONTROL_OWNER_FLY_CAM);
    xScrFxLetterbox(1);

    zcam_backupcam = globals.oldSkoolCamera;
}

static S32 zCameraFlyUpdate(xCamera* cam, F32 dt)
{
    S32 i;
    S32 flyIdx;
    S32 numKeys;
    S32 flySize;
    F32 flyLerp;
    F32 flyFrame;
    zFlyKey keys[4];
    F32 matdiff1;
    F32 matdiff2;
    F32 matdiff3;
    xMat3x3 tmpMat;
    xQuat quats[2];
    xQuat qresult;

    if ((globals.pad0->pressed & 0x50000) && zcam_flytime > gSkipTimeFlythrough)
    {
        zcam_flytime = 0.033333335f * zcam_flysize;
    }

    flyFrame = 30.0f * zcam_flytime;
    numKeys = floor(flyFrame);
    flyLerp = flyFrame - floor(flyFrame);

    flySize = (S32)(zcam_flysize >> 6) - 1;
    if (!(numKeys < flySize))
    {
        return 0;
    }

    flyIdx = numKeys;
    if (numKeys - 1 >= 0)
    {
        flyIdx = numKeys - 1;
    }

    keys[0] = *((zFlyKey*)zcam_flydata + flyIdx);
    keys[1] = *((zFlyKey*)zcam_flydata + numKeys);
    keys[2] = *((zFlyKey*)zcam_flydata + (numKeys + 1));

    flyIdx = numKeys + 1;
    if (numKeys + 2 < flySize)
    {
        flyIdx = numKeys + 2;
    }

    keys[3] = *((zFlyKey*)zcam_flydata + flyIdx);

    // Reverses the byte order (endianness) of 64 4-byte blocks
    U8* framePtr = (U8*)&keys[0].frame;
    for (i = 64; i > 0; i--)
    {
        S8 tmp1 = *framePtr;
        S8 tmp2 = *(framePtr + 1);
        *framePtr = *(framePtr + 3);
        *(framePtr + 1) = *(framePtr + 2);
        *(framePtr + 2) = tmp2;
        *(framePtr + 3) = tmp1;

        framePtr += 4;
    }

    if (0 < numKeys)
    {
        //matdiff1 = TranSpeed(&keys[0]);
        //matdiff2 = TranSpeed(&keys[1]);
        //matdiff3 = TranSpeed(&keys[2]);

        if (matdiff2 > 10.0f && matdiff2 > 5.0f * matdiff1 && matdiff2 > 5.0f * matdiff3)
        {
            flyLerp = 0.0f;
        }
        else
        {
            //matdiff1 = MatrixSpeed(&keys[0]);
            //matdiff2 = MatrixSpeed(&keys[1]);
            //matdiff3 = MatrixSpeed(&keys[2]);

            if (matdiff2 > 45.0f && matdiff2 > matdiff1 * 5.0f && matdiff2 > matdiff3 * 5.0f)
            {
                flyLerp = 0.0f;
            }
        }
    }

    for (i = 0; i < 2; i++)
    {
        tmpMat.right.x = -keys[i + 1].matrix[0];
        tmpMat.right.y = -keys[i + 1].matrix[1];
        tmpMat.right.z = -keys[i + 1].matrix[2];

        tmpMat.up.x = keys[i + 1].matrix[3];
        tmpMat.up.y = keys[i + 1].matrix[4];
        tmpMat.up.z = keys[i + 1].matrix[5];

        tmpMat.at.x = -keys[i + 1].matrix[6];
        tmpMat.at.y = -keys[i + 1].matrix[7];
        tmpMat.at.z = -keys[i + 1].matrix[8];

        xQuatFromMat(&quats[i], &tmpMat);
    }

    xQuatSlerp(&qresult, &quats[0], &quats[1], flyLerp);
    xQuatToMat(&qresult, &cam->mat);
    xVec3Lerp(&cam->mat.pos, (xVec3*)&keys[1].matrix[9], (xVec3*)&keys[2].matrix[9], flyLerp);

    zcam_flytime += dt;

    return 1;
}

void zCameraReset(xCamera* cam)
{
    zcam_mode = 0;
    zcam_bbounce = 0;
    zcam_lbbounce = 0;
    zcam_lconvers = 0;
    zcam_longbounce = 0;
    zcam_highbounce = 0;
    zcam_convers = 0;
    zcam_fly = 0;
    zcam_flypaused = 0;
    zcam_cutscene = 0;
    zcam_reward = 0;

    zcam_fovcurr = 75.0f;
    zcam_overrot_tmr = 0.0f;
    wall_jump_enabled = WJVS_DISABLED;
    lassocam_enabled = '\0';
    stop_track = 0;
    zcam_mintgtheight = FLOAT_MIN;
    zcam_centering = '\0';
    zcam_lastcentering = '\0';
    sNearToggleEnabled = '\0';

    cam->fov = 75.0f;

    sCamTweakLerp -= 0.0f / sCamTweakTime;
    if (sCamTweakLerp < 0.0f)
    {
        sCamTweakLerp = 0.0f;
    }

    sCamTweakPitchCur =
        sCamTweakPitch[1] * sCamTweakLerp + sCamTweakPitch[0] * (1.0f - sCamTweakLerp);

    sCamTweakDistMultCur =
        sCamTweakDistMult[1] * sCamTweakLerp + sCamTweakDistMult[0] * (1.0f - sCamTweakLerp);

    zCamTweakLook* tweak = (zcam_near != 0) ? &zcam_neartweak : &zcam_fartweak;

    sCamD = sCamTweakDistMultCur * tweak->dist * icos(tweak->pitch + sCamTweakPitchCur);
    sCamH = sCamTweakDistMultCur * tweak->dist * isin(tweak->pitch + sCamTweakPitchCur) + tweak->h;
    sCamPitch = tweak->pitch + sCamTweakPitchCur;

    F32 dist = (zcam_highbounce != 0) ? zcam_highbounce_d :
                                        (wall_jump_enabled == WJVS_ENABLED ? zcam_wall_d : sCamD);
    F32 camDist = dMultiplier * dist + dOffset;
    F32 height = (zcam_highbounce != 0) ? zcam_highbounce_h :
                                          (wall_jump_enabled == WJVS_ENABLED ? zcam_wall_h : sCamH);
    F32 camHeight = dMultiplier * height + dOffset;

    F32 pitch = (zcam_highbounce != 0) ? zcam_highbounce_pitch : sCamPitch;

    xCameraReset(cam, camDist, camHeight, pitch);

    input_enabled = true;
    dMultiplier = 1.0f;
    dOffset = 0.0f;
    hMultiplier = 1.0f;
    hOffset = 0.0f;

    if (follow_cam != NULL)
    {
        follow_cam->cfg_common.priority = '\0';
    }
}
