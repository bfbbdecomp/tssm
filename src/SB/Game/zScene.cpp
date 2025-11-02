#include "zScene.h"

#include "zRewards.h"
#include "zRenderState.h"
#include "zEntDestructObj.h"
#include "zParCmd.h"
#include "zAnimList.h"
#include "zDispatcher.h"
#include "zCameraTweak.h"
#include "zCamMarker.h"
#include "zMusic.h"
#include "zGrid.h"
#include "xDecal.h"
#include "xFog.h"
#include "zTalkBox.h"
#include "zUIFont.h"
#include "zEntButton.h"
#include "xSkyDome.h"
#include "zLOD.h"
#include "zGame.h"
#include "zGoo.h"
#include "iModel.h"
#include "xShadow.h"
#include "zLightning.h"
#include "zLight.h"
#include "zActionLine.h"
#include "xPtankPool.h"
#include "zParPTank.h"
#include "zNPCMgr.h"
#include "xScrFx.h"

U8 HACK_BASETYPE;
static S32 bytesNeeded;
static S32 availOnDisk;
static S32 neededFiles;
static F32 offsetx;
static F32 offsety;
static U32 enableScreenAdj;
volatile static F32 oldOffsetx;
volatile static F32 oldOffsety;
static S32 sMemDepthSceneStart = -1;
static S32 sMemDepthJustHIPStart = -1;
_zEnv* gCurEnv;
U32 gTransitionSceneID;
F32 gSceneUpdateTime;
static xVec3 sOldPosPlayer;
static xVec3 sOldPosCamera;
static U32 sSuddenMove;

struct zSceneLevel
{
    const char* desc;
    const char* prefix;
};

struct IDBasePair
{
    U32 id;
    xBase* base;
};

static IDBasePair* scobj_idbps;
static S32 scobj_size = -1;
static S32 nidbps = -1;

extern U32 _1251;
extern char byte_803D0884;
extern U32 _2014;

extern U32 _1250;
extern F32 _1373;
extern F32 _1374;
extern F32 _1375;
extern F32 _1493;
extern F32 _1494;
extern F32 _1495;
extern F32 _1496_0;
extern U32 _2013;
extern F32 _2094;
extern F32 _2095_0;
extern F32 _2096_0;
extern F32 _2097_0;
extern F32 _2242;

static void zSceneObjHashtableInit(S32 count);
static void zSceneObjHashtableExit();
static S32 zSceneObjHashtableUsage();
static void zSceneObjHashtableAdd(U32 id, xBase* base);
static xBase* zSceneObjHashtableGet(U32 id);
static xBase* zSceneExitSoundIteratorCB(xBase* b, zScene*, void*);
static void zSceneAutoSave();

U32 zScene_ScreenAdjustMode()
{
    return enableScreenAdj;
}

void zSceneSetOldScreenAdj()
{
    oldOffsetx = offsetx;
    oldOffsety = offsety;
}

void zSceneEnableScreenAdj(U32 enable)
{
    enableScreenAdj = enable;
}
void zSceneCardCheckStartup_set(S32 needed, S32 available, S32 files)
{
    bytesNeeded = needed;
    availOnDisk = available;
    neededFiles = files;
}

void zScene_UpdateFlyToInterface(F32 dt)
{
}

U32 zSceneGetLevelIndex()
{
    return zSceneGetLevelIndex(globals.sceneCur->sceneID);
}

const char* zSceneGetLevelName(U32 sceneID)
{
    char c1 = (sceneID >> 24) & 0xFF;
    char c2 = (sceneID >> 16) & 0xFF;

    // Add in the updated LevelTable to this file

    // for (S32 i = 0; i < sizeof(sLevelTable) / sizeof(sLevelTable[0]); i++)
    // {
    //     if (c1 == sLevelTable[i].prefix[0] && c2 == sLevelTable[i].prefix[1])
    //     {
    //         return sLevelTable[i].desc;
    //     }
    // }

    return "Level Not Found";
}

void zSceneMemLvlChkCB()
{
}

const char* zSceneGetName(xBase* b)
{
    if (b)
    {
        const char* n = xSTAssetName(b->id);

        if (n)
        {
            return n;
        }
    }

    return "";
}

const char* zSceneGetName(U32 gameID)
{
    xBase* b = zSceneFindObject(gameID);

    if (b)
    {
        return zSceneGetName(b);
    }

    return "";
}

