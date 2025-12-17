#include "xCamera.h"

#include "xstransvc.h"
#include "xMath.h"
#include "xMathInlines.h"
#include "xScene.h"
#include "xCollideFast.h"
#include "xScrFx.h"
#include "zGlobals.h"
#include "zMain.h"

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
    cam->yaw_goal = xAngleClampFast(atan2(v.x, v.z));
    F32 fVar2 = v.y;

    if (v.y > 1.0f)
    {
        fVar2 = 1.0f;
    }

    if (fVar2 < -1.0f)
    {
        fVar2 = -1.0f;
    }

    cam->pitch_goal = -(F32)asin(fVar2);
    cam->roll_goal = roll;

    if (0.0f == time)
    {
        cam->yaw_cur = cam->yaw_goal;
        cam->pitch_cur = cam->pitch_goal;
        cam->roll_cur = cam->roll_goal;
    }

    cam->flags = (cam->flags & ~0xF80) | 0x80;

    xMat3x3Euler(&cam->mat, cam->yaw_goal, cam->pitch_goal, cam->roll_goal);

    if (0.0f == time)
    {
        *(xMat3x3*)&cam->omat = *(xMat3x3*)&cam->mat;
    }

    if (0.0f == time)
    {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = 0.0f;
    }
    else
    {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = 0.0f;
}

void xCameraRotate(xCamera* cam, const xMat3x3& m, F32 time, F32 accel, F32 decl)
{
    xVec3 eu;

    cam->flags = cam->flags & ~0xF80 | 0x80;

    xMat3x3GetEuler(&m, &eu);

    cam->yaw_goal = eu.x;
    cam->pitch_goal = eu.y;
    cam->roll_goal = eu.z;

    if (0.0f == time)
    {
        cam->yaw_cur = eu.x;
        cam->pitch_cur = eu.y;
        cam->roll_cur = eu.z;
    }

    *(xMat3x3*)&cam->mat = m;

    if (0.0f == time)
    {
        *(xMat3x3*)&cam->omat = m;
    }

    if (0.0f == time)
    {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = 0.0f;
    }
    else
    {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = 0.0f;
}

void xCameraLookYPR(xCamera* cam, U32 flags, F32 yaw, F32 pitch, F32 roll, F32 tm, F32 tm_acc,
                    F32 tm_dec)
{
    cam->flags = (cam->flags & ~0xF80) | (flags & 0xF80) | 0x80;
    cam->yaw_goal = yaw;
    cam->pitch_goal = pitch;
    cam->roll_goal = roll;

    if (tm <= 0.0f)
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

        F32 s = 1.0f / (tm - 0.5f * (tm_acc - tm_dec));

        cam->yaw_epv = s * xDangleClamp(yaw - cam->yaw_cur);
        cam->pitch_epv = s * xDangleClamp(pitch - cam->pitch_cur);
        cam->roll_epv = s * xDangleClamp(roll - cam->roll_cur);
    }
}

void xCameraFOV(xCamera* cam, F32 fov, F32 maxSpeed, F32 dt)
{
    F32 speed = maxSpeed * dt;

    if (cam->fov != fov)
    {
        if (speed != 0.0f)
        {
            F32 currentFOV = fov - cam->fov;
            if (iabs(currentFOV) > speed)
            {
                F32 newFOV = currentFOV * (speed / currentFOV);
                cam->fov += newFOV;
            }
            else
            {
                cam->fov = fov;
            }
        }
        else
        {
            cam->fov = fov;
        }
    }
    return;
}

void xCameraMove(xCamera* cam, const xVec3& loc, F32 maxSpeed)
{
    xVec3 var_28;
    F32 f1;

    xVec3Sub(&var_28, &loc, &cam->mat.pos);

    f1 = xVec3Length(&var_28);

    if (f1 > 0.0f)
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
    cam->tm_acc = cam->tm_dec = cam->tmr = 0.0f;
}

