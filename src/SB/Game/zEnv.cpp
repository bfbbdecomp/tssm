#include "zEnv.h"

#include "zGlobals.h"
#include "zLight.h"
#include "xstransvc.h"
#include "xGroup.h"
#include "zSurface.h"

// DWARF says void zEnvEventCB(xBase* to, U32 toEvent, F32* toParam, xBase* toParamWidget)
void zEnvEventCB(xBase* a0, xBase* a1, U32 a2, const F32* a3, xBase* a4, U32 a5)
{
}

void zEnvLoad(_zEnv* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void zEnvSave(_zEnv* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void zEnvRender(xEnv* env)
{
    RpWorld* world = env->geom->world;
    S32 num = RpWorldGetNumMaterials(world);

    for (S32 i = 0; i < num; i++)
    {
        xSurface* sp = zSurfaceGetSurface(i);
        zSurfaceProps* pp = (zSurfaceProps*)sp->moprops;

        if (pp && pp->asset && pp->texanim_flags & 0x1)
        {
            RpMaterial* mp = RpWorldGetMaterial(world, i);

            if (mp)
            {
                xGroup* g = (xGroup*)pp->texanim[0].group_ptr;

                if (g)
                {
                    U32 texid = xGroupGetItem(g, pp->texanim[0].group_idx);
                    RwTexture* texptr = (RwTexture*)xSTFindAsset(texid, NULL);

                    if (texptr)
                    {
                        RpMaterialSetTexture(mp, texptr);
                    }
                }
            }
        }
    }

    xEnvRender(env, FALSE);
}

void zEnvStartingCamera(_zEnv*)
{
    return;
}

void zEnvSetup(_zEnv* env)
{
    // TODO
    // I don't think the struct is correct

    // This function seems to be correct in BFBB, our globals struct is fucked though.
    // Keep this the way it is, and fix the struct

    xBaseSetup(env);
    iEnvLightingBasics((iEnv*)globals.sceneCur->env->geom, (xEnvAsset*)env->easset);
}

// Again, Copied from Battle, Needs a good amount of work
void zEnvInit(_zEnv* env, xEnvAsset* easset)
{
    U32 bufsize;
    void* buf;

    xBaseInit(env, easset);

    env->easset = easset;
    //env->eventFunc = zEnvEventCB;

    if (env->linkCount)
    {
        env->link = (xLinkAsset*)(env->easset + 1);
    }

    globals.sceneCur->zen = env;
    globals.sceneCur->zen = env;

    xSTAssetCountByType('BSP ');

    buf = xSTFindAsset(easset->bspAssetID, &bufsize);

    if (buf)
    {
        xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 0);
    }

    zSurfaceRegisterMapper(easset->bspMapperID);

    if (easset->bspCollisionAssetID)
    {
        buf = xSTFindAsset(easset->bspCollisionAssetID, &bufsize);

        if (buf)
        {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 1);
        }
    }

    if (easset->bspFXAssetID)
    {
        buf = xSTFindAsset(easset->bspFXAssetID, &bufsize);

        if (buf)
        {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 2);
        }
    }

    if (easset->bspCameraAssetID)
    {
        buf = xSTFindAsset(easset->bspCameraAssetID, &bufsize);

        if (buf)
        {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 3);
        }
    }

    if (globals.sceneCur->env && globals.sceneCur->env->geom)
    {
        zLightResetAll(globals.sceneCur->env);
        iLightInit(globals.sceneCur->env->geom->world);
    }
}

// Inlining issue
void zEnvInit(void* env, void* easset)
{
    zEnvInit((_zEnv*)env, (xEnvAsset*)easset);
}
