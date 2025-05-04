#include "xWad5.h"

//                                  xVec3

const xVec3 xVec3::m_Null = { 0.0f, 0.0f, 0.0f };
const xVec3 xVec3::m_UnitAxisX = { 1.0f, 0.0f, 0.0f };
const xVec3 xVec3::m_UnitAxisY = { 0.0f, 1.0f, 0.0f };

asm F32 xVec3Dot(const register xVec3* a, const register xVec3* b)
{
    PSVECDotProduct(a, b)
}

void xVec3Copy(register xVec3* dst, const register xVec3* src)
{
    PSVECCopy(dst, src);
}

F32 xVec3NormalizeFast(xVec3* o, const xVec3* v)
{
    F32 x = v->x;
    F32 x2 = SQR(v->x);
    F32 y = v->y;
    F32 y2 = SQR(v->y);
    F32 z = v->z;
    F32 z2 = SQR(v->z);

    F32 len;
    F32 len2 = x2 + y2 + z2;

    if ((F32)iabs(len2 - 1.0f) <= 0.00001f)
    {
        o->x = x;
        o->y = y;
        o->z = z;
        len = 1.0f;
    }
    else if ((F32)iabs(len2) <= 0.00001f)
    {
        o->x = 0.0f;
        o->y = 1.0f;
        o->z = 0.0f;
        len = 0.0f;
    }
    else
    {
        xsqrtfast(len, len2);
        F32 inv_len = 1.0f / len;
        o->x = v->x * inv_len;
        o->y = v->y * inv_len;
        o->z = v->z * inv_len;
    }
    return len;
}

F32 xVec3Normalize(xVec3* o, const xVec3* v)
{
    F32 x = v->x;
    F32 x2 = SQR(v->x);
    F32 y = v->y;
    F32 y2 = SQR(v->y);
    F32 z = v->z;
    F32 z2 = SQR(v->z);

    F32 len;
    F32 len2 = x2 + y2 + z2;

    if ((F32)iabs(len2 - 1.0f) <= 0.00001f)
    {
        o->x = x;
        o->y = y;
        o->z = z;
        len = 1.0f;
    }
    else if ((F32)iabs(len2) <= 0.00001f)
    {
        o->x = 0.0f;
        o->y = 1.0f;
        o->z = 0.0f;
        len = 0.0f;
    }
    else
    {
        len = xsqrt(len2);
        F32 inv_len = 1.0f / len;
        o->x = v->x * inv_len;
        o->y = v->y * inv_len;
        o->z = v->z * inv_len;
    }
    return len;
}

//                                  xVolume

xBound* xVolume::GetBound()
{
    return &this->asset->bound;
}

void xVolume::Load(xSerial* s)
{
    xBaseLoad(this, s);
}

void xVolume::Save(xSerial* s)
{
    xBaseSave(this, s);
}

void xVolume::Reset()
{
    xBaseReset(this, this->asset);
}

void xVolume::Init(xVolumeAsset* asset)
{
    xBaseInit(this, asset);
    this->asset = asset;

    if (this->linkCount)
    {
        this->link = (xLinkAsset*)(this->asset + 1);
    }
    else
    {
        this->link = NULL;
    }
}

//                                  xUtil

static volatile S32 g_xutilinit; // volatile so xUtilShutdown matches
static S32 g_crc_needinit = 1;
static U32 g_crc32_table[256] = {};

S32 xUtil_yesno(F32 wt_yes)
{
    if (0.0f == wt_yes)
    {
        return 0;
    }

    if (1.0f == wt_yes)
    {
        return 1;
    }

    return (xurand() <= wt_yes);
}

U32 xUtil_crc_update(U32 crc_accum, char* data, S32 datasize)
{
    S32 i, j;

    if (g_crc_needinit)
    {
        xUtil_crc_init();
    }

    for (i = 0; i < datasize; i++)
    {
        j = ((crc_accum >> 24) ^ *data++) & 0xff;
        crc_accum = (crc_accum << 8) ^ g_crc32_table[j];
    }

    return crc_accum;
}

