#include "zConditional.h"

S32 zConditionalEventCB(xBase*, xBase*, U32, const F32*, xBase*);

void zConditionalLoad(_zConditional* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void zConditionalSave(_zConditional* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void zConditionalInit(void* b, void* asset)
{
    _zConditional* cond = (_zConditional*)b;

    xBaseInit((xBase*)cond, (xBaseAsset*)asset);
    //cond->eventFunc = (xBaseEventCB)zConditionalEventCB;
    //cond->asset = asset;

    if (cond->linkCount != 0)
    {
        cond->link = (xLinkAsset*)(cond->asset + 1);
    }
    else
    {
        cond->link = NULL;
    }
    zVarInit(zVarEntryTable);
}
