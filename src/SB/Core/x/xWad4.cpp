#include "xWad4.h"
#include "rwplcore.h"
#include "xPad.h"
#include "zGame.h"
#include "zScene.h"
#include <math.h>

//                                              xRay / xScene

void xRayHitsEnt(xScene* sc, xRay3* r, xQCData* qcr, xEnt* ent, void* colldata)
{
    xCollis* coll = (xCollis*)colldata;
    xCollis c;

    if (ent->chkby == XENT_COLLTYPE_NONE)
    {
        return;
    }

    if (qcr && !xQuickCullIsects(qcr, &ent->bound.qcd))
    {
        return;
    }

    xRayHitsBound(r, &ent->bound, &c);

    if (!(c.flags & 0x1))
    {
        return;
    }

    if (c.dist < coll->dist)
    {
        coll->dist = c.dist;
        coll->oid = ent->id;
        coll->optr = ent;
        coll->mptr = ent->model;
    }
}

void xRayHitsTikiLandableEnt(xScene* sc, xRay3* r, xQCData* qcr, xEnt* ent, void* colldata)
{
    xCollis* coll = (xCollis*)colldata;
    xCollis c;

    if (ent->chkby == XENT_COLLTYPE_NONE)
    {
        return;
    }

    if (ent->collType == XENT_COLLTYPE_NPC)
    {
        return;
    }

    if (ent->baseType == eBaseTypeBoulder)
    {
        return;
    }

    if (qcr && !xQuickCullIsects(qcr, &ent->bound.qcd))
    {
        return;
    }

    xRayHitsBound(r, &ent->bound, &c);

    if (!(c.flags & 0x1))
    {
        return;
    }

    if (ent->collLev == 0x5)
    {
        iRayHitsModel(r, ent->model, &c);

        if (!(c.flags & 0x1))
        {
            return;
        }
    }

    if (c.dist < coll->dist)
    {
        coll->dist = c.dist;
        coll->oid = ent->id;
        coll->optr = ent;
        coll->mptr = ent->model;
    }
}

void xSceneForAllNPCs(xScene* sc, xSceneEntCallback func, void* data)
{
    for (U16 i = 0; i < sc->num_npcs; i++)
    {
        if (!func(sc->npcs[i], sc, data))
        {
            break;
        }
    }
}

void xSceneForAllDynamics(xScene* sc, xSceneEntCallback func, void* data)
{
    for (U16 i = 0; i < sc->num_dyns; i++)
    {
        if (!func(sc->dyns[i], sc, data))
        {
            break;
        }
    }
}

void xSceneForAllStatics(xScene* sc, xSceneEntCallback func, void* data)
{
    for (U16 i = 0; i < sc->num_stats; i++)
    {
        if (!func(sc->stats[i], sc, data))
        {
            break;
        }
    }
}

void xSceneForAllEnts(xScene* sc, xSceneEntCallback func, void* data)
{
    for (U16 i = 0; i < sc->num_act_ents; i++)
    {
        if (!func(sc->act_ents[i], sc, data))
        {
            break;
        }
    }
}

xBase* xSceneResolvID(xScene* sc, U32 id)
{
    if (sc->resolvID)
    {
        return sc->resolvID(id);
    }

    return NULL;
}

void xSceneAddEnt(xScene* sc, xEnt* ent)
{
    if (ent->collType == XENT_COLLTYPE_TRIG)
    {
        sc->trigs[sc->num_trigs++] = ent;
    }
    else if (ent->collType == XENT_COLLTYPE_STAT)
    {
        sc->stats[sc->num_stats++] = ent;
    }
    else if (ent->collType == XENT_COLLTYPE_DYN)
    {
        sc->dyns[sc->num_dyns++] = ent;
    }
    else if (ent->collType == XENT_COLLTYPE_NPC)
    {
        sc->npcs[sc->num_npcs++] = ent;
    }

    sc->act_ents[sc->num_act_ents++] = ent;
}

void xSceneSetup(xScene* sc)
{
    xEnvSetup(sc->env);
}

void xSceneLoad(xScene* sc, xSerial* s)
{
}

void xSceneSave(xScene* sc, xSerial* s)
{
}

void xSceneExit(xScene* sc)
{
}

void xSceneInit(xScene* sc, U16 num_trigs, U16 num_stats, U16 num_dyns, U16 num_npcs)
{
    sc->flags = 0;
    sc->num_trigs_allocd = num_trigs;
    sc->trigs = (xEnt**)xMemAllocSize(num_trigs * sizeof(xEnt*));
    sc->num_stats_allocd = num_stats;
    sc->stats = (xEnt**)xMemAllocSize(num_stats * sizeof(xEnt*));
    sc->num_dyns_allocd = num_dyns;
    sc->dyns = (xEnt**)xMemAllocSize(num_dyns * sizeof(xEnt*));
    sc->num_npcs_allocd = num_npcs;
    sc->npcs = (xEnt**)xMemAllocSize(num_npcs * sizeof(xEnt*));
    sc->num_ents_allocd = (U32)num_trigs + (U32)num_stats + (U32)num_dyns + (U32)num_npcs;
    sc->num_act_ents = 0;
    sc->act_ents = (xEnt**)xMemAllocSize(sc->num_ents_allocd * sizeof(xEnt*));
    sc->num_trigs = 0;
    sc->num_stats = 0;
    sc->num_dyns = 0;
    sc->num_npcs = 0;

    sc->resolvID = NULL;

    xAnimInit();
    xModelInit();
    xAnimPoolInit(&sc->mempool, 50, 1, 0x1, 4);
    xModelPoolInit(49, 64);
    xModelPoolInit(74, 8);
    xModelPoolInit(324, 1);
}

//                                      xQuickCull WEAK

//                                              xSG

S32 g_xsginit;
st_XSAVEGAME_DATA g_xsgdata = {};
st_XSAVEGAME_LEADER g_leaders[3] = {};
static XSGAutoData g_autodata;

void ASG_ISG_changed(void*, en_CHGCODE what)
{
    XSGAutoData* asg = xSGAutoSave_GetCache();
    switch (what)
    {
    case ISG_CHG_NONE:
        break;
    case ISG_CHG_TARGET:
        asg->MarkInvalid();
        break;
    case ISG_CHG_GAMELIST:
        asg->MarkInvalid();
        break;
    }
}

void XSGAutoData::Discard()
{
    this->flg_autodata = 0;
    this->lastTarg = -1;
    this->lastGame = -1;
    this->lastPhysicalSlot = -1;
    if (this->isg_monitor != NULL)
    {
        this->HWDisconnect(this->isg_monitor);
    }
    this->isg_monitor = NULL;
}

S32 XSGAutoData::SetCache(S32 targ, S32 game, S32 physicalSlot)
{
    this->Discard();
    this->isg_monitor = this->HWConnect(targ);
    if (this->isg_monitor == NULL)
    {
        return 0;
    }

    this->lastTarg = targ;
    this->lastGame = game;
    this->lastPhysicalSlot = physicalSlot;
    this->flg_autodata |= 1;
    return 1;
}

S32 XSGAutoData::IsValid()
{
    if (this->flg_autodata & 1 && !XSGAutoData::HWCheckConnect(this->lastTarg))
    {
        this->flg_autodata &= 0x0fffffffe;
    }
    return this->flg_autodata & 1;
}

XSGAutoData* xSGAutoSave_GetCache()
{
    return &g_autodata;
}

S32 xSG_ld_findcltblk(st_XSAVEGAME_DATA* xsgdata)
{
    S32 ival = 0;
    U32 tag = 0;
    S32 maxamt = 0;
    S32 actamt = 0;
    st_XSAVEGAME_CLIENT* dfltclt = &xsgdata->dfltloadclt;

    xsgdata->walkpos = xsgdata->loadbuf;
    xsgdata->walkremain = xsgdata->loadsize;
    xSGReadData(xsgdata, 0, &ival, 1);
    xSGReadData(xsgdata, 0, &ival, 1);
    xSGReadData(xsgdata, 0, &ival, 1);
    xSGReadData(xsgdata, 0, &ival, 1);

    while (xsgdata->walkremain > 0)
    {
        tag = 0;
        S32 found = 0;
        maxamt = 0;
        actamt = 0;

        xSGReadData(xsgdata, 0, &tag, 1);
        xSGReadData(xsgdata, 0, &maxamt, 1);
        xSGReadData(xsgdata, 0, &actamt, 1);
        xUtil_idtag2string(tag, 0);

        for (S32 i = 0; i < xsgdata->stkcnt; i++)
        {
            st_XSAVEGAME_CLIENT* clt = &xsgdata->cltstk[i];

            if (clt->idtag == tag)
            {
                found = true;
                clt->blokpos = xsgdata->walkpos;
                clt->blokact = actamt;
                clt->blokmax = maxamt;
                clt->readpos = xsgdata->walkpos;
                clt->readamt = 0;
                clt->readremain = actamt;
                break;
            }
        }

        if (!found)
        {
            if (dfltclt->idtag != 0)
            {
                xUtil_idtag2string(tag, 0);
                dfltclt->blokpos = xsgdata->walkpos;
                dfltclt->blokact = actamt;
                dfltclt->blokmax = maxamt;
                dfltclt->readpos = xsgdata->walkpos;
                dfltclt->readamt = 0;
                dfltclt->readremain = actamt;

                S32 rc =
                    dfltclt->cltload(dfltclt->cltdata, xsgdata, (st_XSAVEGAME_READCONTEXT*)dfltclt,
                                     tag, dfltclt->blokact);
                if (rc == 0)
                {
                    xUtil_idtag2string(tag, 0);
                }

                rc = dfltclt->readremain;
                if (rc > 0)
                {
                    xsgdata->walkpos += rc + (maxamt - actamt);
                    xsgdata->walkremain -= dfltclt->readremain + (maxamt - actamt);
                }
            }
            else
            {
                xUtil_idtag2string(tag, 0);
                xsgdata->walkpos += maxamt;
                xsgdata->walkremain -= maxamt;
            }
        }
        else
        {
            xsgdata->walkpos += maxamt;
            xsgdata->walkremain -= maxamt;
        }
    }
    return 1;
}

void xSG_cb_ISGChange(void*, en_CHGCODE what)
{
    XSGAutoData* asg = xSGAutoSave_GetCache();
    switch (what)
    {
    case ISG_CHG_NONE:
        break;
    case ISG_CHG_TARGET:
        asg->MarkInvalid();
        break;
    case ISG_CHG_GAMELIST:
        asg->MarkInvalid();
        break;
    }
}

S32 xSG_sv_flipproc(st_XSAVEGAME_DATA* xsgdata)
{
    S32 result = 1;
    S32 rc;
    char bfill = 0xbf;

    for (S32 i = 0; i < xsgdata->stkcnt; i++)
    {
        st_XSAVEGAME_CLIENT* clt = &xsgdata->cltstk[i];

        rc = xSG_smem_cltopen(xsgdata, clt);
        if (rc != 0)
        {
            rc = clt->cltproc(clt->cltdata, xsgdata, (st_XSAVEGAME_WRITECONTEXT*)clt);
            if (rc == 0)
            {
                xUtil_idtag2string(clt->idtag, 0);
                result = 0;
            }

            S32 needfill = clt->maxamt - clt->realamt;
            xUtil_idtag2string(clt->idtag, 0);
            while (needfill-- > 0)
            {
                xSGWriteData(xsgdata, NULL, &bfill, 1);
            }
        }

        if (rc != 0 && xSG_smem_cltclose(xsgdata, clt) == 0)
        {
            result = 0;
            xUtil_idtag2string(clt->idtag, 0);
        }

        if (result == 0)
        {
            break;
        }
    }
    return result;
}

S32 xSG_sv_flipinfo(st_XSAVEGAME_DATA* xsgdata)
{
    S32 result = 1;
    S32 cltamt = 0;
    S32 cltmax = 0;

    for (S32 i = 0; i < xsgdata->stkcnt; i++)
    {
        st_XSAVEGAME_CLIENT* clt = &xsgdata->cltstk[i];
        cltamt = 0;
        cltmax = 0;

        S32 rc = clt->cltinfo(clt->cltdata, xsgdata, &cltamt, &cltmax);
        if (rc == 0)
        {
            xUtil_idtag2string(clt->idtag, 0);
            result = 0;
        }
        else
        {
            clt->needamt = cltamt;
            clt->maxamt = cltmax;
            xsgdata->cltneed += cltamt;
            xsgdata->cltmax += cltmax;
            xsgdata->chdrneed += 0xc;
        }

        if (result == 0)
        {
            xsgdata->stage |= 0x40000000;
            break;
        }
    }
    return result;
}

S32 xSGReadData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, F32* buff, S32 n)
{
    return xSGReadData(xsgdata, rctxt, (char*)buff, sizeof(F32), n);
}

S32 xSGReadData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, U32* buff, S32 n)
{
    return xSGReadData(xsgdata, rctxt, (char*)buff, sizeof(U32), n);
}

