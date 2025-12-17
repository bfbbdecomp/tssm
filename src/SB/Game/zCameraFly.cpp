#include "xLinkAsset.h"
#include <types.h>

#include "xBase.h"
#include "xEvent.h"
#include "xScene.h"

#include "zBase.h"
#include "zCameraFly.h"
#include "zCamera.h"
#include "zGlobals.h"
#include "zMusic.h"

extern U32 zcam_flyasset_current;

void zCameraFlyEventCB(xBase* from, xBase* to, U32 toEvent, F32* toParam, xBase* b3, U32 unknown)
{
    zCameraFly* fly = (zCameraFly*)to;

    switch (toEvent)
    {
    case eEventEnable:
        fly->baseFlags |= 1;
        break;

    case eEventDisable:
        fly->baseFlags &= ~1;
        break;

    case eEventRun:
        if (fly->baseFlags & 1)
        {
            zCameraFlyStart(fly->casset->flyID);
        }
        break;

    case eEventStop:
        break;

    case eEventSceneBegin:
        break;

    default:
        break;
    }
}

void zCameraFly_Load(zCameraFly* fly, xSerial* s)
{
    xBaseLoad((xBase*)fly, s);
}

void zCameraFly_Save(zCameraFly* fly, xSerial* s)
{
    xBaseSave((xBase*)fly, s);
}
void zCameraFly_Update(xBase* to, xScene* scene, F32 dt)
{
}
void zCameraFly_Setup(zCameraFly* fly)
{
    fly->baseFlags |= (U16)2;
}

void zCameraFly_Init(xBase& data, xDynAsset& asset, size_t)
{
    xBaseInit(&data, &asset);
    zCameraFly* fly = (zCameraFly*)&data;
    fly->casset = (CameraFly_asset*)&asset;
    data.eventFunc = zCameraFlyEventCB;
    if (data.linkCount != 0)
    {
        data.link = (xLinkAsset*)&asset;
    }
    else
    {
        data.link = NULL;
    }
}
