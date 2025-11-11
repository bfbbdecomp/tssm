#include "xCamera.h"

#include "xstransvc.h"
#include "xMath.h"
#include "xMathInlines.h"
#include "xScene.h"
#include "xCollideFast.h"
#include "xScrFx.h"

#include "iMath.h"

#include <PowerPC_EABI_Support\MSL_C\MSL_Common\cmath>
#include <PowerPC_EABI_Support\MSL_C\MSL_Common\cstring>

struct cameraFXShake
{
    F32 magnitude;
    xVec3 dir;
    F32 cycleTime;
    F32 cycleMax;
    F32 dampen;
    F32 dampenRate;
    F32 rotate_magnitude;
    F32 radius;
    xVec3* epicenterP;
    xVec3 epicenter;
    xVec3* player;
};

struct cameraFXZoom
{
    F32 holdTime;
    F32 vel;
    F32 accel;
    F32 distance;
    U32 mode;
    F32 velCur;
    F32 distanceCur;
    F32 holdTimeCur;
};

#define CAMERAFX_ZOOM_MODE_0 0
#define CAMERAFX_ZOOM_MODE_1 1
#define CAMERAFX_ZOOM_MODE_2 2
#define CAMERAFX_ZOOM_MODE_3 3

struct cameraFX
{
    S32 type;
    S32 flags;
    F32 elapsedTime;
    F32 maxTime;
    union
    {
        cameraFXShake shake;
        cameraFXZoom zoom;
    };
};

#define CAMERAFX_TYPE_SHAKE 2

struct cameraFXTableEntry
{
    S32 type;
    void (*func)(cameraFX*, F32, xMat4x3*, xMat4x3*);
    void (*funcKill)(cameraFX*);
};

extern F32 _764;
extern F32 _765;
extern F32 _766;
extern F32 _785;
extern F32 _786;
extern F32 _787;
extern F32 _788;
extern F32 _789;
extern F32 _790;
extern F32 _830;
extern F32 _831;
extern F32 _873;
extern F32 _874;
extern F32 _880;
extern F32 _888;
extern F32 _895;
extern F32 _1234;
extern F32 _1235;
extern F32 _1236;
extern F32 _1237;
extern F32 _1238;
extern F32 _1239;
extern F32 _1240;
extern F32 _1241;
extern F32 _1242;
extern F32 _1283;
extern F64 _1286;
extern F32 _1404;
extern F32 _1405;
extern F32 _1534;
extern F32 _1584;
extern F32 _1585;
extern F32 _1586;
extern F32 _1757;
extern F32 _1758;
extern F32 _1766;
extern F32 _1772;

extern S32 sCamCollis;
extern volatile S32 xcam_collis_owner_disable;
extern S32 xcam_do_collis;
extern F32 xcam_collis_radius;
extern F32 xcam_collis_stiffness;
extern RpAtomic* sInvisWallHack;
extern xMat4x3 sCameraFXMatOld;
extern cameraFX sCameraFX[10];
extern cameraFXTableEntry sCameraFXTable[3];

static void xCameraFXInit();
void add_camera_tweaks();

F32 xasin(F32 x)
{
    return std::asinf(x);
}