xBase* zSceneFindObject(U32 gameID)
{
    return 0;
}

static void zSceneObjHashtableAdd(U32 id, xBase* base)
{
    S32 k, chkd;

    chkd = id & (scobj_size - 1);

    for (k = 0; k < scobj_size; k++)
    {
        IDBasePair* idbp = &scobj_idbps[chkd];

        if (idbp->id == 0)
        {
            idbp->id = id;
            idbp->base = base;
            nidbps++;
            break;
        }

        chkd++;

        if (chkd == scobj_size)
        {
            chkd = 0;
        }
    }
}

void zSceneRenderPreFX()
{
    // PORT CODE FROM BATTLE
}

void zSceneRenderPostFX()
{
    // PORT CODE FROM BATTLE
}

void zSceneRender()
{
    zSceneRenderPreFX();
    xScrFxRender(globals.oldSkoolCamera.lo_cam);
    zSceneRenderPostFX();
}

void zSceneUpdate(F32 dt)
{
    // REFERENCE BFBB
}

S32 zSceneSetup_serialTraverseCB(U32 clientID, xSerial* xser)
{
    char uiName[16];
    S32 val = 0;
    xBase* b;

    xser->Read_b1(&val);

    if (val == 0)
    {
        return 1;
    }

    sprintf(uiName, "%s", xUtil_idtag2string(clientID, 0));

    uiName[1] = ' ';

    strcat(uiName, " VISITED");

    b = zSceneFindObject(xStrHash(uiName));

    if (b)
    {
        b->baseFlags |= 0x1;
    }

    return 1;
}

static void ActivateCB(xBase* base)
{
    base->baseFlags &= (U8)~0x40;
}

static void DeactivateCB(xBase* base)
{
    base->baseFlags |= 0x40;
}

void zSceneSetup()
{
    // REFERENCE BFBB
}

void zSceneReset()
{
    zScene* s = globals.sceneCur;

    zEntPlayerPreReset();

    gOccludeCount = 0;

    xScrFxLetterboxReset();
    zEntPickup_FlushGrabbed();
    zEntPickup_SceneReset();
    zMusicReset();
    //zThrown_Reset();
    zShrapnel_Reset();
    zFX_SceneReset();
    xFXSceneReset();
    xShadowManager_Reset();
    //ztalkbox::reset_all();
    //zCameraTweakGlobal_Reset();
    xPadDestroyRumbleChain(0);

    globals.cmgr = NULL;

    zEntEventAll(NULL, 0, eEventRoomEnd, NULL);
    zEntEventAll(NULL, 0, eEventSceneEnd, NULL);

    zSceneSave(s, NULL);
    zSceneAutoSave();

    xSndStopAll(~SND_CAT_UI);
    xSndUpdate();
    iSndWaitForDeadSounds();

    for (U32 i = 0; i < s->num_base; i++)
    {
        if (s->base[i])
        {
            switch (s->base[i]->baseType)
            {
            case eBaseTypePickup:
            case eBaseTypeCamera:
            case eBaseTypeDoor:
            case eBaseTypeSavePoint:
            case eBaseTypeItem:
            case eBaseTypeStatic:
            case eBaseTypeDynamic:
            case eBaseTypeMovePoint:
            case eBaseTypeTimer:
            case eBaseTypeBubble:
            case eBaseTypePortal:
            case eBaseTypeGroup:
            case eBaseTypeSFX:
            case eBaseTypeFFX:
            case eBaseTypeVFX:
            case eBaseTypeButton:
            case eBaseTypeProjectile:
            case eBaseTypeSurface:
            case eBaseTypeDestructObj:
            case eBaseTypeGust:
            case eBaseTypeVolume:
            case eBaseTypeDispatcher:
            case eBaseTypeCond:
            case eBaseTypeUI:
            case eBaseTypeUIFont:
            case eBaseTypeProjectileType:
            case eBaseTypeFog:
            case eBaseTypeLight:
            case eBaseTypeParticleEmitter:
            case eBaseTypeParticleSystem:
            case eBaseTypeCutsceneMgr:
            default:
            {
                zEntEvent(NULL, 0, s->base[i], eEventReset, NULL, NULL, 1);
                break;
            }
            case eBaseTypeCounter:
            {
                //xCounterReset(s->base[i]);
                break;
            }
            case eBaseTypePlayer:
            {
                zEntPlayerReset((xEnt*)s->base[i]);
                break;
            }
            case eBaseTypePlatform:
            {
                //zPlatform_Reset((zPlatform*)s->base[i], globals.sceneCur);
                break;
            }
            case eBaseTypePendulum:
            {
                //zPendulum_Reset((_zPendulum*)s->base[i], globals.sceneCur);
                break;
            }
            case eBaseTypeHangable:
            {
                //zEntHangable_Reset((zEntHangable*)s->base[i]);
                break;
            }
            case eBaseTypeLobMaster:
            {
                zEntEvent(s->base[i], eEventReset);
                break;
            }
            case eBaseTypeEGenerator:
            {
                //zEGenerator_Reset((zEGenerator*)s->base[i], globals.sceneCur);
                break;
            }
            case eBaseTypeEnv:
            {
                _zEnv* env = (_zEnv*)s->base[i];

                if (env)
                {
                    if (zSceneFindObject(env->easset->startCameraAssetID))
                    {
                        //zCameraReset(&globals.camera);
                    }

                    xClimateInitAsset(&gClimate, env->easset);
                }
            }
            }
        }
    }

    zNPCMgr_sceneReset();
    zEntPlayer_LoadCheckPoint();
    zSceneLoad(globals.sceneCur, NULL);
    zSceneEnableVisited(s);
    xSerialTraverse(zSceneSetup_serialTraverseCB);
    xSkyDome_Setup();
    xUpdateCull_Reset(globals.updateMgr);
    xUpdateCull_Update(globals.updateMgr, 100);
    zGridReset(s);

    zEntEventAll(NULL, 0, eEventSceneBegin, NULL);
    zEntEventAll(NULL, 0, eEventRoomBegin, NULL);
    zEntEventAll(NULL, 0, eEventSceneReset, NULL);

    //zNPCTiki_InitStacking(globals.sceneCur);
    //zNPCTiki_InitFX(globals.sceneCur);

    xUpdateCull_Update(globals.updateMgr, 100);
    zLOD_Update(100);

    //zGameExtras_SceneReset();
}

