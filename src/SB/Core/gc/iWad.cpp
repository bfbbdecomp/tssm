#include "iWad.h"

static iTime sStartupTime;
static F32 sGameTime;
extern U32 add;
extern U32 size;
extern OSHeapHandle the_heap;
extern U16 last_error;
extern OSContext* last_context;
static st_ISGSESSION g_isgdata_MAIN = { 0 };

//                                                                          iTime

void iFuncProfileFuncs(int, int, float)
{
}

void iFuncProfileDump()
{
}

void iProfileClear(U32 sceneID)
{
}

void iTimeSetGame(F32 time)
{
    sGameTime = time;
}

void iTimeGameAdvance(F32 elapsed)
{
    sGameTime += elapsed;
}

F32 iTimeGetGame()
{
    return sGameTime;
}

F32 iTimeDiffSec(iTime time, iTime t1)
{
    time = t1 - time;
    return (F32)time / (GET_BUS_FREQUENCY() / 4);
}

F32 iTimeDiffSec(iTime time)
{
    return (F32)time / (GET_BUS_FREQUENCY() / 4);
}

iTime iTimeGet()
{
    return OSGetTime() - sStartupTime;
}

// WIP.
char* iGetCurrFormattedTime(char* input)
{
    OSTime ticks = OSGetTime();
    OSCalendarTime td;
    OSTicksToCalendarTime(ticks, &td);
    bool pm = false;
    // STUFF.
    char* ret = input;
    // STUFF.
    if (pm)
    {
        ret[8] = 'P';
        ret[9] = '.';
        ret[10] = 'M';
        ret[11] = '.';
    }
    else
    {
        ret[8] = 'A';
        ret[9] = '.';
        ret[10] = 'M';
        ret[11] = '.';
    }
    ret[12] = '\0';
    return ret + (0xd - (S32)input);
}

// Template for future use. TODO
char* iGetCurrFormattedDate(char* input)
{
    return NULL;
}

S32 iGetMonth()
{
    OSTime ticks = OSGetTime();
    OSCalendarTime td;
    OSTicksToCalendarTime(ticks, &td);
    return td.mon + 1;
}

S32 iGetDay()
{
    OSTime ticks = OSGetTime();
    OSCalendarTime td;
    OSTicksToCalendarTime(ticks, &td);
    return td.mday;
}

S32 iGetHour()
{
    OSTime ticks = OSGetTime();
    OSCalendarTime td;
    OSTicksToCalendarTime(ticks, &td);
    return td.hour;
}

S32 iGetMinute()
{
    OSTime ticks = OSGetTime();
    OSCalendarTime td;
    OSTicksToCalendarTime(ticks, &td);
    return td.min;
}

extern "C" {

void* malloc(U32 __size)
{
    if ((S32)__size <= 0)
    {
        return NULL;
    }

    iMemInit();

    if (__size == NULL)
    {
        OSAllocFromHeap(the_heap, __size);
    }

    return 0;
}

void free(void* __ptr)
{
    if (__ptr != NULL)
    {
        OSFreeToHeap(the_heap, __ptr);
    }
}
}

S32 RenderWareExit()
{
    RwEngineStop();
    RwEngineClose();
    return RwEngineTerm();
}

void iSystemExitRWFailSafe()
{
    RwEngineStop();
    RwEngineClose();
}

void iSystemExit() // TO-DO - Need to update function calls
{
    xDebugExit();
    xMathExit();
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();
    xSndMgrExit();
    xPadKill();
    xMemExit();
    OSPanic("GGVE", 0x1e7,
            "(With apologies to Jim Morrison) This the end, my only friend, The End.");
}

void MemoryProtectionErrorHandler(U16 last, OSContext* ctx, U64 unk1, U64 unk2)
{
    last_error = last;
    last_context = ctx;
}

U16 my_dsc(U16 dsc)
{
    return dsc;
}

void iVSync()
{
    VIWaitForRetrace();
}

//                                                                               iSnd

void iSndSuspendCD(U32)
{
}

void iSndSetExternalCallback(void* func_ptr)
{
}

void iSndExit()
{
}

// void iSndStartStereo(iSndHandle id1, iSndHandle id2)
// {
// }

// void iSndExit()
// {
//     FSOUND_Close();
// }

//                                                                              iSaveGame

// Not 100% on what this does or if it's correctly defined for all cases. Seems to be used for allocation alignment
#define ALIGN_THING(x, n) (n + x - 1 & -x)

// name is a total guess for now
struct st_ISG_TPL_TEX
{
    struct UnkIn
    {
        U32 unk_0;
        U32 unk_4;
        void* unk_8;
        U32 unk_c;
        U32 unk_10[4];
        U8 unk_20;
        U8 unk_21;
        U8 unk_22;
        U8 unk_23;
    };
    struct UnkOut
    {
        U8 unk_0;
        U8 unk_1;
        U8 unk_2;
        U8 unk_3;
        U32 unk_4;
        void* unk_8;
    };

    UnkIn* unk_0;
    UnkOut* unk_4;
};

// WIP. Looks like some sort of header for a file with embedded texture information.
struct st_ISG_TPL_TEXPALETTE
{
    U32 magic;
    U32 count;
    st_ISG_TPL_TEX* unk_8;
};

struct IconData
{
    char game_name[0x20];
    char buf0[0x20];
    char buf1[0x1800];
    char buf2[8][0x800];
    char pad[0x200];
    char footer[0x200];
};

static volatile S32 g_isginit;
static S32 g_legalSectSize[] = { 0x2000, 0, -1 };
static st_ISG_TPL_TEXPALETTE* g_rawicon;
static st_ISG_TPL_TEXPALETTE* g_rawbanr;
static U32 g_iconsize;
static U32 g_banrsize;
static U8 isMounted;

S32 iSGAutoSave_Monitor(st_ISGSESSION* isg, S32 idx_target)
{
    if (isg == NULL)
    {
        return 0;
    }

    U32 ret = iSGTgtState(isg, idx_target, NULL);
    if (ret == 0 || (ret & 1) == 0)
    {
        globals.autoSaveFeature = 0;
        return 0;
    }
    return 1;
}

void iSGAutoSave_Startup()
{
}

U8 iSGCheckMemoryCard(st_ISGSESSION* isgdata, S32 index)
{
    s32 memSize;
    s32 sectorSize;

    switch (CARDProbeEx(index, &memSize, &sectorSize))
    {
    case CARD_RESULT_READY:
        return 1;
    }
    return 0;
}

static S32 iSG_curKosher(CARDStat* stat, CARDFileInfo* info) // FIX
{
    S32 rc = 1;

    if ((stat->iconAddr < 1 || stat->iconAddr > 0x7fffffff) &&
        (stat->commentAddr < 1 || stat->commentAddr > 0x7fffffff))
    {
        rc = 0;
    }
    else
    {
        char stuff[0x200] = { 0 };
        sprintf(stuff, "SPONGEBOB:WHENROBOTSATTACK::RyanNeilDan");
        void* alloc = xMemPushTemp(0x5e00 + 0x1f);
        // align buf address to 32 bytes
        char* buf = (char*)((U32)alloc + 0x1f & ~0x1f);

        S32 result;
        do
        {
            result = CARDRead(info, buf, 0x5e00, 0);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            if (memcmp(buf + 0x5a40, stuff, strlen(stuff) - 1))
            {
                rc = 0;
            }

            if (memcmp(buf, "Battle for Bikini Bottom", strlen("Battle for Bikini Bottom") - 1))
            {
                rc = 0;
            }
        }

        xMemPopTemp(alloc);
    }

    return rc;
}

static S32 iSG_mc_fdel(st_ISG_MEMCARD_DATA* mcdata, const char* fname)
{
    if (mcdata->chan < 0 || mcdata->chan > 1)
    {
        return 0;
    }

    if (isMounted == 0)
    {
        return 0;
    }

    s32 result;
    do
    {
        result = CARDDelete(mcdata->chan, fname);
    } while (result == CARD_RESULT_BUSY);

    S32 ret;
    if (result == CARD_RESULT_NOFILE)
    {
        ret = 1;
    }
    else if (result == CARD_RESULT_READY)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
        switch (result)
        {
        case CARD_RESULT_IOERROR:
            mcdata->unk_12c = 1;
            break;
        case CARD_RESULT_NOPERM:
        case CARD_RESULT_FATAL_ERROR:
        case CARD_RESULT_WRONGDEVICE:
            break;
        }
    }
    return ret;
}

static S32 iSG_mc_fclose(st_ISG_MEMCARD_DATA* mcdata, CARDStat* stat)
{
    S32 ret = 0;
    s32 result;
    do
    {
        result = CARDClose(&mcdata->finfo);
    } while (result == CARD_RESULT_BUSY);

    if (result == CARD_RESULT_READY)
    {
        ret = 1;
    }

    if (stat != NULL)
    {
        memcpy(stat, &mcdata->fstat, sizeof(CARDStat));
    }

    memset(&mcdata->finfo, 0, sizeof(CARDFileInfo));
    memset(&mcdata->fstat, 0, sizeof(CARDStat));

    return ret;
}

static S32 iSG_mc_fclose(st_ISG_MEMCARD_DATA* mcdata)
{
    return iSG_mc_fclose(mcdata, NULL);
}

static S32 iSG_mc_fopen(st_ISG_MEMCARD_DATA* mcdata, const char* fname, S32 fsize,
                        en_ISG_IOMODE mode, en_ASYNC_OPERR* operr)
{
    S32 ret = 0;
    CARDFileInfo* finfo = &mcdata->finfo;
    CARDStat* stat = &mcdata->fstat;
    if (operr != NULL)
    {
        *operr = ISG_OPERR_NONE;
    }

    if (mode == ISG_IOMODE_READ)
    {
        s32 result;
        do
        {
            result = CARDOpen(mcdata->chan, (char*)fname, finfo);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            ret = 1;
        }
    }
    else
    {
        S32 res = iSG_mc_fdel(mcdata, fname);
        if (mcdata->unk_12c == 0 && res > 0)
        {
            s32 result;
            do
            {
                result = CARDCreate(mcdata->chan, (char*)fname, fsize, finfo);
            } while (result == CARD_RESULT_BUSY);

            if (result == CARD_RESULT_READY)
            {
                ret = 1;
                mcdata->unk_98 = 0;
            }
            else if (operr != NULL)
            {
                switch (result)
                {
                case CARD_RESULT_WRONGDEVICE:
                case CARD_RESULT_NOCARD:
                    *operr = ISG_OPERR_NOCARD;
                    break;
                case CARD_RESULT_IOERROR:
                case CARD_RESULT_BROKEN:
                    mcdata->unk_12c = 1;
                    *operr = ISG_OPERR_DAMAGE;
                    break;
                case CARD_RESULT_NOENT:
                case CARD_RESULT_INSSPACE:
                    *operr = ISG_OPERR_NOROOM;
                    break;
                default:
                    *operr = ISG_OPERR_OTHER;
                    break;
                case CARD_RESULT_READY:
                case CARD_RESULT_BUSY:
                case CARD_RESULT_EXIST:
                case CARD_RESULT_NOPERM:
                    break;
                }
            }
        }
    }

    if (ret != 0)
    {
        s32 result;
        do
        {
            result = CARDGetStatus(mcdata->chan, finfo->fileNo, stat);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            ret = 1;
        }
    }

    if (ret != 0)
    {
        if (memcmp(stat->gameName, "GQPE", 4) != 0 || memcmp(stat->company, "78", 2) != 0)
        {
            iSG_mc_fclose(mcdata);
            ret = 0;
        }
    }

    return ret;
}

static S32 iSG_fileKosher(st_ISG_MEMCARD_DATA* mcdata, const char* param2, int param3, int* param4)
{
    S32 rc = 0;
    en_ASYNC_OPERR operr = ISG_OPERR_NONE;

    if (param4)
    {
        *param4 = 0;
    }

    if (iSG_mc_fopen(mcdata, param2, -1, ISG_IOMODE_READ, &operr) == 0)
    {
        return -1;
    }

    S32 ret = iSG_curKosher(&mcdata->fstat, &mcdata->finfo);
    iSG_mc_fclose(mcdata);

    if (ret == 0)
    {
        rc = 0;
        if (param3 && iSG_mc_fdel(mcdata, param2) && param4)
        {
            *param4 = 1;
        }
    }
    else
    {
        rc = 1;
    }
    return rc;
}