void xCameraMove(xCamera* cam, const xVec3& loc)
{
    // xVec3::operator= needs to be inlined? And use lwz/stw for some reason?
    cam->omat.pos = cam->mat.pos = loc;
    cam->flags &= ~0x3E;
    cam->tm_acc = cam->tm_dec = cam->tmr = 0.0f;
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

    if (tm <= 0.0f)
    {
        if (cam->tgt_mat)
        {
            cam->dcur = dgoal;
            cam->hcur = hgoal;
            cam->pcur = pgoal;

            U32 uVar1 = cam->flags;
            xMat4x3* tgt_mat = cam->tgt_mat;
            if ((uVar1 & 0x10) != 0)
            {
                cam->mat.pos.y = hgoal;
            }
            else
            {
                cam->mat.pos.y = hgoal + (tgt_mat->pos).y;
            }
            if ((uVar1 & 0x20) != 0)
            {
                cam->mat.pos.x = dgoal * isin(pgoal) + tgt_mat->pos.x;
                cam->mat.pos.z = dgoal * icos(pgoal) + tgt_mat->pos.z;
            }
            else
            {
                F32 angle = xAngleClampFast(atan2(tgt_mat->at.x, tgt_mat->at.z));
                cam->mat.pos.x = dgoal * isin(pgoal + angle) + tgt_mat->pos.x;
                cam->mat.pos.z = dgoal * icos(pgoal + angle) + tgt_mat->pos.z;
            }
            cam->omat.pos = cam->mat.pos;
            cam->yaw_cur = cam->yaw_goal = cam->pcur + ((cam->pcur >= PI) ? -PI : PI);
        }
    }
    else
    {
        cam->flags |= 0x1;
        cam->tm_acc = tm - tm_acc;
        cam->tm_dec = tm_dec;
        cam->tmr = tm;

        F32 s = 1.0f / (tm - 0.5f * (tm_acc - tm_dec));

        cam->depv = s * (dgoal - cam->dcur);
        cam->hepv = s * (hgoal - cam->hcur);
        cam->pepv = xDangleClamp(pgoal - cam->pcur) * s * 0.5f * (dgoal + cam->dcur);
    }
}

