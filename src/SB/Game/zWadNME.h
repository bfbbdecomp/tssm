#ifndef ZWADNME_H
#define ZWADNME_H

#include "zNMECommon.h"
#include "xModelBlur.h"
#include "zStreamedSound.h"
#include "zBossCam.h"

enum en_denstage
{
    DEN_STAGE_UNKNOWN,
    DEN_STAGE_INTRO,
    DEN_STAGE_A,
    DEN_STAGE_B,
    DEN_STAGE_C,
    DEN_STAGE_OUTRO,
    DEN_STAGE_NOMORE
};

enum en_dbcmode
{
    DEN_CAMMOD_UNKNOWN,
    DEN_CAMMOD_NORMAL,
    DEN_CAMMOD_BINARY,
    DEN_CAMMOD_NOMORE
};

enum en_vis
{
    VIS_PREPARE_TO_DIE,
    VIS_HORROR_FLIK,
    VIS_OH_DEAR_GAHD,
    VIS_GET_BACK_HERE,
    VIS_I_SEEE_YOUUU,
    VIS_GREY_AND_FOGGY,
    VIS_LIKE_A_BIRDIE,
    VIS_EARTHWORM_JIM,
    VIS_NOBODY_AROUND,
    VIS_NOMORE
};

enum en_npcgspot
{
    NME_GSPOT_START = 0x20,
    NME_GSPOT_RESUME,
    NME_GSPOT_LOOP,
    NME_GSPOT_FINISH,
    NME_GSPOT_STARTALT,
    NME_GSPOT_ALTA,
    NME_GSPOT_ALTB,
    NME_GSPOT_PATROLPAUSE,
    NME_GSPOT_NEXT,
    NME_GSPOT_NOMORE,
    NME_GSPOT_FORCEINT = 0x7fffffff
};

enum en_diehow
{
    DIE_SILENT,
    DIE_DUPLOONLY,
    DIE_MUFFLED,
    DIE_QUIETLY,
    DIE_LOUDLY,
    DIE_NORMAL,
    DIE_NOMORE,
    DIE_FORCE = 0x7fffffff
};

struct zNMEGoalCommon : xGoal
{
    U32 anid_played;
    struct
    {
        S32 flg_npcgauto : 8;
        S32 flg_npcgable : 16;
        S32 bul_entered : 1;
        S32 bul_resumed : 1;
        S32 bul_unused : 6;
    };
    struct
    {
        S32 flg_info : 16;
        S32 flg_user : 16;
    };

    S32 CollReview();
    S32 GoalHandleMail();
    void Clear();
    S32 Enter();
    S32 Resume();
    S32 Process(en_trantype* trantyp, F32 dt, void* ctxt);
};

struct zNMECritter : zNMECommon
{
    zNMENavNet privNavNet;

    void Init(xEntAsset* entass);
    zMovePoint* MvptFirstFollow();
    zNMENavNet* Gimme_NavNet();
};

struct zNMECritBasic : zNMECritter
{
    S32 TypeHandleMail(NMEMsg* mail);
    void Process(F32 dt);
    void Reset();
    xAnimTable* CreateAnimTable(xAnimTable* table);
    void SelfSetup();
};

struct zNMEBoss : zNMECommon
{
};

struct Health_3
{
    S32 pts_healthMax;
};

struct Ranges_2
{
    F32 rad_tooclose[2];
    F32 rad_lob;
    F32 rad_lobgrey;
    F32 rad_aware;
    F32 rad_awaregrey;
};

struct Attack_3
{
    F32 spd_proj;
    S32 num_salvo;
    F32 tym_shoot;
    F32 dst_spreadHorz;
};

struct Battle_6
{
    F32 tym_reload;
    F32 tym_leapDelay;
    S32 num_atakPerLeap;
};

struct NMECfgDennis
{
    Health_3 health;
    Ranges_2 ranges;
    Attack_3 attack;
    Battle_6 battle;
};

struct DennisLetMeKnow : xPSYNote
{
};

struct zNMEDennis;

struct zNMESBBat : zNMECommon
{
    zNMEDennis* npc_bossman;

    void Reset();
    void Process(F32 dt);
    void Setup();
    void Init(xEntAsset* asset);
    void SelfSetup();
    U32 AnimPick(S32 gid);
    xAnimTable* CreateAnimTable(xAnimTable* table);
};

struct zNMEDennis : zNMEBoss
{
    S32 pts_health; // 0x1c0
    NMECfgDennis* cfg_dennis;
    F32 tmr_nextLeap; // 0x1c8
    S32 cnt_attacksTilEvade;
    zMovePoint* nav_evade;
    zNMESBBat* npc_sbbat;
    union
    {
        en_denstage stageCurr;
        S32 stageCurrNum;
    };
    union
    {
        xModelBlur moveBlur;
        xModelBlur moveBlurShort;
    };
    xModelBlur moveBlurLong;
    zStreamedSoundList oneLiners;
    DennisLetMeKnow tellMeWhen;
    S32 flg_firstTimestep;
    zBossCam_Binary cam_boss;
    en_dbcmode mod_camCurr;
    en_dbcmode mod_camWant;
    xGroup* grp_mvpt;

    zMovePoint* PickEvade(zMovePoint* nav_from);
    S32 DenMailSys(NMESysEvent* sysmail);
    S32 DenMailDamage(NMEDamageInfo* dmgmail);
    S32 TypeHandleMail(NMEMsg* mail);
    void BossCamSet(en_dbcmode mod_desired, U8 forceCut);
    en_vis VisionTarget();
    void MeterRender();
    void RenderHud();
    void PostProcess();
    void NewTime(F32 dt);
    void BUpdate(xVec3* pos);
    void Process(F32 dt);
    S32 IsAlive();
    void Destroy();
    void Reset();
    void Setup();
    void Init(xEntAsset* asset);
    xAnimTable* CreateAnimTable(xAnimTable* table);
    U32 AnimPick(S32 gid, en_npcgspot gspot);
    zMovePoint* MvptFirstFollow();
    void SelfSetup();
    S32 DfltVulnFlags();
    void CreateConfig();
};

struct zNMEGoalDenDamage : zNMEGoalCommon
{
    S32 GoalHandleMail(NMEMsg* mail);
    S32 ChkExistence(zNMEDennis* npc, F32 dt, S32& nextgoal, en_trantype& trantype);
    S32 Process(en_trantype* trantype, F32 dt, void* ctxt);
    S32 Exit(F32 dt, void* ctxt);
    S32 Enter();
};

struct zNMEGoalDead : zNMEGoalCommon
{
    en_diehow stylinDyin;
    U16 old_moreFlags;
    S16 notUsed;

    void SendDeathNotice(zNMECommon* npc);
    S32 Exit();
    S32 Enter();
};

struct zNMEGoalDenDead : zNMEGoalDead
{
    S32 Exit(F32 dt, void* ctxt);
    S32 Enter();
};

struct zNMEGoalDenEvade : zNMEGoalCommon
{
    xParabola* parab;
    xFXRibbon* ribbon;
    F32 tmr_leaping;
    S8 cnt_skipRibbonJoint;
    S8 status;
    S8 padded[2];

    S32 GoalHandleMail(NMEMsg* mail);
    void RibbonLoop(zNMEDennis* npc);
    S32 PathDirect(zNMEDennis*, F32 dt);
    S32 PathParabola(zNMEDennis* npc);
    S32 UpdateAnim(zNMEDennis* npc, F32 dt);
    S32 Process(en_trantype* trantype, F32 dt, void* ctxt);
    S32 Exit();
    S32 Enter();
};

#endif