S32 xSGReadData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, S32* buff, S32 n)
{
    return xSGReadData(xsgdata, rctxt, (char*)buff, sizeof(S32), n);
}

S32 xSGReadData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, char* buff, S32 n)
{
    return xSGReadData(xsgdata, rctxt, buff, sizeof(char), n);
}

S32 xSGReadData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, char* buff, S32 elesiz,
                S32 n)
{
    S32 cnt;
    if (elesiz == 0)
    {
        return 0;
    }

    cnt = n * elesiz;
    if (cnt == 0)
    {
        return cnt;
    }

    st_XSAVEGAME_CLIENT* clt = (st_XSAVEGAME_CLIENT*)rctxt;
    if (clt != NULL && cnt > clt->readremain)
    {
        xUtil_idtag2string(clt->idtag, 0);
        return 0;
    }

    if (clt != NULL)
    {
        memcpy(buff, clt->readpos, cnt);
    }
    else
    {
        memcpy(buff, xsgdata->walkpos, cnt);
    }

    xsgdata->walkpos += cnt;
    xsgdata->walkremain -= cnt;

    if (clt != NULL)
    {
        clt->readpos += cnt;
        clt->readamt += cnt;
        clt->readremain -= cnt;
    }

    return cnt / elesiz;
}

S32 xSGWriteData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt, F32* data, S32 n)
{
    return xSGWriteData(xsgdata, wctxt, (char*)data, sizeof(F32), n);
}

S32 xSGWriteData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt, S32* data, S32 n)
{
    return xSGWriteData(xsgdata, wctxt, (char*)data, sizeof(S32), n);
}

S32 xSGWriteData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt, char* data, S32 n)
{
    return xSGWriteData(xsgdata, wctxt, data, sizeof(char), n);
}

S32 xSGWriteStrLen(const char* str)
{
    S32 len = strlen(str);
    len += (len + 1 & 1) + 1;
    return len;
}

S32 xSGWriteData(st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt, char* data,
                 S32 elesiz, S32 n)
{
    S32 cnt;
    S32 is_ok = 1;

    if (xsgdata->stage & 0x40000000)
    {
        return 0;
    }
    if (elesiz == 0)
    {
        return 0;
    }

    cnt = n * elesiz;
    if (cnt == 0)
    {
        return cnt;
    }

    if (xsgdata->upd_tally != 0 && xsgdata->totamt + cnt > xsgdata->memsize)
    {
        xsgdata->stage |= 0x40000000;
        return 0;
    }

    void* mcprc;
    mcprc = memcpy(xsgdata->buf_curpos, data, cnt);
    if (mcprc == NULL)
    {
        is_ok = 0;
    }
    else
    {
        xsgdata->buf_curpos += cnt;
    }

    // I wonder if this was supposed to be a boolean and
    if (is_ok & xsgdata->upd_tally)
    {
        xsgdata->totamt += cnt;
        if (wctxt != NULL)
        {
            st_XSAVEGAME_CLIENT* clt = (st_XSAVEGAME_CLIENT*)wctxt;
            clt->realamt += cnt;
        }
    }

    if (is_ok == 0)
    {
        xsgdata->stage |= 0x40000000;
        return 0;
    }
    else
    {
        return cnt / elesiz;
    }

    return cnt;
}

#pragma inline_depth(0)
S32 xSG_cb_leader_load(void*, st_XSAVEGAME_DATA* original_xsgdata, st_XSAVEGAME_READCONTEXT* rctxt,
                       U32, S32)
{
    char fundata[32] = {};
    st_XSAVEGAME_LEADER discard = {};
    // TODO: Fix last two args in function signature
    xSGReadData(original_xsgdata, rctxt, discard.gameLabel, 1, sizeof(st_XSAVEGAME_LEADER));
    xSGReadData(original_xsgdata, rctxt, fundata, 1, 0x16);
    return 1;
}

S32 xSG_cb_leader_svproc(void* cltdata, st_XSAVEGAME_DATA* original_xsgdata,
                         st_XSAVEGAME_WRITECONTEXT* wctxt)
{
    st_XSAVEGAME_DATA* xsg = (st_XSAVEGAME_DATA*)cltdata;
    st_XSAVEGAME_LEADER leader = {};
    char fundata[] = "--TakeMeToYourLeader--";

    strncpy(leader.gameLabel, xsg->label, 0x40);
    leader.gameLabel[63] = 0;
    leader.gametime = xsg->playtime;
    leader.progress = xsg->progress;
    leader.thumbIconIdx = xsg->thumbIconIdx;

    // TODO: Fix last two args in function signature
    xSGWriteData(original_xsgdata, wctxt, leader.gameLabel, 1, sizeof(st_XSAVEGAME_LEADER));
    xSGWriteData(original_xsgdata, wctxt, fundata, 1, 0x16);
    return 1;
}
#pragma inline_depth(5)

S32 xSG_cb_leader_svinfo(void*, st_XSAVEGAME_DATA*, S32* cur_space, S32* max_fullgame)
{
    *cur_space = 0x100;
    *max_fullgame = 0x100;
    return 1;
}

en_XSGASYNC_STATUS xSGAsyncStatus(st_XSAVEGAME_DATA* xsgdata, S32 block, en_XSG_WHYFAIL* whyFail,
                                  char* errmsg)
{
    en_XSGASYNC_STATUS xstat = XSG_ASTAT_NOOP;
    en_ASYNC_OPERR whyerr = ISG_OPERR_NONE;

    if (errmsg != NULL)
    {
        errmsg[0] = '\0';
    }

    en_ASYNC_OPSTAT istat = iSGPollStatus(xsgdata->isgsess, 0, block);
    switch (istat)
    {
    case ISG_OPSTAT_FAILURE:
        xstat = XSG_ASTAT_FAILED;
        whyerr = iSGOpError(xsgdata->isgsess, errmsg);
        break;
    case ISG_OPSTAT_INPROG:
        xstat = XSG_ASTAT_INPROG;
        break;
    case ISG_OPSTAT_SUCCESS:
        xstat = XSG_ASTAT_SUCCESS;
        break;
    }

    if (whyFail != NULL && whyerr != ISG_OPERR_NONE)
    {
        en_XSG_WHYFAIL con = XSG_WHYERR_NONE;

        switch (whyerr)
        {
        case ISG_OPERR_NONE:
            break;
        case ISG_OPERR_NOCARD:
            con = XSG_WHYERR_NOCARD;
            break;
        case ISG_OPERR_NOROOM:
        case ISG_OPERR_SVNOSPACE:
            con = XSG_WHYERR_NOROOM;
            break;
        case ISG_OPERR_DAMAGE:
            con = XSG_WHYERR_DAMAGE;
            break;
        case ISG_OPERR_SVWRITE:
            con = XSG_WHYERR_CARDYANKED;
            break;
        default:
            con = XSG_WHYERR_OTHER;
            break;
        }
        *whyFail = con;
    }
    return xstat;
}

S32 xSGWrapup(st_XSAVEGAME_DATA* xsgdata)
{
    S32 result = 1;
    S32 rc = 0;

    if (xsgdata->stage & 0x40000000)
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 1))
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 0x10))
    {
        result = 0;
        xsgdata->stage |= 0x40000000;
    }

    if (xsgdata->mode != XSG_MODE_LOAD)
    {
        return result;
    }
    if (result == 0)
    {
        return result;
    }

    if (result != 0)
    {
        rc = xSG_ld_readhead(xsgdata);
    }
    if (rc < 0)
    {
        xsgdata->stage |= 0x40000000;
        return -1;
    }

    if (rc == 0)
    {
        result = 0;
    }
    if (result != 0)
    {
        rc = xSG_ld_validate(xsgdata);
    }

    if (rc < 0)
    {
        xsgdata->stage |= 0x40000000;
        return -1;
    }

    if (rc == 0)
    {
        result = 0;
    }
    if (result != 0)
    {
        rc = xSG_ld_findcltblk(xsgdata);
    }

    if (rc == 0)
    {
        result = 0;
    }
    if (result != 0)
    {
        rc = xSG_ld_flipload(xsgdata);
    }

    if (rc == 0)
    {
        result = 0;
    }
    if (result == 0)
    {
        xsgdata->stage |= 0x40000000;
    }

    return result;
}

S32 xSGProcess(st_XSAVEGAME_DATA* xsgdata)
{
    S32 result = 1;
    if (xsgdata->stage & 0x40000000)
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 1))
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 8))
    {
        result = 0;
    }

    if (result != 0 && xsgdata->mode == XSG_MODE_SAVE)
    {
        if (xSG_smem_blkopen(xsgdata) == 0)
        {
            result = 0;
        }

        if (result != 0 && xSG_sv_flipproc(xsgdata) == 0)
        {
            result = 0;
        }
        if (result != 0 && xSG_sv_bldchksum(xsgdata) == 0)
        {
            result = 0;
        }
        if (result != 0 && xSG_smem_blkclose(xsgdata) == 0)
        {
            result = 0;
        }

        if (result != 0)
        {
            xSG_sv_commit(xsgdata);
        }
    }
    else if (result != 0 && xsgdata->mode == XSG_MODE_LOAD)
    {
        if (xSG_ld_readgame(xsgdata) == 0)
        {
            result = 0;
        }
    }

    xsgdata->stage |= 0x10;
    if (result == 0)
    {
        xsgdata->stage |= 0x40000000;
    }
    return result;
}


S32 xSGSetup(st_XSAVEGAME_DATA* xsgdata, S32 gidx, char* label, S32 progress, iTime playtime,
             S32 thumbIconIdx)
{
    S32 result = 1;
    if (xsgdata->stage & 0x40000000)
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 1))
    {
        result = 0;
    }

    if (xsgdata->mode == XSG_MODE_SAVE)
    {
        xsgdata->gslot = gidx;
        strncpy(xsgdata->label, label, 63);
        xsgdata->label[63] = '\0';
        xsgdata->playtime = playtime;
        xsgdata->progress = progress;
        xsgdata->thumbIconIdx = thumbIconIdx;
        if (result != 0 && xSG_sv_flipinfo(xsgdata) == 0)
        {
            result = 0;
        }
        if (result != 0 && xSG_sv_prepdest(xsgdata) == 0)
        {
            result = 0;
        }
    }
    else
    {
        if (xSG_ld_prepload(xsgdata) == 0)
        {
            result = 0;
        }
    }

    xsgdata->stage |= 8;
    if (result == 0)
    {
        xsgdata->stage |= 0x40000000;
    }
    return result;
}

#pragma inline_depth(0)
S32 xSGSetup(st_XSAVEGAME_DATA* xsgdata)
{
    return xSGSetup(xsgdata, 0, "nothing", -1, 0, 0);
}
#pragma inline_depth(5)

S32 xSGAddLoadClient(st_XSAVEGAME_DATA* xsgdata, U32 clttag, void* cltdata,
                     S32 (*loadfunc)(void*, st_XSAVEGAME_DATA*, st_XSAVEGAME_READCONTEXT*, U32,
                                     S32))
{
    S32 result = 1;
    if (xsgdata->stage & 0x40000000)
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 1))
    {
        result = 0;
    }
    else if (xsgdata->stkcnt + 1 >= 128)
    {
        xUtil_idtag2string(clttag, 0);
        result = 0;
    }

    if (clttag != 0 || xsgdata->dfltloadclt.idtag == 0)
    {
        if (clttag == 0)
        {
            xsgdata->dfltloadclt.idtag = 1;
            xsgdata->dfltloadclt.cltload = loadfunc;
            xsgdata->dfltloadclt.cltdata = cltdata;
            xsgdata->dfltloadclt.blokact = 0;
            xsgdata->dfltloadclt.blokmax = 0;
            xsgdata->dfltloadclt.blokpos = NULL;
            xsgdata->dfltloadclt.readpos = NULL;
            xsgdata->dfltloadclt.readremain = 0;
            xsgdata->dfltloadclt.readamt = 0;
        }
        else
        {
            st_XSAVEGAME_CLIENT* clt = &xsgdata->cltstk[xsgdata->stkcnt++];
            clt->idtag = clttag;
            clt->cltload = loadfunc;
            clt->cltdata = cltdata;
            clt->blokact = 0;
            clt->blokmax = 0;
            clt->blokpos = NULL;
            clt->readpos = NULL;
            clt->readremain = 0;
            clt->readamt = 0;
        }
    }

    xsgdata->stage |= 2;
    if (result == 0)
    {
        xsgdata->stage |= 0x40000000;
    }
    return result;
}

S32 xSGAddSaveClient(st_XSAVEGAME_DATA* xsgdata, U32 clttag, void* cltdata,
                     S32 (*infofunc)(void*, st_XSAVEGAME_DATA*, S32*, S32*),
                     S32 (*procfunc)(void*, st_XSAVEGAME_DATA*, st_XSAVEGAME_WRITECONTEXT*))
{
    S32 result = 1;
    if (xsgdata->stage & 0x40000000)
    {
        result = 0;
    }
    else if (!(xsgdata->stage & 1))
    {
        result = 0;
    }

    if (xsgdata->stkcnt + 1 >= 128)
    {
        xUtil_idtag2string(clttag, 0);
        result = 0;
    }

    st_XSAVEGAME_CLIENT* clt = &xsgdata->cltstk[xsgdata->stkcnt++];
    clt->idtag = clttag;
    clt->cltinfo = infofunc;
    clt->cltproc = procfunc;
    clt->cltdata = cltdata;
    clt->needamt = 0;
    clt->realamt = 0;
    clt->buf_sizepos = NULL;

    xsgdata->stage |= 4;
    if (result == 0)
    {
        xsgdata->stage |= 0x40000000;
    }
    return result;
}