S32 iSGCheckForCorruptFiles(st_ISGSESSION* isgdata, char files[][64])
{
    if (isgdata->slot < 0)
    {
        return 0;
    }

    S32 i;
    char* name;
    st_ISG_MEMCARD_DATA* mcdata = &isgdata->mcdata[isgdata->slot];
    S32 ret = 0;
    memset(files, NULL, 0xc0);

    for (i = 0; i < ISG_NUM_FILES; ++i)
    {
        name = iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, i);
        if (iSG_fileKosher(mcdata, name, 0, NULL) == 0)
        {
            strcpy(files[ret], name);
            ret++;
        }
    }
    return ret;
}

S32 iSGCheckForWrongDevice()
{
    char* workarea = (char*)RwMalloc(0x10000 - 0x6000);

    S32 ret = -1;
    for (S32 i = 0; i < ISG_NUM_SLOTS; ++i)
    {
        s32 result = CARDMount(i, workarea, NULL);

        switch (result)
        {
        case CARD_RESULT_READY:
        case CARD_RESULT_BROKEN:
        case CARD_RESULT_ENCODING:
            CARDUnmount(i);
            break;
        case CARD_RESULT_WRONGDEVICE:
            ret = i;
            i = ISG_NUM_SLOTS;
            break;
        }
    }

    RwFree(workarea);

    return ret;
}

static void iSG_cb_asyndone(s32, s32)
{
}

static S32 iSG_mcqa_fwrite(st_ISG_MEMCARD_DATA* mcdata, char* buf, S32 len)
{
    S32 ret = 0;
    S32 x = 1000;
    CARDStat* fstat = &mcdata->fstat;
    s32 result;
    iTRCDisk::TRCCheck();
    do
    {
        result = CARDGetResultCode(mcdata->chan);
    } while (result == CARD_RESULT_BUSY);

    if (result != CARD_RESULT_READY)
    {
        return 0;
    }

    CARDGetXferredBytes(mcdata->chan);
    s32 asynresult;
    do
    {
        asynresult = CARDWriteAsync(&mcdata->finfo, buf, len, mcdata->unk_98, iSG_cb_asyndone);
    } while (asynresult == CARD_RESULT_BUSY);

    if (asynresult != CARD_RESULT_READY)
    {
        return 0;
    }

    result = CARD_RESULT_BUSY;
    iTRCDisk::TRCCheck();
    do
    {
        if (x++ > 500)
        {
            result = CARDGetResultCode(mcdata->chan);
            xSndMgrUpdate();
            CARDGetXferredBytes(mcdata->chan);
            x = 0;
        }
        iTRCDisk::TRCCheck();
    } while (result == CARD_RESULT_BUSY);

    if (asynresult == CARD_RESULT_READY)
    {
        mcdata->unk_98 += len;

        do
        {
            result = CARDGetStatus(mcdata->chan, mcdata->finfo.fileNo, fstat);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            ret = 1;
        }
    }
    return ret;
}

static S32 iSG_mcqa_fread(st_ISG_MEMCARD_DATA* mcdata, char* buf, S32 len, S32 offset)
{
    S32 x = 1000;
    s32 result = CARDGetResultCode(mcdata->chan);
    do
    {
        result = CARDGetResultCode(mcdata->chan);
    } while (result == CARD_RESULT_BUSY);

    if (result != CARD_RESULT_READY)
    {
        return 0;
    }

    CARDGetXferredBytes(mcdata->chan);
    do
    {
        result = CARDReadAsync(&mcdata->finfo, buf, len, offset, iSG_cb_asyndone);
    } while (result == CARD_RESULT_BUSY);

    if (result != CARD_RESULT_READY)
    {
        return 0;
    }

    result = CARD_RESULT_BUSY;
    do
    {
        if (x++ > 500)
        {
            xSndMgrUpdate();
            CARDGetXferredBytes(mcdata->chan);
            result = CARDGetResultCode(mcdata->chan);
            x = 0;
        }
        iTRCDisk::TRCCheck();
    } while (result == CARD_RESULT_BUSY);
    return result != 0 ? 0 : 1;
}

static S32 iSG_mc_format(st_ISG_MEMCARD_DATA* mcdata, S32, S32* canRecover)
{
    if (mcdata->unk_12c != 0)
    {
        if (canRecover != NULL)
        {
            *canRecover = 0;
        }
        return 0;
    }

    if (canRecover != NULL)
    {
        *canRecover = 1;
    }

    s32 result;
    do
    {
        result = CARDFormat(mcdata->chan);
    } while (result == CARD_RESULT_BUSY);

    S32 ret;
    if (result == CARD_RESULT_READY)
    {
        ret = 1;
    }
    else
    {
        ret = 0;

        switch (result)
        {
        case CARD_RESULT_IOERROR:
            mcdata->unk_12c = 1;
            if (canRecover != NULL)
            {
                *canRecover = 0;
            }
            break;
        case CARD_RESULT_NOCARD:
            if (canRecover != NULL)
            {
                *canRecover = -1;
            }
            ret = -1;
            break;
        case CARD_RESULT_FATAL_ERROR:
        case CARD_RESULT_WRONGDEVICE:
        case CARD_RESULT_NOFILE:
            break;
        }
    }
    return ret;
}

static void iSG_cb_unmount(s32 chan, s32 result)
{
    st_ISGSESSION* session = &g_isgdata_MAIN;
    if (chan != session->slot)
    {
        return;
    }
    session->slot = -1;
    if (session->chgfunc != NULL)
    {
        session->chgfunc(session->cltdata, ISG_CHG_TARGET);
    }
    memset(&session->mcdata[chan], 0, sizeof(st_ISG_MEMCARD_DATA));
}

static S32 iSG_tpl_unpack(st_ISG_TPL_TEXPALETTE* tpl)
{
    if (tpl->magic != 0x20af30)
    {
        return 0;
    }

    tpl->unk_8 = (st_ISG_TPL_TEX*)((U32)tpl + (U32)tpl->unk_8);
    for (S32 i = 0; i < tpl->count; ++i)
    {
        st_ISG_TPL_TEX* x = &tpl->unk_8[i];
        if (x->unk_0 != NULL)
        {
            x->unk_0 = (st_ISG_TPL_TEX::UnkIn*)((U32)tpl + (U32)x->unk_0);
            if (x->unk_0->unk_23 == 0)
            {
                x->unk_0->unk_8 = (void*)((U32)tpl + (U32)x->unk_0->unk_8);
                x->unk_0->unk_23 = 1;
            }
        }
        if (x->unk_4 != NULL)
        {
            x->unk_4 = (st_ISG_TPL_TEX::UnkOut*)((U32)tpl + (U32)x->unk_4);
            if (x->unk_4->unk_2 == 0)
            {
                x->unk_4->unk_8 = tpl;
                x->unk_4->unk_2 = 1;
            }
        }
    }
    return 1;
}

static st_ISG_TPL_TEX* iSG_tpl_TEXGet(st_ISG_TPL_TEXPALETTE* tpl, U32 n)
{
    return &tpl->unk_8[n];
}

static char* iSG_bfr_icondata(char* param1, CARDStat* stat, char* param3, int param4)
{
    IconData data = { 0 };
    static st_ISG_TPL_TEXPALETTE* ico_pal;
    static st_ISG_TPL_TEX* ico_desc;
    static U32 i = 0;

    sprintf(data.footer, "SPONGEBOB:WHENROBOTSATTACK::RyanNeilDan");
    strncpy(data.game_name, "Battle for Bikini Bottom", sizeof(data.game_name));
    data.game_name[0x1f] = NULL;

    if (param3)
    {
        strncpy(data.buf0, param3, sizeof(data.buf0));
    }
    data.buf0[0x1f] = NULL;

    ico_pal = g_rawbanr;
    ico_desc = iSG_tpl_TEXGet(ico_pal, 0);
    memcpy(data.buf1, ico_desc->unk_0->unk_8, sizeof(data.buf1));

    ico_pal = g_rawicon;
    for (i = 0; i < 8; i++)
    {
        ico_desc = iSG_tpl_TEXGet(ico_pal, i);
        memcpy(data.buf2[i], ico_desc->unk_0->unk_8, sizeof(data.buf2[i]));
    }

    memcpy(param1, &data, sizeof(data));

    S32 t = ALIGN_THING(param4, 0x200);
    return param1 + (t + (sizeof(IconData) - 1) & -t);
}

static S32 iSG_bnr_unpack(st_ISG_TPL_TEXPALETTE* tpl)
{
    return iSG_tpl_unpack(tpl);
}

#pragma inline_depth(0)
static S32 iSG_load_icondata()
{
    g_rawicon = (st_ISG_TPL_TEXPALETTE*)iFileLoad("/SBGCIcon.tpl", NULL, &g_iconsize);
    g_rawbanr = (st_ISG_TPL_TEXPALETTE*)iFileLoad("/SBGCBanner.tpl", NULL, &g_banrsize);
    iSG_tpl_unpack(g_rawicon);
    iSG_tpl_unpack(g_rawbanr);

    return g_rawicon && (S32)g_iconsize && g_rawbanr && (S32)g_banrsize ? 1 : 0;
}
#pragma inline_depth(5)

static S32 iSG_get_finfo(st_ISG_MEMCARD_DATA* mcdata, const char* dpath)
{
    S32 rc = 0;
    en_ASYNC_OPERR operr = ISG_OPERR_NONE;

    if (iSG_mc_fopen(mcdata, dpath, -1, ISG_IOMODE_READ, &operr))
    {
        rc = 1;
        memcpy(&mcdata->unk_b0, &mcdata->fstat, sizeof(CARDStat));
        memcpy(&mcdata->unk_9c, &mcdata->finfo, sizeof(CARDFileInfo));
        iSG_mc_fclose(mcdata);
    }

    return rc;
}

static S32 iSG_cubeicon_size(S32 slot, S32 param2)
{
    if ((U32)slot > 1)
    {
        return -1;
    }

    S32 t = ALIGN_THING(param2, 0x200);

    // FIXME: Macro not quite right
    // return ALIGN_THING(t, sizeof(IconData));
    return -t & sizeof(IconData) + t - 1;
}

static S32 iSG_isSpaceForFile(st_ISG_MEMCARD_DATA* mcdata, S32 param2, const char* param3,
                              S32* param4, S32* param5, S32* param6)
{
    S32 rc = 0;
    S32 result = 0;
    s32 byteNotUsed = 0;
    s32 filesNotUsed = 0;
    S32 len;

    if (mcdata->unk_0 == 0)
    {
        return 0;
    }
    len = iSG_cubeicon_size(mcdata->chan, mcdata->sectorSize);
    len = len + param2;
    len = ALIGN_THING(mcdata->sectorSize, len);

    do
    {
        result = CARDFreeBlocks(mcdata->chan, &byteNotUsed, &filesNotUsed);
    } while (result == CARD_RESULT_BUSY);

    if (result == CARD_RESULT_READY)
    {
        if (param5)
        {
            *param5 = byteNotUsed / mcdata->sectorSize;
        }
        if (param4)
        {
            *param4 = len / mcdata->sectorSize;
        }
    }
    else
    {
        return 0;
    }

    if (param6)
    {
        *param6 = 1;
    }

    if (iSG_get_finfo(mcdata, param3))
    {
        if (param6)
        {
            *param6 = *param6 - 1 & ~(*param6 - 1 >> 0x1f); // FIXME: Fakematch
        }
        if (len <= mcdata->unk_b0.length + byteNotUsed)
        {
            rc = 1;
        }
    }
    else
    {
        if (len <= byteNotUsed && filesNotUsed > 0)
        {
            rc = 1;
        }
    }

    return rc;
}

static S32 iSG_mc_isGCcard(st_ISG_MEMCARD_DATA* mcdata, int* param2, int* param3)
{
    S32 result = 0;
    S32 rc = 0;

    s32 xferBytes = 0;
    u16 encoding = 0;
    s32 memSize = 0;
    s32 sectorSize = 0;

    if (param2)
    {
        *param2 = 0;
    }

    if (param3)
    {
        *param3 = 0;
    }

    if (mcdata->unk_0 == 0)
    {
        return 0;
    }
    if (mcdata->unk_12c)
    {
        return 0;
    }

    do
    {
        result = CARDProbeEx(mcdata->chan, &memSize, &sectorSize);
    } while (result == CARD_RESULT_BUSY);

    if (result == CARD_RESULT_READY)
    {
        rc = 1;
    }

    if (rc != 0)
    {
        do
        {
            result = CARDCheckEx(mcdata->chan, &xferBytes);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            rc = 1;
        }
        else if (result == CARD_RESULT_BROKEN)
        {
            rc = 2;
        }
        else
        {
            if (result == CARD_RESULT_ENCODING)
            {
                rc = 3;
                if (param2)
                {
                    *param2 = 1;
                }
            }
            else
            {
                rc = 0;
                if (result == CARD_RESULT_ENCODING && param2)
                {
                    *param2 = 1;
                }
                if (result == CARD_RESULT_IOERROR && param3)
                {
                    *param3 = 1;
                }
            }
        }
    }

    if (rc == 1)
    {
        do
        {
            result = CARDGetEncoding(mcdata->chan, &encoding);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY && encoding && param2)
        {
            *param2 = 1;
        }
    }

    return rc;
}