void zSceneLoad(zScene* ent)
{
    // REFERENCE BFBB
}

void zSceneSave(zScene* ent)
{
    // REFERENCE BFBB
}

void zSceneSwitch(_zPortal* p, S32 forceSameScene)
{
    // REFERENCE BFBB
}

void zSceneUpdateSFXWidgets()
{
    zScene* s = globals.sceneCur;
    xSFXUpdateEnvironmentalStreamSounds((xSFX*)s->baseList[eBaseTypeSFX],
                                        s->baseCount[eBaseTypeSFX]);
}

void zSceneExit(S32 beginReload)
{
    // REFERENCE BFBB
}

void zSceneInit(U32 theSceneID, S32 reloadInProgress)
{
    F32 pdone;
    zScene* s;
    U32 i;

    U8 rgba_bkgrd[4];
    *(U32*)rgba_bkgrd = _1250;

    gTransitionSceneID = theSceneID;
    gOccludeCount = 0;

    char b[5];
    *(U32*)b = _1251;
    b[4] = byte_803D0884;

    sprintf(b, xUtil_idtag2string(theSceneID, 0));
    xStrupr(b);

    theSceneID = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];

    xUtil_idtag2string(theSceneID, 0);

    xFogClearFog();
    xSndSceneInit();

    if (!reloadInProgress)
    {
        sMemDepthSceneStart = xMemPushBase();
    }

    zGameScreenTransitionBegin();
    zParPTankInit();

    pdone = _1373;

    if (globals.useHIPHOP && !reloadInProgress)
    {
        zGameScreenTransitionUpdate(pdone, "... scene preload ...\n");

        S32 ver_hop = xSTPreLoadScene(theSceneID, NULL, 0x2);

        if (ver_hop >= 0x000A000F)
        {
            xSTQueueSceneAssets(theSceneID, 0x2);

            do
            {
                rgba_bkgrd[0] = 0;
                rgba_bkgrd[1] = 0;
                rgba_bkgrd[2] = 0;
                rgba_bkgrd[3] = 0;

                pdone += _1373;

                zGameScreenTransitionUpdate(pdone, "... scene loading ...\n", rgba_bkgrd);
            } while (xSTLoadStep(theSceneID) < _1374);

            xSTDisconnect(theSceneID, 0x2);
        }
    }

    sMemDepthJustHIPStart = xMemPushBase();
    s = (zScene*)xMemAllocSize(sizeof(zScene));

    globals.sceneCur = s;

    xSceneInit(s, 200, 2048, 2068, 250);

    s->env = (xEnv*)xMemAllocSize(sizeof(xEnv));
    s->sceneID = theSceneID;

    iTime time;

    time = iTimeGet();
    xUtil_idtag2string(theSceneID, 0);
    iTimeDiffSec(time);

    xSTPreLoadScene(theSceneID, NULL, 0x1);

    time = iTimeGet();
    xUtil_idtag2string(theSceneID, 0);
    iTimeDiffSec(time);

    xSTQueueSceneAssets(theSceneID, 0x1);

    time = iTimeGet();
    xUtil_idtag2string(theSceneID, 0);
    iTimeDiffSec(time);

    pdone += _1373;

    zGameScreenTransitionUpdate(pdone, "... scene asset queue ...\n");

    time = iTimeGet();
    xUtil_idtag2string(theSceneID, 0);
    iTimeDiffSec(time);

    do
    {
        rgba_bkgrd[0] = 0;
        rgba_bkgrd[1] = 0;
        rgba_bkgrd[2] = 0;
        rgba_bkgrd[3] = 0;

        pdone += _1373;

        zGameScreenTransitionUpdate(pdone, "... scene loading ...\n", rgba_bkgrd);
    } while (xSTLoadStep(theSceneID) < _1374);

    xSTDisconnect(theSceneID, 0x1);

    time = iTimeGet();
    xUtil_idtag2string(theSceneID, 0);
    iTimeDiffSec(time);

    pdone += _1373;

    zGameScreenTransitionUpdate(pdone, "...initializing scene - sound\n");

    zEntPlayer_LoadSounds();
    iSndInitSceneLoaded();
    xPadRumbleEnable(globals.currentActivePad, globals.option_vibration);

    xSTSwitchScene(theSceneID, NULL, NULL);

    globals.sceneCur->resolvID = zSceneFindObject;
    globals.sceneCur->id2Name = zSceneGetName;
    globals.sceneCur->base2Name = zSceneGetName;

    g_xSceneCur = globals.sceneCur;

    xModelPipeNumTables = xSTAssetCountByType('PIPT');

    for (S32 i = 0; i < xModelPipeNumTables; i++)
    {
        void* data = xSTFindAssetByType('PIPT', i, NULL);

        xModelPipeCount[i] = *(S32*)data;
        xModelPipeData[i] = (xModelPipeInfo*)((S32*)data + 1);
    }

    xModelLookupCount = 0;

    //PipeForAllSceneModels(PipeCountStuffCB);

    if (xModelLookupCount)
    {
        xModelLookupList =
            (xModelPipeLookup*)xMemAllocSize(xModelLookupCount * sizeof(xModelPipeLookup));

        xModelLookupCount = 0;

        //PipeForAllSceneModels(PipeAddStuffCB);
    }

    xModelBucket_PreCountReset();
    //PipeForAllSceneModels(xModelBucket_PreCountBucket);

    xModelBucket_PreCountAlloc(256);
    //PipeForAllSceneModels(xModelBucket_InsertBucket);

    xModelBucket_Init();
    //add_scene_tweaks();
    xPTankPoolSceneEnter();
    zParPTankSceneEnter();
    xDecalInit();
    xFXScenePrepare();
    zLasso_scenePrepare();
    zDispatcher_scenePrepare();

    S32 total_npcs = xSTAssetCountByType('VIL ');
    zNPCMgr_scenePrepare(total_npcs);

    zAnimListInit();
    zGooInit(24);

    zGameScreenTransitionUpdate(_1375, "...initializing scene - base types\n");

    for (i = 0; i < eBaseTypeCount; i++)
    {
        s->baseCount[i] = 0;
        s->baseList[i] = NULL;
    }

    zCollGeom_Init();
    zUI_Init();
    zUIFontInit();
    ztextbox::init();
    //ztalkbox::init();
    ztaskbox::init();

    xModelInstStaticAlloc = 1;
    s->num_base = 0;

    // for (i = 0; sInitTable[i].name; i++)
    // {
    //     U32 typeCount = xSTAssetCountByType(sInitTable[i].assetType);

    //     s->baseCount[sInitTable[i].baseType] = typeCount;
    //     s->num_base += typeCount;

    //     if (sInitTable[i].querySubObjects)
    //     {
    //         for (U32 j = 0; j < typeCount; j++)
    //         {
    //             s->num_base += sInitTable[i].querySubObjects(
    //                 xSTFindAssetByType(sInitTable[i].assetType, j, NULL));
    //         }
    //     }
    // }

    //add_dynamic_types(*s);

    if (s->num_base)
    {
        s->base = (xBase**)xMemAllocSize(s->num_base * sizeof(xBase*));
    }
    else
    {
        s->base = NULL;
    }

    zSceneObjHashtableInit(4096);
    xFFXPoolInit(12);
    xFFXShakePoolInit(3);
    xFFXRotMatchPoolInit(1);
    xEntSceneInit();
    xEntMotionDebugInit(s->baseCount[eBaseTypePlatform] + s->baseCount[eBaseTypePendulum] +
                        s->baseCount[eBaseTypeButton]);
    zLightSetVolume(NULL);
    //xPartitionReset();
    xFXRibbonSceneEnter();

    U32 base_idx = 0;

    // for (i = 0; sInitTable[i].name; i++)
    // {
    //     HACK_BASETYPE = sInitTable[i].baseType;

    //     if (sInitTable[i].func)
    //     {
    //         base_idx = sInitTable[i].func(s, &sInitTable[i], base_idx);
    //     }
    // }

    //init_dynamic_types(*s, base_idx);

    s->num_update_base = 0;

    for (i = 0; i < s->num_base; i++)
    {
        // if (BaseTypeNeedsUpdate(s->base[i]->baseType))
        // {
        //     s->num_update_base++;
        // }
    }

    s->update_base = (xBase**)xMemAllocSize(s->num_update_base * sizeof(xBase*));

    base_idx = 0;

    for (i = 0; i < s->num_base; i++)
    {
        // if (BaseTypeNeedsUpdate(s->base[i]->baseType))
        // {
        //     s->update_base[base_idx] = s->base[i];
        // }
    }

    xModelInstStaticAlloc = 0;

    zGameScreenTransitionEnd();
    zSceneObjHashtableUsage();
    zUI_ScenePortalInit(s);
    zLightResolveLinks();
    zRenderStateInit();
    xFXStreakInit();
    xFXShineInit();
    xFXFireworksInit("PAREMIT_FIREWORKS_TRAIL", "PAREMIT_FIREWORKS1", "PAREMIT_FIREWORKS2",
                     "Fireworks_explode", "Fireworks_trail");
    //zFeetGetIDs();
    zLightningInit();
    zParCmdFindClipVolumes();
    zEntDestructObj_FindFX();
    zShrapnel_SceneInit(globals.sceneCur);
    zCameraTweakGlobal_Reset();
    zActionLineInit();
    xScrFxLetterboxReset();
    xShadowManager_Init(eBaseTypeNPC + 10);

    S32 lkitCount = xSTAssetCountByType('LKIT');
    void* lkitData;

    for (S32 i = 0; i < lkitCount; i++)
    {
        lkitData = xSTFindAssetByType('LKIT', i, NULL);

        xLightKit_Prepare(lkitData);
    }

    xClimateInit(&gClimate);
    zSceneInitEnvironmentalSoundEffect();

    //sHackSmoothedUpdate = 1;

    //FootstepHackSceneEnter();
    zEntPickup_SceneEnter();
    xFXSceneInit();
    //zGame_HackGalleryInit();
    iSndSuspendCD(0);
}