void xCameraDoCollisions(S32 do_collis, S32 owner)
{
    S32 temp_r3;
    S32 temp_r5;

    temp_r5 = xcam_collis_owner_disable & ~(1 << owner);
    temp_r3 = temp_r5 | ((do_collis == 0) << owner);
    xcam_collis_owner_disable = temp_r5;
    xcam_collis_owner_disable = temp_r3;
    xcam_do_collis = (temp_r3 == 0);
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

static void _xCameraUpdate(xCamera* cam, F32 dt);

// Inlining issue
void xCameraUpdate(xCamera* cam, F32 dt)
{
    S32 i;
    // num_updates_f doesn't exist in the dwarf but needed to match
    F32 num_updates_f = ceil(144.0f * dt);
    S32 num_updates = num_updates_f;
    F32 sdt = dt / num_updates;

    for (i = 0; i < num_updates; i++)
    {
        sCamCollis = (i == num_updates - 1);

        _xCameraUpdate(cam, sdt);
    }
}

static void _xCameraUpdate(xCamera* cam, F32 dt)
{
    if (!cam->tgt_mat)
        return;

    static F32 last_dt = 1.0f / 60;

    //xCam_worldtocyl(cam->dcur, cam->hcur, cam->pcur, cam->tgt_mat, &cam->mat.pos, cam->flags);

    F32 wcvx = cam->mat.pos.x - cam->omat.pos.x;
    F32 wcvy = cam->mat.pos.y - cam->omat.pos.y;
    F32 wcvz = cam->mat.pos.z - cam->omat.pos.z;
    F32 m = 1.0f / last_dt;
    wcvx *= m;
    wcvy *= m;
    wcvz *= m;

    cam->omat.pos = cam->mat.pos;

    //xCam_buildbasis(cam);

    F32 dcv = wcvx * cam->mbasis.at.x + wcvz * cam->mbasis.at.z;
    F32 hcv = wcvy;
    F32 pcv = wcvx * cam->mbasis.right.x + wcvz * cam->mbasis.right.z;
    wcvx *= dt;
    wcvy *= dt;
    wcvz *= dt;

    cam->mat.pos.x += wcvx;
    cam->mat.pos.y += wcvy;
    cam->mat.pos.z += wcvz;

    if (cam->flags & 0x1)
    {
        F32 tnext = cam->tmr - dt;
        if (tnext <= 0.0f)
        {
            cam->flags &= ~0x1;
            cam->tmr = 0.0f;
            cam->omat.pos = cam->mat.pos;
        }
        else
        {
            F32 dtg = cam->dgoal - cam->dcur;
            F32 htg = cam->hgoal - cam->hcur;
            F32 ptg = (cam->dgoal + cam->dcur) * xDangleClamp(cam->pgoal - cam->pcur) * 0.5f;
            F32 dsv, hsv, psv;
            if (tnext <= cam->tm_dec)
            {
                F32 T_inv = 1.0f / cam->tmr;
                dsv = (2.0f * dtg - dcv * dt) * T_inv;
                hsv = (2.0f * htg - hcv * dt) * T_inv;
                psv = (2.0f * ptg - pcv * dt) * T_inv;
            }
            else if (tnext <= cam->tm_acc)
            {
                F32 T_inv = 1.0f / (2.0f * cam->tmr - dt - cam->tm_dec);
                dsv = (2.0f * dtg - dcv * dt) * T_inv;
                hsv = (2.0f * htg - hcv * dt) * T_inv;
                psv = (2.0f * ptg - pcv * dt) * T_inv;
            }
            else
            {
                F32 it = cam->tm_acc + (cam->tmr - dt) - cam->tm_dec;
                F32 ot = 2.0f / (cam->tmr + cam->tm_acc - cam->tm_dec);
                F32 T_inv = 1.0f / (cam->tmr - cam->tm_acc);
                dsv = (2.0f * dtg - (dtg * ot + cam->depv) * 0.5f * it - dcv * dt) * T_inv;
                hsv = (2.0f * htg - (htg * ot + cam->hepv) * 0.5f * it - hcv * dt) * T_inv;
                psv = (2.0f * ptg - (ptg * ot + cam->pepv) * 0.5f * it - pcv * dt) * T_inv;
            }
            F32 dpv = dsv - dcv;
            F32 hpv = hsv - hcv;
            F32 ppv = psv - pcv;
            F32 vax = cam->mbasis.right.x * ppv + cam->mbasis.at.x * dpv;
            F32 vay = cam->mbasis.right.y * ppv + hpv;
            F32 vaz = cam->mbasis.right.z * ppv + cam->mbasis.at.z * dpv;
            vax *= dt;
            vay *= dt;
            vaz *= dt;
            cam->mat.pos.x += vax;
            cam->mat.pos.y += vay;
            cam->mat.pos.z += vaz;
            cam->tmr = tnext;
        }
    }
    else
    {
        if (cam->flags & 0x2)
        {
            //if (xeq(cam->dcur / cam->dgoal, 1.0f, 1e-5f))
            {
            }
            //else
            {
                F32 dtg = cam->dgoal - cam->dcur;
                //xCam_CorrectD(cam, dtg, dcv, dt);
            }
        }
        else if (cam->dmax > cam->dmin)
        {
            if (cam->dcur < cam->dmin)
            {
                F32 dtg = cam->dmin - cam->dcur;
                //xCam_CorrectD(cam, dtg, dcv, dt);
            }
            else if (cam->dcur > cam->dmax)
            {
                F32 dtg = cam->dmax - cam->dcur;
                //xCam_CorrectD(cam, dtg, dcv, dt);
            }
        }

        if (cam->flags & 0x4)
        {
            //if (xeq(cam->hcur / cam->hgoal, 1.0f, 1e-5f))
            {
            }
            //else
            {
                F32 htg = cam->hgoal - cam->hcur;
                //xCam_CorrectH(cam, htg, hcv, dt);
            }
        }
        else if (cam->hmax > cam->hmin)
        {
            if (cam->hcur < cam->hmin)
            {
                F32 htg = cam->hmin - cam->hcur;
                //xCam_CorrectH(cam, htg, hcv, dt);
            }
            else if (cam->hcur > cam->hmax)
            {
                F32 htg = cam->hmax - cam->hcur;
                //xCam_CorrectH(cam, htg, hcv, dt);
            }
        }

        if (cam->flags & 0x8)
        {
            //if (xeq(cam->pcur / cam->pgoal, 1.0f, 1e-5f))
            {
            }
            //else
            {
                F32 ptg = cam->dcur * xDangleClamp(cam->pgoal - cam->pcur);
                //xCam_CorrectP(cam, ptg, pcv, dt);
            }
        }
        else if (cam->pmax > cam->pmin)
        {
            F32 dphi = xDangleClamp(cam->pmax - cam->pcur);
            F32 dplo = xDangleClamp(cam->pmin - cam->pcur);
            if (dplo > 0.0f && (dphi > 0.0f || xabs(dplo) <= xabs(dphi)))
            {
                F32 ptg = (1e-5f + dplo) * cam->dcur;
                //xCam_CorrectP(cam, ptg, pcv, dt);
            }
            else if (dphi < 0.0f)
            {
                F32 ptg = (dphi - 1e-5f) * cam->dcur;
                //xCam_CorrectP(cam, ptg, pcv, dt);
            }
            else
            {
                //xCam_DampP(cam, pcv, dt);
            }
        }
        else
        {
            //xCam_DampP(cam, pcv, dt);
        }
    }

    if (cam->flags & 0x80)
    {
        xVec3 oeu, eu;
        xMat3x3GetEuler(&cam->mat, &eu);
        xMat3x3GetEuler(&cam->omat, &oeu);

        F32 m = 1.0f / last_dt;
        F32 ycv = m * xDangleClamp(eu.x - oeu.x);
        F32 pcv = m * xDangleClamp(eu.y - oeu.y);
        F32 rcv = m * xDangleClamp(eu.z - oeu.z);
        ycv *= cam->yaw_ccv;
        pcv *= cam->pitch_ccv;
        rcv *= cam->roll_ccv;

        cam->omat = cam->mat;
        cam->yaw_cur += ycv * dt;
        cam->pitch_cur += pcv * dt;
        cam->roll_cur += rcv * dt;

        if (cam->flags & 0x40)
        {
            F32 tnext = cam->ltmr - dt;
            if (tnext <= 0.0f)
            {
                cam->flags &= ~0x40;
                cam->ltmr = 0.0f;
            }
            else
            {
                F32 ytg = xDangleClamp(cam->yaw_goal - cam->yaw_cur);
                F32 ptg = xDangleClamp(cam->pitch_goal - cam->pitch_cur);
                F32 rtg = xDangleClamp(cam->roll_goal - cam->roll_cur);
                F32 ysv, psv, rsv;
                if (tnext <= cam->ltm_dec)
                {
                    F32 T_inv = 1.0f / cam->ltmr;
                    ysv = (2.0f * ytg - ycv * dt) * T_inv;
                    psv = (2.0f * ptg - pcv * dt) * T_inv;
                    rsv = (2.0f * rtg - rcv * dt) * T_inv;
                }
                else if (tnext <= cam->ltm_acc)
                {
                    F32 T_inv = 1.0f / (2.0f * cam->ltmr - dt - cam->ltm_dec);
                    ysv = (2.0f * ytg - ycv * dt) * T_inv;
                    psv = (2.0f * ptg - pcv * dt) * T_inv;
                    rsv = (2.0f * rtg - rcv * dt) * T_inv;
                }
                else
                {
                    F32 it = cam->ltm_acc + (cam->ltmr - dt) - cam->ltm_dec;
                    F32 ot = 2.0f / (cam->ltmr + cam->ltm_acc - cam->ltm_dec);
                    F32 T_inv = 1.0f / (cam->ltmr - cam->ltm_acc);
                    ysv = ((2.0f * ytg - (ytg * ot + cam->yaw_epv) * 0.5f * it) - ycv * dt) * T_inv;
                    psv =
                        ((2.0f * ptg - (ptg * ot + cam->pitch_epv) * 0.5f * it) - pcv * dt) * T_inv;
                    rsv =
                        ((2.0f * rtg - (rtg * ot + cam->roll_epv) * 0.5f * it) - rcv * dt) * T_inv;
                }
                F32 ypv = ysv - ycv;
                F32 ppv = psv - pcv;
                F32 rpv = rsv - rcv;
                cam->yaw_cur += ypv * dt;
                cam->pitch_cur += ppv * dt;
                cam->roll_cur += rpv * dt;
                xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);
                cam->ltmr = tnext;
            }
        }
        else
        {
            //if (xeq(cam->yaw_cur, cam->yaw_goal, 1e-5f))
            {
            }
            //else
            {
                F32 ytg = xDangleClamp(cam->yaw_goal - cam->yaw_cur);
                //xCam_CorrectYaw(cam, ytg, ycv, dt);
            }

            //if (xeq(cam->pitch_cur, cam->pitch_goal, 1e-5f))
            {
            }
            //else
            {
                F32 ptg = xDangleClamp(cam->pitch_goal - cam->pitch_cur);
                //xCam_CorrectPitch(cam, ptg, pcv, dt);
            }

            //if (xeq(cam->roll_cur, cam->roll_goal, 1e-5f))
            {
            }
            //else
            {
                F32 rtg = xDangleClamp(cam->roll_goal - cam->roll_cur);
                //xCam_CorrectRoll(cam, rtg, rcv, dt);
            }

            xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);
        }
    }
    else
    {
        xQuatFromMat(&cam->orn_cur, &cam->mat);

        xQuat oq;
        xQuatFromMat(&oq, &cam->omat);

        xQuat qdiff_o_c;
        xQuatDiff(&qdiff_o_c, &oq, &cam->orn_cur);

        xRot rot_cv;
        //xQuatToAxisAngle(&qdiff_o_c, &rot_cv.axis, &rot_cv.angle);
        rot_cv.angle *= m;
        rot_cv.angle = 0.0f; // lol

        cam->omat = cam->mat;

        xVec3 f;
        xMat3x3RMulVec(&f, cam->tgt_mat, &cam->focus);
        xVec3AddTo(&f, &cam->tgt_mat->pos);

        xVec3 v;
        F32 dist;
        //xVec3NormalizeDistXZMacro(&v, &cam->mat.pos, &cam->tgt_mat->pos, &dist);
        v.y = 0.0f;

        if (cam->tgt_mat->at.x * v.x + cam->tgt_mat->at.y * v.y + cam->tgt_mat->at.z * v.z < 0.0f)
        {
            F32 mpx = f.x - cam->tgt_mat->pos.x;
            F32 mpy = f.y - cam->tgt_mat->pos.y;
            F32 mpz = f.z - cam->tgt_mat->pos.z;
            F32 s = (mpx * v.x + mpy * v.y + mpz * v.z) * -2.0f;
            mpx = v.x * s;
            mpy = v.y * s;
            mpz = v.z * s;
            f.x += mpx;
            f.y += mpy;
            f.z += mpz;
        }

        xMat3x3 des_mat;
        xMat3x3LookAt(&des_mat, &f, &cam->mat.pos);

        xMat3x3 latgt;
        xMat3x3LookAt(&latgt, &cam->tgt_mat->pos, &cam->mat.pos);

        F32 ang_dist = xacos(latgt.at.x * des_mat.at.x + latgt.at.y * des_mat.at.y +
                             latgt.at.z * des_mat.at.z);

        if (ang_dist > DEG2RAD(30.0f))
        {
            xQuat a;
            xQuatFromMat(&a, &latgt);

            xQuat b;
            xQuatFromMat(&b, &des_mat);

            xQuat o;
            F32 s = PI - ang_dist;
            if (s < DEG2RAD(90.0f))
            {
                if (s > DEG2RAD(5.0f))
                {
                    xQuatSlerp(&o, &a, &b, s / ang_dist);
                }
                else
                {
                    o = a;
                }
            }
            else
            {
                xQuatSlerp(&o, &a, &b, DEG2RAD(30.0f) / ang_dist);
            }

            xQuatToMat(&o, &des_mat);
        }

        xQuat desq;
        xQuatFromMat(&desq, &des_mat);

        //xCameraLook(cam, 0, &desq, 0.25f, 0.0f, 0.0f);

        xQuat difq;
        xQuatConj(&difq, &cam->orn_cur);
        xQuatMul(&difq, &difq, &desq);

        xQuat newq;
        xQuatSlerp(&newq, &cam->orn_cur, &desq, 25.5f * dt);
        xQuatToMat(&newq, &cam->mat);
    }

    while (xcam_do_collis && sCamCollis)
    {
        xSweptSphere sws;

        xVec3 tgtpos;
        tgtpos.x = cam->tgt_mat->pos.x;
        tgtpos.y = 0.7f + cam->tgt_mat->pos.y;
        tgtpos.z = cam->tgt_mat->pos.z;

        xSweptSpherePrepare(&sws, &tgtpos, &cam->mat.pos, 0.07f);
        //xSweptSphereToEnv(&sws, globals.sceneCur->env);

        xRay3 ray;
        xVec3Copy(&ray.origin, &sws.start);
        xVec3Sub(&ray.dir, &sws.end, &sws.start);

        ray.max_t = xVec3Length(&ray.dir);

        F32 one_len = 1.0f / MAX(ray.max_t, 1e-5f);
        xVec3SMul(&ray.dir, &ray.dir, one_len);

        ray.flags = 0x800;
        if (!(ray.flags & 0x400))
        {
            ray.flags |= 0x400;
            ray.min_t = 0.0f;
        }

        //xRayHitsGrid(&colls_grid, globals.sceneCur, &ray, SweptSphereHitsCameraEnt, &sws.qcd, &sws);
        //xRayHitsGrid(&colls_oso_grid, globals.sceneCur, &ray, SweptSphereHitsCameraEnt, &sws.qcd, &sws);

        if (sws.curdist != sws.dist)
        {
            F32 stopdist = MAX(sws.curdist, 0.6f);
            cam->mat.pos.x = ray.origin.x + stopdist * ray.dir.x;
            cam->mat.pos.y = ray.origin.y + stopdist * ray.dir.y;
            cam->mat.pos.z = ray.origin.z + stopdist * ray.dir.z;
        }

        break;
    }

    last_dt = dt;

    iCameraUpdatePos(cam->lo_cam, &cam->mat);
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
                    xSweptSphereToBox(*sws, ent->bound.box.box);
                    return;
                }
                else if (ent->bound.type == XBOUND_TYPE_OBB)
                {
                    xSweptSphereToOBB(sws, &ent->bound.box.box, ent->bound.mat);
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
            xMat3x3Tolocal(&lr.origin, ent->bound.mat, &ray->origin);
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
        F32 dx__ = cam->mat.pos.x - cam->tgt_mat->pos.x;
        F32 dz__ = cam->mat.pos.z - cam->tgt_mat->pos.z;

        F32 dist2 = SQR(dx__) + SQR(dz__);
        F32 dist_inv;
        F32 d2d;
        S32 iVar7;
        U32 uVar5;

        if ((F32)iabs(dist2 - 1.0f) <= 0.00001f)
        {
            cam->mbasis.at.x = dx__;
            cam->mbasis.at.z = dz__;
            d2d = 1.0f;
        }
        else if ((F32)iabs(dist2) <= 0.00001f)
        {
            cam->mbasis.at.x = 0.0f;
            cam->mbasis.at.z = 0.0f;
            d2d = 0.0f;
        }
        else
        {
            if (dist2 > 0.0f)
            {
                uVar5 = *((U32*)&dist2) & 0x7f800000;
                if (uVar5 == 0x7f800000)
                {
                    if ((*((U32*)&dist2) & 0x7fffff) == 0)
                        iVar7 = 2;
                    else
                        iVar7 = 1;
                }
                else if (uVar5 < 0x7f800000 && uVar5 == 0)
                {
                    if ((*((U32*)&dist2) & 0x7fffff) == 0)
                        iVar7 = 3;
                    else
                        iVar7 = 5;
                }
                else
                {
                    iVar7 = 4;
                }

                if (iVar7 != 2)
                {
                    F32 inv_sqrt = 1.0f / dist2;
                    inv_sqrt = -(inv_sqrt * inv_sqrt * dist2 - 3.0f) * inv_sqrt * 0.5f;
                    if (inv_sqrt > 0.00001f)
                        dist_inv = 1.0f / inv_sqrt;
                    else
                        dist_inv = 100000.0f;
                }
                else
                {
                    dist_inv = 1.0f;
                }
            }
            else
            {
                dist_inv = 1.0f;
            }

            cam->mbasis.at.x = dx__ * dist_inv;
            cam->mbasis.at.z = dz__ * dist_inv;
            d2d = dist2;
        }

        if (d2d < 0.00001f)
        {
            cam->mbasis.at.x = cam->mat.at.x;
            cam->mbasis.at.z = cam->mat.at.z;

            dist2 = SQR(cam->mbasis.at.x) + SQR(cam->mbasis.at.z);

            if (dist2 > 0.001f)
            {
                if (dist2 > 0.0f)
                {
                    uVar5 = *((U32*)&dist2) & 0x7f800000;
                    if (uVar5 == 0x7f800000)
                    {
                        if ((*((U32*)&dist2) & 0x7fffff) == 0)
                            iVar7 = 2;
                        else
                            iVar7 = 1;
                    }
                    else if (uVar5 < 0x7f800000 && uVar5 == 0)
                    {
                        if ((*((U32*)&dist2) & 0x7fffff) == 0)
                            iVar7 = 3;
                        else
                            iVar7 = 5;
                    }
                    else
                    {
                        iVar7 = 4;
                    }

                    if (iVar7 != 2)
                    {
                        F32 inv_sqrt = 1.0f / dist2;
                        inv_sqrt = -(inv_sqrt * inv_sqrt * dist2 - 3.0f) * inv_sqrt * 0.5f;
                        if (inv_sqrt > 0.00001f)
                            dist_inv = 1.0f / inv_sqrt;
                        else
                            dist_inv = 100000.0f;
                    }
                    else
                    {
                        dist_inv = 1.0f;
                    }
                }
                else
                {
                    dist_inv = 1.0f;
                }

                cam->mbasis.at.x *= dist_inv;
                cam->mbasis.at.z *= dist_inv;
            }
            else
            {
                cam->mbasis.at.x = isin(cam->pcur);
                cam->mbasis.at.z = icos(cam->pcur);
            }
        }

        cam->mbasis.at.y = 0.0f;
        cam->mbasis.up.x = 0.0f;
        cam->mbasis.up.y = 1.0f;
        cam->mbasis.up.z = 0.0f;
        cam->mbasis.right.x = cam->mbasis.at.z;
        cam->mbasis.right.y = 0.0f;
        cam->mbasis.right.z = -cam->mbasis.at.x;
    }
}