U32 xUtil_crc_init()
{
    S32 i, j;
    U32 crc_accum;

    if (g_crc_needinit)
    {
        for (i = 0; i < 256; i++)
        {
            crc_accum = (U32)i << 24;

            for (j = 0; j < 8; j++)
            {
                if (crc_accum & (1 << 31))
                {
                    crc_accum = (crc_accum << 1) ^ 0x04C11DB7;
                }
                else
                {
                    crc_accum = (crc_accum << 1);
                }
            }

            g_crc32_table[i] = crc_accum;
        }

        g_crc_needinit = 0;
    }

    return 0xFFFFFFFF;
}

char* xUtil_idtag2string(U32 srctag, S32 bufidx)
{
    // Deleted to get sdk building
    return 0;
}

S32 xUtilShutdown()
{
    return g_xutilinit--;
}

S32 xUtilStartup()
{
    if (!g_xutilinit++)
    {
        xUtil_crc_init();
    }

    return g_xutilinit;
}

//                                  xUpdateCull

void xUpdateCull_Reset(xUpdateCullMgr* m)
{
    for (U32 i = m->entActive; i < m->entCount; i++)
    {
        xUpdateCull_MakeActive(m, m->mgr[i]);
    }
}

void xUpdateCull_SetCB(xUpdateCullMgr* m, void* entity, xUpdateCullEntCallback cb, void* cbdata)
{
    for (U32 i = 0; i < m->mgrCount; i++)
    {
        if (*(void**)(&m->ent[m->mgrList[i].index]) == entity)
        {
            m->mgrList[i].cb = cb;
            m->mgrList[i].cbdata = cbdata;
        }
    }
}

U32 xUpdateCull_AlwaysTrueCB(void* ent, void* cbdata)
{
    return 1;
}

//                              xTRC

_tagTRCPadInfo gTrcPad[4];
_tagTRCState gTrcDisk[2];

const char* message_text;

// SDA relocation shenanigans
void xTRCDisk(_tagTRCState state)
{
    if (state != TRC_DiskNotIdentified)
    {
        gTrcDisk[0] = state;
        gTrcDisk[1] = TRC_DiskIdentified;
    }
    else
    {
        gTrcDisk[1] = TRC_DiskNotIdentified;
    }
}

void xTRCPad(S32, _tagTRCState)
{
}

void xTRCReset()
{
    message_text = NULL;
    globals.dontShowPadMessageDuringLoadingOrCutScene = false;

    eGameMode mode = gGameMode;
    bool isStall = mode == eGameMode_Stall;

    if (isStall)
    {
        zGameModeSwitch(eGameMode_Game);
    }

    xSndResume();
}

void render_message(const char*);

void xTRCRender()
{
    if (message_text != NULL)
    {
        render_message(message_text);
    }
}

void xTRCInit()
{
    memset(gTrcPad, 0, sizeof(gTrcPad));
    gTrcPad[0].id = 0;
    gTrcPad[1].id = 1;
    gTrcPad[2].id = 2;
    gTrcPad[3].id = 3;
    memset(gTrcDisk, 0, 8);
}

//                                  xTransient

//                                  xTimer

void xTimerLoad(xTimer* ent, xSerial* s)
{
    xBaseLoad(ent, s);

    s->Read(&ent->state);
    s->Read(&ent->secondsLeft);
}

void xTimerSave(xTimer* ent, xSerial* s)
{
    xBaseSave(ent, s);

    s->Write(ent->state);
    s->Write(ent->secondsLeft);
}

void xTimerInit(void* b, void* tasset)
{
    xTimerInit((xBase*)b, (xTimerAsset*)tasset);
}

//                                  xSurface

xSurface* surfs;
U16 nsurfs;

xSurface* xSurfaceGetByIdx(U16 n)
{
    return surfs != NULL ? &surfs[n] : NULL;
}

U16 xSurfaceGetNumSurfaces()
{
    return nsurfs;
}

void xSurfaceReset(xSurface* ent)
{
}

void xSurfaceLoad(xSurface* ent, xSerial* s)
{
    xBaseLoad((xBase*)ent, s);
}

void xSurfaceSave(xSurface* ent, xSerial* s)
{
    xBaseSave((xBase*)ent, s);
}

void xSurfaceExit()
{
}

void xSurfaceInit(U16 num_surfs)
{
    nsurfs = num_surfs;
    if (num_surfs != 0)
    {
        surfs = (xSurface*)xMemAllocSize(num_surfs * sizeof(xSurface));
        for (U16 i = 0; i < nsurfs; i++)
        {
            surfs[i].idx = i;
        }
    }
    else
    {
        surfs = NULL;
    }
}