void xCameraRotate(xCamera* cam, const xVec3& v, F32 roll, F32 time, F32 accel, F32 decl)
{
    cam->yaw_goal = xatan2(v.x, v.z);
    cam->pitch_goal = -xasin(v.y);
    cam->roll_goal = roll;

    if (_765 == time)
    {
        cam->yaw_cur = cam->yaw_goal;
        cam->pitch_cur = cam->pitch_goal;
        cam->roll_cur = cam->roll_goal;
    }

    cam->flags = cam->flags & ~0xF80 | 0x80;

    xMat3x3Euler(&cam->mat, cam->yaw_goal, cam->pitch_goal, cam->roll_goal);

    if (_765 == time)
    {
        *(xMat3x3*)&cam->omat = *(xMat3x3*)&cam->mat;
    }

    if (_765 == time)
    {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = _765;
    }
    else
    {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = _765;
}

void xCameraRotate(xCamera* cam, const xMat3x3& m, F32 time, F32 accel, F32 decl)
{
    xVec3 eu;

    cam->flags = cam->flags & ~0xF80 | 0x80;

    xMat3x3GetEuler(&m, &eu);

    cam->yaw_goal = eu.x;
    cam->pitch_goal = eu.y;
    cam->roll_goal = eu.z;

    if (_765 == time)
    {
        cam->yaw_cur = eu.x;
        cam->pitch_cur = eu.y;
        cam->roll_cur = eu.z;
    }

    *(xMat3x3*)&cam->mat = m;

    if (_765 == time)
    {
        *(xMat3x3*)&cam->omat = m;
    }

    if (_765 == time)
    {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = _765;
    }
    else
    {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = _765;
}

void xCameraLookYPR(xCamera* cam, U32 flags, F32 yaw, F32 pitch, F32 roll, F32 tm, F32 tm_acc,
                    F32 tm_dec)
{
    cam->flags = (cam->flags & ~0xF80) | (flags & 0xF80) | 0x80;
    cam->yaw_goal = yaw;
    cam->pitch_goal = pitch;
    cam->roll_goal = roll;

    if (tm <= _765)
    {
        if (cam->tgt_mat)
        {
            cam->yaw_cur = yaw;
            cam->pitch_cur = pitch;
            cam->roll_cur = roll;

            xMat3x3Euler(&cam->mat, yaw, pitch, roll);

            *(xMat3x3*)&cam->omat = *(xMat3x3*)&cam->mat;
        }
    }
    else
    {
        cam->flags |= 0x40;
        cam->ltm_acc = tm - tm_acc;
        cam->ltm_dec = tm_dec;
        cam->ltmr = tm;

        F32 s = _788 / (tm - _766 * (tm_acc - tm_dec));

        cam->yaw_epv = s * xDangleClamp(yaw - cam->yaw_cur);
        cam->pitch_epv = s * xDangleClamp(pitch - cam->pitch_cur);
        cam->roll_epv = s * xDangleClamp(roll - cam->roll_cur);
    }
}

void xCameraFOV(xCamera* cam, F32 fov, F32 maxSpeed, F32 dt)
{
    F32 speed = maxSpeed * dt;
    F32 currentFOV = xCameraGetFOV(cam);

    if (currentFOV != fov)
    {
        if (speed != _765)
        {
            F32 len = fov - currentFOV;

            if ((F32)iabs(len) > speed)
            {
                len *= speed / len;
                xCameraSetFOV(cam, currentFOV + len);
            }
            else
            {
                xCameraSetFOV(cam, fov);
            }
        }
        else
        {
            xCameraSetFOV(cam, fov);
        }
    }
}

void xCameraMove(xCamera* cam, const xVec3& loc, F32 maxSpeed)
{
    xVec3 var_28;
    F32 f1;

    xVec3Sub(&var_28, &loc, &cam->mat.pos);

    f1 = xVec3Length(&var_28);

    if (f1 > maxSpeed)
    {
        xVec3SMul(&var_28, &var_28, maxSpeed / f1);
        xVec3Add(&cam->mat.pos, &cam->mat.pos, &var_28);
    }
    else
    {
        cam->mat.pos = loc;
    }

    cam->omat.pos = cam->mat.pos;
    cam->flags &= ~0x3E;
    cam->tm_acc = cam->tm_dec = cam->tmr = _765;
}

void xCameraMove(xCamera* cam, const xVec3& loc)
{
    cam->omat.pos = cam->mat.pos = loc;
    cam->flags &= ~0x3E;
    cam->tm_acc = cam->tm_dec = cam->tmr = _765;
}

static void xCam_cyltoworld(xVec3* v, const xMat4x3* tgt_mat, F32 d, F32 h, F32 p, U32 flags)
{
    if (flags & 0x10)
    {
        v->y = h;
    }
    else
    {
        v->y = h + tgt_mat->pos.y;
    }

    if (flags & 0x20)
    {
        v->x = d * isin(p) + tgt_mat->pos.x;
        v->z = d * icos(p) + tgt_mat->pos.z;
    }
    else
    {
        p += xatan2(tgt_mat->at.x, tgt_mat->at.z);

        v->x = d * isin(p) + tgt_mat->pos.x;
        v->z = d * icos(p) + tgt_mat->pos.z;
    }
}

void xCameraMove(xCamera* cam, U32 flags, F32 dgoal, F32 hgoal, F32 pgoal, F32 tm, F32 tm_acc,
                 F32 tm_dec)
{
    cam->flags = (cam->flags & ~0x3E) | (flags & 0x3E);
    cam->dgoal = dgoal;
    cam->hgoal = hgoal;
    cam->pgoal = pgoal;

    if (tm <= _765)
    {
        if (cam->tgt_mat)
        {
            cam->dcur = dgoal;
            cam->hcur = hgoal;
            cam->pcur = pgoal;

            xCam_cyltoworld(&cam->mat.pos, cam->tgt_mat, dgoal, hgoal, pgoal, cam->flags);

            cam->omat.pos = cam->mat.pos;
            cam->yaw_cur = cam->yaw_goal = cam->pcur + ((cam->pcur >= _786) ? _1534 : _786);
        }
    }
    else
    {
        cam->flags |= 0x1;
        cam->tm_acc = tm - tm_acc;
        cam->tm_dec = tm_dec;
        cam->tmr = tm;

        F32 s = _788 / (tm - _766 * (tm_acc - tm_dec));

        cam->depv = s * (dgoal - cam->dcur);
        cam->hepv = s * (hgoal - cam->hcur);
        cam->pepv = xDangleClamp(pgoal - cam->pcur) * s * _766 * (dgoal + cam->dcur);
    }
}

void xCameraDoCollisions(S32 do_collis, S32 owner)
{
    xcam_collis_owner_disable &= ~(1 << owner);
    xcam_collis_owner_disable |= !do_collis << owner;

    xcam_do_collis = (xcam_collis_owner_disable == 0);
}

void xCameraSetTargetOMatrix(xCamera* cam, xMat4x3* mat)
{
    cam->tgt_omat = mat;
}

void xCameraSetTargetMatrix(xCamera* cam, xMat4x3* mat)
{
    cam->tgt_mat = mat;
}

void xCameraSetScene(xCamera* cam, xScene* sc)
{
    cam->sc = sc;

    iCameraAssignEnv(cam->lo_cam, sc->env->geom);
}

static void _xCameraUpdate(xCamera* cam, F32 dt)
{
    // lol nope
}

// Inlining issue
void xCameraUpdate(xCamera* cam, F32 dt)
{
    S32 i;
    S32 num_updates;
    F32 sdt;

    num_updates = std::ceilf(_1283 * dt);

    sdt = dt / num_updates;

    for (i = 0; i < num_updates; i++)
    {
        sCamCollis = (i == num_updates - 1);

        _xCameraUpdate(cam, sdt);
    }
}

void SweptSphereHitsCameraEnt(xScene*, xRay3* ray, xQCData* qcd, xEnt* ent, void* data)
{
    xSweptSphere* sws = (xSweptSphere*)data;

    if (ent->camcollModel && ent->chkby & 0x10 && xQuickCullIsects(qcd, &ent->bound.qcd))
    {
        if (!xEntIsVisible(ent))
        {
            if (ent->model->Data != sInvisWallHack)
            {
                return;
            }

            if (ent->collLev != 5)
            {
                if (ent->bound.type == XBOUND_TYPE_BOX)
                {
                    xSweptSphereToBox(sws, &ent->bound.box.box, NULL);
                    return;
                }
                else if (ent->bound.type == XBOUND_TYPE_OBB)
                {
                    xSweptSphereToBox(sws, &ent->bound.box.box, ent->bound.mat);
                    return;
                }
                else
                {
                    return;
                }
            }
        }

        U32 result = 0;

        switch (ent->bound.type)
        {
        case XBOUND_TYPE_SPHERE:
        {
            F32 oldrad = ent->bound.sph.r;

            ent->bound.sph.r += sws->radius;

            result = xRayHitsSphereFast(ray, &ent->bound.sph);

            ent->bound.sph.r = oldrad;

            break;
        }
        case XBOUND_TYPE_BOX:
        {
            xBox tmpbox;
            tmpbox.upper.x = ent->bound.box.box.upper.x + sws->radius;
            tmpbox.upper.y = ent->bound.box.box.upper.y + sws->radius;
            tmpbox.upper.z = ent->bound.box.box.upper.z + sws->radius;
            tmpbox.lower.x = ent->bound.box.box.lower.x - sws->radius;
            tmpbox.lower.y = ent->bound.box.box.lower.y - sws->radius;
            tmpbox.lower.z = ent->bound.box.box.lower.z - sws->radius;

            result = xRayHitsBoxFast(ray, &tmpbox);

            break;
        }
        case XBOUND_TYPE_OBB:
        {
            xBox tmpbox;
            xRay3 lr;
            xMat3x3 mn;

            F32 f31 = xVec3Length(&ent->bound.mat->right);

            xMat3x3Normalize(&mn, ent->bound.mat);
            xMat4x3Tolocal(&lr.origin, ent->bound.mat, &ray->origin);
            xMat3x3Tolocal(&lr.dir, &mn, &ray->dir);

            lr.max_t = ray->max_t / f31;
            lr.min_t = ray->min_t / f31;
            lr.flags = ray->flags;

            tmpbox.upper.x = ent->bound.box.box.upper.x + sws->radius / f31;
            tmpbox.upper.y = ent->bound.box.box.upper.y + sws->radius / f31;
            tmpbox.upper.z = ent->bound.box.box.upper.z + sws->radius / f31;
            tmpbox.lower.x = ent->bound.box.box.lower.x - sws->radius / f31;
            tmpbox.lower.y = ent->bound.box.box.lower.y - sws->radius / f31;
            tmpbox.lower.z = ent->bound.box.box.lower.z - sws->radius / f31;

            result = xRayHitsBoxFast(&lr, &tmpbox);

            break;
        }
        }

        if (result)
        {
            xModelInstance* collmod = ent->camcollModel;

            xSweptSphereToModel(sws, collmod->Data, collmod->Mat);
        }
    }
}

static void xCam_buildbasis(xCamera* cam)
{
    if (cam->tgt_mat)
    {
        F32 d2d;
        F32 dx__ = cam->mat.pos.x - cam->tgt_mat->pos.x;
        F32 dz__ = cam->mat.pos.z - cam->tgt_mat->pos.z;

        F32 dist2 = SQR(dx__) + SQR(dz__);
        F32 dist_inv;

        if ((F32)iabs(dist2 - _788) <= _830)
        {
            cam->mbasis.at.x = dx__;
            cam->mbasis.at.z = dz__;

            d2d = _788;
        }
        else if ((F32)iabs(dist2) <= _830)
        {
            cam->mbasis.at.x = _765;
            cam->mbasis.at.z = _765;

            d2d = _765;
        }
        else
        {
            d2d = xsqrt(dist2);
            dist_inv = _788 / d2d;

            cam->mbasis.at.x = dx__ * dist_inv;
            cam->mbasis.at.z = dz__ * dist_inv;
        }

        if (d2d < _830)
        {
            cam->mbasis.at.x = cam->mat.at.x;
            cam->mbasis.at.z = cam->mat.at.z;

            dist2 = xsqrt(SQR(cam->mbasis.at.x) + SQR(cam->mbasis.at.z));

            if (dist2 > _831)
            {
                // non-matching: wrong registers
                dist_inv = _788 / dist2;

                cam->mbasis.at.x *= dist_inv;
                cam->mbasis.at.z *= dist_inv;
            }
            else
            {
                cam->mbasis.at.x = isin(cam->pcur);
                cam->mbasis.at.z = icos(cam->pcur);
            }
        }

        cam->mbasis.at.y = _765;
        cam->mbasis.up.x = _765;
        cam->mbasis.up.y = _788;
        cam->mbasis.up.z = _765;
        cam->mbasis.right.x = cam->mbasis.at.z;
        cam->mbasis.right.y = _765;
        cam->mbasis.right.z = -cam->mbasis.at.x;
    }
}

void xCameraReset(xCamera* cam, F32 d, F32 h, F32 pitch)
{
    sInvisWallHack = (RpAtomic*)xSTFindAsset(0xB8895D14, NULL);

    xMat4x3Identity(&cam->mat);

    cam->omat = cam->mat;
    cam->focus.x = _765;
    cam->focus.y = _765;
    cam->focus.z = _785;
    cam->tran_accum.x = _765;
    cam->tran_accum.y = _765;
    cam->tran_accum.z = _765;
    cam->flags = 0;

    F32 goal_p = _786;

    if (cam->tgt_mat)
    {
        goal_p += xatan2(cam->tgt_mat->at.x, cam->tgt_mat->at.z);
    }

    xCameraMove(cam, 0x2E, d, h, goal_p, _765, _787, _787);

    cam->pitch_goal = pitch;
    cam->pitch_cur = pitch;
    cam->roll_cur = _765;

    xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);

    cam->omat = cam->mat;
    cam->yaw_ct = _788;
    cam->yaw_cd = _788;
    cam->yaw_ccv = _789;
    cam->yaw_csv = _788;
    cam->pitch_ct = _788;
    cam->pitch_cd = _788;
    cam->pitch_ccv = _790;
    cam->pitch_csv = _788;
    cam->roll_ct = _788;
    cam->roll_cd = _788;
    cam->roll_ccv = _790;
    cam->roll_csv = _788;
    cam->flags |= 0x80;

    xcam_do_collis = 1;
    xcam_collis_owner_disable = 0;
}

void xCameraExit(xCamera* cam)
{
    if (cam->lo_cam)
    {
        iCameraDestroy(cam->lo_cam);
        cam->lo_cam = NULL;
    }
}

void xCameraInit(xCamera* cam, U32 width, U32 height)
{
    xCameraFXInit();

    cam->lo_cam = iCameraCreate(width, height, 1);

    xCameraSetFOV(cam, _764);

    cam->bound.sph.center.x = _765;
    cam->bound.sph.center.y = _765;
    cam->bound.sph.center.z = _765;
    cam->bound.sph.r = _766;
    cam->tgt_mat = NULL;
    cam->tgt_omat = NULL;
    cam->tgt_bound = NULL;
    cam->sc = NULL;
    cam->tran_accum.x = _765;
    cam->tran_accum.y = _765;
    cam->tran_accum.z = _765;

    add_camera_tweaks();
}

void add_camera_tweaks()
{
}
