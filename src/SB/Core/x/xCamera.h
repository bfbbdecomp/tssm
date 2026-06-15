#ifndef XCAMERA_H
#define XCAMERA_H

#include "iCamera.h"

#include "xBase.h"
#include "xMath3.h"
#include "xBound.h"
#include "xMath2.h"
#include "xMathInlines.h"

#include <rwcore.h>

#include "xPad.h"

struct xScene;

enum _tagTransType
{
    eTransType_None,
    eTransType_Interp1,
    eTransType_Interp2,
    eTransType_Interp3,
    eTransType_Interp4,
    eTransType_Linear,
    eTransType_Interp1Rev,
    eTransType_Interp2Rev,
    eTransType_Interp3Rev,
    eTransType_Interp4Rev,
    eTransType_Total
};

enum xCamCoordType
{
    XCAM_COORD_CART,
    XCAM_COORD_CYLINDER,
    XCAM_COORD_SPHERE,
    XCAM_COORD_MAX,
    XCAM_COORD_INVALID = 0xffffffff,
};

enum xCamOrientType
{
    XCAM_ORIENT_QUAT,
    XCAM_ORIENT_EULER,
    XCAM_ORIENT_MAX,
    XCAM_ORIENT_INVALID = 0xffffffff,
};

struct _tagxCamFollowAsset
{
    F32 rotation;
    F32 distance;
    F32 height;
    F32 rubber_band;
    F32 start_speed;
    F32 end_speed;
};

struct _tagxCamShoulderAsset
{
    F32 distance;
    F32 height;
    F32 realign_speed;
    F32 realign_delay;
};

struct _tagp2CamStaticAsset
{
    U32 unused;
};

struct _tagxCamPathAsset
{
    U32 assetID;
    F32 time_end;
    F32 time_delay;
};

struct _tagp2CamStaticFollowAsset
{
    F32 rubber_band;
};

struct xCamAsset : xBaseAsset
{
    xVec3 pos;
    xVec3 at;
    xVec3 up;
    xVec3 right;
    xVec3 view_offset;
    S16 offset_start_frames;
    S16 offset_end_frames;
    F32 fov;
    F32 trans_time;
    _tagTransType trans_type;
    U32 flags;
    F32 fade_up;
    F32 fade_down;
    union
    {
        _tagxCamFollowAsset cam_follow;
        _tagxCamShoulderAsset cam_shoulder;
        _tagp2CamStaticAsset cam_static;
        _tagxCamPathAsset cam_path;
        _tagp2CamStaticFollowAsset cam_staticFollow;
    };
    U32 valid_flags;
    U32 markerid[2];
    U8 cam_type;
    U8 pad[3];
};

struct xCamera : xBase
{
    RwCamera* lo_cam;
    // Offset: 0x14
    xMat4x3 mat;
    // Offset: 0x54
    xMat4x3 omat;
    xMat3x3 mbasis;
    xBound bound;
    xMat4x3* tgt_mat;
    xMat4x3* tgt_omat;
    xBound* tgt_bound;
    xVec3 focus;
    xScene* sc;
    xVec3 tran_accum;
    F32 fov;
    U32 flags;
    F32 tmr;
    F32 tm_acc;
    F32 tm_dec;
    F32 ltmr;
    F32 ltm_acc;
    F32 ltm_dec;
    F32 dmin;
    F32 dmax;
    F32 dcur;
    F32 dgoal;
    F32 hmin;
    F32 hmax;
    F32 hcur;
    F32 hgoal;
    F32 pmin;
    F32 pmax;
    F32 pcur;
    F32 pgoal;
    F32 depv;
    F32 hepv;
    F32 pepv;
    F32 orn_epv;
    F32 yaw_epv;
    F32 pitch_epv;
    F32 roll_epv;
    xQuat orn_cur;
    xQuat orn_goal;
    xQuat orn_diff;
    F32 yaw_cur;
    F32 yaw_goal;
    F32 pitch_cur;
    F32 pitch_goal;
    F32 roll_cur;
    F32 roll_goal;
    F32 dct;
    F32 dcd;
    F32 dccv;
    F32 dcsv;
    F32 hct;
    F32 hcd;
    F32 hccv;
    F32 hcsv;
    F32 pct;
    F32 pcd;
    F32 pccv;
    F32 pcsv;
    F32 orn_ct;
    F32 orn_cd;
    F32 orn_ccv;
    F32 orn_csv;
    F32 yaw_ct;
    F32 yaw_cd;
    F32 yaw_ccv;
    F32 yaw_csv;
    F32 pitch_ct;
    F32 pitch_cd;
    F32 pitch_ccv;
    F32 pitch_csv;
    F32 roll_ct;
    F32 roll_cd;
    F32 roll_ccv;
    F32 roll_csv;
    xVec4 frustplane[12];
    F32 smoothOutwardSlidePos;
};