S32 xSGGameProgress(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    S32 progress = -1;
    if (xSG_chdir_gamedir(xsgdata))
    {
        st_XSAVEGAME_LEADER* lead = &g_leaders[gidx];
        progress = lead->progress;
    }
    return progress;
}

S32 xSGGameThumbIndex(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    S32 idx_thum = -1;
    if (xSG_chdir_gamedir(xsgdata))
    {
        st_XSAVEGAME_LEADER* lead = &g_leaders[gidx];
        idx_thum = lead->thumbIconIdx;
    }
    return idx_thum;
}

char* xSGGameLabel(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    char* da_name = NULL;
    if (xSG_chdir_gamedir(xsgdata) && g_leaders[gidx].gameLabel != NULL)
    {
        da_name = g_leaders[gidx].gameLabel;
    }
    return da_name;
}

// HUDE TO DO
// CHANGE STRINGS, THEYRE PROBABLY NOT RIGHT
// PORTED FROM BATTLE
char* xSGGameModDate(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    static char da_date[32] = {};
    da_date[0] = '\0';
    if (xSG_chdir_gamedir(xsgdata))
    {
        char* slotname = xSG_cm_slotname(xsgdata, gidx);
        char* date = iSGFileModDate(xsgdata->isgsess, slotname);
        if (date != NULL)
        {
            strcpy(da_date, date);
        }
        else
        {
            strcpy(da_date,
#if 1 // Need these strings and can't seem get them any other way
                   0x409 + "The Mystery Machine\0"
                           "Mystery Machine\0"
                           "On Edge in the Hedge!\0"
                           "Hedges\0"
                           "It's a Mean Greenhouse, Scooby!\0"
                           "Greenhouse\0"
                           "Chills & Spills on Haunted Hill!\0"
                           "Haunted Hill\0"
                           "Scared Stiff at Skull Cliff!\0"
                           "Skull Cliff\0"
                           "Misbehavin'? Cause a Cave In\0"
                           "Caves\0"
                           "A Tight Spot for a Grave Plot!\0"
                           "Graves\0"
                           "A One Way Trip to the Crypt!\0"
                           "Crypts\0"
                           "Gloom & Doom Down in the Tomb!\0"
                           "Tombs\0"
                           "Shock on the Dock!\0"
                           "Docks\0"
                           "Fishy Clues for Scooby-Doo?\0"
                           "Cannery Row\0"
                           "Fear on the Pier!\0"
                           "Piers\0"
                           "Coast for Some Ghosts!\0"
                           "Shoreline\0"
                           "Going Down Witch Way?\0"
                           "Coastal Point\0"
                           "Lighthouse Fright House!\0"
                           "Lighthouse\0"
                           "Wreck on the Deck!\0"
                           "Danger Rocks\0"
                           "Aghast by the Mast!\0"
                           "Masts\0"
                           "Shiver Your Timbers, Scooby!\0"
                           "Shipwrecks\0"
                           "Clamor in the Manor!\0"
                           "Manor\0"
                           "Mind Your Manors!\0"
                           "Stairway\0"
                           "All Scares Upstairs!\0"
                           "Upstairs\0"
                           "Don't Look Down, Scooby-Doo!\0"
                           "Hallways\0"
                           "Panic in the Attic!\0"
                           "Attic\0"
                           "A Dark and Stormy Knight!\0"
                           "Rooftops\0"
                           "Who's Yella' in the Cellar?\0"
                           "Cellar\0"
                           "Creepy Crawlies in the Hallways\0"
                           "Catacombs\0"
                           "Gusts Abound Underground\0"
                           "Tunnels\0"
                           "Little Lab of Horrors\0"
                           "Secret Lab\0"
                           "Mastermind Unmasked!\0"
                           "Mastermind\0"
                           "Cower in the Tower!\0"
                           "Tower\0"
                           "Monster Gallery\0"
#endif
                           "");
        }
    }
    return da_date;
}

S32 xSGGameSize(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    S32 size = 0;
    if (xSG_chdir_gamedir(xsgdata))
    {
        char* fname = xSG_cm_slotname(xsgdata, gidx);
        size = iSGFileSize(xsgdata->isgsess, fname);
    }
    return size;
}

S32 xSGGameIsEmpty(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    return xSGGameSize(xsgdata, gidx) <= 0;
}

void xSGGameSet(st_XSAVEGAME_DATA* xsgdata, S32 gidx)
{
    xsgdata->gslot = gidx;
}

U8 xSGCheckMemoryCard(st_XSAVEGAME_DATA* xsgdata, S32 index)
{
    return iSGCheckMemoryCard(xsgdata->isgsess, index);
}

S32 xSGTgtHaveRoomStartup(st_XSAVEGAME_DATA* xsgdata, S32 tidx, S32 fsize, S32 slotidx,
                          S32* bytesNeeded, S32* availOnDisk, S32* needFile)
{
    char fname[256] = {};

    if (slotidx < 0)
    {
        return iSGTgtHaveRoomStartup(xsgdata->isgsess, tidx, fsize, NULL, NULL, bytesNeeded,
                                     availOnDisk, needFile);
    }

    strcpy(fname, iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, slotidx));
    return iSGTgtHaveRoomStartup(xsgdata->isgsess, tidx, fsize, NULL, fname, bytesNeeded,
                                 availOnDisk, needFile);
}

S32 xSGTgtHaveRoom(st_XSAVEGAME_DATA* xsgdata, S32 tidx, S32 fsize, S32 slotidx, S32* bytesNeeded,
                   S32* availOnDisk, S32* needFile)
{
    char fname[256] = {};

    if (slotidx < 0)
    {
        return iSGTgtHaveRoom(xsgdata->isgsess, tidx, fsize, NULL, NULL, bytesNeeded, availOnDisk,
                              needFile);
    }

    strcpy(fname, iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, slotidx));
    return iSGTgtHaveRoom(xsgdata->isgsess, tidx, fsize, NULL, fname, bytesNeeded, availOnDisk,
                          needFile);
}

S32 xSGTgtHasGameDir(st_XSAVEGAME_DATA* xsgdata, S32 tidx)
{
    S32 rc = iSGTgtSetActive(xsgdata->isgsess, tidx);
    if (rc != 0)
    {
        return xSG_chdir_gamedir(xsgdata);
    }
    return rc;
}

S32 xSGTgtSelect(st_XSAVEGAME_DATA* xsgdata, S32 tidx)
{
    S32 result = iSGTgtSetActive(xsgdata->isgsess, tidx);
    if (result != 0 && xSG_chdir_gamedir(xsgdata) != 0)
    {
        xSG_grab_leaders(xsgdata);
    }
    return result;
}

S32 xSGTgtFormatTgt(st_XSAVEGAME_DATA* xsgdata, S32 tidx, S32* canRecover)
{
    return iSGTgtFormat(xsgdata->isgsess, tidx, 0, canRecover);
}

S32 xSGTgtIsFormat(st_XSAVEGAME_DATA* xsgdata, S32 tidx, S32* badEncode)
{
    S32 result = 0;
    U32 state = iSGTgtState(xsgdata->isgsess, tidx, NULL);
    if (state & 0x4000000)
    {
        if (badEncode != NULL)
        {
            *badEncode = 1;
        }
    }
    else if (state & 2)
    {
        result = 1;
    }

    S32 rc = -1;
    if (!(state & 0x1000000))
    {
        rc = result;
    }
    return rc;
}

S32 xSGTgtPhysSlotIdx(st_XSAVEGAME_DATA* xsgdata, S32 tidx)
{
    return iSGTgtPhysSlotIdx(xsgdata->isgsess, tidx);
}

S32 xSGTgtCount(st_XSAVEGAME_DATA* xsgdata, S32* max)
{
    return iSGTgtCount(xsgdata->isgsess, max);
}

S32 xSGCheckForCorruptFiles(st_XSAVEGAME_DATA* xsgdata, char files[][64])
{
    return iSGCheckForCorruptFiles(xsgdata->isgsess, files);
}

S32 xSGDone(st_XSAVEGAME_DATA* xsgdata)
{
    S32 result = 1;
    if (!(xsgdata->stage & 1))
    {
        result = 0;
    }

    if (xsgdata->isgsess != NULL)
    {
        iSGSessionEnd(xsgdata->isgsess);
    }
    if (xsgdata->mode == XSG_MODE_SAVE)
    {
        if (xsgdata->membuf != (char*)0x0)
        {
            xUtil_idtag2string('XSGS', 0);
            xMemPopTemp(xsgdata->membuf);
            xsgdata->membuf = (char*)0x0;
            xsgdata->memsize = 0;
            xsgdata->buf_curpos = (char*)0x0;
        }
    }
    else if (xsgdata->mode == XSG_MODE_LOAD)
    {
        if (xsgdata->membuf != NULL)
        {
            xUtil_idtag2string('XSGL', 0);
            xMemPopTemp(xsgdata->loadbuf);
            xsgdata->loadbuf = NULL;
            xsgdata->loadsize = 0;
            xsgdata->walkpos = NULL;
            xsgdata->walkremain = 0;
        }
        if (xsgdata->loadbuf != NULL)
        {
            xMemPopTemp(xsgdata->loadbuf);
            xsgdata->loadbuf = NULL;
            xsgdata->walkpos = NULL;
            xsgdata->walkremain = 0;
        }
    }

    memset(&g_xsgdata, 0, sizeof(st_XSAVEGAME_DATA));
    memset(&g_leaders, 0, sizeof(g_leaders));
    return result;
}

#pragma inline_depth(0)
st_XSAVEGAME_DATA* xSGInit(en_SAVEGAME_MODE mode)
{
    st_XSAVEGAME_DATA* xsgdata = &g_xsgdata;
    if (g_xsgdata.stage != 0)
    {
        return NULL;
    }

    xsgdata->mode = mode;
    xsgdata->chksum = xUtil_crc_init();
    xsgdata->upd_tally = 1;
    xsgdata->file_chksum = 0;
    xsgdata->read_chksum = xUtil_crc_init();
    xsgdata->stage |= 1;
    xsgdata->isgsess = iSGSessionBegin(xsgdata, xSG_cb_ISGChange, 0);

    if (mode == XSG_MODE_LOAD)
    {
        xSGAddLoadClient(xsgdata, 'LEDR', xsgdata, xSG_cb_leader_load);
    }
    else
    {
        xSGAddSaveClient(xsgdata, 'LEDR', xsgdata, xSG_cb_leader_svinfo, xSG_cb_leader_svproc);
    }

    memset(&g_leaders, 0, sizeof(g_leaders));
    return xsgdata;
}
#pragma inline_depth(5)

S32 xSGShutdown()
{
    g_xsginit--;
    if (g_xsginit == 0)
    {
        iSGShutdown();
        xUtilShutdown();
    }
    return g_xsginit;
}

S32 xSGStartup()
{
    if (g_xsginit++ == 0)
    {
        xUtilStartup();
        iSGStartup();
        memset(&g_xsgdata, 0, sizeof(st_XSAVEGAME_DATA));
    }
    xSGAutoSave_Startup();
    return g_xsginit;
}

//                                              xRumble

//                                              RyzMemGrow

void RyzMemGrow::Done()
{
    this->amt_last = this->amt;
    this->ptr_last = this->ptr;
    this->user_last = this->user;
    this->amt = 0;
    this->ptr = NULL;
    this->user = NULL;
    this->flg_grow = 0;
}

RyzMemGrow* RyzMemGrow::Resume(xBase*)
{
    this->amt = this->amt_last;
    this->ptr = this->ptr_last;
    this->user = this->user_last;
    this->flg_grow = 0b11;
    return this;
}

RyzMemGrow* RyzMemGrow::Init(xBase* growuser)
{
    char* dat;
    if (this->ptr != NULL)
    {
        return this;
    }
    this->amt_last = 0;
    this->ptr_last = NULL;
    this->user_last = NULL;
    this->amt = 0x20;
    dat = (char*)xMemAllocSize(this->amt);
    this->ptr = dat;
    this->user = growuser;
    this->flg_grow = 1;
    return this;
}

//                                                xRender

//                                                xRenderState

void xRenderFixIMEnd()
{
}

void xRenderFixIMBegin()
{
}

void xRenderFixUntexturedEnd(RpAtomic* atomic)
{
}

void xRenderFixUntexturedBegin(RpAtomic* atomic)
{
}

S32 old_alpha_discard_value_gc;
bool alphaDiscardCurrentlySet;