// Looks equivalent. Can't get variable initializtions to match.
S32 iSG_mcidx2slot(S32 param1, S32* out_slot, S32* param3)
{
    s32 cardReady[ISG_NUM_SLOTS] = {};
    S32 ret = 0;
    S32 idk = 0;
    s32 memSize = 0;
    s32 sectorSize = 0;
    *out_slot = -1;

    for (S32 i = 0; i < ISG_NUM_SLOTS; i++)
    {
        s32 result;
        do
        {
            result = CARDProbeEx(i, &memSize, &sectorSize);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            cardReady[i] = TRUE;
        }
    }

    for (S32 i = 0; i < ISG_NUM_SLOTS; i++)
    {
        if (cardReady[i])
        {
            if (idk == param1)
            {
                *out_slot = i;
                ret = 1;
                break;
            }
            idk++;
        }
    }

    if (param3)
    {
        for (S32 i = 0; i < ISG_NUM_SLOTS; i++)
        {
            param3[i] = cardReady[i];
        }
    }

    return ret;
}

static S32 iSG_mc_fread(st_ISG_MEMCARD_DATA* mcdata, char* buf, S32 len, S32 offset)
{
    return iSG_mcqa_fread(mcdata, buf, len, offset);
}

S32 iSGReadLeader(st_ISGSESSION* isgdata, const char* fname, char* databuf, S32 numbytes, S32 async)
{
    S32 bufsize;
    S32 iconsize;
    S32 allocsize;
    char* readbuf;

    S32 readret = 0;
    st_ISG_MEMCARD_DATA* data;
    void* alloc = NULL;

    en_ASYNC_OPERR operr = ISG_OPERR_NONE;
    if (isgdata->slot < 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        isgdata->unk_268 = ISG_OPERR_NOCARD;
        return 0;
    }
    data = &isgdata->mcdata[isgdata->slot];

    if (data->unk_12c != 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        isgdata->unk_268 = ISG_OPERR_DAMAGE;
        return 0;
    }

    iTRCDisk::TRCCheck();
    iconsize = iSG_cubeicon_size(data->chan, data->sectorSize);
    S32 sectorsize200 = ALIGN_THING(data->sectorSize, 0x200);
    if ((S32)databuf % 32 != 0 || numbytes - (numbytes / sectorsize200) * sectorsize200 != 0)
    {
        S32 tmpsize = (numbytes + 0x1ff & ~0x1ff);
        allocsize = tmpsize + 0x1f;
        alloc = xMemPushTemp(allocsize);
        memset(alloc, 0, allocsize);
        bufsize = tmpsize;
        readbuf = (char*)((U32)alloc + 0x1f & ~0x1f);
    }
    else
    {
        readbuf = databuf;
        bufsize = numbytes;
    }

    iTRCDisk::TRCCheck();

    if (iSG_mc_fopen(data, fname, -1, ISG_IOMODE_READ, &operr) != 0)
    {
        readret = (bool)iSG_mc_fread(data, (char*)readbuf, bufsize, iconsize);
        iSG_mc_fclose(data);
    }

    if (readret != 0 && alloc != NULL)
    {
        memcpy(databuf, readbuf, numbytes);
    }
    if (alloc != NULL)
    {
        xMemPopTemp(alloc);
    }

    isgdata->unk_268 = ISG_OPERR_NONE;
    if (readret != 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_SUCCESS;
    }
    else
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        if (data->unk_12c != 0)
        {
            isgdata->unk_268 = ISG_OPERR_DAMAGE;
        }
        else
        {
            isgdata->unk_268 = ISG_OPERR_OTHER;
        }
    }
    return readret;
}

static void iSG_upd_icostat(CARDStat*, CARDStat* stat)
{
    CARDSetCommentAddress(stat, 0);
    CARDSetBannerFormat(stat, CARD_STAT_BANNER_RGB5A3);
    CARDSetIconAddress(stat, 0x40);
    CARDSetIconAnim(stat, CARD_STAT_ANIM_LOOP);

    for (S32 i = 0; i < CARD_ICON_MAX; ++i)
    {
        CARDSetIconFormat(stat, i, CARD_STAT_BANNER_RGB5A3);
        CARDSetIconSpeed(stat, i, CARD_STAT_SPEED_MIDDLE);
    }
}

static void iSG_timestamp(CARDStat*)
{
}

static S32 iSG_mc_fwrite(st_ISG_MEMCARD_DATA* mcdata, char* buf, S32 len)
{
    return iSG_mcqa_fwrite(mcdata, buf, len);
}

S32 iSGSaveFile(st_ISGSESSION* isgdata, const char* fname, char* data, S32 n, S32 async, char* arg5)
// Only 25%, adding cus it may be useful for the actual function
{
    void* alloc;
    S32 allocsize;
    char* icondata;
    S32 iconsize;

    S32 writeret = 0;
    en_ASYNC_OPERR operr = ISG_OPERR_NONE;
    CARDStat statA = { 0 };
    CARDStat statB = { 0 };

    ResetButton::DisableReset();
    if (isgdata->slot < 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        isgdata->unk_268 = ISG_OPERR_NOCARD;
        return 0;
    }

    st_ISG_MEMCARD_DATA* mcdata = &isgdata->mcdata[isgdata->slot];
    if (mcdata->unk_12c != 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        isgdata->unk_268 = ISG_OPERR_DAMAGE;
        return 0;
    }

    iTRCDisk::TRCCheck();
    iconsize = iSG_cubeicon_size(isgdata->slot, mcdata->sectorSize);
    S32 sectorsize200 = ALIGN_THING(mcdata->sectorSize, 0x200);
    iconsize += ALIGN_THING(sectorsize200, n);

    allocsize = iconsize + 0x1f;
    alloc = xMemPushTemp(allocsize);
    memset(alloc, 0, allocsize);
    icondata = (char*)((U32)alloc + 0x1f & 0xFFFFFFE0);

    memcpy(iSG_bfr_icondata(icondata, &statA, arg5, mcdata->sectorSize), data, n);
    iTRCDisk::TRCCheck();

    if (iSG_mc_fopen(mcdata, fname, iconsize, ISG_IOMODE_WRITE, &operr) != 0)
    {
        if (iSG_mc_fwrite(mcdata, icondata, iconsize) != 0)
        {
            writeret = 1;
        }
        else
        {
            writeret = 0;
            operr = ISG_OPERR_SVWRITE;
        }

        if (writeret != 0)
        {
            iSG_upd_icostat(&statA, &mcdata->fstat);
            iSG_timestamp(&mcdata->fstat);
            s32 result;
            do
            {
                result = CARDSetStatus(mcdata->chan, mcdata->finfo.fileNo, &mcdata->fstat);
            } while (result == CARD_RESULT_BUSY);

            if (result != CARD_RESULT_READY)
            {
                writeret = 0;
            }
        }

        if (writeret != 0)
        {
            s32 result;
            do
            {
                result = CARDSetAttributes(mcdata->chan, mcdata->finfo.fileNo, CARD_ATTR_PUBLIC);
            } while (result == CARD_RESULT_BUSY);
        }

        iSG_mc_fclose(mcdata, &statB);
    }
    xMemPopTemp(alloc);

    if (writeret == 0 && mcdata->unk_12c == 0)
    {
        iSG_mc_fdel(mcdata, fname);
    }

    ResetButton::EnableReset();
    iTRCDisk::TRCCheck();

    isgdata->unk_268 = ISG_OPERR_NONE;
    if (writeret != 0)
    {
        isgdata->unk_26c = ISG_OPSTAT_SUCCESS;
    }
    else
    {
        isgdata->unk_26c = ISG_OPSTAT_FAILURE;
        if (operr != ISG_OPERR_NONE)
        {
            isgdata->unk_268 = operr;
        }
        else
        {
            isgdata->unk_268 = ISG_OPERR_UNKNOWN;
        }
    }
    return writeret;
}

S32 iSGSetupGameDir(st_ISGSESSION* isgdata, const char* dname, S32 force_iconfix)
{
    return 1;
}

static S32 iSG_mc_isformatted(st_ISG_MEMCARD_DATA* mcdata)
{
    S32 result = 0;
    S32 rc = 0;
    s32 xferBytes = 0;

    if (mcdata->unk_0 == 0)
    {
        rc = 0;
    }
    else if (mcdata->unk_12c)
    {
        rc = 0;
    }
    else
    {
        do
        {
            result = CARDCheckEx(mcdata->chan, &xferBytes);
        } while (result == CARD_RESULT_BUSY);

        if (result == CARD_RESULT_READY)
        {
            rc = 1;
        }
        else if (result == CARD_RESULT_BROKEN)
        {
            rc = 0;
        }
        else
        {
            rc = 0;
        }
    }

    return rc;
}

S32 iSGSelectGameDir(st_ISGSESSION* isgdata, const char* dname)
{
    if (isgdata->slot < 0)
    {
        return 0;
    }
    st_ISG_MEMCARD_DATA* data = &isgdata->mcdata[isgdata->slot];

    if (data->unk_0 == 0)
    {
        return 0;
    }

    if (iSG_mc_isGCcard(data, NULL, NULL) == 0)
    {
        return 0;
    }

    if (iSG_mc_isformatted(data) == 0)
    {
        return 0;
    }

    S32 count = 0;
    for (S32 idx = 0; idx < ISG_NUM_FILES; ++idx)
    {
        const char* n = iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, idx);
        if (iSG_get_finfo(data, n) != 0)
        {
            count++;
        }
    }

    return count != 0;
}

void iSGMakeTimeStamp(char* str)
{
    OSCalendarTime calendar_time = { 0 };
    OSTime time = OSGetTime();
    OSTicksToCalendarTime(time, &calendar_time);
    sprintf(str, "%02d/%02d/%04d %02d:%02d:%02d", calendar_time.mon + 1, calendar_time.mday,
            calendar_time.year, calendar_time.hour, calendar_time.min, calendar_time.sec);
}

static S32 iSG_get_fmoddate(st_ISG_MEMCARD_DATA* mcdata, const char* fname, int* sec, int* min,
                            int* hr, int* mon, int* day, int* yr)
{
    S32 rc = 1;
    OSCalendarTime time = { 0 };

    if (iSG_get_finfo(mcdata, fname) == 0)
    {
        rc = 0;
    }
    else
    {
        OSTime t = mcdata->unk_b0.time;
        t = OSSecondsToTicks(t);
        OSTicksToCalendarTime(t, &time);

        if (sec)
        {
            *sec = time.sec;
        }
        if (min)
        {
            *min = time.min;
        }
        if (hr)
        {
            *hr = time.hour;
        }
        if (mon)
        {
            *mon = time.mon + 1;
        }
        if (day)
        {
            *day = time.mday;
        }
        if (mon)
        {
            *mon = time.mon + 1;
        }
        if (day)
        {
            *day = time.mday;
        }
        if (yr)
        {
            *yr = time.year;
        }
    }

    return rc;
}

char* iSGFileModDate(st_ISGSESSION* isgdata, const char* fname, S32* sec, S32* min, S32* hr,
                     S32* mon, S32* day, S32* yr)
{
    static char datestr[0x40] = { 0 };
    int sec_str = 0;
    int min_str = 0;
    int hr_str = 0;
    int mon_str = 0;
    int day_str = 0;
    int yr_str = 0;
    if (iSG_get_fmoddate(&isgdata->mcdata[isgdata->slot], fname, &sec_str, &min_str, &hr_str,
                         &mon_str, &day_str, &yr_str) != 0)
    {
        sprintf(datestr, "%02d/%02d/%04d %02d:%02d:%02d", mon_str, day_str, yr_str, hr_str, min_str,
                sec_str);

        if (sec != NULL)
        {
            *sec = sec_str;
        }
        if (min != NULL)
        {
            *min = min_str;
        }
        if (hr != NULL)
        {
            *hr = hr_str;
        }
        if (mon != NULL)
        {
            *mon = mon_str;
        }
        if (day != NULL)
        {
            *day = day_str;
        }
        if (yr != NULL)
        {
            *yr = yr_str;
        }
    }
    else
    {
        sprintf(datestr, "<Unknown Modification>");
    }
    return datestr;
}