static U32 BaseTypeNeedsUpdate(U8 baseType)
{
    if (baseType == eBaseTypeUnknown || baseType == eBaseTypePlayer || baseType == eBaseTypeEnv ||
        baseType == eBaseTypeCamera || baseType == eBaseTypeStatic ||
        baseType == eBaseTypeMovePoint || baseType == eBaseTypeBubble ||
        baseType == eBaseTypePortal || baseType == eBaseTypeGroup || baseType == eBaseTypeSFX ||
        baseType == eBaseTypeFFX || baseType == eBaseTypeVFX || baseType == eBaseTypeCounter ||
        baseType == eBaseTypeProjectile || baseType == eBaseTypeGust ||
        baseType == eBaseTypeVolume || baseType == eBaseTypeDispatcher ||
        baseType == eBaseTypeCond || baseType == eBaseTypeUI ||
        baseType == eBaseTypeProjectileType || baseType == eBaseTypeLobMaster ||
        baseType == eBaseTypeCutsceneMgr || baseType == eBaseTypeHud ||
        baseType == eBaseTypeNPCProps || baseType == eBaseTypeParticleEmitterProps ||
        baseType == eBaseTypeCruiseBubble || baseType == eBaseTypeTextBox ||
        baseType == eBaseTypeTalkBox || baseType == eBaseTypeTaskBox ||
        baseType == eBaseTypeBoulderGenerator || baseType == eBaseTypeNPCSettings ||
        baseType == eBaseTypeTurret)
    {
        return 0;
    }

    return 1;
}