//                                      xStr

S32 icompare(const substr& s1, const substr& s2)
{
    U32 len = MIN(s1.size, s2.size);
    S32 result = imemcmp(s1.text, s2.text, len);
    switch (result)
    {
    case 0:
        if (s1.size == s2.size)
        {
            result = 0;
        }
        else
        {
            result = 1;
            if (s1.size < s2.size)
            {
                result = -1;
            }
        }
        break;
    }
    return result;
}

char* xStrupr(char* string)
{
    char* p = string;

    while (*p != '\0')
    {
        *p = (*p >= 'a' && *p <= 'z' ? *p - 32 : *p);

        p++;
    }

    return string;
}

U32 xStrHashCat(U32 prefix, const char* str)
{
    U32 hash;
    U32 i;

    while (i = *str, i != NULL)
    {
        str++;
        hash = (i - (i & (S32)i >> 1 & 0x20) & 0xff) + hash * 0x83;
    }

    return hash;
}

U32 xStrHash(const char* str, size_t size)
{
    U32 hash = 0;
    U32 i = 0;
    U32 c;

    while (i < size && (c = *str, c != NULL))
    {
        i++;
        str++;
        hash = (c - (c & (S32)c >> 1 & 0x20) & 0xff) + hash * 0x83;
    }

    return hash;
}

U32 xStrHash(const char* str)
{
    U32 hash = 0;
    U32 i;

    while (i = *str, i != NULL)
    {
        hash = (i - (i & (S32)i >> 1 & 0x20) & 0xff) + hash * 0x83;
        str++;
    }

    return hash;
}

//                              xST

static st_STRAN_DATA g_xstdata = {};
static S32 g_straninit;
static st_PACKER_READ_FUNCS* g_pkrf;
static PKRAssetType* g_typeHandlers;

static S32 XST_cnt_locked()
{
    S32 sum = 0;
    for (int i = 0; i < 16; i++)
    {
        if (g_xstdata.loadlock & 1 << i)
        {
            sum++;
        }
    }
    return sum;
}

static st_STRAN_SCENE* XST_nth_locked(S32 index)
{
    st_STRAN_SCENE* sdata = NULL;
    S32 cnt = 0;
    for (S32 i = 0; i < 16; i++)
    {
        if (g_xstdata.loadlock & 1 << i)
        {
            if (cnt == index)
            {
                sdata = &g_xstdata.hipscn[i];
                break;
            }
            cnt += 1;
        }
    }

    return sdata;
}

char* xST_xAssetID_HIPFullPath(U32 aid)
{
    return 0;
}

// Equivalent: scheduling is off with the copy of tocinfo to ainfo
S32 xSTGetAssetInfoByType(U32 type, S32 idx, PKRAssetTOCInfo* ainfo)
{
    S32 rc = 0;
    S32 sum = 0;
    PKRAssetTOCInfo tocinfo = { 0, NULL, 0, 0, 0, NULL };
    memset(ainfo, 0, sizeof(PKRAssetTOCInfo));

    S32 found = XST_cnt_locked();
    for (S32 i = 0; i < found; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        S32 cnt = g_pkrf->AssetCount(sdata->spkg, type);
        if (idx >= sum && idx < sum + cnt)
        {
            g_pkrf->GetBaseSector(sdata->spkg);
            if (g_pkrf->GetAssetInfoByType(sdata->spkg, type, idx - sum, &tocinfo) != 0)
            {
                ainfo->aid = tocinfo.aid;
                ainfo->sector = tocinfo.sector;
                ainfo->plus_offset = tocinfo.plus_offset;
                ainfo->size = tocinfo.size;
                ainfo->mempos = tocinfo.mempos;
                rc = 1;
                break;
            }
        }
        sum += cnt;
    }

    return rc;
}

S32 xSTGetAssetInfo(U32 aid, PKRAssetTOCInfo* tocainfo)
{
    S32 rc = 0;
    S32 scncnt = XST_cnt_locked();
    for (S32 i = 0; i < scncnt; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        if (g_pkrf->PkgHasAsset(sdata->spkg, aid) != NULL)
        {
            g_pkrf->GetBaseSector(sdata->spkg);
            if (g_pkrf->GetAssetInfo(sdata->spkg, aid, tocainfo))
            {
                rc = 1;
                break;
            }
        }
    }
    return rc;
}