#pragma inline_depth(0)
char* iSGFileModDate(st_ISGSESSION* isgdata, const char* fname)
{
    return iSGFileModDate(isgdata, fname, NULL, NULL, NULL, NULL, NULL, NULL);
}
#pragma inline_depth(5)

static S32 iSG_get_fsize(st_ISG_MEMCARD_DATA* mcdata, const char* param2)
{
    S32 rc = -1;

    if (iSG_get_finfo(mcdata, param2))
    {
        rc = mcdata->unk_b0.length;
    }

    if (rc < 0)
    {
        rc = -1;
    }

    return rc;
}

S32 iSGFileSize(st_ISGSESSION* isgdata, const char* fname)
{
    S32 ret = 0;
    if (isgdata->slot < 0)
    {
        return -1;
    }
    st_ISG_MEMCARD_DATA* data = &isgdata->mcdata[isgdata->slot];
    iTRCDisk::TRCCheck();
    ret = iSG_get_fsize(data, fname);
    if (ret >= 0)
    {
        ret -= iSG_cubeicon_size(data->chan, data->sectorSize);
        if (ret < 0)
        {
            ret = -1;
        }
    }
    return ret;
}

#pragma inline_depth(0)
S32 iSGTgtHaveRoomStartup(st_ISGSESSION* isgdata, S32 tidx, S32 fsize, const char* dpath,
                          const char* fname, S32* bytesNeeded, S32* availOnDisk, S32* needFile)
{
    st_ISG_MEMCARD_DATA* data;
    S32 i;
    S32 count;
    S32 opened;
    S32 is_space;
    if (isgdata->slot < 0)
    {
        return 0;
    }
    data = &isgdata->mcdata[isgdata->slot];
    if (data->unk_0 == NULL)
    {
        return 0;
    }
    count = 0;
    if (fname == NULL)
    {
        for (i = 0; i < ISG_NUM_FILES; ++i)
        {
            iTRCDisk::TRCCheck();
            fname = iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, i);
            if (iSG_get_finfo(data, fname) != 0)
            {
                count++;
            }
        }
    }

    is_space = iSG_isSpaceForFile(data, fsize, fname, bytesNeeded, availOnDisk, needFile);
    if (count > 0 && *bytesNeeded > *availOnDisk)
    {
        if (needFile != NULL && *bytesNeeded > *availOnDisk)
        {
            *needFile = 0;
            return 0;
        }
        return 1;
    }

    CARDFileInfo fileInfo;
    opened = 0;
    for (i = 0; i < CARD_MAX_FILE && isgdata->slot >= 0 && isgdata->slot < ISG_NUM_SLOTS; ++i)
    {
        if (CARDFastOpen(isgdata->slot, i, &fileInfo) == 0)
        {
            opened++;
            CARDClose(&fileInfo);
        }
    }

    if (opened >= CARD_MAX_FILE)
    {
        if (count > 0)
        {
            if (needFile != NULL)
            {
                *needFile = 0;
            }
            return 0;
        }
        *needFile = -1;
        return 0;
    }

    if (*bytesNeeded > *availOnDisk)
    {
        return 0;
    }
    return (count >= 3) ? 0 : is_space;
}
#pragma inline_depth(5)

#pragma inline_depth(0)
S32 iSGTgtHaveRoom(st_ISGSESSION* isgdata, S32 tidx, S32 fsize, const char* dpath,
                   const char* fname, S32* bytesNeeded, S32* availOnDisk, S32* needFile)
{
    st_ISG_MEMCARD_DATA* data;
    S32 i;
    S32 count;
    S32 opened;
    S32 is_space;
    if (isgdata->slot < 0)
    {
        return 0;
    }
    data = &isgdata->mcdata[isgdata->slot];
    if (data->unk_0 == NULL)
    {
        return 0;
    }
    count = 0;
    if (fname == NULL)
    {
        for (i = 0; i < ISG_NUM_FILES; ++i)
        {
            iTRCDisk::TRCCheck();
            fname = iSGMakeName(ISG_NGTYP_GAMEFILE, NULL, i);
            if (iSG_get_finfo(data, fname) != 0)
            {
                count++;
            }
        }
    }

    is_space = iSG_isSpaceForFile(data, fsize, fname, bytesNeeded, availOnDisk, needFile);
    if (count > 0 && *bytesNeeded > *availOnDisk)
    {
        if (needFile != NULL && *bytesNeeded > *availOnDisk)
        {
            *needFile = 0;
            return 0;
        }
        return 1;
    }

    CARDFileInfo fileInfo;
    opened = 0;
    for (i = 0; i < CARD_MAX_FILE && isgdata->slot >= 0 && isgdata->slot < ISG_NUM_SLOTS; ++i)
    {
        if (CARDFastOpen(isgdata->slot, i, &fileInfo) == 0)
        {
            opened++;
            CARDClose(&fileInfo);
        }
    }

    if (opened >= CARD_MAX_FILE)
    {
        if (count > 0)
        {
            if (needFile != NULL)
            {
                *needFile = 0;
            }
            return 1;
        }
        return 0;
    }

    return (*bytesNeeded > *availOnDisk) ? 0 : is_space;
}
#pragma inline_depth(5)

static S32 iSG_mc_exists(S32 slot)
{
    S32 ret = 0;
    s32 memSize = 0;
    s32 sectorSize = 0;

    if (slot < -1)
    {
        return 0;
    }

    S32 result;
    do
    {
        result = CARDProbeEx(slot, &memSize, &sectorSize);
    } while (result == CARD_RESULT_BUSY);

    if (result == CARD_RESULT_READY)
    {
        ret = 1;
    }

    return ret;
}

U32 iSGTgtState(st_ISGSESSION* isgdata, S32 tgtidx, const char* dpath)
{
    S32 isSectSizeValid = 0;
    S32 state = 0;

    S32 rc = 0;

    S32 slot = 0;
    S32 x = 0;
    S32 y = 0;
    iTRCDisk::TRCCheck();

    iSG_mcidx2slot(tgtidx, &slot, NULL);
    if (slot != isgdata->slot)
    {
        iSGTgtSetActive(isgdata, tgtidx);
    }

    if (isgdata->slot < 0)
    {
        return 0x1000000;
    }

    st_ISG_MEMCARD_DATA* data = &isgdata->mcdata[isgdata->slot];
    if (data->unk_0 == NULL)
    {
        return 0;
    }

    if (iSG_mc_exists(isgdata->slot))
    {
        state |= 1;
    }
    else
    {
        return 0;
    }

    if (data->unk_12c != 0)
    {
        return state | 0x1000000;
    }

    rc = iSG_mc_isGCcard(data, &x, &y);
    if (x != 0)
    {
        state |= 0x4000000;
    }

    if (y != 0)
    {
        state |= 0x2000000;
    }

    if (rc == 0)
    {
        return state;
    }

    if (g_legalSectSize[0] > 0)
    {
        for (S32 i = 0; g_legalSectSize[i] > 0; i++)
        {
            if (data->sectorSize == g_legalSectSize[i])
            {
                isSectSizeValid = 1;
                break;
            }
        }
        if (isSectSizeValid == 0)
        {
            state |= 0x8000004;
            return state & ~4;
        }
    }

    if (iSG_mc_isformatted(data) == 0)
    {
        return state | 4;
    }
    else
    {
        return state | 0xe;
    }

    return state;
}

S32 iSGTgtFormat(st_ISGSESSION* isgdata, S32 tgtidx, S32 async, S32* canRecover)
{
    S32 slot = 0;
    S32 rc = 0;
    iTRCDisk::TRCCheck();
    if (iSG_mc_exists(isgdata->slot) == 0)
    {
        return 0;
    }

    iSG_mcidx2slot(tgtidx, &slot, NULL);
    // FIXME: r3 and r0 are swapped here for some reason ...
    if (slot != isgdata->slot)
    {
        return 0;
    }

    st_ISG_MEMCARD_DATA* data = &isgdata->mcdata[isgdata->slot];
    if (data->unk_0 == NULL)
    {
        return 0;
    }

    if (iSG_mc_isGCcard(data, NULL, NULL) == 0)
    {
        return 0;
    }

    rc = iSG_mc_format(data, 0, canRecover);
    if (rc == -1)
    {
        return -1;
    }
    return rc == 0 ? 0 : 1;
}

S32 iSGTgtPhysSlotIdx(st_ISGSESSION* isgdata, S32 tidx)
{
    S32 idx = -1;
    if (iSG_mcidx2slot(tidx, &idx, NULL))
    {
        return idx;
    }
    return -1;
}

S32 iSGTgtCount(st_ISGSESSION* isgdata, S32* max)
{
    s32 memSize = 0;
    s32 sectorSize = 0;
    S32 ret = 0;

    if (max != NULL)
    {
        *max = 2;
    }

    for (S32 i = 0; i < ISG_NUM_SLOTS; ++i)
    {
        iTRCDisk::TRCCheck();
        s32 result;
        do
        {
            result = CARDProbeEx(i, &memSize, &sectorSize);
        } while (result == CARD_RESULT_BUSY);
        if (result == CARD_RESULT_READY)
        {
            ret++;
        }
    }

    return ret;
}

static S32 iSG_mc_unmount(S32 slot)
{
    S32 rc = 0;
    S32 result;

    do
    {
        result = CARDUnmount(slot);
    } while (result == CARD_RESULT_BUSY);

    if (result == CARD_RESULT_READY)
    {
        rc = 1;
    }
    else if (result == CARD_RESULT_NOCARD)
    {
        rc = 1;
    }

    return rc;
}

void iSGSessionEnd(st_ISGSESSION* isgdata)
{
    iTRCDisk::TRCCheck();
    for (S32 i = 0; i < ISG_NUM_SLOTS; i++)
    {
        if (isgdata->mcdata[i].unk_0)
        {
            if (isgdata->mcdata[i].unk_0)
            {
                CARDUnmount(i);
            }
            isgdata->mcdata[i].unk_0 = 0;
        }
    }

    memset(isgdata, 0, sizeof(st_ISGSESSION));
}

static S32 iSG_chk_icondata()
{
    return 1;
}

st_ISGSESSION* iSGSessionBegin(void* cltdata, void (*chgfunc)(void*, en_CHGCODE), S32 monitor)
{
    iTRCDisk::TRCCheck();
    memset(&g_isgdata_MAIN, 0, sizeof(st_ISGSESSION));

    g_isgdata_MAIN.slot = -1;
    g_isgdata_MAIN.chgfunc = chgfunc;
    g_isgdata_MAIN.cltdata = cltdata;

    return &g_isgdata_MAIN;
}

char* iSGMakeName(en_NAMEGEN_TYPE type, const char* base, S32 idx)
{
    static volatile S32 rotate = 0; // fakematch??
    static char rotatebuf[8][32] = { 0 };

    const char* fmt_sd = "%s%02d";
    char* use_buf = rotatebuf[rotate++];
    if (rotate == 8)
    {
        rotate = 0;
    }

    *use_buf = NULL;
    switch (type)
    {
    case ISG_NGTYP_GAMEFILE:

        if (base != NULL)
        {
            sprintf(use_buf, fmt_sd, base, idx);
        }
        else
        {
            sprintf(use_buf, fmt_sd, "SpongeBob", idx);
        }
        break;
    case ISG_NGTYP_GAMEDIR:
    case ISG_NGTYP_CONFIG:
    case ISG_NGTYP_ICONTHUM:
        break;
    }

    return use_buf;
}

S32 iSGShutdown()
{
    OSFreeToHeap(__OSCurrHeap, g_rawicon);
    OSFreeToHeap(__OSCurrHeap, g_rawbanr);
    g_rawicon = NULL;
    g_iconsize = 0;
    g_rawbanr = NULL;
    g_banrsize = 0;
    return 1;
}

#pragma inline_depth(0)
S32 iSGStartup() // TO-DO - 92%
{
    if (g_isginit++ != 0)
    {
        return g_isginit;
    }
    else
    {
        CARDInit();
        iSG_load_icondata();
        return g_isginit;
    }
}
#pragma inline_depth(5)

//                                                                          iPar

void iParMgrRender()
{
}

void iParMgrUpdate(float)
{
}

//                                                                          iPad

PADStatus sPadData[PAD_MAX_CONTROLLERS];

void iPadKill()
{
}

void iPadStopRumble()
{
    PADControlMotor(mPad[globals.currentActivePad].port, 0); // Scheduling memes.
}

void iPadStopRumble(_tagxPad* pad)
{
    PADControlMotor(pad->port, 0);
}