void xRenderStateResetAlphaDiscard()
{
    alphaDiscardCurrentlySet = false;
    RwGameCubeSetAlphaCompare(6, old_alpha_discard_value_gc, 0, 6, old_alpha_discard_value_gc);
    if ((U8)old_alpha_discard_value_gc != 0)
    {
        _rwDlRenderStateSetZCompLoc(FALSE);
    }
    else
    {
        _rwDlRenderStateSetZCompLoc(TRUE);
    }
}

void xRenderStateSetAlphaDiscard(S32 ref)
{
    alphaDiscardCurrentlySet = true;
    RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEALPHATESTFUNCTIONREF,
                                                   &old_alpha_discard_value_gc);
    RwGameCubeSetAlphaCompare(6, ref, 0, 6, ref);
    if (ref > 0)
    {
        _rwDlRenderStateSetZCompLoc(FALSE);
    }
    else
    {
        _rwDlRenderStateSetZCompLoc(TRUE);
    }
}

void xRenderSceneExit()
{
}

void xRenderSceneEnter()
{
}

//                                                xRenderBuffer

void xRenderBufferInit()
{
    gRenderBuffer.m_mode = 0;
    gRenderBuffer.m_indexCount = 0;
    gRenderBuffer.m_vertexCount = 0;
    gRenderBuffer.m_vertexTypeSize = 36;
    gRenderBuffer.m_index = gRenderArr.m_index;
    gRenderBuffer.m_vertex = gRenderArr.m_vertex;
    gRenderBuffer.m_vertexTZ = gRenderArr.m_vertexTZ;
}

//                                              xQuickCull cont.

//                                                  xPTank

//                                                  PKR

static st_PACKER_READ_FUNCS g_pkr_read_funcmap_original = { 1,
                                                            PKR_ReadInit,
                                                            PKR_ReadDone,
                                                            PKR_LoadLayer,
                                                            PKR_GetAssetSize,
                                                            PKR_LoadAsset,
                                                            PKR_AssetByType,
                                                            PKR_AssetCount,
                                                            PKR_IsAssetReady,
                                                            PKR_SetActive,
                                                            PKR_AssetName,
                                                            PKR_GetBaseSector,
                                                            PKR_GetAssetInfo,
                                                            PKR_GetAssetInfoByType,
                                                            PKR_PkgHasAsset,
                                                            PKR_getPackTimestamp,
                                                            PKR_Disconnect };
static st_PACKER_READ_FUNCS g_pkr_read_funcmap = g_pkr_read_funcmap_original;
st_PACKER_READ_DATA g_readdatainst[16] = {};

st_HIPLOADFUNCS* g_hiprf;
U32 g_loadlock;
S32 pkr_sector_size;
volatile S32 g_packinit;
volatile S32 g_memalloc_pair;
volatile S32 g_memalloc_runtot;
volatile S32 g_memalloc_runfree;

void* PKR_getmem(U32 id, S32 amount, U32, S32 align, S32 isTemp, char** memtrue)
{
    if (amount == 0)
    {
        return NULL;
    }

    void* memptr;

    if (isTemp)
    {
        memptr = xMemPushTemp(amount + align);

        if (memtrue != NULL)
        {
            *memtrue = (char*)memptr;
        }

        if (align != 0)
        {
            // TODO: wtf is this
            memptr = (void*)(-align & (U32)((S32)memptr + align - 1));
        }
    }
    else
    {
        memptr = xMemAlloc(gActiveHeap, amount, align);
    }

    if (memptr != NULL)
    {
        memset(memptr, 0, amount);
    }

    g_memalloc_pair++;
    g_memalloc_runtot += amount;
    if (g_memalloc_runtot < 0)
    {
        g_memalloc_runtot = amount;
    }

    if (memptr != NULL)
    {
        xUtil_idtag2string(id, 0);
    }
    else
    {
        xUtil_idtag2string(id, 0);
    }

    return memptr;
}

void PKR_bld_typecnt(st_PACKER_READ_DATA* pr)
{
    st_PACKER_LTOC_NODE* laynode;
    st_PACKER_ATOC_NODE* assnode;
    S32 j;
    S32 i;
    S32 typcnt[129] = {};
    st_XORDEREDARRAY* tmplist;
    U32 lasttype = 0;
    S32 lastidx = 0;

    for (i = 0; i < pr->laytoc.cnt; i++)
    {
        laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        for (j = 0; j < laynode->assref.cnt; j++)
        {
            assnode = (st_PACKER_ATOC_NODE*)laynode->assref.list[j];
            if (!(assnode->loadflag & 0x100000) && !(assnode->loadflag & 0x200000))
            {
                S32 idx;
                if (lasttype != 0 && assnode->asstype == lasttype)
                {
                    idx = lastidx;
                }
                else
                {
                    idx = PKR_typeHdlr_idx(pr, assnode->asstype);
                    lastidx = idx;
                    lasttype = assnode->asstype;
                }

                if (idx < 0)
                {
                    xUtil_idtag2string(assnode->asstype, 0);
                    assnode->Name();
                    typcnt[128]++;
                }
                else
                {
                    typcnt[idx]++;
                }
            }
        }
    }

    for (i = 0; i < 129; i++)
    {
        if (typcnt[i] >= 1)
        {
            XOrdInit(&pr->typelist[i], typcnt[i] > 1 ? typcnt[i] : 2, false);
        }
    }

    for (i = 0; i < pr->laytoc.cnt; i++)
    {
        st_PACKER_LTOC_NODE* laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        for (j = 0; j < laynode->assref.cnt; j++)
        {
            st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)laynode->assref.list[j];
            if (!(assnode->loadflag & 0x100000) && !(assnode->loadflag & 0x200000))
            {
                S32 idx;
                if (lasttype != 0 && assnode->asstype == lasttype)
                {
                    idx = lastidx;
                }
                else
                {
                    idx = PKR_typeHdlr_idx(pr, assnode->asstype);
                    lastidx = idx;
                    lasttype = assnode->asstype;
                }

                st_XORDEREDARRAY* tmplist;
                if (idx < 0)
                {
                    tmplist = &pr->typelist[128];
                }
                else
                {
                    tmplist = &pr->typelist[idx];
                }
                XOrdAppend(tmplist, assnode);
            }
        }
    }
}

S32 LOD_r_LHDR(st_HIPLOADDATA* pkg, st_PACKER_READ_DATA* pr)
{
    S32 i;
    S32 ival = 0;
    S32 refcnt = 0;

    g_hiprf->readLongs(pkg, &ival, 1);
    en_LAYER_TYPE laytyp = (en_LAYER_TYPE)ival;
    g_hiprf->readLongs(pkg, &refcnt, 1);
    st_PACKER_LTOC_NODE* laynode = PKR_newlaynode(laytyp, refcnt);
    XOrdAppend(&pr->laytoc, laynode);

    for (i = 0; i < refcnt; i++)
    {
        g_hiprf->readLongs(pkg, &ival, 1);
        S32 idx = XOrdLookup(&pr->asstoc, (void*)ival, OrdTest_R_AssetID);
        st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)pr->asstoc.list[idx];
        XOrdAppend(&laynode->assref, assnode);

        if (i != refcnt - 1)
        {
            laynode->laysize += assnode->d_size + assnode->d_pad;
        }
        else
        {
            laynode->laysize += assnode->d_size;
        }
    }

    if (laynode->laysize > 0)
    {
        laynode->laysize = laynode->laysize + 0x7ff & 0xfffff800;
    }

    U32 cid = g_hiprf->enter(pkg);
    while (cid != NULL)
    {
        switch (cid)
        {
        case 'LDBG':
            LOD_r_LDBG(pkg, pr, laynode);
            break;
        }
        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }
    return 1;
}

S32 LOD_r_AHDR(st_HIPLOADDATA* pkg, st_PACKER_READ_DATA* pr)
{
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);
    st_PACKER_ATOC_NODE* assnode = PKR_newassnode(pr, ival);
    assnode->ownpr = pr;
    assnode->ownpkg = pkg;
    XOrdAppend(&pr->asstoc, assnode);

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->asstype = ival;
    assnode->typeref = PKR_type2typeref(assnode->asstype, pr->types);

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_off = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_size = ival;
    assnode->readrem = ival;
    assnode->readcnt = 0;
    if (assnode->d_size < 1)
    {
        assnode->loadflag |= 0x200000;
    }

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_pad = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->infoflag = ival;

    U32 cid = g_hiprf->enter(pkg);
    while (cid != 0)
    {
        switch (cid)
        {
        case 'ADBG':
            LOD_r_ADBG(pkg, pr, assnode);
        }
        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    S32 isdup = PKR_FRIEND_assetIsGameDup(assnode->aid, pr, assnode->d_size, assnode->asstype,
                                          assnode->d_chksum, NULL);
    if (isdup)
    {
        assnode->loadflag |= 0x100000;
    }
    return 1;
}

S32 LOD_r_PACK(st_HIPLOADDATA* pkg, st_PACKER_READ_DATA* pr)
{
    U32 cid = g_hiprf->enter(pkg);
    while (cid != 0)
    {
        switch (cid)
        {
        case 'PVER':
            LOD_r_PVER(pkg, pr);
            break;
        case 'PFLG':
            LOD_r_PFLG(pkg, pr);
            break;
        case 'PCNT':
            LOD_r_PCNT(pkg, pr);
            break;
        case 'PCRT':
            LOD_r_PCRT(pkg, pr);
            break;
        case 'PMOD':
            LOD_r_PMOD(pkg, pr);
            break;
        case 'PLAT':
            LOD_r_PLAT(pkg, pr);
            break;
        }
        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }
    return 1;
}

S32 OrdTest_R_AssetID(const void* vkey, void* vitem)
{
    S32 rc;
    if ((U32)vkey < *(U32*)vitem)
    {
        rc = -1;
    }
    else if ((U32)vkey > *(U32*)vitem)
    {
        rc = 1;
    }
    else
    {
        rc = 0;
    }
    return rc;
}

S32 OrdComp_R_Asset(void* vkey, void* vitem)
{
    S32 rc;
    if (*(U32*)vkey < *(U32*)vitem)
    {
        rc = -1;
    }
    else if (*(U32*)vkey > *(U32*)vitem)
    {
        rc = 1;
    }
    else
    {
        rc = 0;
    }
    return rc;
}

S32 PKR_FRIEND_assetIsGameDup(U32 aid, const st_PACKER_READ_DATA* skippr, S32 oursize, U32 ourtype,
                              U32 chksum, char*)
{
    S32 is_dup = 0;
    if (aid == 0x7ab6743a)
    {
        return 0;
    }
    if (aid == 0x98a3f56c)
    {
        return 0;
    }

    for (int i = 0; i < 16; i++)
    {
        if ((g_loadlock & 1 << i) == 0 || &g_readdatainst[i] == skippr)
        {
            continue;
        }

        S32 idx = XOrdLookup(&g_readdatainst[i].asstoc, (void*)aid, OrdTest_R_AssetID);
        if (idx < 0)
        {
            continue;
        }

        st_PACKER_ATOC_NODE* tmp_ass = (st_PACKER_ATOC_NODE*)g_readdatainst[i].asstoc.list[idx];
        if ((tmp_ass->loadflag & 0x80000) == 0 && tmp_ass->asstype != 0x534e4420 &&
            tmp_ass->asstype != 0x534e4453)
        {
            continue;
        }

        tmp_ass->Name();
        xUtil_idtag2string(tmp_ass->asstype, 0);

        if (ourtype != 0 && ourtype != tmp_ass->asstype)
        {
            tmp_ass->Name();
            xUtil_idtag2string(tmp_ass->asstype, 0);
            xUtil_idtag2string(ourtype, 1);
        }
        if (oursize >= 0 && oursize != tmp_ass->d_size)
        {
            tmp_ass->Name();
            xUtil_idtag2string(tmp_ass->asstype, 0);
        }
        if (chksum != 0 && chksum != tmp_ass->d_chksum)
        {
            tmp_ass->Name();
            xUtil_idtag2string(tmp_ass->asstype, 0);
        }
        is_dup = 1;
        break;
    }

    return is_dup;
}

S32 PKR_PkgHasAsset(st_PACKER_READ_DATA* pr, U32 aid)
{
    S32 idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx < 0)
    {
        return 0;
    }

    S32 rc = 1;
    st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)pr->asstoc.list[idx];
    if (assnode->loadflag & 0x100000)
    {
        rc = 0;
    }
    else if (assnode->loadflag & 0x200000)
    {
        rc = 0;
    }
    return rc;
}

S32 PKR_GetAssetInfoByType(st_PACKER_READ_DATA* pr, U32 type, S32 idx, PKRAssetTOCInfo* tocainfo)
{
    memset(tocainfo, 0, sizeof(PKRAssetTOCInfo));
    if (idx < 0)
    {
        idx = 0;
    }

    S32 typeidx = PKR_typeHdlr_idx(pr, type);
    if (typeidx < 0)
    {
        return 0;
    }

    st_XORDEREDARRAY* tmplist = &pr->typelist[typeidx];
    if (idx >= tmplist->cnt)
    {
        return 0;
    }

    st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)tmplist->list[idx];
    tocainfo->aid = assnode->aid;
    tocainfo->typeref = assnode->typeref;
    tocainfo->sector = pr->base_sector + assnode->d_off / pkr_sector_size;
    tocainfo->plus_offset = assnode->d_off % pkr_sector_size;
    tocainfo->size = assnode->d_size;
    tocainfo->mempos = assnode->memloc;

    return 1;
}