static void PipeForAllSceneModels(void (*pipeCB)(RpAtomic* data, U32 pipeFlags, U32 subObjects))
{
    // non-matching: wrong registers

    S32 i, j, k;
    S32 numModels = xSTAssetCountByType('MODL');

    for (i = 0; i < numModels; i++)
    {
        RpAtomic* model = (RpAtomic*)xSTFindAssetByType('MODL', i, NULL);

        if (model)
        {
            PKRAssetTOCInfo ainfo;
            U32 numSubObjects, remainSubObjBits, currSubObjBits;
            RpAtomic* tempmodel;

            xSTGetAssetInfoByType('MODL', i, &ainfo);

            numSubObjects = 0;
            tempmodel = model;

            do
            {
                numSubObjects++;
                tempmodel = iModelFile_RWMultiAtomic(tempmodel);
            } while (tempmodel);

            remainSubObjBits = (1 << numSubObjects) - 1;

            for (j = 0; j < xModelPipeNumTables; j++)
            {
                for (k = 0; k < xModelPipeCount[j]; k++)
                {
                    if (ainfo.aid == xModelPipeData[j][k].ModelHashID)
                    {
                        currSubObjBits = remainSubObjBits & xModelPipeData[j][k].SubObjectBits;

                        if (currSubObjBits)
                        {
                            pipeCB(model, xModelPipeData[j][k].PipeFlags, currSubObjBits);
                        }

                        remainSubObjBits &= ~currSubObjBits;

                        if (!remainSubObjBits)
                        {
                            goto loc_800B1698;
                        }
                    }
                }
            }

            if (remainSubObjBits)
            {
                pipeCB(model, 0, remainSubObjBits);
            }
        }

    loc_800B1698:
        continue;
    }
}