struct xBinaryCamera
{
    struct zone_data
    {
        F32 distance;
        F32 height;
        F32 height_focus;
    };

    struct config
    {
        zone_data zone_rest;
        zone_data zone_above;
        zone_data zone_below;
        F32 move_speed;
        F32 turn_speed;
        F32 stick_speed;
        F32 stick_yaw_vel;
        F32 max_yaw_vel;
        F32 margin_angle;
    };

    config cfg;
    xCamera* camera;
    xQuat cam_dir;
    xVec3* s1;
    xVec3* s2;
    F32 s2_radius;
    xVec2 stick_offset;

    void init();
    void start(xCamera& camera);
    void stop();
    void update(F32 dt);
    void add_tweaks(char const*);
    void set_targets(xVec3 const& par_1, xVec3 const& par_2, F32 par_3);
    void render_debug();
};

struct xCamConfigCommon
{
    U8 priority;
    U8 pad1;
    U8 pad2;
    U8 pad3;
    F32 blend_time;
};

struct xCamBlend;
struct xCam;

struct xCamGroup
{
    xMat4x3 mat;
    xVec3 vel;
    F32 fov;
    F32 fov_default;
    S32 flags;
    xCam* primary;
    analog_data analog;
    xCam* owned[32];
    S32 size;
    S32 primary_index;
    S32 child_flags;
    S32 child_flags_mask;
    xCamBlend* blend_cam[4];
};

struct xCam
{
    struct _class_0
    {
        union
        {
            //xVec3 cart;
            //xCamCoordCylinder cylinder;
            //xCamCoordSphere sphere;
        };
    };

    struct _class_1
    {
        union
        {
            //xQuat quat;
            //xCamOrientEuler euler;
        };
    };

    xMat4x3 mat;
    F32 fov;
    S32 flags;
    U32 owner;
    xCamGroup* group;
    analog_data analog;
    F32 motion_factor;
    xCamCoordType coord_type;
    xCamOrientType orient_type;
    _class_0 coord;
    _class_1 orient;
    U8 _padding[0x2c]; // until the anonymous union structs above are done
    xCamConfigCommon cfg_common;
    S32 group_index;
    S32 group_flags;
    xCamBlend* blender;
};

struct xCamBlend : xCam
{
    xCam* src;
    xCam* dst;
    F32 time;
};

struct xCamScreen
{
    RwCamera* icam;
    F32 fov;
};

F32 xVec3Length(const xVec3* vec);
void xCameraInit(xCamera* cam, U32 width, U32 height);
void xCameraSetScene(xCamera* cam, xScene* sc);
void xCameraReset(xCamera* cam, F32 d, F32 h, F32 pitch);
void xCameraExit(xCamera* cam);
void xCameraUpdate(xCamera* cam, F32 dt);
void xCameraBegin(xCamera* cam, S32);
void xCameraEnd(xCamera* cam, F32 seconds, S32 update_scrn_fx);
void xCameraShowRaster(xCamera* cam);
F32 xCameraGetFOV(const xCamera* cam);
void xCameraSetFOV(xCamera* cam, F32 fov);
void xCameraMove(xCamera* cam, U32 flags, F32 dgoal, F32 hgoal, F32 pgoal, F32 tm, F32 tm_acc,
                 F32 tm_dec);
void xCameraMove(xCamera* cam, const xVec3& loc);
void xCameraRotate(xCamera* cam, const xMat3x3& m, F32 time, F32 accel, F32 decl);
F32 xCameraGetFOV(const xCamera* cam);
void xCameraDoCollisions(S32 do_collis, S32 owner);
void xCameraSetTargetMatrix(xCamera* cam, xMat4x3* mat);
void xCameraFXShake(F32 maxTime, F32 magnitude, F32 cycleMax, F32 rotate_magnitude, F32 radius,
                    xVec3* epicenter, xVec3* player);

#endif