S32 PKR_GetAssetInfo(st_PACKER_READ_DATA* pr, U32 aid, PKRAssetTOCInfo* tocainfo)
{
    memset(tocainfo, 0, sizeof(PKRAssetTOCInfo));
    S32 idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx >= 0)
    {
        st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)pr->asstoc.list[idx];
        tocainfo->aid = aid;
        tocainfo->typeref = assnode->typeref;
        tocainfo->sector = pr->base_sector + assnode->d_off / pkr_sector_size;
        tocainfo->plus_offset = assnode->d_off % pkr_sector_size;
        tocainfo->size = assnode->d_size;
        tocainfo->mempos = assnode->memloc;
    }
    return idx >= 0 ? 1 : 0;
}

U32 PKR_GetBaseSector(st_PACKER_READ_DATA* pr)
{
    return pr->base_sector;
}

char* PKR_AssetName(st_PACKER_READ_DATA* pr, U32 aid)
{
    char* name = NULL;

    if (aid == 0)
    {
        return NULL;
    }
    else
    {
        S32 idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
        if (idx >= 0)
        {
            name = ((st_PACKER_ATOC_NODE*)pr->asstoc.list[idx])->Name();
        }
    }

    return name;
}

U32 PKRAssetIDFromInst(void* inst)
{
    return ((st_PACKER_ATOC_NODE*)inst)->aid;
}

void PKR_Disconnect(st_PACKER_READ_DATA* pr)
{
    if (pr->pkg != NULL)
    {
        g_hiprf->destroy(pr->pkg);
        pr->pkg = NULL;
    }
}

U32 PKR_getPackTimestamp(st_PACKER_READ_DATA* pr)
{
    return pr->time_made;
}

S32 PKR_IsAssetReady(st_PACKER_READ_DATA* pr, U32 aid)
{
    S32 ready = false;
    S32 idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx >= 0)
    {
        st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)pr->asstoc.list[idx];
        if (assnode->loadflag & 0x80000)
        {
            ready = true;
        }
        else
        {
            ready = false;
        }
    }

    return ready;
}

void* PKR_AssetByType(st_PACKER_READ_DATA* pr, U32 type, S32 idx, U32* size)
{
    if (size != NULL)
    {
        *size = 0;
    }

    if (idx < 0)
    {
        idx = 0;
    }

    S32 typeidx = PKR_typeHdlr_idx(pr, type);
    if (typeidx < 0)
    {
        return 0;
    }

    st_XORDEREDARRAY* tmplist = &pr->typelist[typeidx];
    if (idx >= tmplist->cnt)
    {
        return 0;
    }

    st_PACKER_ATOC_NODE* assnode = (st_PACKER_ATOC_NODE*)tmplist->list[idx];
    if (size != NULL)
    {
        *size = assnode->d_size;
    }
    return assnode->memloc;
}

S32 PKR_AssetCount(st_PACKER_READ_DATA* pr, U32 type)
{
    S32 count = 0;
    if (type == 0)
    {
        return pr->asstoc.cnt;
    }
    else
    {
        S32 idx = PKR_typeHdlr_idx(pr, type);
        if (idx >= 0)
        {
            st_XORDEREDARRAY* tmplist = &pr->typelist[idx];
            count = tmplist->cnt;
        }
    }
    return count;
}

U32 PKR_GetAssetSize(st_PACKER_READ_DATA* pr, U32 aid)
{
    st_PACKER_ATOC_NODE* assnode = NULL;
    S32 idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);

    if (idx > -1)
    {
        assnode = (st_PACKER_ATOC_NODE*)pr->asstoc.list[idx];
    }

    if (assnode != NULL)
    {
        if (assnode->x_size > 0)
        {
            return assnode->x_size;
        }
        return assnode->d_size;
    }
    return 0;
}

void* PKR_LoadAsset(st_PACKER_READ_DATA* pr, U32 aid, const char*, void*, int*)
{
    return PKR_FindAsset(pr, aid);
}

S32 PKR_LoadLayer(st_PACKER_READ_DATA* pr, en_LAYER_TYPE layer)
{
    return 0;
}

void PKR_xform_asset(st_PACKER_ATOC_NODE* assnode, S32 dumpable_layer)
{
    if (!(assnode->infoflag & 4))
    {
        if (assnode->typeref == NULL)
        {
            return;
        }

        if (assnode->typeref->readXForm != NULL)
        {
            assnode->Name();
            xUtil_idtag2string(assnode->asstype, 0);
        }
        else
        {
            return;
        }
    }

    PKRAssetType* atype = assnode->typeref;
    if (atype == NULL)
    {
        assnode->Name();
    }
    else if (atype->readXForm == NULL)
    {
        assnode->Name();
    }
    else
    {
        if (assnode->d_size == 0)
        {
            assnode->Name();
            xUtil_idtag2string(assnode->asstype, 0);
            assnode->memloc = NULL;
        }

        char* xformloc =
            (char*)atype->readXForm(assnode->ownpr->userdata, assnode->aid, assnode->memloc,
                                    assnode->d_size, (U32*)&assnode->x_size);
        if (!dumpable_layer && assnode->memloc == xformloc && assnode->x_size != 0)
        {
            assnode->Name();
        }
        else if (assnode->d_size == 0 || assnode->x_size == 0)
        {
            assnode->memloc = NULL;
            assnode->loadflag |= 0x200000;
        }
        else
        {
            assnode->memloc = xformloc;
        }
    }
}

void PKR_updateLayerAssets(st_PACKER_LTOC_NODE* laynode)
{
    st_PACKER_ATOC_NODE* tmpass = NULL;
    for (S32 i = 0; i < laynode->assref.cnt; i++)
    {
        tmpass = (st_PACKER_ATOC_NODE*)laynode->assref.list[i];
        if (tmpass->d_off > 0 && tmpass->d_size > 0)
        {
            break;
        }
        tmpass = NULL;
    }

    if (tmpass != NULL)
    {
        S32 lay_hip_pos = tmpass->d_off;
        for (S32 i = 0; i < laynode->assref.cnt; i++)
        {
            st_PACKER_ATOC_NODE* tmpass = (st_PACKER_ATOC_NODE*)laynode->assref.list[i];
            if (!(tmpass->loadflag & 0x100000))
            {
                if (tmpass->loadflag & 0x200000)
                {
                    tmpass->Name();
                    tmpass->memloc = NULL;
                }
                else
                {
                    tmpass->memloc = laynode->laymem + (tmpass->d_off - lay_hip_pos);
                    tmpass->loadflag |= 0x80000;
                    tmpass->Name();
                }
            }
        }
    }
}

S32 PKR_findNextLayerToLoad(st_PACKER_READ_DATA** work_on_pkg, st_PACKER_LTOC_NODE** next_layer)
{
    st_PACKER_READ_DATA* tmppr;
    st_PACKER_LTOC_NODE* tmplay;

    *next_layer = NULL;
    if (*work_on_pkg != NULL)
    {
        tmppr = *work_on_pkg;
        for (S32 i = 0; i < tmppr->laytoc.cnt; i++)
        {
            tmplay = (st_PACKER_LTOC_NODE*)tmppr->laytoc.list[i];
            if (!(tmplay->flg_ldstat & 0x2000000))
            {
                *next_layer = tmplay;
                *work_on_pkg = tmppr;
                break;
            }
        }
    }

    if (*next_layer == NULL)
    {
        tmppr = g_readdatainst;
        for (S32 i = 0; i < 16; i++, tmppr++)
        {
            if ((g_loadlock & 1 << i) == 0 || tmppr == *work_on_pkg)
            {
                continue;
            }

            for (S32 j = 0; j < tmppr->laytoc.cnt; j++)
            {
                tmplay = (st_PACKER_LTOC_NODE*)tmppr->laytoc.list[j];
                if (!(tmplay->flg_ldstat & 0x2000000))
                {
                    *next_layer = tmplay;
                    *work_on_pkg = tmppr;
                    break;
                }
            }

            if (*next_layer != NULL)
            {
                break;
            }
        }
    }

    return *next_layer != NULL;
}

S32 PKR_LoadStep_Async()
{
    S32 rc;
    static st_PACKER_READ_DATA* curpr = NULL;
    static st_PACKER_LTOC_NODE* asynlay = NULL;

    if (asynlay == NULL)
    {
        PKR_findNextLayerToLoad(&curpr, &asynlay);

        if (asynlay != NULL)
        {
            if (PKR_layerLoadDest(asynlay->laytyp) != PKR_LDDEST_SKIP && asynlay->laysize > 1 &&
                asynlay->assref.cnt > 0)
            {
                asynlay->laymem = PKR_LayerMemReserve(curpr, asynlay);
                PKR_drv_guardLayer(asynlay);
                st_PACKER_ATOC_NODE* tmpass = (st_PACKER_ATOC_NODE*)asynlay->assref.list[0];
                g_hiprf->setSpot(tmpass->ownpkg, tmpass->d_off);

                if (g_hiprf->readBytes(tmpass->ownpkg, asynlay->laymem, asynlay->laysize))
                {
                    asynlay->flg_ldstat |= 0x1000000;
                }
                else
                {
                    PKR_LayerMemRelease(curpr, asynlay);
                    asynlay->flg_ldstat &= 0xfcffffff;
                    curpr = NULL;
                    asynlay = NULL;
                }

                rc = 1;
            }
            else
            {
                rc = 1;
                asynlay->flg_ldstat |= 0x2000000;
                asynlay = NULL;
            }
        }
        else
        {
            rc = 0;
            curpr = NULL;
            asynlay = NULL;
        }
    }
    else
    {
        S32 moretodo = g_hiprf->pollRead(curpr->pkg);
        if (moretodo == 1)
        {
            moretodo = PKR_drv_guardVerify(asynlay);
        }

        if (!moretodo)
        {
            rc = 1;
        }
        else if (moretodo == 1)
        {
            PKR_updateLayerAssets(asynlay);

            if (PKR_layerTypeNeedsXForm(asynlay->laytyp))
            {
                PKR_xformLayerAssets(asynlay);
            }

            if (PKR_layerLoadDest(asynlay->laytyp) == PKR_LDDEST_RWHANDOFF)
            {
                PKR_LayerMemRelease(curpr, asynlay);
            }

            rc = 1;
            asynlay->flg_ldstat |= 0x2000000;
            asynlay = NULL;
        }
        else
        {
            en_PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(asynlay->laytyp);
            if (asynlay->laymem != NULL && loaddest == PKR_LDDEST_RWHANDOFF)
            {
                PKR_LayerMemRelease(curpr, asynlay);
            }

            rc = 1;
            asynlay->flg_ldstat &= 0xfcffffff;
            asynlay = NULL;
            curpr = NULL;
        }
    }

    return rc;
}

S32 PKR_parse_TOC(st_HIPLOADDATA* pkg, st_PACKER_READ_DATA* pr)
{
    S32 done = 0;
    S32 is_ok = 0;
    U32 cid = g_hiprf->enter(pkg);
    while (cid != false)
    {
        switch (cid)
        {
        case 'HIPA':
            done = LOD_r_HIPA(pkg, pr);
            break;
        case 'PACK':
            LOD_r_PACK(pkg, pr);
            if (done)
            {
                if (pr->asscnt > 0)
                {
                    XOrdInit(&pr->asstoc, pr->asscnt, 0);
                    PKR_makepool_anode(pr, pr->asscnt);
                }
                if (pr->laycnt > 0)
                {
                    XOrdInit(&pr->laytoc, pr->laycnt, 0);
                }
            }
            break;
        case 'DICT':
            LOD_r_DICT(pkg, pr);
            PKR_bld_typecnt(pr);
            is_ok = true;
            break;
        case 'STRM':
            LOD_r_STRM(pkg, pr);
            break;
        }

        if (!done)
        {
            break;
        }

        g_hiprf->exit(pkg);

        if (is_ok)
        {
            break;
        }

        cid = g_hiprf->enter(pkg);
    }
    return done;
}

