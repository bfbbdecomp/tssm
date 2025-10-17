#include "zEGenerator.h"

void zEGenerator_TurnOn(zEGenerator* egen)
{
    zEGenAsset* zasset;
    xAnimPlay* aplay;
    xAnimSingle* asing;
    U16 itgt;
    U16 imp;
    S32 i;
    xLinkAsset* link;
    U32 dstAssetID;
    xBase* b;
    xVec3 destOffset;
    xEnt* ent;
    //S32 i;
    _tagLightningAdd add;
}

void zEGenerator_Render(zEGenerator* egen)
{
    xEntRender(egen);
}

void zEGenerator_Update(zEGenerator* egen, xScene* sc, F32 dt)
{
    xBase* b;
    zEnt* ent;
}

void zEGenerator_Move(zEGenerator* egen, xScene* xscn, F32 tmpFloat)
{
}

void zEGenerator_Reset(zEGenerator* egen, xScene* xscn)
{
    zEntReset((zEnt*)egen);
    zEGenAsset* zasset = (zEGenAsset*)egen->asset;
    egen->flags = (U16)zasset->flags;
    if (egen->flags & 1)
    {
        zEGenerator_TurnOn(egen);
    }
    else
    {
        zEGenerator_TurnOff(egen);
    }
    if (zasset->moreFlags & 0b100)
    {
        egen->chkby = XENT_COLLTYPE_NONE;
    }
    else
    {
        egen->chkby = XENT_COLLTYPE_PLYR | XENT_COLLTYPE_NPC;
    }
    egen->bupdate((xEnt*)egen, (xVec3*)&egen->model->Mat->pos);
}

void zEGenerator_Load(zEGenerator* ent, xSerial* s)
{
    zEntLoad(ent, s);
}

void zEGenerator_Save(zEGenerator* ent, xSerial* s)
{
    zEntSave(ent, s);
}

void zEGenerator_Setup(zEGenerator* egen, xScene* xscn)
{
    zEntSetup(egen);
    egen->num_dsts = 0;
    for (S32 i = 0; i < egen->linkCount; i++)
    {
        xLinkAsset* la = (xLinkAsset*)&egen->link[i];
        xBase* b = (xBase*)zSceneFindObject(la->dstAssetID);
        if (b != NULL && la->dstEvent == 0xc0)
        {
            if (b->baseType == eBaseTypeMovePoint)
            {
                egen->num_dsts++;
            }
            else if (b->baseFlags & 0x20)
            {
                egen->num_dsts++;
            }
        }
    }
}

void zEGenerator_Init(void* egen, void* asset)
{
}
