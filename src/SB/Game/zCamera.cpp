#include "zCamera.h"

namespace
{
    WallJumpViewState wall_jump_enabled;
    bool lassocam_enabled;
    U32 stop_track;
    bool input_enabled = true;
    F32 dMultiplier;
    F32 dOffset;
    F32 hMultiplier;
    F32 hOffset;
    zCamSB* follow_cam;
} // namespace

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
    zcam_mintgtheight = -1e+38f;
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
