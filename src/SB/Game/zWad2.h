#ifndef ZWAD2_H
#define ZWAD2_H

#include <types.h>

#include "xCounter.h"
#include "xDebug.h"
#include "xPartition.h"
#include "xWad5.h"

#include "zFMV.h"
#include "zGame.h"
#include "zGameExtras.h"
#include "zNPCMgr.h"
#include "zLight.h"
#include "zLightning.h"
#include "zLOD.h"
#include "zMusic.h"
#include "zNMECommon.h"
#include "zUI.h"

// Dwarf data says that most of these are only used here in zWad2.cpp

enum zHitSource
{
    zHS_EVENT,
    zHS_GENERAL,
    zHS_PROJECTILE,
    zHS_EXPLOSION,
    zHS_LASER,
    zHS_ENERGY,
    zHS_FIRE,
    zHS_SURFACE,
    zHS_MELEE_HIGH,
    zHS_MELEE_MID,
    zHS_MELEE_LOW,
    zHS_MELEE_UP,
    zHS_MELEE_BACK,
    zHS_MELEE_DIZZY,
    zHS_THROW,
    zHS_WATER,
    zHS_DEATHPLANE,
    zHS_INCREDI,
    zHS_KNOCKBACK,
    zHS_LASERBEAM,
    zHS_INFINITE_FALL,
    zHS_COUNT,
    zHS_FORCE_INT = 0xffffffff
};

enum ePOWERUPMENU
{
    ePOWERUPMENU_KarateSpin,
    ePOWERUPMENU_Bash,
    ePOWERUPMENU_BubbleBowl,
    ePOWERUPMENU_SonicWave,
    ePOWERUPMENU_BellyBump,
    ePOWERUPMENU_Cartwheel,
    ePOWERUPMENU_BellyFlop,
    ePOWERUPMENU_Throw,
    ePOWERUPMENU_SBHealth,
    ePOWERUPMENU_PatHealth,
    ePOWERUPMENU_PointsAvail,
    ePOWERUPMENU_Max
};

struct entrail_type
{
    char* model_name;
    S32 bone;
    F32 rate;
    F32 cull_dist;
    xVec3 offset;
    xVec3 offset_rand;
    xVec3 vel;
    xVec3 vel_rand;
};

struct entrail_data
{
    U16 flags;
    U16 type;
    xEnt* ent;
    xVec3 loc;
    xVec3 vel;
    F32 emitted;
};

struct zGooParams
{
    F32 depth;
    F32 orig_depth;
    xEnt* goo_ent;
};

struct zGrapplePoint
{
    U32 flags;
    xVec3* pos;
};

struct zHangableAsset : xDynAsset
{
    U32 object;
    xVec3 pivot;
    xVec3 handle;
    F32 onGravity;
    F32 offGravity;
    F32 maxAngVel;
    F32 minArcDegrees;
    U32 hangFlags;
};

struct zHangable : xBase
{
    zHangableAsset* hangableAsset;
    U32 flags;
};

struct zHangableObject
{
    zHangable* hangable;
    xEnt* object;
    F32 radius;
    xVec3 vel;
    xVec3 accel;
    xVec3 handle;
    xMat3x3 centerMat;
    xVec3 rotAxis;
    xVec3 lastDisp;
    xVec3 objVel;
};

struct zHitSourceMapEntry
{
    char* name;
    zHitSource sourcetype;
};

struct PatTongueVert
{
    xVec3 pos;
    xModelTagWithNormal tag;
};

struct CollisionInfo
{
    xVec3 hitDir;
    S32 damage;
    F32 knockBackSpeed;
    zLightning* l;
    iSndGroupHandle soundHit1;
    iSndGroupHandle soundHit2;
};

struct lightweight_system_base
{
};

struct PowerUpMenuTableEntry
{
    ePOWERUPMENU entry;
    char* name;
    S32 powerup;
    _xCounter* counter;
};

struct PowerUpMovieTableEntry
{
    S32 powerup;
    eFMVFile level[3];
};

struct taskEntry
{
    char name[4];
    U8 taskStatus[8];
    U32 numTasks;
};

struct extraEntry
{
    U8 type;
    U8 id;
    U8 chestsNeeded;
    U8 status;
};

struct extraCostumeEntry
{
    U32 costumeID;
    eSNDCurrentPlayer player;
    U32 gameExtrasFlag;
};

struct zMusicState
{
    U32 uMusicDefault;
    U32 uMusicRestart;
    U8 bPaused;
};

struct NPCBrainTableEntry
{
    char* name;
    en_npctyp type;
    U32 id;
};

struct tagiRenderArrays
{
    U16 m_index[960];
    RxObjSpace3DVertex m_vertex[480];
    F32 m_vertexTZ[480];
};

#endif
