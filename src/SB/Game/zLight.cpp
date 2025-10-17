#include "zLight.h"

void zLightSetVolume(zVolume* vol)
{
    if (!vol)
    {
        sPartitionVolume = 0;
    }
    else
    {
        U32 lp_id = xStrHash("LIGHT_PARTITION" + 9);
        if (vol->id == lp_id)
        {
            sPartitionVolume = vol;
        }
    }
}

void zLightRemoveLocalEnv()
{
    int i;
    const RwLLLink* link;

    for (i = 0; i < gNumTemporaryLights; i++)
    {
        link = gLightWorld->directionalLightList.link.prev;
        link->prev->next = link->next;
        link->next->prev = link->prev;
    }
    gNumTemporaryLights = 0;
}

void zLightAddLocal(xEnt* ent)
{
}

void zLightLoad(_zLight* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void zLightSave(_zLight* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void zLightDestroyAll()
{
    S32 total = sLightTotal;

    for (int i = 0; i < total; i++, sLightTotal--)
    {
        zLightDestroy(sLight[i]);
    }
    sLightTotal = 0;
}

void zLightResolveLinks()
{
    S32 i;
    _zLight* zl;

    for (i = 0; i < sLightTotal; i++)
    {
        zl = sLight[i];
        if (zl->tasset->attachID)
        {
            zl->attached_to = zSceneFindObject(zl->tasset->attachID);
        }
        else
        {
            zl->attached_to = 0;
        }
    }
}

void zLightInit(void* b, void* tasset)
{
    zLightInit((xBase*)b, (zLightAsset*)tasset);
}