S32 PKR_SetActive(st_PACKER_READ_DATA* pr, en_LAYER_TYPE layer)
{
    S32 result;
    S32 rc = 1;
    S32 i;
    S32 j;
    st_PACKER_ATOC_NODE* assnode;
    st_PACKER_LTOC_NODE* laynode;

    if (pr == NULL)
    {
        return 0;
    }

    for (i = 0; i < pr->laytoc.cnt; i++)
    {
        laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        if (layer > PKR_LTYPE_DEFAULT && laynode->laytyp != layer)
        {
            continue;
        }

        for (j = 0; j < laynode->assref.cnt; j++)
        {
            assnode = (st_PACKER_ATOC_NODE*)laynode->assref.list[j];
            if (assnode->loadflag & 0x10000 || (assnode->loadflag & 0x80000) == 0)
            {
                continue;
            }

            if (assnode->typeref == NULL)
            {
                assnode->Name();
                xUtil_idtag2string(assnode->asstype, 0);
            }
            else if (assnode->typeref->assetLoaded != NULL)
            {
                if (!(assnode->typeref->assetLoaded(pr->userdata, assnode->aid, assnode->memloc,
                                                    assnode->d_size)))
                {
                    rc = 0;
                }
                else
                {
                    assnode->loadflag |= 0x10000;
                }
            }
        }
    }

    return rc;
}

void PKR_ReadDone(st_PACKER_READ_DATA* pr)
{
    S32 i;
    S32 j;
    S32 lockid;
    st_PACKER_ATOC_NODE* assnode;
    st_PACKER_LTOC_NODE* laynode;
    st_XORDEREDARRAY* tmplist;

    if (pr == NULL)
    {
        return;
    }
    for (i = pr->laytoc.cnt - 1; i >= 0; i--)
    {
        st_PACKER_LTOC_NODE* laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        for (j = laynode->assref.cnt - 1; j >= 0; j--)
        {
            assnode = (st_PACKER_ATOC_NODE*)laynode->assref.list[j];
            if (assnode->typeref != NULL && assnode->typeref->assetUnloaded != NULL &&
                !(assnode->loadflag & 0x100000))
            {
                assnode->typeref->assetUnloaded(assnode->memloc, assnode->aid);
            }
        }
    }

    for (i = 0; i < pr->laytoc.cnt; i++)
    {
        laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        if (laynode->laymem != NULL)
        {
            PKR_LayerMemRelease(pr, laynode);
            laynode->laymem = NULL;
        }
    }

    PKR_kiilpool_anode(pr);

    for (i = 0; i < pr->laytoc.cnt; i++)
    {
        laynode = (st_PACKER_LTOC_NODE*)pr->laytoc.list[i];
        PKR_oldlaynode(laynode);
    }

    XOrdDone(&pr->asstoc, false);
    XOrdDone(&pr->laytoc, false);

    for (i = 0; i < 129; i++)
    {
        tmplist = &pr->typelist[i];
        if (tmplist->max != 0)
        {
            XOrdDone(tmplist, false);
        }
    }

    if (pr->pkg != NULL)
    {
        g_hiprf->destroy(pr->pkg);
        pr->pkg = NULL;
    }

    lockid = pr->lockid;
    memset(pr, 0, sizeof(st_PACKER_READ_DATA));
    g_loadlock &= ~(1 << lockid);
}

#pragma inline_depth(0)
st_PACKER_READ_DATA* PKR_ReadInit(void* userdata, const char* pkgfile, U32 opts, S32* cltver,
                                  PKRAssetType* typelist, int)
{
    // I'm pretty sure this is just a pointer to an array on the heap
    // But it needs to be an array of pointers to generate correct code ???
    char* tocbuf_RAW[1];
    tocbuf_RAW[0] = NULL;
    st_PACKER_READ_DATA* pr = NULL;
    S32 uselock = -1;

    PKR_alloc_chkidx();
    char* tocbuf_aligned = (char*)PKR_getmem('PTOC', 0x8000, 'PTOC', 0x40, 1, tocbuf_RAW);

    for (S32 i = 0; i < 16; i++)
    {
        if (!(g_loadlock & 1 << i))
        {
            g_loadlock |= 1 << i;
            pr = &g_readdatainst[i];
            uselock = i;
            break;
        }
    }

    if (pr != NULL)
    {
        memset(pr, 0, sizeof(st_PACKER_READ_DATA));
        pr->lockid = uselock;
        pr->userdata = userdata;
        pr->opts = opts;
        pr->types = typelist;
        pr->cltver = -1;
        strncpy(pr->packfile, pkgfile, 0x80);

        if (tocbuf_aligned == NULL)
        {
            pr->pkg = g_hiprf->create(pkgfile, NULL, 0);
        }
        else
        {
            pr->pkg = g_hiprf->create(pkgfile, tocbuf_aligned, 0x8000);
        }

        if (pr->pkg != NULL)
        {
            pr->base_sector = g_hiprf->basesector(pr->pkg);
            PKR_parse_TOC(pr->pkg, pr);
            *cltver = pr->cltver;
            g_hiprf->setBypass(pr->pkg, 1, 1);
        }
        else
        {
            PKR_ReadDone(pr);
            pr = NULL;
            *cltver = -1;
        }
    }
    else
    {
        *cltver = -1;
        pr = NULL;
    }

    PKR_relmem('PTOC', 0x8000, (void*)tocbuf_RAW[0], 'PTOC', 1);
    tocbuf_RAW[0] = NULL;
    return pr;
}
#pragma inline_depth(5)

S32 PKRLoadStep(S32)
{
    return PKR_LoadStep_Async();
}

S32 PKRShutdown()
{
    return g_packinit--;
}

S32 PKRStartup()
{
    if (g_packinit++ == 0)
    {
        g_pkr_read_funcmap = g_pkr_read_funcmap_original;
        g_hiprf = get_HIPLFuncs();
        pkr_sector_size = 32;
    }
    return g_packinit;
}

st_PACKER_READ_FUNCS* PKRGetReadFuncs(S32 apiver)
{
    switch (apiver)
    {
    case 1:
        return &g_pkr_read_funcmap;
    default:
        return NULL;
    }
}

//                                                  xPartition

void xPartitionReset()
{
}

//                                                  xParticle

//                                                  xPar

volatile static U32 sFrameCount;
static S32 sParMgrInit;

#define REG_TABLE_SIZE 255

extern F32 _708;
extern F32 _709;

S32 sParGroupRegTableInit = 0;
S32 sParGroupRegTableCount = 0;
extern xParGroup* sParGroupRegTable[REG_TABLE_SIZE]; // todo: move from bss.s

void xParMgrRender()
{
    iParMgrRender();
}

void xParMgrUpdate(F32 elapsedTime)
{
    iParMgrUpdate(elapsedTime);

    sFrameCount++;

    if (sFrameCount > 10)
    {
        sFrameCount = 1;
    }

    U32 i;
    for (i = sFrameCount; i >= 1; i--)
        ;
    sFrameCount = i;
}

void xParMgrInit()
{
    if (sParMgrInit == 0)
    {
        xParMemInit();
        iParMgrInit();
        sParMgrInit = 1;
    }
    else
    {
        xParMgrKillAllParticles();
        iParMgrInit();
    }
}

void xParGroupKillPar(xParGroup* ps, xPar* p)
{
    if (p == ps->m_root)
    {
        ps->m_root = p->m_next;

        if (ps->m_root)
        {
            ps->m_root->m_prev = NULL;
        }

        p->m_next = NULL;
        p->m_prev = NULL;
    }
    else
    {
        if (p->m_next)
        {
            p->m_next->m_prev = p->m_prev;
        }

        if (p->m_prev)
        {
            p->m_prev->m_next = p->m_next;
        }

        p->m_prev = NULL;
        p->m_next = NULL;
    }

    ps->m_num_of_particles--;

    if (ps->m_flags & XPARGROUP_ALLOCPARS)
    {
        xParFree(p);
    }
    else
    {
        xParGroupAddParToDeadList(ps, p);
    }
}

xPar* xParGroupAddPar(xParGroup* ps)
{
    xPar* p;

    if (ps->m_flags & XPARGROUP_ALLOCPARS)
    {
        p = xParAlloc();

        if (!p)
        {
            S32 myPriority = ps->m_priority;

            for (U8 i = 0; i < REG_TABLE_SIZE; i++)
            {
                if (sParGroupRegTable[i] && sParGroupRegTable[i] != ps &&
                    (sParGroupRegTable[i]->m_flags & XPARGROUP_ALLOCPARS) &&
                    !(sParGroupRegTable[i]->m_flags & XPARGROUP_UNK10) &&
                    sParGroupRegTable[i]->m_priority < myPriority && sParGroupRegTable[i]->m_root)
                {
                    xParGroupKillPar(sParGroupRegTable[i], sParGroupRegTable[i]->m_root);

                    p = xParAlloc();

                    break;
                }
            }
        }
    }
    else if (ps->m_dead)
    {
        p = ps->m_dead;
        ps->m_dead = p->m_next;

        if (ps->m_dead)
        {
            ps->m_dead->m_prev = NULL;
        }
    }
    else
    {
        p = NULL;
    }

    if (p)
    {
        xParInit(p);
        xParGroupAddParP(ps, p);
    }

    return p;
}

#define clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))

void xParGroupAnimate(xParGroup* ps, F32 dt)
{
    xPar* i = ps->m_root;
    xPar* tmp;
    F32 age = dt;

    if (ps->m_flags & XPARGROUP_NOAGING)
    {
        age = _708;
    }

    if (!ps->m_root)
    {
        ps->m_alive = 0;
    }

    while (i)
    {
        if (i->m_lifetime < _708)
        {
            tmp = i;

            i = i->m_next;

            xParGroupKillPar(ps, tmp);
        }
        else
        {
            // non-matching: stack isn't resizing for each float-to-U8 conversion
            i->m_c[0] = i->m_cfl[0] = clamp(i->m_cvel[0] * dt + i->m_cfl[0], _708, _709);
            i->m_c[1] = i->m_cfl[1] = clamp(i->m_cvel[1] * dt + i->m_cfl[1], _708, _709);
            i->m_c[2] = i->m_cfl[2] = clamp(i->m_cvel[2] * dt + i->m_cfl[2], _708, _709);
            i->m_c[3] = i->m_cfl[3] = clamp(i->m_cvel[3] * dt + i->m_cfl[3], _708, _709);
            i->m_size += i->m_sizeVel * dt;
            i->m_lifetime -= age;

            i = i->m_next;
        }
    }
}

struct xCmdInfo
{
    U32 type;
    U32 size;
    xParCmdUpdateFunc func;
};

static xCmdInfo sCmdInfo[XPARCMD_TYPE_COUNT] = {};

extern F32 _658_0;
extern F32 _691;
extern F32 _760;
extern F32 _772;
extern F32 _812;
extern F32 _830_1;
extern F32 _831_0; // TO-DO - WRONG - CHANGE LATER
extern F32 _857_0;
extern F32 _858;
extern F32 _1075;
extern F32 _1076;
extern F32 _1077;

void xParCmd_SizeInOut_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdSizeInOutData* cmd = (xParCmdSizeInOutData*)c->tasset;

    if (cmd->enabled)
    {
        p = ps->m_root;

        S32 i, seg;
        F32 slope_size[3];

        slope_size[0] = _1075 * (cmd->custSize[1] - cmd->custSize[0]);
        slope_size[1] = _1075 * (cmd->custSize[2] - cmd->custSize[1]);
        slope_size[2] = _1075 * (cmd->custSize[3] - cmd->custSize[2]);

        while (p)
        {
            // non-matching: there is definitely a clamp happening here, but it isn't using the CLAMP macro.
            F32 frac = CLAMP(_812 - p->m_lifetime / p->totalLifespan, _691, _812);

            if (frac < _1076)
            {
                seg = 0;
            }
            else if (frac < _1077)
            {
                seg = 1;
            }
            else
            {
                seg = 2;
            }

            for (S32 i = seg; i > 0; i--)
            {
                // non-matching: _1076 is cached before loop
                frac -= _1076;
            }

            p->m_size = frac * slope_size[seg] + cmd->custSize[seg];

            p = p->m_next;
        }
    }
}

void xParCmd_DampenSpeed_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdDampenData* cmd = (xParCmdDampenData*)c->tasset;

    if (cmd->enabled)
    {
        p = ps->m_root;

        F32 damp = dt * cmd->dampSpeed;

        while (p)
        {
            xVec3AddScaled(&p->m_vel, &p->m_vel, damp);

            p = p->m_next;
        }
    }
}

void xParCmdCollideFallSticky_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xParCmdCollideFallSticky& cmd = *(xParCmdCollideFallSticky*)c->tasset;
    F32 xzdamp = _812 - cmd.sticky;
    xPar* p = ps->m_root;

    while (p)
    {
        F32& loc = p->m_pos.y;
        F32& vel = p->m_vel.y;
        F32 dloc = cmd.y - loc;

        if (dloc < _691)
        {
            // lol
        }
        else
        {
            loc = dloc * cmd.bounce + cmd.y;

            if (vel < _691)
            {
                vel = -vel * cmd.bounce;
            }

            p->m_vel.x *= xzdamp;
            p->m_vel.z *= xzdamp;
        }

        p = p->m_next;
    }
}

void xParCmdCollideFall_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xParCmdCollideFall& cmd = *(xParCmdCollideFall*)c->tasset;
    xPar* p = ps->m_root;

    while (p)
    {
        F32& loc = p->m_pos.y;
        F32& vel = p->m_vel.y;
        F32 dloc = cmd.y - loc;

        if (dloc < _691)
        {
            // lol
        }
        else
        {
            loc = dloc * cmd.bounce + cmd.y;

            if (vel < _691)
            {
                vel = -vel * cmd.bounce;
            }
        }

        p = p->m_next;
    }
}