static void PipeAddStuffCB(RpAtomic* data, U32 pipeFlags, U32)
{
    if (pipeFlags)
    {
        xModelLookupList[xModelLookupCount].model = data;
        xModelLookupList[xModelLookupCount].PipeFlags = pipeFlags;
        xModelLookupCount++;
    }
}

static void PipeCountStuffCB(RpAtomic*, U32 pipeFlags, U32)
{
    if (pipeFlags)
    {
        xModelLookupCount++;
    }
}

void zSceneSet(xBase* b, S32 index)
{
    globals.sceneCur->base[index] = b;
}

static U32 zSceneInitFunc_Dispatcher(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    S32 count;

    count = s->baseCount[desc->baseType];

    if (count)
    {
        st_ZDISPATCH_DATA* dpat_pool = zDispatcher_memPool(count);
        s->baseList[desc->baseType] = dpat_pool;

        for (S32 idx = 0; idx < count; idx++)
        {
            xBase* b = zDispatcher_getInst(dpat_pool, idx);
            xEntAsset* asset = (xEntAsset*)xSTFindAssetByType('DPAT', idx, NULL);

            zSceneSet(b, base_idx);
            zDispatcher_Init((st_ZDISPATCH_DATA*)b, asset);
            zSceneObjHashtableAdd(b->id, b);

            base_idx++;
        }
    }

    return base_idx;
}

static U32 zSceneInitFunc_LobMaster(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    return base_idx;
}