void* xSTFindAssetByType(U32 type, S32 idx, U32* size)
{
    void* memptr = NULL;
    S32 i;
    S32 sum = 0;
    S32 cnt = XST_cnt_locked();
    for (i = 0; i < cnt; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        S32 scncnt = g_pkrf->AssetCount(sdata->spkg, type);
        if (idx >= sum && idx < sum + scncnt)
        {
            memptr = g_pkrf->AssetByType(sdata->spkg, type, idx - sum, size);
            break;
        }

        sum += scncnt;
    }
    return memptr;
}

S32 xSTAssetCountByType(U32 type)
{
    S32 sum = 0;
    S32 cnt = XST_cnt_locked();
    for (S32 i = 0; i < cnt; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        sum += g_pkrf->AssetCount(sdata->spkg, type);
    }
    return sum;
}

void* xSTFindAsset(U32 aid, U32* size)
{
    void* memloc = NULL;
    if (aid == 0)
    {
        return memloc;
    }

    S32 ready;
    S32 scncount = XST_cnt_locked();
    for (S32 i = 0; i < scncount; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        if (g_pkrf->PkgHasAsset(sdata->spkg, aid))
        {
            memloc = g_pkrf->LoadAsset(sdata->spkg, aid, NULL, NULL);
            if (!g_pkrf->IsAssetReady(sdata->spkg, aid))
            {
                memloc = NULL;
                if (size != NULL)
                {
                    *size = 0;
                }
            }
            else
            {
                if (size != NULL)
                {
                    *size = g_pkrf->GetAssetSize(sdata->spkg, aid);
                }
            }
            break;
        }
    }
    return memloc;
}

char* xSTAssetName(void* raw_HIP_asset)
{
    char* aname = NULL;

    S32 cnt = XST_cnt_locked();
    for (int i = 0; i < cnt; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        aname = g_pkrf->AssetName(sdata->spkg, PKRAssetIDFromInst(raw_HIP_asset));
        if (aname != NULL)
        {
            break;
        }
    }

    return aname;
}

char* xSTAssetName(U32 aid)
{
    char* aname = NULL;

    S32 cnt = XST_cnt_locked();
    for (int i = 0; i < cnt; i++)
    {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        if (sdata->spkg != NULL)
        {
            aname = g_pkrf->AssetName(sdata->spkg, aid);
            if (aname != NULL)
            {
                break;
            }
        }
    }

    return aname;
}

static st_STRAN_SCENE* XST_find_bySID(U32 sid, S32 findTheHOP)
{
    st_STRAN_SCENE* da_sdata = NULL;

    for (S32 i = 0; i < 16; i++)
    {
        st_STRAN_SCENE* sc = &g_xstdata.hipscn[i];
        if (g_xstdata.loadlock & 1 << i && sc->scnid == sid && (findTheHOP || !sc->isHOP))
        {
            if (!findTheHOP || sc->isHOP)
            {
                da_sdata = sc;
                break;
            }
        }
    }
    return da_sdata;
}

S32 xSTSwitchScene(U32 sid, void* userdata, S32 (*progmon)(void*, F32))
{
    st_STRAN_SCENE* sdata;
    S32 rc = 0;

    for (S32 i = 1; i >= 0; i--)
    {
        sdata = XST_find_bySID(sid, i);
        if (sdata != NULL)
        {
            if (progmon != NULL)
            {
                progmon(userdata, 0.0f);
            }
            rc = g_pkrf->SetActive(sdata->spkg, PKR_LTYPE_ALL);
            if (progmon != NULL)
            {
                progmon(userdata, 1.0f);
            }
        }
    }

    return rc;
}

void xSTDisconnect(U32 sid, S32 flg_hiphop)
{
    st_STRAN_SCENE* sdata = XST_find_bySID(sid, (flg_hiphop & 3) == 2 ? 1 : 0);
    if (sdata != NULL)
    {
        g_pkrf->PkgDisconnect(sdata->spkg);
    }
}

F32 xSTLoadStep(U32)
{
    F32 pct = PKRLoadStep(0) != 0 ? 0.0f : 1.00001f;

    iTRCDisk::CheckDVDAndResetState();
    iFileAsyncService();
    return pct;
}