void xParCmdTexAnim_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdTexAnim* cmd = (xParCmdTexAnim*)c->tasset;
    xParCmdTex* tex = ps->m_cmdTex;

    if (!tex)
    {
        return;
    }

    if (cmd->throttle_time > _691)
    {
        cmd->throttle_time_elapsed -= dt;

        if (cmd->throttle_time_elapsed > _691)
        {
            return;
        }

        cmd->throttle_time_elapsed = cmd->throttle_time;
    }

    p = ps->m_root;

    if (cmd->anim_mode == 0)
    {
        while (p)
        {
            p->m_texIdx[0] = xrand() % tex->cols;
            p->m_texIdx[1] = xrand() % tex->rows;

            p = p->m_next;
        }
    }
    else if (cmd->anim_mode == 1)
    {
        if (tex->cols > 1)
        {
            while (p)
            {
                p->m_texIdx[0]++;

                if (p->m_texIdx[0] >= tex->cols)
                {
                    p->m_texIdx[0] = 0;

                    if (cmd->anim_wrap_mode == 1)
                    {
                        p->m_texIdx[1]++;

                        if (p->m_texIdx[1] >= tex->rows)
                        {
                            p->m_texIdx[1] = 0;
                        }
                    }
                    else if (cmd->anim_wrap_mode == 2)
                    {
                        if (p->m_texIdx[1] == 0)
                        {
                            p->m_texIdx[1] = tex->rows - 1;
                        }
                        else
                        {
                            p->m_texIdx[1]--;
                        }
                    }
                    else if (cmd->anim_wrap_mode == 3)
                    {
                        p->m_texIdx[1]++;

                        if (p->m_texIdx[1] >= tex->rows)
                        {
                            p->m_texIdx[1] = tex->rows - 1;
                            p->m_texIdx[0] = tex->cols - 1;
                        }
                    }
                }

                p = p->m_next;
            }
        }
    }
    else if (cmd->anim_mode == 2)
    {
        if (tex->cols > 1)
        {
            while (p)
            {
                if (p->m_texIdx[0] == 0)
                {
                    p->m_texIdx[0] = tex->cols - 1;

                    if (cmd->anim_wrap_mode == 1)
                    {
                        p->m_texIdx[1]++;

                        if (p->m_texIdx[1] >= tex->rows)
                        {
                            p->m_texIdx[1] = 0;
                        }
                    }
                    else if (cmd->anim_wrap_mode == 2)
                    {
                        if (p->m_texIdx[1] == 0)
                        {
                            p->m_texIdx[1] = tex->rows - 1;
                        }
                        else
                        {
                            p->m_texIdx[1]--;
                        }
                    }
                }
                else
                {
                    p->m_texIdx[0]--;
                }

                p = p->m_next;
            }
        }
    }
    else if (cmd->anim_mode == 3)
    {
        if (tex->rows > 1)
        {
            while (p)
            {
                if (p->m_texIdx[1] == 0)
                {
                    p->m_texIdx[1] = tex->rows - 1;

                    if (cmd->anim_wrap_mode == 1)
                    {
                        p->m_texIdx[0]++;

                        if (p->m_texIdx[0] >= tex->cols)
                        {
                            p->m_texIdx[0] = 0;
                        }
                    }
                    else if (cmd->anim_wrap_mode == 2)
                    {
                        if (p->m_texIdx[0] == 0)
                        {
                            p->m_texIdx[0] = tex->cols - 1;
                        }
                        else
                        {
                            p->m_texIdx[0]--;
                        }
                    }
                }
                else
                {
                    p->m_texIdx[1]--;
                }

                p = p->m_next;
            }
        }
    }
    else if (cmd->anim_mode == 4)
    {
        if (tex->rows > 1)
        {
            while (p)
            {
                p->m_texIdx[1]++;

                if (p->m_texIdx[1] >= tex->rows)
                {
                    p->m_texIdx[1] = 0;

                    if (cmd->anim_wrap_mode == 1)
                    {
                        p->m_texIdx[0]++;

                        if (p->m_texIdx[0] >= tex->cols)
                        {
                            p->m_texIdx[0] = 0;
                        }
                    }
                    else if (cmd->anim_wrap_mode == 2)
                    {
                        if (p->m_texIdx[0] == 0)
                        {
                            p->m_texIdx[0] = tex->cols - 1;
                        }
                        else
                        {
                            p->m_texIdx[0]--;
                        }
                    }
                }

                p = p->m_next;
            }
        }
    }
}

void xParCmdTex_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
}

void xParCmdRotateAround_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdRotateAround* cmd = (xParCmdRotateAround*)c->tasset;

    F32 yaw = _857_0 * (dt * cmd->yaw) / _858;
    F32 radius_growth = dt * cmd->radius_growth;

    while (p)
    {
        xVec3 at;
        xVec3Sub(&at, &cmd->pos, &p->m_pos);

        at.y = _691;

        xMat3x3 lookmat;

        F32 radius = xMat3x3LookVec(&lookmat, &at);

        xVec3 angles;
        xMat3x3GetEuler(&lookmat, &angles);

        angles.x += yaw;

        xMat3x3 rotmat;
        xMat3x3Euler(&rotmat, angles.x, angles.y, angles.z);

        xVec3 var_BC, var_C8;

        // non-matching: f0 and f1 swapped

        var_BC.x = _691;
        var_BC.y = _691;
        var_BC.z = radius + radius_growth;

        xMat3x3RMulVec(&var_C8, &rotmat, &var_BC);

        p->m_pos.x = var_C8.x + cmd->pos.x;
        p->m_pos.z = var_C8.z + cmd->pos.z;

        p = p->m_next;
    }
}

void xParCmdVelocityApply_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;

    while (p)
    {
        xVec3Add(&p->m_pos, &p->m_pos, &p->m_vel);

        p = p->m_next;
    }
}

void xParCmdRotPar_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    // Removed ported code cus it wasnt even close to correct (2%)
}

void xParCmdApplyWind_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;

    // non-matching: f2 and f3 are combined into one register
    F32 f2 = _812 * (((xParCmdApplyWind*)c->tasset)->unknown * dt);
    F32 f3 = _812 * (((xParCmdApplyWind*)c->tasset)->unknown * dt);

    while (p)
    {
        p->m_vel.x += f2;
        p->m_vel.z += f3;

        p = p->m_next;
    }
}

void xParCmdRandomVelocityPar_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdRandomVelocityPar* cmd = (xParCmdRandomVelocityPar*)c->tasset;
    F32 f31 = cmd->x * dt;
    F32 f30 = cmd->y * dt;
    F32 f29 = cmd->z * dt;

    while (p)
    {
        xMat3x3 var_88;

        F32 y = _760 * (f31 * xurand()) - f31;
        F32 x = _760 * (f30 * xurand()) - f30;
        F32 z = _760 * (f29 * xurand()) - f29;

        xMat3x3Euler(&var_88, x, y, z);
        xMat3x3LMulVec(&p->m_vel, &var_88, &p->m_vel);

        p = p->m_next;
    }
}

void xParCmdAlpha3rdPolyReg_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
}

void xParCmdScale_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
}

void xParCmdSmokeAlpha_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
}

void xParCmdScale3rdPolyReg_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
}

void xParCmdMoveRandomPar_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    // Not even close. (5%)
    xPar* p = ps->m_root;
    xParCmdMoveRandomPar* cmd = (xParCmdMoveRandomPar*)c->tasset;
    F32 f31 = cmd->dim.x * (dt * _772);
    F32 f30 = cmd->dim.z * (dt * _772);

    while (p)
    {
        p->m_pos.x += f31 * (xurand() - _772);
        p->m_pos.z += f30 * (xurand() - _772);

        p = p->m_next;
    }
}

void xParCmdMoveRandom_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdMove* cmd = (xParCmdMove*)c->tasset;

    xVec3 var_28;
    var_28 = cmd->dir;

    var_28.x *= _760 * xurand();
    var_28.y *= _760 * xurand();
    var_28.z *= _760 * xurand();

    xVec3Sub(&var_28, &var_28, &cmd->dir);
    xVec3SMulBy(&var_28, dt);

    p = ps->m_root;

    while (p)
    {
        xVec3Add(&p->m_pos, &p->m_pos, &var_28);

        p = p->m_next;
    }
}

void xParCmdMove_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdMove* cmd = (xParCmdMove*)c->tasset;

    xVec3 var_28;
    var_28 = cmd->dir;

    xVec3SMulBy(&var_28, dt);

    p = ps->m_root;

    while (p)
    {
        xVec3Add(&p->m_pos, &p->m_pos, &var_28);

        p = p->m_next;
    }
}

void xParCmdAccelerate_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p;
    xParCmdAccelerate* cmd = (xParCmdAccelerate*)c->tasset;

    xVec3 var_28;
    var_28 = cmd->acc;

    xVec3SMulBy(&var_28, dt);

    p = ps->m_root;

    while (p)
    {
        p->m_vel.x += var_28.x;
        p->m_vel.y += var_28.y;
        p->m_vel.z += var_28.z;

        p = p->m_next;
    }
}

void xParCmdOrbitLine_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdOrbitLine* cmd = (xParCmdOrbitLine*)c->tasset;
    F32 mdt = cmd->gravity * dt;

    while (p)
    {
        xVec3 var_34, var_40, var_4C, var_58;

        xVec3Sub(&var_34, &p->m_pos, &cmd->p);
        xVec3Cross(&var_4C, &var_34, &cmd->axis);
        xVec3Cross(&var_40, &cmd->axis, &var_4C);
        xVec3Sub(&var_58, &var_40, &var_34);

        F32 f31 = xVec3Length2(&var_58);

        if (f31 < cmd->maxRadiusSqr)
        {
            F32 f1 = xVec3LengthFast(var_58.x, var_58.y, var_58.z);

            F32 force = mdt / (f1 + (f31 + cmd->epsilon));

            p->m_vel.x += var_58.x * force;
            p->m_vel.y += var_58.y * force;
            p->m_vel.z += var_58.z * force;
        }

        p = p->m_next;
    }
}

void xParCmdOrbitPoint_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdOrbitPoint* cmd = (xParCmdOrbitPoint*)c->tasset;
    F32 mdt = cmd->gravity * dt;

    while (p)
    {
        xVec3 var_38;

        xVec3Sub(&var_38, &cmd->center, &p->m_pos);

        F32 f31 = xVec3Length2(&var_38);

        if (f31 < cmd->maxRadiusSqr)
        {
            F32 f1 = xVec3LengthFast(var_38.x, var_38.y, var_38.z);

            F32 force = mdt / (f1 + (f31 + cmd->epsilon));

            p->m_vel.x += var_38.x * force;
            p->m_vel.y += var_38.y * force;
            p->m_vel.z += var_38.z * force;
        }

        p = p->m_next;
    }
}

void xParCmdFollow_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdFollow* cmd = (xParCmdFollow*)c->tasset;
    F32 mdt = cmd->gravity * dt;

    while (p && p->m_next)
    {
        xVec3 var_38;

        xVec3Sub(&var_38, &p->m_next->m_pos, &p->m_pos);

        F32 f31 = xVec3Length2(&var_38);
        F32 f1 = xVec3LengthFast(var_38.x, var_38.y, var_38.z);

        F32 force = mdt / (f1 * (f31 + cmd->epsilon));

        p->m_vel.x += var_38.x * force;
        p->m_vel.y += var_38.y * force;
        p->m_vel.z += var_38.z * force;

        p = p->m_next;
    }
}

void xParCmdAge_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    F32 age_rate = ((xParCmdAge*)c->tasset)->unknown * dt;

    while (p)
    {
        p->m_lifetime -= age_rate;
        p = p->m_next;
    }
}

void xParCmdKillSlow_Update(xParCmd* c, xParGroup* ps, F32 dt)
{
    xPar* p = ps->m_root;
    xParCmdKillSlow* cmd = (xParCmdKillSlow*)c->tasset;
    F32 speedLimit = cmd->speedLimitSqr * dt;

    if (cmd->kill_less_than)
    {
        while (p)
        {
            if (xVec3Length2(&p->m_vel) < speedLimit)
            {
                p->m_lifetime = _658_0;
            }

            p = p->m_next;
        }
    }
    else
    {
        while (p)
        {
            if (xVec3Length2(&p->m_vel) > speedLimit)
            {
                p->m_lifetime = _658_0;
            }

            p = p->m_next;
        }
    }
}

xParCmdUpdateFunc xParCmdGetUpdateFunc(U32 parType)
{
    for (S32 i = 0; i < XPARCMD_TYPE_COUNT; i++)
    {
        if (sCmdInfo[i].type == parType)
        {
            return sCmdInfo[i].func;
        }
    }

    return NULL;
}

void xParCmdRegister(U32 parType, U32 size, xParCmdUpdateFunc func)
{
    sCmdInfo[parType].type = parType;
    sCmdInfo[parType].size = size;
    sCmdInfo[parType].func = func;
}