S32 iPadUpdate(_tagxPad* pad, U32* on)
{
    U16 buttons;
    U32 temp_on;

    if (pad->port == 0)
    {
        PADRead(sPadData);
        PADClamp(sPadData);
    }

    PADStatus* padData = (PADStatus*)&sPadData;
    switch (padData[pad->port].err)
    {
    case PAD_ERR_NO_CONTROLLER:
        xTRCPad(pad->port, TRC_PadMissing);
        PADReset(0x80000000 >> pad->port);
        goto defaultError;

    case PAD_ERR_NOT_READY:
    case PAD_ERR_TRANSFER:
        return 0;

    default:
    defaultError:
        return 0;

    case PAD_ERR_NONE:
        temp_on = iPadConvFromGCN(padData[pad->port].button, 1, 0x80);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 2, 0x20);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 4, 0x40);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 8, 0x10);
        buttons = sPadData[pad->port].button;

        if (buttons & 0x10)
        {
            temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x20, 0x2000);
            temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x40, 0x200);
            if (sPadData[pad->port].triggerLeft >= 0x18)
            {
                temp_on |= 0x200;
            }
            if (sPadData[pad->port].triggerRight >= 0x18)
            {
                temp_on |= 0x2000;
            }
            temp_on |= 0x100000;
        }
        else
        {
            temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x20, 0x1000);
            temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x40, 0x100);
            if (sPadData[pad->port].triggerLeft >= 0x18)
            {
                temp_on |= 0x100;
            }
            if (sPadData[pad->port].triggerRight >= 0x18)
            {
                temp_on |= 0x1000;
            }
        }

        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x100, 0x10000);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x200, 0x80000);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x800, 0x40000);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x400, 0x20000);
        temp_on |= iPadConvFromGCN(sPadData[pad->port].button, 0x1000, 0x1);
        (*on) = temp_on;

        pad->analog1.x = iPadConvStick(sPadData[pad->port].stickX);
        pad->analog1.y = -iPadConvStick(sPadData[pad->port].stickY); // Scheduling memes.
        pad->analog2.x = iPadConvStick(sPadData[pad->port].substickX);
        pad->analog2.y = -iPadConvStick(sPadData[pad->port].substickY); // Same as above.
        if (gTrcPad[pad->port].state != TRC_PadInserted)
        {
            xTRCPad(pad->port, TRC_PadInserted);
        }
    }
    return 1;
}

_tagxPad* iPadEnable(_tagxPad* pad, S16 port)
{
    pad->port = port;
    pad->slot = 0;
    pad->state = ePad_Enabled;
    gTrcPad[pad->port].state = TRC_PadInserted;
    pad->flags |= 3;
    pad->flags |= 4;
    return pad;
}

S32 iPadInit()
{
    PADInit();
    return 1;
}

//                                                                                  iMorph

//                                                                                  iModel

//                                                                                  iMemMgr

extern xMemInfo_tag gMemInfo;
extern OSHeapHandle he;
extern OSHeapHandle hs;
extern unsigned char _stack_addr[];
extern U32 HeapSize;
extern U32 mem_top_alloc;
extern U32 mem_base_alloc;

#pragma inline_depth(0)
void iMemExit()
{
    free((void*)gMemInfo.DRAM.addr);
    gMemInfo.DRAM.addr = 0;
}
#pragma inline_depth(5)

//                                                              I dont have a name for these yet

void iBoxBoundVec(xBox* o, const xBox* b, const xVec3* v)
{
    xVec3Init(&o->lower, MIN(v->x, b->lower.x), MIN(v->y, b->lower.y), MIN(v->z, b->lower.z));
    xVec3Init(&o->upper, MAX(v->x, b->upper.x), MAX(v->y, b->upper.y), MAX(v->z, b->upper.z));
}

void iBoxInitBoundVec(xBox* b, const xVec3* v)
{
    xVec3Copy(&b->lower, v);
    xVec3Copy(&b->upper, v);
}

void iBoxIsectVec(const xBox* b, const xVec3* v, xIsect* isx)
{
    if (v->x >= b->lower.x && v->x <= b->upper.x && v->y >= b->lower.y && v->y <= b->upper.y &&
        v->z >= b->lower.z && v->z <= b->upper.z)
    {
        isx->penned = -1.0f;
    }
    else
    {
        isx->penned = 1.0f;
    }
}

void iBoxVecDist(const xBox* box, const xVec3* v, xIsect* isx)
{
    if (v->x < box->lower.x)
    {
        if (v->y < box->lower.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x00000000;
                isx->flags |= 0x80000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x01000000;
                isx->flags |= 0x40000000;
            }
            else
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x02000000;
                isx->flags |= 0x80000000;
            }
        }
        else if (v->y <= box->upper.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x03000000;
                isx->flags |= 0x40000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x04000000;
                isx->flags |= 0x20000000;
            }
            else
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x05000000;
                isx->flags |= 0x40000000;
            }
        }
        else
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x06000000;
                isx->flags |= 0x80000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x07000000;
                isx->flags |= 0x40000000;
            }
            else
            {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x08000000;
                isx->flags |= 0x80000000;
            }
        }
    }
    else if (v->x <= box->upper.x)
    {
        if (v->y < box->lower.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x09000000;
                isx->flags |= 0x40000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x0A000000;
                isx->flags |= 0x20000000;
            }
            else
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x0B000000;
                isx->flags |= 0x40000000;
            }
        }
        else if (v->y <= box->upper.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = 0.0f;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x0C000000;
                isx->flags |= 0x20000000;
            }
            else if (v->z <= box->upper.z)
            {
            }
            else
            {
                isx->norm.x = 0.0f;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x0E000000;
                isx->flags |= 0x20000000;
            }
        }
        else
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x0F000000;
                isx->flags |= 0x40000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x10000000;
                isx->flags |= 0x20000000;
            }
            else
            {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x11000000;
                isx->flags |= 0x40000000;
            }
        }
    }
    else
    {
        if (v->y < box->lower.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x12000000;
                isx->flags |= 0x80000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x13000000;
                isx->flags |= 0x40000000;
            }
            else
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x14000000;
                isx->flags |= 0x80000000;
            }
        }
        else if (v->y <= box->upper.y)
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x15000000;
                isx->flags |= 0x40000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x16000000;
                isx->flags |= 0x20000000;
            }
            else
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x17000000;
                isx->flags |= 0x40000000;
            }
        }
        else
        {
            if (v->z < box->lower.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x18000000;
                isx->flags |= 0x80000000;
            }
            else if (v->z <= box->upper.z)
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = isx->flags & ~0x1F000000 | 0x19000000;
                isx->flags |= 0x40000000;
            }
            else
            {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = isx->flags & ~0x1F000000 | 0x1A000000;
                isx->flags |= 0x80000000;
            }
        }
    }

    isx->dist = xVec3Length(&isx->norm);
}

void iSphereBoundVec(xSphere* o, const xSphere* s, const xVec3* v)
{
    F32 scale;
    xSphere temp;
    xSphere* tp;
    U32 usetemp;
    xIsect isx;

    usetemp = (o == s);

    iSphereIsectVec(s, v, &isx);

    if (isx.penned <= 0.0f)
    {
        if (!usetemp)
        {
            memcpy(o, s, sizeof(xSphere));
        }
    }
    else
    {
        if (usetemp)
        {
            tp = &temp;
        }
        else
        {
            tp = o;
        }

        xVec3Copy(&tp->center, &isx.norm);

        scale = (isx.dist - s->r) / (2.0f * isx.dist);

        xVec3SMul(&tp->center, &tp->center, scale);
        xVec3Add(&tp->center, &tp->center, &s->center);

        tp->r = 0.5f * (isx.dist + s->r);

        if (usetemp)
        {
            memcpy(o, tp, sizeof(xSphere));
        }
    }
}

void iSphereInitBoundVec(xSphere* s, const xVec3* v)
{
    xVec3Copy(&s->center, v);

    s->r = 0.00001f;
}

void iMath3Init()
{
}

//                                                                          Sin, Cos, Tan

F32 itan(F32 x)
{
    return std::tanf(x);
}

F32 icos(F32 x)
{
    return std::cosf(x);
}

F32 isin(F32 x)
{
    return std::sinf(x);
}

//                                                                          iLight

void iLightEnv(iLight* light, S32 env)
{
    RwUInt32 flags = 0;

    switch (env)
    {
    case ILIGHT_ENV_NONE:
        flags = 0;
        break;
    case ILIGHT_ENV_ATOMIC:
        flags = rpLIGHTLIGHTWORLD;
        break;
    case ILIGHT_ENV_WORLD:
        flags = rpLIGHTLIGHTATOMICS;
        break;
    case ILIGHT_ENV_ATOMICWORLD:
        flags = rpLIGHTLIGHTATOMICS | rpLIGHTLIGHTWORLD;
        break;
    }

    RpLightSetFlags(light->hw, flags);
}

void iLightDestroy(iLight* light)
{
    RwFrame* frame;

    light->type = ILIGHT_TYPE_NONE;

    _rwFrameSyncDirty();

    frame = RpLightGetFrame(light->hw);

    if (frame)
    {
        _rwFrameSyncDirty();
        RwFrameDestroy(frame);
    }

    RpLightDestroy(light->hw);
}

void iLightSetPos(iLight* light, xVec3* pos)
{
    RwFrame* f = RpLightGetFrame(light->hw);
    RwMatrix* m = RwFrameGetMatrix(f);

    RwMatrixGetPos(m)->x = pos->x;
    RwMatrixGetPos(m)->y = pos->y;
    RwMatrixGetPos(m)->z = pos->z;

    RwMatrixUpdate(m);

    RwFrameUpdateObjects(f);
}

void iLightSetColor(iLight* light, _xFColor* col)
{
    RpLightSetColor(light->hw, (RwRGBAReal*)col);
}

void iLightModify(iLight* light, U32 flags) // Remove xVec ops
{
    if (flags & 0x5)
    {
        RwFrame* frame = RpLightGetFrame(light->hw);
        RwMatrix temp;

        *(xVec3*)RwMatrixGetRight(&temp) = g_O3;
        *(xVec3*)RwMatrixGetUp(&temp) = g_O3;
        *(xVec3*)RwMatrixGetAt(&temp) = light->dir;
        *(xVec3*)RwMatrixGetPos(&temp) = light->sph.center;

        RwFrameTransform(frame, &temp, rwCOMBINEREPLACE);
    }

    if (flags & 0x2)
    {
        RpLightSetRadius(light->hw, light->sph.r);
    }

    if (flags & 0x8)
    {
        RpLightSetColor(light->hw, (RwRGBAReal*)&light->color);
    }

    if (flags & 0x10)
    {
        if (light->type == ILIGHT_TYPE_SPOT || light->type == ILIGHT_TYPE_SPOTSOFT)
        {
            RpLightSetConeAngle(light->hw, light->coneangle);
        }
    }
}

iLight* iLightCreate(iLight* light, U32 type)
{
    RwFrame* frame;

    switch (type)
    {
    case ILIGHT_TYPE_POINT:
        light->hw = RpLightCreate(rpLIGHTPOINT);
        break;
    case ILIGHT_TYPE_SPOT:
        light->hw = RpLightCreate(rpLIGHTSPOT);
        break;
    case ILIGHT_TYPE_SPOTSOFT:
        light->hw = RpLightCreate(rpLIGHTSPOTSOFT);
        break;
    default:
        return NULL;
    }

    if (!light->hw)
    {
        return NULL;
    }

    frame = RwFrameCreate();

    if (!frame)
    {
        RpLightDestroy(light->hw);
        return NULL;
    }

    RpLightSetFlags(light->hw, rpLIGHTLIGHTATOMICS);
    RpLightSetFrame(light->hw, frame);

    RwFrameUpdateObjects(frame);

    light->type = type;
    light->sph.center.x = 0.0f;
    light->sph.center.y = 0.0f;
    light->sph.center.z = 0.0f;
    light->sph.r = 0.0f;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
    light->color.a = 1.0f;
    light->dir.x = 0.0f;
    light->dir.y = 0.0f;
    light->dir.z = 1.0f;
    light->coneangle = 0.0f;

    return light;
}

void iLightInit(RpWorld* world)
{
    gLightWorld = world;
}

//                                                                              Don't know what to call this

// iFxUVCreatepipe
//_iGCURenderCallback

//                                                                              iFMV

// FIXME: These should be in a RW header somewhere
extern GXRenderModeObj* _RwDlRenderMode;
extern "C" {
void RwGameCubeGetXFBs(void*, void*);
}

//.bss
static U32 Bink_surface_type[5];