void xCameraReset(xCamera* cam, F32 d, F32 h, F32 pitch)
{
    sInvisWallHack = (RpAtomic*)xSTFindAsset(0xB8895D14, NULL);

    xMat4x3Identity(&cam->mat);

    cam->omat = cam->mat;
    cam->focus.x = 0.0f;
    cam->focus.y = 0.0f;
    cam->focus.z = 10.0f;
    cam->tran_accum.x = 0.0f;
    cam->tran_accum.y = 0.0f;
    cam->tran_accum.z = 0.0f;
    cam->flags = 0;

    F32 goal_p = 3.1415927f;

    if (cam->tgt_mat)
    {
        goal_p += atan2(cam->tgt_mat->at.x, cam->tgt_mat->at.z);
    }

    xCameraMove(cam, 0x2E, d, h, goal_p, 0.0f, 0.66666669f, 0.66666669f);

    cam->pitch_goal = pitch;
    cam->pitch_cur = pitch;
    cam->roll_cur = 0.0f;

    xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);

    cam->omat = cam->mat;
    cam->yaw_ct = 1.0f;
    cam->yaw_cd = 1.0f;
    cam->yaw_ccv = 0.65f;
    cam->yaw_csv = 1.0f;
    cam->pitch_ct = 1.0f;
    cam->pitch_cd = 1.0f;
    cam->pitch_ccv = 0.7f;
    cam->pitch_csv = 1.0f;
    cam->roll_ct = 1.0f;
    cam->roll_cd = 1.0f;
    cam->roll_ccv = 0.7f;
    cam->roll_csv = 1.0f;
    cam->flags |= 0x80;

    xcam_do_collis = 1;
    xcam_collis_owner_disable = 0;
    cam->smoothOutwardSlidePos = 10.0f;
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
    cam->lo_cam = globals.screen->icam;
    cam->fov = 75.0;
    cam->bound.sph.center.x = 0.0f;
    cam->bound.sph.center.y = 0.0f;
    cam->bound.sph.center.z = 0.0f;
    cam->bound.sph.r = 0.5f;
    cam->tgt_mat = NULL;
    cam->tgt_omat = NULL;
    cam->tgt_bound = NULL;
    cam->sc = NULL;
    cam->tran_accum.x = 0.0f;
    cam->tran_accum.y = 0.0f;
    cam->tran_accum.z = 0.0f;
}