static U32 zSceneInitFunc_Volume(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    S32 count;

    count = s->baseCount[desc->baseType];

    zVolumeInit();

    if (count)
    {
        s->baseList[desc->baseType] = zVolumeGetVolume(0);

        for (S32 idx = 0; idx < count; idx++)
        {
            xBase* b = zVolumeGetVolume(idx);

            zLightSetVolume((zVolume*)b);

            zSceneSet(b, base_idx);
            zSceneObjHashtableAdd(b->id, b);

            base_idx++;
        }
    }

    return base_idx;
}

static U32 zSceneInitFunc_Surface(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    S32 count;

    count = s->baseCount[desc->baseType];

    if (count)
    {
        s->baseList[desc->baseType] = xSurfaceGetByIdx(0);

        for (S32 idx = 0; idx < count; idx++)
        {
            xBase* b = xSurfaceGetByIdx(idx);

            zSceneSet(b, base_idx);
            zSceneObjHashtableAdd(b->id, b);

            base_idx++;
        }
    }

    return base_idx;
}

static U32 zSceneInitFunc_Camera(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    S32 count;
    zCamMarker* camBlock;

    count = s->baseCount[desc->baseType];

    if (count)
    {
        camBlock = (zCamMarker*)xMemAllocSize(count * sizeof(zCamMarker));
        s->baseList[desc->baseType] = camBlock;

        for (S32 idx = 0; idx < count; idx++)
        {
            xBase* b = &camBlock[idx];
            xCamAsset* assetCam = (xCamAsset*)xSTFindAssetByType('CAM ', idx, NULL);

            zSceneSet(b, base_idx);
            zCamMarkerInit(b, assetCam);
            zSceneObjHashtableAdd(b->id, b);

            base_idx++;
        }
    }

    return base_idx;
}

static U32 zSceneInitFunc_Player(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    // REFERENCE BFBB
    return 0;
}

static U32 zSceneInitFunc_MovePoint(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    S32 count;
    U32 assetSize;
    zMovePoint* movpBlock;

    assetSize = 0;
    count = s->baseCount[desc->baseType];
    movpBlock = zMovePoint_GetMemPool(count);

    if (movpBlock)
    {
        s->baseList[desc->baseType] = movpBlock;

        for (S32 idx = 0; idx < count; idx++)
        {
            xBase* b = zMovePoint_GetInst(idx);
            xBaseAsset* basset = (xBaseAsset*)xSTFindAssetByType('MVPT', idx, &assetSize);

            zSceneSet(b, base_idx);
            zMovePointInit(zMovePoint_GetInst(idx), (xMovePointAsset*)basset);
            zSceneObjHashtableAdd(b->id, b);

            base_idx++;
        }
    }

    return base_idx;
}

static U32 zSceneInitFunc_Default(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    U8* block;
    S32 count;
    U32 assetSize, offset;
    xBase* b;

    block = NULL;
    assetSize = 0;
    offset = desc->sizeRuntime;
    count = s->baseCount[desc->baseType];

    if (count)
    {
        block = (U8*)xMemAllocSize(count * offset);

        s->baseList[desc->baseType] = (xBase*)block;
    }

    for (S32 i = 0; i < count; i++)
    {
        void* asset = xSTFindAssetByType(desc->assetType, i, &assetSize);
        b = (xBase*)(block + i * offset);

        zSceneSet(b, base_idx);

        if (desc->objectInitFunc)
        {
            desc->objectInitFunc(b, asset);
        }

        zSceneObjHashtableAdd(b->id, b);

        base_idx++;
    }

    return base_idx;
}

static U32 zSceneInitFunc_DefaultEnt(zScene* s, zSceneObjectInstanceDesc* desc, U32 base_idx)
{
    U8* block;
    S32 count;
    U32 assetSize, offset;
    xBase* b;

    block = NULL;
    assetSize = 0;
    offset = desc->sizeRuntime;
    count = s->baseCount[desc->baseType];

    if (count)
    {
        block = (U8*)xMemAllocSize(count * offset);

        s->baseList[desc->baseType] = (xBase*)block;
    }

    for (S32 i = 0; i < count; i++)
    {
        void* asset = xSTFindAssetByType(desc->assetType, i, &assetSize);
        b = (xBase*)(block + i * offset);

        zSceneSet(b, base_idx);

        if (desc->objectInitFunc)
        {
            desc->objectInitFunc(b, asset);
        }

        //xSceneAddEnt(s, s->ents[base_idx]);
        zSceneObjHashtableAdd(b->id, b);

        base_idx++;
    }

    return base_idx;
}