//.sbss
static S32 frame_num;
U32 fuckingSurfaceType; //???
static HBINK Bink;
static HRAD3DIMAGE Image;
static S32 Paused;
static void* pixels;
static volatile F32 vol;
S32 ip;
s32 oof;
void* iFMV::mXFBs[2];
void* iFMV::mCurrentFrameBuffer;
GXRenderModeObj* iFMV::mRenderMode;

// .sdata
static float Width_scale = 1.0f;
static float Height_scale = 1.0f;
U8 iFMV::mFirstFrame = 1;

static void* arammalloc(size_t size)
{
    return (void*)ARAlloc(size);
}

// Something weird is going on here...
static void aramfree(void* mem)
{
    u32 vol;
    ARFree(&vol);
}

static void DrawFrame(float arg0, float arg1, float arg2, float arg3)
{
    GXRenderModeObj* rm = _RwDlRenderMode;
    Mtx idt;
    Mtx44 mtx;

    GXSetViewport(0.0f, 0.0f, rm->fbWidth, rm->efbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, rm->fbWidth, rm->efbHeight);
    GXSetDispCopySrc(0, 0, rm->fbWidth, rm->efbHeight);
    GXSetDispCopyDst(rm->fbWidth, rm->xfbHeight);
    GXSetDispCopyYScale((float)rm->xfbHeight / rm->efbHeight);
    GXSetCopyFilter(rm->aa, rm->sample_pattern, GX_TRUE, rm->vfilter);

    if (rm->aa)
    {
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    }
    else
    {
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GXSetDispCopyGamma(GX_GM_1_0);
    C_MTXOrtho(mtx, 0.0f, 480.0f, 0.0f, 640.0f, 0.0f, 10000.0f);
    GXSetProjection(mtx, GX_ORTHOGRAPHIC);
    PSMTXIdentity(idt);
    GXLoadPosMtxImm(idt, 0);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_NRM_NBT, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_NRM_NBT, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_NRM_NBT, GX_RGBA6, 0);

    GXSetNumTexGens(1);
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3c);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    if (rm->field_rendering)
    {
        u32 field = VIGetNextField();
        GXSetViewportJitter(0.0f, 0.0f, rm->fbWidth, rm->efbHeight, 0.0f, 1.0f, field);
    }
    else
    {
        GXSetViewport(0.0f, 0.0f, rm->fbWidth, rm->efbHeight, 0.0f, 1.0f);
    }

    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    Blit_RAD_3D_image(Image, arg0, arg1, arg2, arg3, 1.0f);
}

static void xDrawLine2D_LocaliFMVVersion(F32 arg0, F32 arg1, F32 arg2, F32 arg3)
{
    RwRGBA color = { -1, -1, -1, -1 };

    F32 nearz = RwIm2DGetNearScreenZ();
    void* texraster_state;
    RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &texraster_state);
    void* vtx_alpha_state;
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vtx_alpha_state);
    RwIm2DVertex verts[2];

    // RwIm2DVertexSetRealRGBA

    RwIm2DVertexSetScreenX(&verts[0], arg0);
    RwIm2DVertexSetScreenY(&verts[0], arg1);
    RwIm2DVertexSetScreenZ(&verts[0], nearz);
    RwIm2DVertexSetIntRGBA(&verts[0], color.red, color.green, color.blue, color.alpha);

    RwIm2DVertexSetScreenX(&verts[1], arg2);
    RwIm2DVertexSetScreenY(&verts[1], arg3);
    RwIm2DVertexSetScreenZ(&verts[1], nearz);
    RwIm2DVertexSetIntRGBA(&verts[1], color.red, color.green, color.blue, color.alpha);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, NULL);

    // { arg2, arg3, nearz, -1 };
    RwIm2DRenderLine(verts, 2, 0, 1);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, texraster_state);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, vtx_alpha_state);
}

static void Show_frame()
{
    RwRGBA color = { 0 };
    RwCamera* cam = iCameraCreate(640, 480, FALSE);
    RwCameraClear(cam, &color, rwCAMERACLEARIMAGE);

    RwCameraBeginUpdate(cam);
    Width_scale = 640 / Bink->unk_0;
    Height_scale = 480 / Bink->unk_4;
    xDrawLine2D_LocaliFMVVersion(0.0f, 0.0f, 0.0f, 0.0f);
    DrawFrame(0.0f, 0.0f, Width_scale, Height_scale);
    RwCameraEndUpdate(cam);
    RwCameraShowRaster(cam, NULL, 0);

    iCameraDestroy(cam);
}

static void PlayFMV(char* fname, u32 buttons, F32 time)
{
    GXCullMode cull_mode;
    GXGetCullMode(&cull_mode);
    iFMV::InitDisplay(_RwDlRenderMode);
    iPadStopRumble(globals.pad0);
    RADSetAudioMemory(arammalloc, aramfree);
    RADSetMemory(iFMVmalloc, iFMVfree);
    Setup_surface_array();

    for (char* c = fname; *c != NULL; c++)
    {
        if (*c == '\\')
        {
            *c = '/';
        }
    }

    tag_xFile file;
    DVDFileInfo* pfinfo = &file.ps.fileInfo;
    do
    {
        if (iTRCDisk::IsDiskIDed())
        {
            Bink = BinkOpen(fname, NULL);
            if (Bink == NULL)
            {
                BinkGetError();
            }
        }
        if (iTRCDisk::TRCCheck() != 0)
        {
            break;
        }
        if (Bink == NULL)
        {
            iFileOpen(fname, 0, &file);
            DVDSeekAsyncPrio(pfinfo, 0, NULL, 2);
            if (iTRCDisk::TRCCheck())
            {
                DVDCancel(&pfinfo->cb);
                break;
            }
            else
            {
                DVDCancel(&pfinfo->cb);
            }
        }
    } while (Bink == NULL);

    if (Bink != NULL)
    {
        if (Bink->unk_f0 != 0)
        {
            for (ip = 0; ip <= Bink->unk_f0; ++ip)
            {
                vol = gSnd.categoryVolFader[SND_CAT_CUTSCENE];
                vol = vol * vol;
                vol = vol * 32768.0f;
                BinkSetVolume(Bink, ip, vol);
            }
        }

        Image = Open_RAD_3D_image(NULL, Bink->unk_0, Bink->unk_4, fuckingSurfaceType);
        if (Image != NULL)
        {
            if (frame_num != 0)
            {
                BinkGoto(Bink, frame_num, 0);
            }
            oof = 0;

            do
            {
                if (iTRCDisk::TRCCheck())
                {
                    goto superbreak;
                }
                if (BinkWait(Bink) == 0)
                {
                    Decompress_frame(Bink, Image, 1);
                    Show_frame();
                    BinkNextFrame(Bink);
                }
                else if (Paused)
                {
                    Show_frame();
                }
                xPadUpdate(globals.currentActivePad, 0.0f);

                F32 t = (float)Bink->unk_c / (Bink->unk_14 / Bink->unk_18);
                if (buttons && t >= time && globals.pad0->pressed & buttons)
                {
                    frame_num = -1;
                    goto superbreak;
                }
            } while (Bink->unk_c < Bink->unk_8 - 1);
            frame_num = -1;
        }
    superbreak:
        if (frame_num != -1)
        {
            frame_num = Bink->unk_c;
        }
        Close_RAD_3D_image(Image);
        Image = NULL;
        BinkClose(Bink);
        Bink = NULL;
    }

    GXSetCullMode(cull_mode);
    iVSync();
    xPadUpdate(globals.currentActivePad, 0.0f);
}

U32 iFMVPlay(char* filename, U32 buttons, F32 time, U32 unk0, bool skippable, bool lockController)
{
    if (filename == NULL)
    {
        return 1;
    }

    frame_num = 0;
    while (frame_num >= 0)
    {
        PlayFMV(filename, buttons, time);
    }
    return 0;
}

S32 iFMVSystemInit()
{
    return 1;
}

void _MovieFree(void* mem)
{
    RwFree(mem);
}

void* _MovieAlloc(U32 size)
{
    return RwMalloc(size);
}

//                                                                      iFile

struct file_queue_entry
{
    tag_xFile* file;
    void* buf;
    U32 size;
    U32 offset;
    IFILE_READSECTOR_STATUS stat;
    void (*callback)(tag_xFile* file);
    U32 asynckey;
};

extern file_queue_entry file_queue[4];

static U32 tbuffer[1024 + 8];
static U32* buffer32;
volatile U32 iFileSyncAsyncReadActive;

U32 iFileGetSectorSize(xFileDriveType drive)
{
    // FIXME: Am I dumb? How does this not create jumps in objdiff?
    if (drive != 0)
    {
        return 0;
    }
    else
    {
        return 32;
    }
}

void iFileGetInfo(tag_xFile* file, U32* addr, U32* length)
{
    if (addr)
    {
        *addr = file->ps.fileInfo.startAddr;
    }

    if (length)
    {
        *length = file->ps.fileInfo.length;
    }
}

U32 iFileFind(const char* name, S32 x, tag_xFile* file)
{
    return iFileOpen(name, 0, file);
}

void iFileSetPath(const char* path)
{
}

U32 iFileGetSize(tag_xFile* file)
{
    return file->ps.fileInfo.length;
}

U32 iFileClose(tag_xFile* file)
{
    tag_iFile* ps = &file->ps;
    S32 ret;

    ret = DVDClose(&file->ps.fileInfo);
    ret = DVDClose(&file->ps.fileInfo);

    if (!ret)
    {
        return 1;
    }

    ps->flags = 0;
    return 0;
}

static void async_cb(s32 result, DVDFileInfo* fileInfo)
{
    file_queue_entry* entry = &file_queue[(S32)fileInfo->cb.userData];
    s32 r7 = DVD_RESULT_FATAL_ERROR;

    switch (result)
    {
    case DVD_RESULT_FATAL_ERROR:
    {
        xTRCDisk(TRC_DiskFatal);
        return;
    }
    case DVD_RESULT_GOOD:
    case DVD_RESULT_IGNORED:
    {
        if (result >= DVD_RESULT_GOOD)
        {
            r7 = result;
        }

        break;
    }
    }

    if (r7 < DVD_RESULT_GOOD)
    {
        entry->stat = IFILE_RDSTAT_FAIL;

        if (entry->callback)
        {
            entry->callback(entry->file);
        }
    }
    else if (r7 + entry->offset >= entry->size ||
             r7 + entry->offset + entry->file->ps.offset >= entry->file->ps.fileInfo.length)
    {
        entry->stat = IFILE_RDSTAT_DONE;
        entry->offset = entry->size;

        if (entry->callback)
        {
            entry->callback(entry->file);
        }

        entry->file->ps.asynckey = -1;
    }
    else
    {
        entry->offset += r7;
        entry->stat = IFILE_RDSTAT_INPROG;

        s32 length;
        if ((entry->size - entry->offset < 0x8000))
        {
            length = ALIGN(entry->size - entry->offset, 4);
        }
        else
        {
            length = 0x10000 - 0x8000;
        }

        if (length + entry->offset + entry->file->ps.offset > entry->file->ps.fileInfo.length)
        {
            // length = OSRoundUp32B(entry->file->ps.fileInfo.length - entry->file->ps.offset -
            //     entry->offset);
            length = entry->file->ps.fileInfo.length;
            length -= entry->file->ps.offset;
            length -= entry->offset;
            length = length + 32 - 1;
            length = length & ~(32 - 1);
        }

        void* addr = (void*)((U32)entry->buf + entry->offset);
        DVDReadAsync(&entry->file->ps.fileInfo, addr, length,
                     entry->file->ps.offset + entry->offset, async_cb);
    }
}

S32 iFileReadAsync(tag_xFile* file, void* buf, U32 aSize, void (*callback)(tag_xFile*),
                   S32 priority)
{
    static S32 fopcount = 1;
    tag_iFile* ps = &file->ps;
    S32 i;

    for (i = 0; i < 4; i++)
    {
        if (file_queue[i].stat != IFILE_RDSTAT_QUEUED && file_queue[i].stat != IFILE_RDSTAT_INPROG)
        {
            S32 id = fopcount++ << 2;
            S32 asynckey = id + i;

            file_queue[i].file = file;
            file_queue[i].buf = buf;
            file_queue[i].size = aSize;
            file_queue[i].offset = 0;
            file_queue[i].stat = IFILE_RDSTAT_QUEUED;
            file_queue[i].callback = callback;
            file_queue[i].asynckey = asynckey;

            aSize = (aSize < 32) ? ALIGN(aSize, 4) : 32;

            ps->fileInfo.cb.userData = (void*)i;

            DVDReadAsync(&ps->fileInfo, buf, aSize, ps->offset, async_cb);

            ps->asynckey = asynckey;

            return id + i;
        }
    }

    return -1;
}