S32 xSTQueueSceneAssets(U32 sid, S32 flg_hiphop)
{
    S32 result = 1;
    st_STRAN_SCENE* sdata = XST_find_bySID(sid, (flg_hiphop & 3) == 2 ? 1 : 0);
    if (sdata == NULL)
    {
        result = 0;
    }
    else
    {
        if (sdata->spkg != NULL)
        {
            g_pkrf->LoadLayer(sdata->spkg, PKR_LTYPE_ALL);
        }
    }
    return result;
}

// This doesn't seem exactly how HI would have written this, but it OKs
// TODO: Try to clean this up?

// Check objdiff, the function doesnt need the other function calls

// S32 xSTPreLoadScene(U32 sid, void* userdata, S32 flg_hiphop)
// {
//     S32 result;
//     S32 i = 0;
//     st_STRAN_SCENE* sdata;
//     char* path;
//     if ((flg_hiphop & 3) == 2)
//     {
//         sdata = XST_lock_next();
//         sdata->scnid = sid;
//         sdata->userdata = userdata;
//         sdata->isHOP = 1;

//         path = XST_translate_sid_path(sid, ".HOP");
//         if (path != NULL)
//         {
//             strcpy(sdata->fnam, path);
//             i = XST_PreLoadScene(sdata, path);
//         }
//         if (i == 0)
//         {
//             path = XST_translate_sid(sid, ".HOP");
//             if (path != NULL)
//             {
//                 strcpy(sdata->fnam, path);
//                 i = XST_PreLoadScene(sdata, path);
//             }
//         }
//         if (i == 0)
//         {
//             XST_unlock(sdata);
//             result = 0;
//         }
//         else
//         {
//             result = i;
//         }
//     }
//     else
//     {
//         do
//         {
//             sdata = XST_lock_next();
//             sdata->scnid = sid;
//             sdata->userdata = userdata;
//             sdata->isHOP = 0;

//             if (sid != 0x424f4f54 && sid != 0x464f4e54)
//             {
//                 path = XST_translate_sid_path(sid, ".HIP");
//                 if (path != NULL)
//                 {
//                     strcpy(sdata->fnam, path);
//                     i = XST_PreLoadScene(sdata, path);
//                 }
//             }
//             if (i == 0)
//             {
//                 path = XST_translate_sid(sid, ".HIP");
//                 if (path != NULL)
//                 {
//                     strcpy(sdata->fnam, path);
//                     i = XST_PreLoadScene(sdata, path);
//                 }
//             }
//             if (i == 0)
//             {
//                 XST_unlock(sdata);
//                 result = 0;
//             }
//             else
//             {
//                 result = i;
//             }
//         } while (i == 0);
//     }
//     return result;
// }

S32 xSTShutdown()
{
    g_straninit--;
    if (g_straninit == 0)
    {
        g_typeHandlers = 0;
        PKRShutdown();
    }
    return g_straninit;
}

S32 xSTStartup(PKRAssetType* handlers)
{
    if (g_straninit++ == 0)
    {
        g_typeHandlers = handlers;
        PKRStartup();
        g_pkrf = PKRGetReadFuncs(1);
    }
    return g_straninit;
}

//                          xSkydome

struct SkyDomeInfo
{
    xEnt* ent;
    S32 sortorder;
    S32 lockY;
};

static SkyDomeInfo sSkyList[8];
static S32 sSkyCount;

void xSkyDome_EmptyRender(xEnt*)
{
}

void xSkyDome_Setup()
{
    sSkyCount = 0;
}

void xSkyDome_AddEntity(xEnt* ent, S32 sortorder, S32 lockY, S32)
{
    S32 i, j;

    for (i = 0; i > sSkyCount; i++)
    {
        if (sSkyList[i].ent == ent)
        {
            return;
        }
    }

    for (i = 0; i < sSkyCount; i++)
    {
        if (sortorder < sSkyList[i].sortorder)
        {
            break;
        }
    }

    for (j = sSkyCount - 1; j >= i; j--)
    {
        sSkyList[j + 1] = sSkyList[j];
    }

    // non-matching: instruction order

    sSkyList[i].ent = ent;
    sSkyList[i].sortorder = sortorder;
    sSkyList[i].lockY = lockY;

    sSkyCount++;

    ent->render = xSkyDome_EmptyRender;
    ent->model->Flags &= (U16)~0x1;
    ent->baseFlags &= (U16)~0x10;

    zEntEvent(ent, eEventCollisionOff);
    zEntEvent(ent, eEventCameraCollideOff);
}