void xParCmdInit()
{
    xParCmdRegister(XPARCMD_TYPE_MOVE, sizeof(xParCmdMove), xParCmdMove_Update);
    xParCmdRegister(XPARCMD_TYPE_MOVERANDOM, sizeof(xParCmdMove), xParCmdMoveRandom_Update);
    xParCmdRegister(XPARCMD_TYPE_ACCELERATE, sizeof(xParCmdAccelerate), xParCmdAccelerate_Update);
    xParCmdRegister(XPARCMD_TYPE_VELOCITYAPPLY, sizeof(xParCmdAsset), xParCmdVelocityApply_Update);
    xParCmdRegister(XPARCMD_TYPE_UNK5, sizeof(xParCmdUnk5), NULL);
    xParCmdRegister(XPARCMD_TYPE_KILLSLOW, sizeof(xParCmdKillSlow), xParCmdKillSlow_Update);
    xParCmdRegister(XPARCMD_TYPE_FOLLOW, sizeof(xParCmdFollow), xParCmdFollow_Update);
    xParCmdRegister(XPARCMD_TYPE_ORBITPOINT, sizeof(xParCmdOrbitPoint), xParCmdOrbitPoint_Update);
    xParCmdRegister(XPARCMD_TYPE_ORBITLINE, sizeof(xParCmdOrbitLine), xParCmdOrbitLine_Update);
    xParCmdRegister(XPARCMD_TYPE_MOVERANDOMPAR, sizeof(xParCmdMoveRandomPar),
                    xParCmdMoveRandomPar_Update);
    xParCmdRegister(XPARCMD_TYPE_SCALE3RDPOLYREG, sizeof(xParCmdScale3rdPolyReg),
                    xParCmdScale3rdPolyReg_Update);
    xParCmdRegister(XPARCMD_TYPE_TEX, sizeof(xParCmdTex), xParCmdTex_Update);
    xParCmdRegister(XPARCMD_TYPE_TEXANIM, sizeof(xParCmdTexAnim), xParCmdTexAnim_Update);
    xParCmdRegister(XPARCMD_TYPE_RANDOMVELOCITYPAR, sizeof(xParCmdRandomVelocityPar),
                    xParCmdRandomVelocityPar_Update);
    xParCmdRegister(XPARCMD_TYPE_AGE, sizeof(xParCmdAge), xParCmdAge_Update);
    xParCmdRegister(XPARCMD_TYPE_ALPHA3RDPOLYREG, sizeof(xParCmdAlpha3rdPolyReg),
                    xParCmdAlpha3rdPolyReg_Update);
    xParCmdRegister(XPARCMD_TYPE_APPLYWIND, sizeof(xParCmdApplyWind), xParCmdApplyWind_Update);
    xParCmdRegister(XPARCMD_TYPE_ROTPAR, sizeof(xParCmdRotPar), xParCmdRotPar_Update);
    xParCmdRegister(XPARCMD_TYPE_ROTATEAROUND, sizeof(xParCmdRotateAround),
                    xParCmdRotateAround_Update);
    xParCmdRegister(XPARCMD_TYPE_SMOKEALPHA, sizeof(xParCmdSmokeAlpha), xParCmdSmokeAlpha_Update);
    xParCmdRegister(XPARCMD_TYPE_SCALE, sizeof(xParCmdScale), xParCmdScale_Update);
    xParCmdRegister(XPARCMD_TYPE_COLLIDEFALL, sizeof(xParCmdCollideFall),
                    xParCmdCollideFall_Update);
    xParCmdRegister(XPARCMD_TYPE_COLLIDEFALLSTICKY, sizeof(xParCmdCollideFallSticky),
                    xParCmdCollideFallSticky_Update);
    xParCmdRegister(XPARCMD_TYPE_SHAPER, sizeof(xParCmdShaperData), xParCmd_Shaper_Update);
    xParCmdRegister(XPARCMD_TYPE_ALPHAINOUT, sizeof(xParCmdAlphaInOutData),
                    xParCmd_AlphaInOut_Update);
    xParCmdRegister(XPARCMD_TYPE_SIZEINOUT, sizeof(xParCmdSizeInOutData), xParCmd_SizeInOut_Update);
    xParCmdRegister(XPARCMD_TYPE_DAMPENSPEED, sizeof(xParCmdDampenData),
                    xParCmd_DampenSpeed_Update);
}

//                                      xPad

void xPadKill()
{
    iPadKill();
}

void xPadRumbleEnable(S32 idx, S32 enable)
{
    _tagxPad* p = &mPad[idx];
    if (p->state != ePad_Enabled)
    {
        enable = 0;
    }
    if (enable)
    {
        if (p->flags & ePadInit_EnableRumble4)
        {
            p->flags |= ePadInit_Complete8a;
        }
    }
    else
    {
        if (p->flags & ePadInit_Complete8a)
        {
            p->flags ^= ePadInit_Complete8a;
            //xPadDestroyRumbleChain(mPad + idx);
        }
    }
}

// WIP.
_tagxPad* xPadEnable(S32 idx)
{
    _tagxPad* p = &mPad[idx];
    if (p->state == ePad_Disabled && idx == 0)
    {
        p = iPadEnable(&mPad[0], 0);
        if (p->state == ePad_Enabled || p->state != ePad_Disabled)
        {
            if (p->flags & ePadInit_EnableRumble4)
            {
                p->flags |= ePadInit_Complete8a;
            }
            else if (p->flags & ePadInit_Complete8a)
            {
                p->flags ^= ePadInit_Complete8a;
            }
        }
    }
    return p;
}

S32 xPadInit()
{
    memset(mPad, 0, sizeof(mPad));
    //memset(mRumbleList, 0, sizeof(mRumbleList));
    S32 code = iPadInit();
    if (!code)
    {
        return 0;
    }
    //gPlayerPad = mPad;
    return 1;
}

// WIP.
S32 xPadUpdate(S32 idx, F32 time_passed)
{
    U32 new_on;
    _tagxPad* p = &mPad[idx];
    S32 ret;
    U32 fake_dpad;
    S32 i;

    if (idx == 0)
    {
        if (zScene_ScreenAdjustMode() == 0)
        {
            // not yet implemented
            if (/*!zMenuRunning() &&*/ !zGameIsPaused())
            {
                p->flags &= ~0x10; // unknown flag 0x10
            }
            else
            {
                p->flags |= 0x10;
            }
        }
        else
        {
            p->flags &= ~0x10;
        }

        if (p->al2d_timer >= 0.35f)
            p->al2d_timer = 0.35f;
        if (p->ar2d_timer >= 0.35f)
            p->ar2d_timer = 0.35f;
        if (p->d_timer >= 0.35f)
            p->d_timer = 0.35f;

        fake_dpad = 0;
        ret = iPadUpdate(&mPad[idx], &new_on);

        if (!ret)
        {
            p->pressed = 0;
            p->released = 0;
            return 1;
        }

        if (p->flags & 0x10)
        {
            if (p->flags & ePadInit_WaitStable2)
            {
                if (p->analog1.x < 50)
                    fake_dpad |= XPAD_BUTTON_LEFT;
                else if (p->analog1.x > 49)
                    fake_dpad |= XPAD_BUTTON_RIGHT;

                if (p->analog1.y < 50)
                    fake_dpad |= XPAD_BUTTON_UP;
                else if (p->analog1.y > 49)
                    fake_dpad |= XPAD_BUTTON_DOWN;

                if (fake_dpad == 0)
                {
                    p->al2d_timer = 0.0f;
                }
                else
                {
                    p->al2d_timer -= time_passed;
                    if (p->al2d_timer <= 0.0f)
                    {
                        new_on |= fake_dpad;
                        p->al2d_timer = 0.35f;
                    }
                }
            }

            if (p->flags & ePadInit_EnableAnalog3)
            {
                S32 a2x = p->analog2.x;
                S32 a2y = p->analog2.y;
                if (a2x <= 50 || a2x >= 207 || a2y <= 50 || a2y >= 207)
                {
                    p->ar2d_timer -= time_passed;
                    if (p->ar2d_timer <= 0.0f)
                    {
                        p->ar2d_timer = 0.35f;

                        if (a2x < 50)
                            new_on |= XPAD_BUTTON_LEFT;
                        else if (a2x > 49)
                            new_on |= XPAD_BUTTON_RIGHT;

                        if (a2y < 50)
                            new_on |= XPAD_BUTTON_UP;
                        else if (a2y > 49)
                            new_on |= XPAD_BUTTON_DOWN;
                    }
                }
                else
                {
                    p->ar2d_timer = 0.0f;
                }
            }
        }

        p->pressed = new_on & ~p->on;
        p->released = p->on & ~new_on;
        p->on = new_on;

        for (i = 0; i < 22; i += 2)
        {
            U32 mask0 = 1 << i;
            U32 mask1 = 1 << (i + 1);

            if (p->pressed & mask0)
                p->down_tmr[0] = 0.0f;
            else if (p->released & mask0)
                p->up_tmr[0] = 0.0f;

            if (p->on & mask0)
                p->down_tmr[0] += time_passed;
            else
                p->up_tmr[0] += time_passed;

            if (p->pressed & mask1)
                p->down_tmr[1] = 0.0f;
            else if (p->released & mask1)
                p->up_tmr[1] = 0.0f;

            if (p->on & mask1)
                p->down_tmr[1] += time_passed;
            else
                p->up_tmr[1] += time_passed;

            p = (_tagxPad*)(p->value + 8);
        }

        if (p->flags & 0x10)
        {
            if (!(p->on &
                  (XPAD_BUTTON_UP | XPAD_BUTTON_DOWN | XPAD_BUTTON_LEFT | XPAD_BUTTON_RIGHT)))
            {
                p->d_timer = 0.0f;
            }
            else
            {
                p->d_timer -= time_passed;
                if (p->d_timer <= 0.0f)
                {
                    p->d_timer = 0.35f;

                    if (p->on & XPAD_BUTTON_UP)
                        p->pressed |= XPAD_BUTTON_UP;
                    else if (p->on & XPAD_BUTTON_DOWN)
                        p->pressed |= XPAD_BUTTON_DOWN;

                    if (p->on & XPAD_BUTTON_LEFT)
                        p->pressed |= XPAD_BUTTON_LEFT;
                    else if (p->on & XPAD_BUTTON_RIGHT)
                        p->pressed |= XPAD_BUTTON_RIGHT;
                }
            }
        }

        return 1;
    }

    return 0;
}

void xPadNormalizeAnalog(_tagxPad& pad, S32 inner_zone, S32 outer_zone)
{
    _tagPadAnalog* src = &pad.analog1;

    for (S32 i = 0; i < 2; i++, src++)
    {
        analog_data* data = &pad.analog[i];
        S8 x = src->x;
        F32 normX = 0.0f;

        if (x <= 1)
        {
            if ((x <= -inner_zone) && (-outer_zone <= x))
            {
                normX = F32(x + inner_zone) / F32(outer_zone - inner_zone);
            }
        }
        else if ((inner_zone <= x) && (x <= outer_zone))
        {
            normX = F32(x - inner_zone) / F32(outer_zone - inner_zone);
        }
        data->offset.x = normX;

        S8 y = src->y;
        F32 normY = 0.0f;
        if (y <= 1)
        {
            if ((y <= -inner_zone) && (-outer_zone <= y))
            {
                normY = F32(y + inner_zone) / F32(outer_zone - inner_zone);
            }
        }
        else if ((inner_zone <= y) && (y <= outer_zone))
        {
            normY = F32(y - inner_zone) / F32(outer_zone - inner_zone);
        }
        data->offset.y = normY;

        if ((data->offset.x == 0.0f) && (data->offset.y == 0.0f))
        {
            data->mag = 0.0f;
            data->dir.x = 1.0f;
            data->dir.y = 0.0f;
            data->ang = 0.0f;
        }
        else
        {
            if (data->offset.x > 1.0f)
                data->offset.x = 1.0f;
            else if (data->offset.x < -1.0f)
                data->offset.x = -1.0f;

            if (data->offset.y > 1.0f)
                data->offset.y = 1.0f;
            else if (data->offset.y < -1.0f)
                data->offset.y = -1.0f;

            F32 mag = data->offset.x * data->offset.x + data->offset.y * data->offset.y;

            if (mag >= 0.0f)
            {
                if (!isinf(mag))
                {
                    F32 inv_sqrt = 1.0 / sqrt(mag);
                    F32 val = -(inv_sqrt * inv_sqrt * mag - 3.0f) * inv_sqrt * 0.5f;
                    mag = 100000.0f;
                    if (val > 0.0000099999997f)
                    {
                        mag = 1.0f / val;
                    }
                }
            }

            data->mag = mag;

            F32 inv_mag = 1.0f / data->mag;
            data->dir.x = data->offset.x * inv_mag;
            data->dir.y = data->offset.y * inv_mag;

            data->ang = xAngleClampFast(atan2(data->dir.y, data->dir.x));
        }
    }
}