IFILE_READSECTOR_STATUS iFileReadAsyncStatus(S32 key, S32* amtToFar)
{
    if (key != file_queue[key & 0x3].asynckey)
    {
        return IFILE_RDSTAT_EXPIRED;
    }

    if (amtToFar)
    {
        *amtToFar = file_queue[key & 0x3].offset;
    }

    return file_queue[key & 0x3].stat;
}

static void ifilereadCB(tag_xFile* file)
{
    iFileSyncAsyncReadActive = 0;
}

U32 iFileRead(tag_xFile* file, void* buf, U32 size)
{
    tag_iFile* ps = &file->ps;

    iFileSeek(file, ps->offset, IFILE_SEEK_SET);

    iFileSyncAsyncReadActive = 1;

    iFileReadAsync(file, buf, size, ifilereadCB, 0);

    while (iFileSyncAsyncReadActive)
    {
        iTRCDisk::TRCCheck();
    }

    return size;
}

U32 iFileOpen(const char* name, S32 flags, tag_xFile* file)
{
    tag_iFile* ps = &file->ps;

    if (flags & IFILE_OPEN_ABSPATH)
    {
        strcpy(ps->path, name);
    }
    else
    {
        iFileFullPath(name, ps->path);
    }

    ps->entrynum = DVDConvertPathToEntrynum(ps->path);

    if (ps->entrynum == -1)
    {
        return 1;
    }

    if (!DVDFastOpen(ps->entrynum, &ps->fileInfo))
    {
        ps->entrynum = -1;
        return 1;
    }

    ps->unkC4 = 0;
    ps->flags = 0x1;

    iFileSeek(file, 0, IFILE_SEEK_SET);

    return 0;
}

U32* iFileLoad(const char* name, U32* buffer, U32* size)
{
    char path[128];
    tag_xFile file;
    S32 fileSize, alignedSize;

    iFileFullPath(name, path);
    iFileOpen(name, IFILE_OPEN_ABSPATH, &file);

    fileSize = iFileGetSize(&file);

    if (!buffer)
    {
        buffer = (U32*)OSAlloc(OSRoundUp32B(fileSize));
    }

    alignedSize = OSRoundUp32B(fileSize);

    iFileRead(&file, buffer, alignedSize);

    if (size)
    {
        *size = alignedSize;
    }

    iFileClose(&file);

    return buffer;
}

//                                                                                iEnv

static S32 sBeginDrawFX;
static RpWorld* sPipeWorld;
static RwCamera* sPipeCamera;
static iEnv* lastEnv;

// This is named JspPS2_ClumpRender on PS2
static void Jsp_ClumpRender(RpClump* clump, xJSPNodeInfo* nodeInfo)
{
    S32 backcullon = 1;
    S32 zbufferon = 1;
    RwLLLink* cur = rwLinkListGetFirstLLLink(&clump->atomicList);
    RwLLLink* end = rwLinkListGetTerminator(&clump->atomicList);

    while (cur != end)
    {
        RpAtomic* apAtom = rwLLLinkGetData(cur, RpAtomic, inClumpLink);

        if (RpAtomicGetFlags(apAtom) & rpATOMICRENDER)
        {
            RwFrame* frame = RpAtomicGetFrame(apAtom);

            if (!iModelCull(apAtom, &frame->ltm))
            {
                if (backcullon)
                {
                    if (nodeInfo->nodeFlags & 0x4)
                    {
                        backcullon = 0;
                        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
                    }
                }
                else
                {
                    if (!(nodeInfo->nodeFlags & 0x4))
                    {
                        backcullon = 1;
                        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);
                    }
                }

                if (zbufferon)
                {
                    if (nodeInfo->nodeFlags & 0x2)
                    {
                        zbufferon = 0;
                        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
                    }
                }
                else
                {
                    if (!(nodeInfo->nodeFlags & 0x2))
                    {
                        zbufferon = 1;
                        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
                    }
                }

                RpAtomicRender(apAtom);
            }
        }

        cur = rwLLLinkGetNext(cur);
        nodeInfo++;
    }
}

void iEnvEndRenderFX(iEnv*)
{
    iEnv* env = lastEnv;

    if (lastEnv && env->fx && globalCamera && sBeginDrawFX)
    {
        RpWorldRemoveCamera(env->fx, globalCamera);
        RpWorldAddCamera(env->world, globalCamera);

        sBeginDrawFX = 0;
    }
}

#pragma inline_depth(0)
void iEnvRender(iEnv* env, bool)
{
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

    if (env->jsp)
    {
        Jsp_ClumpRender(env->jsp->clump, env->jsp->jspNodeList);
    }
    else
    {
        RpWorldRender(env->world);
    }

    lastEnv = env;
}
#pragma inline_depth(5)

void iEnvSetup(iEnv* env)
{
    lastEnv = env;
}

void iEnvLightingBasics(iEnv*, xEnvAsset*)
{
}

void iEnvDefaultLighting(iEnv*)
{
}

void iEnvFree(iEnv* env)
{
    _rwFrameSyncDirty();

    RpWorldDestroy(env->world);
    env->world = NULL;

    if (env->fx)
    {
        RpWorldDestroy(env->fx);
        env->fx = NULL;
    }

    if (env->collision)
    {
        RpWorldDestroy(env->collision);
        env->collision = NULL;
    }
}

static void SetPipelineCB(RpAtomic* atomic, void* data)
{
    if (data == 0)
    {
        return;
    }
    (atomic->pipeline) = (RxPipeline*)data;
}

//                                                                                 iXF / iDraw

void iDrawEnd()
{
    return;
}

void iDrawBegin()
{
    return;
}

void iDrawSetFBMSK(U32 abgr)
{
    size_t tmp, hi;

    hi = abgr >> 24;

    if (hi == 0)
    {
        GXSetAlphaUpdate(GX_TRUE);
    }
    else if (hi == 255)
    {
        GXSetAlphaUpdate(GX_FALSE);
    }

    tmp = abgr & 0x00FFFFFF;

    if (tmp == 0)
    {
        GXSetColorUpdate(GX_TRUE);
    }
    else
    {
        GXSetColorUpdate(GX_FALSE);
    }
}

//                                                                          iCS

U32 read_sizzze = 0;

S32 iCSLoadStep(xCutscene* csn)
{
    S32 bytes;
    XFILE_READSECTOR_STATUS cdstat;
    U32 skipAccum;
    U32 tmpSize;
    void* foundModel;
    U32 i;

    if (csn->Waiting)
    {
        cdstat = xFileReadAsyncStatus(csn->File.ps.asynckey, &bytes);

        if (cdstat == XFILE_RDSTAT_INPROG || cdstat == XFILE_RDSTAT_QUEUED)
        {
            return 0;
        }
    }

    if (csn->DataLoading >= 0)
    {
        skipAccum = 0;

        while (csn->DataLoading < (S32)csn->Info->NumData)
        {
            if (csn->Data[csn->DataLoading].DataType == XCUTSCENEDATA_TYPE_6)
            {
                foundModel = NULL;
            }
            else
            {
                foundModel = xSTFindAsset(csn->Data[csn->DataLoading].AssetID, &tmpSize);
            }

            if (foundModel || csn->Data[csn->DataLoading].ChunkSize == 0)
            {
                csn->Data[csn->DataLoading].DataPtr = foundModel;

                skipAccum += ALIGN(csn->Data[csn->DataLoading].ChunkSize, 2048);

                csn->DataLoading++;
            }
            else
            {
                if (skipAccum)
                {
                    iCSFileAsyncSkip(csn, skipAccum);

                    skipAccum = 0;
                }

                if (csn->GotData)
                {
                    if (csn->Data[csn->DataLoading].DataType == XCUTSCENEDATA_TYPE_6)
                    {
                        csn->Data[csn->DataLoading].DataPtr =
                            RwMalloc(csn->Data[csn->DataLoading].ChunkSize);

                        memcpy(csn->Data[csn->DataLoading].DataPtr, csn->AlignBuf,
                               csn->Data[csn->DataLoading].ChunkSize);
                    }
                    else
                    {
                        csn->Data[csn->DataLoading].DataPtr =
                            iModelFileNew(csn->AlignBuf, csn->Data[csn->DataLoading].ChunkSize);
                    }

                    csn->Data[csn->DataLoading].DataType |= 0x80000000;
                    csn->DataLoading++;
                    csn->GotData = 0;
                }
                else
                {
                    iCSFileAsyncRead(csn, csn->AlignBuf,
                                     ALIGN(csn->Data[csn->DataLoading].ChunkSize, 2048));

                    csn->GotData = 1;
                    return 0;
                }
            }
        }

        if (skipAccum)
        {
            iCSFileAsyncSkip(csn, skipAccum);
        }

        csn->DataLoading = -1;
    }

    if (csn->DataLoading == -1)
    {
        if (csn->GotData)
        {
            iCSSoundSetup(csn);

            if (csn->SndNumChannel != 0)
            {
                xSndPauseAll(1, 1);
                xSndUpdate();

                if (csn->SndNumChannel == 2)
                {
                    // Stereo

                    csn->SndHandle[0] = xSndPlay(csn->SndAssetID[0], 0.9f, -99999.0f, 255, 0x200, 0,
                                                 SND_CAT_CUTSCENE, 0.0f);
                    csn->SndHandle[1] = xSndPlay(csn->SndAssetID[1], 0.9f, -99999.0f, 255, 0x200, 0,
                                                 SND_CAT_CUTSCENE, 0.0f);
                }
                else
                {
                    // Mono

                    csn->SndHandle[0] = xSndPlay(csn->SndAssetID[0], 0.9f, -99999.0f, 255, 0x200, 0,
                                                 SND_CAT_CUTSCENE, 0.0f);
                    csn->SndHandle[1] = 0;
                }
            }

            csn->GotData = 0;
            csn->DataLoading = -2;
        }
        else
        {
            iCSFileAsyncRead(csn, csn->Play, csn->TimeChunkOffs[1] - csn->TimeChunkOffs[0]);

            csn->GotData = 1;
            return 0;
        }
    }

    if (csn->DataLoading == -2)
    {
        if (csn->Info->NumTime > 1)
        {
            iCSFileAsyncRead(csn, csn->Stream, csn->TimeChunkOffs[2] - csn->TimeChunkOffs[1]);
        }

        csn->DataLoading = -3;
    }

    for (i = 0; i < csn->SndNumChannel; i++)
    {
    }

    csn->Ready = 1;
    return 1;
}

void iCSFileClose(xCutscene* csn)
{
    DVDCancelAllAsync(false); // TO-DO Im not sure this is right, but it matches

    csn->Opened = 0;
}

static void iCSAsyncReadCB(tag_xFile* file)
{
    S32 bytes;
    xCutscene* csn;

    if (file)
    {
        if (iFileReadAsyncStatus(file->ps.asynckey, &bytes) == IFILE_RDSTAT_DONE)
        {
            iFileSeek(file, bytes, IFILE_SEEK_CUR);
        }

        csn = xCutscene_CurrentCutscene();
        csn->Waiting = 0;
    }
}

void iCSFileAsyncRead(xCutscene* csn, void* dest, U32 size)
{
    U32* buf = (U32*)dest;
    U32 i;

    read_sizzze = size;
    csn->Waiting = 1;

    for (i = 0; i < size / 4; i++)
    {
        buf[i] = 0xDEADBEEF;
    }

    iFileReadAsync(&csn->File, dest, size, iCSAsyncReadCB, 0);
}

void* iCSSoundGetData(xSndVoiceInfo* vp, U32* size)
{
    U32 channelIndex;
    U32 dataIndex;
    xCutscene* csn;
    void* retdata;
    xCutsceneData* data;
    U32 numChannel;
    U32 numData;
    S32 sndChannelIndex;
    U32 r4;
    U32 sndChannelReq;

    csn = xCutscene_CurrentCutscene();

    numChannel = csn->SndNumChannel;
    retdata = NULL;
    sndChannelIndex = -1;

    for (channelIndex = 0; channelIndex < numChannel; channelIndex++)
    {
        if (csn->SndAssetID[channelIndex] == vp->assetID)
        {
            sndChannelIndex = channelIndex;
            sndChannelReq = csn->SndChannelReq[channelIndex];
        }
    }

    if (sndChannelIndex == -1)
    {
        return NULL;
    }

    if (!csn->Waiting && csn->Stream->ChunkIndex == sndChannelReq)
    {
        numData = csn->Stream->NumData;
        data = (xCutsceneData*)(csn->Stream + 1);
    }
    else
    {
        numData = csn->Play->NumData;
        data = (xCutsceneData*)(csn->Play + 1);

        if (csn->SndChannelReq[sndChannelIndex] != csn->Play->ChunkIndex)
        {
            csn->SndChannelReq[sndChannelIndex] = csn->Play->ChunkIndex;
        }
    }

    r4 = 0;

    for (dataIndex = 0; dataIndex < numData; dataIndex++)
    {
        if (data->DataType == XCUTSCENEDATA_TYPE_SOUND)
        {
            if (!retdata)
            {
                retdata = (void*)(data + 1);
                *size = data->ChunkSize;
            }

            if (sndChannelIndex == r4)
            {
                retdata = (void*)(data + 1);
                *size = data->ChunkSize;

                break;
            }
            else
            {
                r4++;
            }
        }

        data = (xCutsceneData*)((U8*)data + ALIGN(data->ChunkSize, 16) + sizeof(xCutsceneData));
    }

    if (!retdata)
    {
        return NULL;
    }

    while ((U32)retdata & 0x1F)
    {
        retdata = (void*)((U8*)retdata + 16);
        *size -= 16;
    }

    csn->SndChannelReq[sndChannelIndex]++;

    return retdata;
}