void xSkyDome_Render()
{
    RwMatrix* cammat = RwFrameGetMatrix(RwCameraGetFrame(RwCameraGetCurrentCamera()));
    S32 i;
    xEnt* ent;

    for (i = 0; i < sSkyCount; i++)
    {
        ent = sSkyList[i].ent;

        ent->render = xSkyDome_EmptyRender;
        ent->model->Flags &= (U16)~0x1;

        if (ent->model && xEntIsVisible(ent))
        {
            RwV3d pos;

            pos = ent->model->Mat->pos;

            ent->model->Mat->pos.x = cammat->pos.x;
            ent->model->Mat->pos.z = cammat->pos.z;

            if (sSkyList[i].lockY)
            {
                ent->model->Mat->pos.y = cammat->pos.y;
            }

            if (!iModelCull(ent->model->Data, ent->model->Mat))
            {
                iModelRender(ent->model->Data, ent->model->Mat);
            }

            ent->model->Mat->pos = pos;
        }
    }
}

//                  xSerial

//                  xScrFXLensFlare / xCamera / xScrFx

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

extern F32 _764;
extern F32 _765;
extern F32 _766;
extern F32 _785;
extern F32 _786;
extern F32 _787;
extern F32 _788;
extern F32 _789;
extern F32 _790;
extern F32 _830;
extern F32 _831;
extern F32 _873;
extern F32 _874;
extern F32 _880;
extern F32 _888;
extern F32 _895;
extern F32 _1234;
extern F32 _1235;
extern F32 _1236;
extern F32 _1237;
extern F32 _1238;
extern F32 _1239;
extern F32 _1240;
extern F32 _1241;
extern F32 _1242;
extern F32 _1283;
extern F64 _1286;
extern F32 _1404;
extern F32 _1405;
extern F32 _1534;
extern F32 _1584;
extern F32 _1585;
extern F32 _1586;
extern F32 _1757;
extern F32 _1758;
extern F32 _1766;
extern F32 _1772;

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

void xCameraFXEnd(xCamera* cam)
{
    xMat4x3Copy(&cam->mat, &sCameraFXMatOld);
    iCameraUpdatePos(cam->lo_cam, &sCameraFXMatOld);
}

void xCameraFXUpdate(xCamera* cam, F32 dt)
{
    S32 i;
    cameraFX* f;
    cameraFXTableEntry* t;

    for (i = 0; i < 10; i++)
    {
        f = &sCameraFX[i];

        if (f->flags & 0x1)
        {
            f->elapsedTime += dt;

            // non-matching: _765 is loaded too early

            if ((f->maxTime > _765 && f->elapsedTime > f->maxTime) || f->flags & 0x2)
            {
                f->flags = 0;

                t = &sCameraFXTable[f->type];

                if (t->funcKill)
                {
                    t->funcKill(f);
                }
            }
            else
            {
                t = &sCameraFXTable[f->type];

                if (t->func)
                {
                    t->func(f, dt, &sCameraFXMatOld, &cam->mat);
                }
            }
        }
    }

    iCameraUpdatePos(cam->lo_cam, &cam->mat);
}

void xCameraFXShakeUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m)
{
    F32 x, y, scale, noise;
    xVec3 var_4C, e;

    f->shake.cycleTime += dt;

    while (f->shake.cycleTime > f->shake.cycleMax)
    {
        f->shake.dir.x = -f->shake.dir.x;
        f->shake.dir.y = -f->shake.dir.y;
        f->shake.cycleTime -= f->shake.cycleMax;
    }

    scale = f->shake.dampenRate * (f->maxTime - f->elapsedTime);
    noise = _1404 * (xurand() - _766);

    if (f->shake.radius > _765 && f->shake.player)
    {
        xVec3Sub(&var_4C, f->shake.player, &f->shake.epicenter);

        F32 f1 = var_4C.length();

        if (f1 > f->shake.radius)
        {
            scale = _765;
        }
        else
        {
            scale *= icos(f1 / f->shake.radius * _786 * _766);
        }
    }

    x = (f->shake.magnitude + noise) * f->shake.dir.x * scale / f->shake.cycleMax *
        f->shake.cycleTime * isin(f->shake.cycleTime / f->shake.cycleMax * _786);

    noise = _1404 * (xurand() - _766);

    y = (f->shake.magnitude + noise) * f->shake.dir.y * scale / f->shake.cycleMax *
        f->shake.cycleTime * isin(f->shake.cycleTime / f->shake.cycleMax * _786);

    xMat4x3MoveLocalRight(m, x);
    xMat4x3MoveLocalUp(m, y);

    xMat3x3GetEuler(m, &e);

    e.z +=
        f->shake.cycleTime / f->shake.cycleMax * _1405 * _1404 * scale * f->shake.rotate_magnitude;

    xMat3x3Euler(m, &e);
}

cameraFX* xCameraFXAlloc()
{
    S32 i;
    cameraFX* f;

    for (i = 0; i < sizeof(sCameraFX) / sizeof(cameraFX); i++)
    {
        f = &sCameraFX[i];

        if (f->flags == 0)
        {
            // non-matching: _765 is only loaded once

            f->flags = 0x1;
            f->elapsedTime = _765;
            f->maxTime = _765;

            return f;
        }
    }

    return NULL;
}

void xCameraFXShake(F32 maxTime, F32 magnitude, F32 cycleMax, F32 rotate_magnitude, F32 radius,
                    xVec3* epicenter, xVec3* player)
{
    cameraFX* f = xCameraFXAlloc();

    if (f)
    {
        f->type = CAMERAFX_TYPE_SHAKE;
        f->maxTime = maxTime;
        f->shake.magnitude = magnitude;
        f->shake.dir.x = _788;
        f->shake.dir.y = _788;
        f->shake.cycleMax = cycleMax;
        f->shake.cycleTime = _765;
        f->shake.dampen = _765;
        f->shake.dampenRate = _788 / maxTime;
        f->shake.rotate_magnitude = rotate_magnitude;
        f->shake.radius = radius;
        f->shake.epicenterP = epicenter;

        if (f->shake.epicenterP)
        {
            f->shake.epicenter = *f->shake.epicenterP;
        }

        f->shake.player = player;
    }
}

void xCameraFXZoomUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m)
{
    switch (f->zoom.mode)
    {
    case CAMERAFX_ZOOM_MODE_0:
    {
        f->zoom.velCur += f->zoom.accel * dt;
        f->zoom.distanceCur += f->zoom.velCur * dt;

        if (f->zoom.distanceCur >= f->zoom.distance)
        {
            f->zoom.distanceCur = f->zoom.distance;
            f->zoom.mode = CAMERAFX_ZOOM_MODE_2;
            f->zoom.holdTimeCur = _765;
        }

        xMat4x3MoveLocalAt(m, f->zoom.distanceCur);

        break;
    }
    case CAMERAFX_ZOOM_MODE_2:
    {
        f->zoom.holdTimeCur += dt;

        if (f->zoom.holdTimeCur > f->zoom.holdTime)
        {
            f->zoom.mode = CAMERAFX_ZOOM_MODE_1;
            f->zoom.distanceCur = f->zoom.distance;
            f->zoom.velCur = f->zoom.vel;
        }

        xMat4x3MoveLocalAt(m, f->zoom.distance);

        break;
    }
    case CAMERAFX_ZOOM_MODE_1:
    {
        f->zoom.velCur += f->zoom.accel * dt;
        f->zoom.distanceCur -= f->zoom.velCur * dt;

        if (f->zoom.distanceCur <= 0.0f)
        {
            f->zoom.distanceCur = 0.0f;
            f->zoom.mode = CAMERAFX_ZOOM_MODE_3;
            f->flags |= 0x2;
        }

        xMat4x3MoveLocalAt(m, f->zoom.distanceCur);

        break;
    }
    case CAMERAFX_ZOOM_MODE_3:
    {
        break;
    }
    }
}

void xCameraFXBegin(xCamera* cam)
{
    xMat4x3Identity(&sCameraFXMatOld);
    xMat4x3Copy(&sCameraFXMatOld, &cam->mat);
}