void iCSSoundSetup(xCutscene* csn)
{
    xCutsceneData* data;
    U32 dataIndex;
    U32 numData;

    data = (xCutsceneData*)(csn->Play + 1);
    numData = csn->Play->NumData;

    for (dataIndex = 0; dataIndex < numData; dataIndex++)
    {
        if (data->DataType == XCUTSCENEDATA_TYPE_SOUND)
        {
            if (csn->SndNumChannel >= 2)
            {
                break;
            }

            csn->SndAssetID[csn->SndNumChannel] = data->AssetID;
            csn->SndNumChannel++;
        }

        data = (xCutsceneData*)((U8*)data + ALIGN(data->ChunkSize, 16) + sizeof(xCutsceneData));
    }
}

//                                                              iAsync

S32 asyncThread;
bool asyncThreadDone = true;
S64 asyncDirtyFrameList;

void iAsyncResume()
{
    S32 unk0;

    if ((asyncThreadDone == '\0') &&
        (unk0 = OSIsThreadSuspended((OSThread*)+0x7fb57520), unk0 != 0))
    {
        OSResumeThread((OSThread*)+0x7fb57520);
    }
}

void iAsyncSuspend()
{
    S32 unk0;

    if ((asyncThreadDone == '\0') &&
        (unk0 = OSIsThreadSuspended((OSThread*)+0x7fb57520), unk0 == 0))
    {
        OSSuspendThread((OSThread*)+0x7fb57520);
    }
}

void iAsyncEndUpdate()
{
    OSEnableInterrupts();
}

void iAsyncBeginUpdate()
{
    OSDisableInterrupts();
}

void iAsyncTerminate()
{
    asyncThreadDone = true;
}

S32 iAsyncIsDone()
{
    return asyncThreadDone;
}

void iAsyncStart(void (*func)(void*))
{
    // FIXME: Very hacky and not all that close. Relocs need to be fixed.
    ThreadParam threadParams;
    asyncDirtyFrameList = (S64)&asyncDirtyFrameList;
    threadParams.status = (S64)&asyncDirtyFrameList;
    asyncThreadDone = 0;
    OSCreateThread((OSThread*)-0x7fb57520, (OSThreadStartFunction)func, 0, (void*)0x80442278,
                   0x2000, 10, 1);
    OSResumeThread((OSThread*)-0x7fb57520);
}

//                                                              iAnimSKB

void iAnimEvalSKB(iAnimSKBHeader* data, F32 time, U32 flags, xVec3* tran, xQuat* quat)
{
    U32 i, tidx, bcount, tcount;
    iAnimSKBKey* keys;
    F32* times;
    U16* offsets;
    S32 asdf; // unused
    F32 scalex, scaley, scalez;

    tcount = data->TimeCount;
    bcount = data->BoneCount;

    keys = (iAnimSKBKey*)(data + 1);
    times = (F32*)(keys + data->KeyCount);
    offsets = (U16*)(times + tcount);

    if (time < 0.0f)
    {
        time = 0.0f;
    }

    if (time > times[tcount - 1])
    {
        time = times[tcount - 1];
    }

    tidx = (tcount - 1) % 4;

    while (times[tidx] < time)
    {
        tidx += 4;
    }

    while (tidx && time <= times[tidx])
    {
        tidx--;
    }

    offsets += tidx * bcount;

    if (flags & 0x1)
    {
        bcount = 1;
    }

    if (flags & 0x2)
    {
        bcount--;
        offsets++;
    }

    if (tcount == 1)
    {
        // non-matching: float constants are loaded outside of loop

        scalex = data->Scale[0];
        scaley = data->Scale[1];
        scalez = data->Scale[2];

        for (i = 0; i < bcount; i++, quat++, tran++)
        {
            iAnimSKBKey* k = &keys[i];

            quat->v.x = k->Quat[0] * (1.0f / SHRT_MAX);
            quat->v.y = k->Quat[1] * (1.0f / SHRT_MAX);
            quat->v.z = k->Quat[2] * (1.0f / SHRT_MAX);
            quat->s = k->Quat[3] * (1.0f / SHRT_MAX);

            tran->x = k->Tran[0] * scalex;
            tran->y = k->Tran[1] * scaley;
            tran->z = k->Tran[2] * scalez;
        }
    }
    else
    {
        // non-matching: float constants are loaded outside of loop

        scalex = data->Scale[0];
        scaley = data->Scale[1];
        scalez = data->Scale[2];

        for (i = 0; i < bcount; i++, quat++, tran++)
        {
            // no idea if this part even functionally matches.
            // come back to this when not lazy

            RtQuatSlerpCache qcache;
            RtQuat q1, q2;
            RwReal time1, time2, lerp;
            iAnimSKBKey* k = &keys[*offsets];
            U32 costheta, theta; // unused

            offsets++;

            time1 = time - times[k->TimeIndex];
            time2 = times[k[1].TimeIndex] - times[k[0].TimeIndex];
            lerp = time1 / time2;

            q1.imag.x = k[0].Quat[0] * (1.0f / SHRT_MAX);
            q1.imag.y = k[0].Quat[1] * (1.0f / SHRT_MAX);
            q1.imag.z = k[0].Quat[2] * (1.0f / SHRT_MAX);
            q1.real = k[0].Quat[3] * (1.0f / SHRT_MAX);

            q2.imag.x = k[1].Quat[0] * (1.0f / SHRT_MAX);
            q2.imag.y = k[1].Quat[1] * (1.0f / SHRT_MAX);
            q2.imag.z = k[1].Quat[2] * (1.0f / SHRT_MAX);
            q2.real = k[1].Quat[3] * (1.0f / SHRT_MAX);

            RtQuatSetupSlerpCache(&q1, &q2, &qcache);
            RtQuatSlerp((RtQuat*)quat, &q1, &q2, lerp, &qcache);

            tran->x =
                lerp * (scalex * k[1].Tran[0] - scalex * k[0].Tran[0]) + scalex * k[0].Tran[0];
            tran->y =
                lerp * (scaley * k[1].Tran[1] - scaley * k[0].Tran[1]) + scaley * k[1].Tran[1];
            tran->z =
                lerp * (scalez * k[1].Tran[2] - scalez * k[0].Tran[2]) + scalez * k[1].Tran[2];
        }
    }
}

//                                                          iAnimBlend

// non-matching: incorrect instruction order and regalloc
void iAnimBlend(F32 BlendFactor, F32 BlendRecip, U16* BlendTimeOffset, F32* BoneTable,
                U32 BoneCount, xVec3* Tran1, xQuat* Quat1, xVec3* Tran2, xQuat* Quat2,
                xVec3* TranDest, xQuat* QuatDest)
{
    U32 i;
    U32 invert = 0;
    RtQuat* q2;
    RtQuat ident = { 0.0f, 0.0f, 0.0f, 1.0f };
    xVec3* t2;

    if (!Quat2)
    {
        q2 = &ident;
        invert = 1;
        t2 = (xVec3*)&ident.imag;
    }
    else
    {
        q2 = (RtQuat*)Quat2;
        t2 = Tran2;
    }

    if (BlendFactor < 0.0f)
    {
        BlendFactor = -BlendFactor;
        invert ^= 1;
    }

    if (!BoneTable && !BlendTimeOffset)
    {
        F32 lerp = BlendFactor * BlendRecip;

        if (lerp < 0.0f)
        {
            lerp = 0.0f;
        }
        else if (lerp > 1.0f)
        {
            lerp = 1.0f;
        }

        if (invert)
        {
            lerp = 1.0f - lerp;
        }

        if (Quat1)
        {
            // non-matching: 0.0f constant is loaded outside of loop

            for (i = 0; i < BoneCount; i++)
            {
                RtQuatSlerpCache qcache;

                RtQuatSetupSlerpCache((RtQuat*)Quat1, q2, &qcache);
                RtQuatSlerp((RtQuat*)QuatDest, (RtQuat*)Quat1, q2, lerp, &qcache);

                Quat1++;

                if (Quat2)
                {
                    q2++;
                }

                QuatDest++;
            }
        }

        if (Tran1)
        {
            if (Quat2)
            {
                for (i = 0; i < BoneCount; i++, TranDest++, t2++, Tran1++)
                {
                    TranDest->x = lerp * (t2->x - Tran1->x) + Tran1->x;
                    TranDest->y = lerp * (t2->y - Tran1->y) + Tran1->y;
                    TranDest->z = lerp * (t2->z - Tran1->z) + Tran1->z;
                }
            }
            else
            {
                for (i = 0; i < BoneCount; i++, TranDest++, Tran1++)
                {
                    TranDest->x = lerp * (t2->x - Tran1->x) + Tran1->x;
                    TranDest->y = lerp * (t2->y - Tran1->y) + Tran1->y;
                    TranDest->z = lerp * (t2->z - Tran1->z) + Tran1->z;
                }
            }
        }
    }
    else
    {
        F32 baselerp;

        if (!BlendTimeOffset)
        {
            baselerp = BlendFactor * BlendRecip;

            if (baselerp < 0.0f)
            {
                baselerp = 0.0f;
            }
            else if (baselerp > 1.0f)
            {
                baselerp = 1.0f;
            }

            if (invert)
            {
                baselerp = 1.0f - baselerp;
            }
        }

        for (i = 0; i < BoneCount; i++)
        {
            F32 lerp;

            if (BlendTimeOffset)
            {
                baselerp = -(1 / 1024.0f * BlendTimeOffset[i * 2] - BlendFactor);

                if (BlendTimeOffset[i * 2 + 1] != 0)
                {
                    baselerp *= 1 / 1024.0f * BlendTimeOffset[i * 2 + 1];

                    if (baselerp < 0.0f)
                    {
                        baselerp = 0.0f;
                    }
                    else if (baselerp > 1.0f)
                    {
                        baselerp = 1.0f;
                    }
                }
                else
                {
                    if (baselerp < 0.0f)
                    {
                        baselerp = 0.0f;
                    }
                    else
                    {
                        baselerp = 1.0f;
                    }
                }

                if (invert)
                {
                    baselerp = 1.0f - baselerp;
                }
            }

            if (BoneTable)
            {
                lerp = baselerp * BoneTable[i];
            }
            else
            {
                lerp = baselerp;
            }

            if (Quat1)
            {
                RtQuatSlerpCache qcache;

                RtQuatSetupSlerpCache((RtQuat*)Quat1, q2, &qcache);
                RtQuatSlerp((RtQuat*)QuatDest, (RtQuat*)Quat1, q2, lerp, &qcache);

                Quat1++;

                if (Quat2)
                {
                    q2++;
                }

                QuatDest++;
            }

            if (Tran1)
            {
                TranDest->x = lerp * (t2->x - Tran1->x) + Tran1->x;
                TranDest->y = lerp * (t2->y - Tran1->y) + Tran1->y;
                TranDest->z = lerp * (t2->z - Tran1->z) + Tran1->z;

                Tran1++;

                if (Quat2)
                {
                    t2++;
                }

                TranDest++;
            }
        }
    }
}

U32 iAnimBoneCount(void* RawData)
{
    if (*(U32*)RawData == '1BKS')
    {
        return ((iAnimSKBHeader*)RawData)->BoneCount;
    }

    return 0;
}

F32 iAnimDuration(void* RawData)
{
    //return iAnimDurationSKB((iAnimSKBHeader*)RawData);
}

void iAnimEval(void* RawData, F32 time, U32 flags, xVec3* tran, xQuat* quat)
{
    iAnimEvalSKB((iAnimSKBHeader*)RawData, time, flags, tran, quat);
}

void iAnimInit()
{
}
