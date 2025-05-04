#include "xWad1.h"
#include "dolphin/types.h"
#include "types.h"

//                                           xCutscene

extern xCutscene sActiveCutscene;
extern xCutsceneInfo* sCutTocInfo;
extern U32 sCutTocCount;
extern void* RwEngineInstance;
extern xModelInstance sCutsceneFakeModel[8];

extern F32 _672; // 1.0

extern F32 lbl_803CCB3C; // 0.0
extern F32 lbl_803CCB40; // 0.033333335

xCutscene* xCutscene_CurrentCutscene()
{
    return &sActiveCutscene;
}

S32 xCutscene_LoadStart(xCutscene* csn)
{
    U32 cnt;

    cnt = iCSFileOpen(csn);

    if (cnt <= 0)
    {
        return 0;
    }

    csn->Opened = 1;
    return 1;
}

S32 xCutscene_Destroy(xCutscene* csn)
{
    csn->Ready = 0;
    xSndSetExternalCallback(0);
    if (csn->SndStarted != 0)
    {
        xSndStop(csn->SndHandle[0]);
        if (csn->SndNumChannel == 2)
        {
            xSndStop(csn->SndHandle[1]);
        }
        xSndUpdate();
        csn->SndStarted = 0;
    }
    xSndPauseAll(0, 0);
    xSndUpdate();
    if (csn->Opened != 0)
    {
        iCSFileClose(csn);
    }
    for (int i = 0; i < csn->Info->NumData; i++)
    {
        // if ((((U32*)csn->Data->DataType + i) & 0x80000000) &&
        //     ((RpAtomic*)((U32*)((U32*)csn->Data->DataType + i) + 3) != NULL))
        // {
        //     if ((((U32*)csn->Data->DataType + i) & 0xfffffff) == 6)
        //     {
        //         RwFree();
        //     }
        //     else
        //     {
        //         iModelUnload((RpAtomic*)((U32*)((U32*)csn->Data->DataType + i) + 3));
        //     }
        //     (U32)((U32*)csn->Data->DataType + i) = (((U32*)csn->Data->DataType + i) & 0xfffffff);
        // }
    }
    RwFree(csn->RawBuf);
    memset(csn, 0, sizeof(xCutscene));
    return 1;
}

// Damn RwEngineInstance ruining this (as well as the members being accessed incorrectly by Ghidra)
xCutscene* xCutscene_Create(U32 id, U32)
{
    xCutscene* csn;
    xCutsceneInfo* cnfo;
    U32 maxload;

    xSndPauseAll(1, 1);
    memset(&sActiveCutscene, 0, 0x198);
    sActiveCutscene.PlaybackSpeed = _672;

    for (int i = 0; i < sCutTocCount; i++)
    {
        cnfo = &sCutTocInfo[i];
        if (sCutTocInfo[i].AssetID == id)
        {
            break;
        }
    }
    maxload = cnfo->MaxBufEven + cnfo->MaxBufOdd;
    if (cnfo->MaxModel > maxload)
    {
        maxload = cnfo->MaxModel;
    }
    // sActiveCutscene.RawBuf = RwFree(maxload + 0x3c);
    sActiveCutscene.AlignBuf = sActiveCutscene.RawBuf;
    while ((int)sActiveCutscene.AlignBuf & 0x3f != 0)
    {
        sActiveCutscene.AlignBuf = (void*)((int)sActiveCutscene.AlignBuf + 4);
    }
    sActiveCutscene.TimeChunkOffs = (U32*)(cnfo[1].SoundLeft + cnfo->NumData * 0x10 + -0x30);
    sActiveCutscene.Info = cnfo;
    sActiveCutscene.Data = (xCutsceneData*)(cnfo + 1);
    sActiveCutscene.Visibility = sActiveCutscene.TimeChunkOffs + cnfo->NumTime + 1;
    sActiveCutscene.BreakList =
        (xCutsceneBreak*)(sActiveCutscene.TimeChunkOffs + cnfo->NumTime + 1 + cnfo->VisSize);
    sActiveCutscene.Play = (xCutsceneTime*)sActiveCutscene.AlignBuf;
    sActiveCutscene.Stream = (xCutsceneTime*)((int)sActiveCutscene.AlignBuf + cnfo->MaxBufEven);
    return &sActiveCutscene;
}

// Non-matching: scheduling
void xCutscene_Init(void* toc)
{
    memset(&sActiveCutscene, 0, 0x198);
    sCutTocCount = 0;
    sCutTocInfo = 0;
    if (toc != NULL)
    {
        sCutTocCount = *(S32*)toc;
        sCutTocInfo = (xCutsceneInfo*)((S32*)toc + 1);
    }
    for (int i = 0; i < 8; i++)
    {
        *(volatile int*)&sCutsceneFakeModel[i].Mat =
            (volatile int)xMemAlloc(gActiveHeap, sizeof(RwMatrixTag) * 65, 0);
        *(volatile int*)&sCutsceneFakeModel[i].Bucket =
            (volatile int)xMemAlloc(gActiveHeap, sizeof(xModelBucket*) * 2, 0);
        *(volatile int*)sCutsceneFakeModel[i].Bucket =
            (volatile int)xMemAlloc(gActiveHeap, sizeof(xModelBucket), 0);
        *(volatile int*)&sCutsceneFakeModel[i].Bucket[1] =
            (volatile int)*sCutsceneFakeModel[i].Bucket;
        *(volatile int*)&sCutsceneFakeModel[i].PipeFlags = (volatile int)0x6530;
        *(volatile float*)&sCutsceneFakeModel[i].RedMultiplier = (volatile float)1.0f;
        *(volatile float*)&sCutsceneFakeModel[i].GreenMultiplier = (volatile float)1.0f;
        *(volatile float*)&sCutsceneFakeModel[i].BlueMultiplier = (volatile float)1.0f;
        *(volatile float*)&sCutsceneFakeModel[i].Alpha = (volatile float)1.0f;
    }
}

//                                              xCounter

int xCounterEventCB(xBase*, xBase* to, U32 toEvent, const F32*, xBase*, U32)
{
    _xCounter* t = (_xCounter*)to;

    switch (toEvent)
    {
    case eEventDecrement:
    {
        if (t->state != 1 && (!(t->counterFlags & 0x1) || t->count != 2))
        {
            t->count--;

            if (t->count == 0)
            {
                zEntEvent(t, t, eEventExpired);
            }
            else if (t->count > 0 && t->count <= 20)
            {
                zEntEvent(t, t, t->count + (eEventCount1 - 1));
            }
        }
        break;
    }
    case eEventIncrement:
    {
        if (t->state != 1)
        {
            t->count++;

            if (t->count == 0)
            {
                zEntEvent(t, t, eEventExpired);
            }
            else if (t->count > 0 && t->count <= 20)
            {
                zEntEvent(t, t, t->count + (eEventCount1 - 1));
            }
        }
        break;
    }
    case eEventReset:
    {
        t->state = 0;
        t->count = t->asset->count;
        break;
    }
    case eEventExpired:
    {
        if (!(t->counterFlags & 0x1) || t->count != 2)
        {
            t->count = 0;
            t->state = 1;
        }
        break;
    }
    default:
    {
        if (t->state != 1 && toEvent >= eEventCount1 && toEvent <= eEventCount20)
        {
            S16 newCount = toEvent - (eEventCount1 - 1);

            if (!(t->counterFlags & 0x1) || t->count != 2)
            {
                t->count = newCount;
            }
        }
    }
    }

    return 1;
}

void xCounterLoad(_xCounter* ent, xSerial* s)
{
    xBaseLoad(ent, s);

    s->Read(&ent->state);
    s->Read(&ent->count);
}

void xCounterSave(_xCounter* ent, xSerial* s)
{
    xBaseSave(ent, s);

    s->Write(ent->state);
    s->Write(ent->count);
}

void xCounterReset(xBase* b)
{
    _xCounter* t = (_xCounter*)b;

    xBaseInit(t, t->asset);

    if (t->linkCount)
    {
        t->link = (xLinkAsset*)(t->asset + 1);
    }
    else
    {
        t->link = 0;
    }

    t->count = t->asset->count;
    t->state = 0;
}

void xCounterInit(void* b, void* asset)
{
    xCounterInit((xBase*)b, (xCounterAsset*)asset);
}

void xCounterInit()
{
}

//                                         xSwept

//                                          xRay

U32 xRayHitsBoxFast(const xRay3* r, const xBox* b)
{
    xIsect isect;
    iBoxIsectRay(b, r, &isect);
    return isect.penned <= 0.0f || isect.contained <= 0.0f;
}

U32 xRayHitsSphereFast(const xRay3* r, const xSphere* s)
{
    xVec3 length;
    xVec3Sub(&length, &r->origin, &s->center);
    F32 dVar3 = xVec3Dot(&length, &length) - SQR(s->r);
    if (dVar3 <= 0.0f)
    {
        return 1;
    }

    if (r->flags & (1 << 11) && dVar3 > r->max_t * (2.0f * s->r + r->max_t))
    {
        return 0;
    }

    F32 dot = xVec3Dot(&length, &r->dir);
    return (dot >= 0.0f) ? 0 : (SQR(dot) >= dVar3);
}

//                                          xSphere

//                                          xCM

volatile static F32 credits_time;
static xCreditsData* credits_data;
static U32 credits_parentID;

F32 dtscale;

// Equivalent: scheduling
void xCMstop()
{
    credits_time = 10000.0f;
    credits_data = NULL;
    if (credits_parentID != 0)
    {
        zEntEvent(credits_parentID, 0x212);
    }
}

void xCMstart(xCreditsData* data, F32 time, xBase* parent)
{
    if ((credits_data = data) != NULL)
    {
        credits_time = 0.0f;

        if (parent != NULL)
        {
            credits_parentID = parent->id;
        }
    }
}

void xCMrender()
{
    xCMrender(credits_time, credits_data);
}

void xCMupdate(F32 time)
{
    if (credits_data != 0)
    {
        credits_time += (time * dtscale);
        if (credits_time >= *(F32*)((int)&credits_data->dummy + 0x10))
        {
            xCMstop();
        }
    }
}

static void xCMprep(xCreditsData* data)
{
    /*
        data seems to point to this general structure:
        0x00 - xCMheader
        0x18 - xCMcredits
        0x50 - xCMpreset* presets_array[xCMcredits.num_presets]
             - xCMhunk* hunk_array[?]
    */

    char* dp; // In the DWARF. Don't know what this is used for.
    xCMheader* hdr = (xCMheader*)data;

    if (hdr->magic != 0xBEEEEEEF)
    {
        return;
    }

    xCMcredits* cp = (xCMcredits*)(&hdr[1]);
    while ((int)cp - (int)data < hdr->total_size)
    {
        xCMpreset* pp = (xCMpreset*)(&cp[1]);
        xCMhunk* hp = (xCMhunk*)(&pp[cp->num_presets]);

        for (; (int)hp - (int)cp < cp->credits_size; hp = (xCMhunk*)((int)hp + hp->hunk_size))
        {
            switch (pp[hp->preset].align)
            {
            case 0:
            case 1:
            case 2:
            case 3:

                if (hdr->state == 0)
                {
                    if (hp->text1 != NULL)
                    {
                        hp->text1 -= (int)data;
                    }
                    if (hp->text2 != NULL)
                    {
                        hp->text2 -= (int)data;
                    }
                }
                else
                {
                    if (hp->text1 != NULL)
                    {
                        hp->text1 += (int)data;
                    }
                    if (hp->text2 != NULL)
                    {
                        hp->text2 += (int)data;
                    }
                }
                break;
            }
        }
        cp = (xCMcredits*)hp;
    }

    hdr->state = hdr->state == 0 ? 1 : 0;
}

//                                              xClump

//                                              xClimate

_tagClimate* sClimate;

const float snow_life = 3.0f;
const xVec3 snow_vel = { 0.0f, -2.0f, 0.0f };
const xVec3 snow_dvel = { 0.1f, 0.1f, 0.1f };

void xClimateVecFromAngle(F32 angleDegrees, xVec3* vec)
{
    xMat3x3 Mat;

    xMat3x3Identity(&Mat);
    xMat3x3Euler(&Mat, (PI * angleDegrees) / ONEEIGHTY, 0.0f, 0.0f);
    xVec3Init(vec, 0.0f, 0.0f, 1.0f);
    xMat3x3LMulVec(vec, &Mat, vec);
}

void xClimateUpdate(_tagClimate* climate, F32 seconds)
{
    UpdateRain(climate, seconds);
    UpdateWind(climate, seconds);
}

void xClimateSetRain(F32 stre)
{
    sClimate->rain.rain = 1;
    sClimate->rain.strength = stre;
}

void xClimateSetSnow(F32 stre)
{
    sClimate->rain.rain = 0;
    sClimate->rain.strength = stre;
}

// Equivalent
// float ops are being optimized more aggressively
void xClimateInitAsset(_tagClimate* climate, xEnvAsset* asset)
{
    sClimate = climate;
    climate->wind.strength = 0.0f;
    xClimateVecFromAngle(climate->wind.angle, &climate->wind.dir);

    if (asset->climateFlags == 0)
    {
        climate->wind.strength = 0.0f;
        climate->rain.strength = 0.0f;
        return;
    }
    if (asset->climateFlags & 1)
    {
        climate->rain.rain = 1.0f;
        climate->rain.strength = 0.5f * (asset->climateStrengthMax - asset->climateStrengthMin);
        climate->rain.strength += asset->climateStrengthMin;
    }
    else if (asset->climateFlags & 2)
    {
        climate->rain.rain = 0.0f;
        climate->rain.strength = 0.5f * (asset->climateStrengthMax - asset->climateStrengthMin);
        climate->rain.strength += asset->climateStrengthMin;
    }
}

void xClimateInit(_tagClimate* climate)
{
    climate->rain.strength = 0.0f;

    climate->rain.rain_emitter = zParEmitterFind(xStrHash("PAREMIT_RAIN"));
    climate->rain.rain_emitter->emit_flags &= 0xfe;

    climate->rain.snow_emitter = zParEmitterFind(xStrHash("PAREMIT_SNOW"));
    climate->rain.snow_emitter->emit_flags &= 0xfe;
}

//                                              xCam

//                                              xBound

extern F32 _571;
extern F32 _640;
extern F32 _641;
extern F32 _642;
extern F32 _643;
extern F32 _644;

void xBoundUpdate(xBound* b)
{
    if (b->type == XBOUND_TYPE_BOX)
    {
        xVec3Add(&b->box.center, &b->box.box.lower, &b->box.box.upper);
        xVec3SMul(&b->box.center, &b->box.center, _571);
    }
    else if (b->type == XBOUND_TYPE_OBB)
    {
        xVec3Add(&b->box.center, &b->box.box.lower, &b->box.box.upper);
        xVec3SMul(&b->box.center, &b->box.center, _571);
        xMat4x3Toworld(&b->box.center, b->mat, &b->box.center);
    }

    if (b->type != XBOUND_TYPE_NA)
    {
        xQuickCullForBound(&b->qcd, b);
    }
}

void xBoundGetBox(xBox& box, const xBound& bound)
{
    switch (bound.type)
    {
    case XBOUND_TYPE_SPHERE:
    {
        const xSphere& o = bound.sph;

        box.upper.assign(o.center.x + o.r, o.center.y + o.r, o.center.z + o.r);
        box.lower.assign(o.center.x - o.r, o.center.y - o.r, o.center.z - o.r);

        break;
    }
    case XBOUND_TYPE_BOX:
    {
        box = bound.box.box;

        break;
    }
    case XBOUND_TYPE_OBB:
    {
        xBoxInitBoundOBB(&box, &bound.box.box, bound.mat);

        break;
    }
    }
}

void xBoundGetSphere(xSphere& o, const xBound& bound)
{
    switch (bound.type)
    {
    case XBOUND_TYPE_SPHERE:
    {
        o = bound.sph;

        break;
    }
    case XBOUND_TYPE_BOX:
    {
        o.center = bound.box.center;
        o.r = (bound.box.box.upper - bound.box.center).length();

        break;
    }
    case XBOUND_TYPE_OBB:
    {
        const xMat4x3& mat = *bound.mat;
        xVec3 v = (bound.box.box.upper - bound.box.box.lower) * _571;

        o.r = xsqrt(SQR(v.x) * mat.right.length2() + SQR(v.y) * mat.up.length2() +
                    SQR(v.z) * mat.at.length2());
        o.center = bound.box.center;

        break;
    }
    }
}

F32 xsqrt(F32 x)
{
    const F32 half = _571;
    const F32 three = _640;

    if (x <= _641 || isinf(x))
    {
        return x;
    }

    // non-matching: frsp instruction

    F32 guess = __frsqrte(x);
    guess = half * guess * (three - guess * guess * x);

    if (guess > _644)
    {
        return _642 / guess;
    }

    return _643;
}

U32 xBoundSphereHitsOBB(const xSphere* s, const xBox* b, const xMat4x3* m, xCollis* coll)
{
    return xSphereHitsOBB_nu(s, b, m, coll);
}

void xBoundHitsBound(const xBound* a, const xBound* b, xCollis* c)
{
    if (!xQuickCullIsects(&a->qcd, &b->qcd))
    {
        c->flags &= ~0x1;
    }
    else
    {
        // non-matching: jumptable

        switch (a->type | (b->type << 3))
        {
        case (XBOUND_TYPE_SPHERE | (XBOUND_TYPE_SPHERE << 3)):
        {
            xSphereHitsSphere(&a->sph, &b->sph, c);
            break;
        }
        case (XBOUND_TYPE_SPHERE | (XBOUND_TYPE_OBB << 3)):
        {
            xBoundSphereHitsOBB(&a->sph, &b->box.box, b->mat, c);
            break;
        }
        case (XBOUND_TYPE_SPHERE | (XBOUND_TYPE_BOX << 3)):
        {
            xSphereHitsBox(&a->sph, &b->box.box, c);
            break;
        }
        case (XBOUND_TYPE_BOX | (XBOUND_TYPE_SPHERE << 3)):
        {
            xBoxHitsSphere(&a->box.box, &b->sph, c);
            break;
        }
        case (XBOUND_TYPE_BOX | (XBOUND_TYPE_OBB << 3)):
        {
            xBoxHitsObb(&a->box.box, &b->box.box, b->mat, c);
            break;
        }
        case (XBOUND_TYPE_OBB | (XBOUND_TYPE_OBB << 3)):
        {
            break;
        }
        }
    }
}

static void xBoundOBBIsectRay(const xBox* b, const xMat4x3* m, const xRay3* r, xIsect* isect)
{
    xRay3 xfr;
    xBox sbox;
    xVec3 scale;
    xMat4x3 mnormal;

    {
        F32 len2 = SQR(m->right.x) + SQR(m->right.y) + SQR(m->right.z);

        if ((F32)iabs(len2 - _642) <= _644)
        {
            // non-matching: incorrect instruction + order

            scale.x = *(const F32*)&_642;

            mnormal.right.x = m->right.x;
            mnormal.right.y = m->right.y;
            mnormal.right.z = m->right.z;
        }
        else if ((F32)iabs(len2) <= _644)
        {
            // non-matching: incorrect order

            scale.x = _641;

            mnormal.right.x = _641;
            mnormal.right.y = _642;
            mnormal.right.z = _641;
        }
        else
        {
            scale.x = xsqrt(len2);

            F32 len_inv = _642 / scale.x;

            mnormal.right.x = m->right.x * len_inv;
            mnormal.right.y = m->right.y * len_inv;
            mnormal.right.z = m->right.z * len_inv;
        }
    }

    {
        F32 len2 = SQR(m->up.x) + SQR(m->up.y) + SQR(m->up.z);

        if ((F32)iabs(len2 - _642) <= _644)
        {
            // non-matching: incorrect instruction + order

            scale.y = *(const F32*)&_642;

            mnormal.up.x = m->up.x;
            mnormal.up.y = m->up.y;
            mnormal.up.z = m->up.z;
        }
        else if ((F32)iabs(len2) <= _644)
        {
            // non-matching: incorrect order

            scale.y = _641;

            mnormal.up.x = _641;
            mnormal.up.y = _642;
            mnormal.up.z = _641;
        }
        else
        {
            scale.y = xsqrt(len2);

            F32 len_inv = _642 / scale.y;

            mnormal.up.x = m->up.x * len_inv;
            mnormal.up.y = m->up.y * len_inv;
            mnormal.up.z = m->up.z * len_inv;
        }
    }

    {
        F32 len2 = SQR(m->at.x) + SQR(m->at.y) + SQR(m->at.z);

        if ((F32)iabs(len2 - _642) <= _644)
        {
            // non-matching: incorrect instruction + order

            scale.z = *(const F32*)&_642;

            mnormal.at.x = m->at.x;
            mnormal.at.y = m->at.y;
            mnormal.at.z = m->at.z;
        }
        else if ((F32)iabs(len2) <= _644)
        {
            // non-matching: incorrect order

            scale.z = _641;

            mnormal.at.x = _641;
            mnormal.at.y = _642;
            mnormal.at.z = _641;
        }
        else
        {
            scale.z = xsqrt(len2);

            F32 len_inv = _642 / scale.z;

            mnormal.at.x = m->at.x * len_inv;
            mnormal.at.y = m->at.y * len_inv;
            mnormal.at.z = m->at.z * len_inv;
        }
    }

    mnormal.pos = m->pos;

    sbox.upper.x = b->upper.x * scale.x;
    sbox.upper.y = b->upper.y * scale.y;
    sbox.upper.z = b->upper.z * scale.z;
    sbox.lower.x = b->lower.x * scale.x;
    sbox.lower.y = b->lower.y * scale.y;
    sbox.lower.z = b->lower.z * scale.z;

    xMat4x3Tolocal(&xfr.origin, &mnormal, &r->origin);
    xMat3x3Tolocal(&xfr.dir, &mnormal, &r->dir);

    xfr.min_t = r->min_t;
    xfr.max_t = r->max_t;
    xfr.flags = r->flags;

    iBoxIsectRay(&sbox, &xfr, isect);
}

void xRayHitsBound(const xRay3* r, const xBound* b, xCollis* c)
{
    xIsect isect;

    if (b->type == XBOUND_TYPE_SPHERE)
    {
        iSphereIsectRay(&b->sph, r, &isect);
    }
    else if (b->type == XBOUND_TYPE_OBB)
    {
        xBoundOBBIsectRay(&b->box.box, b->mat, r, &isect);
    }
    else if (b->type == XBOUND_TYPE_BOX)
    {
        iBoxIsectRay(&b->box.box, r, &isect);
    }

    if (isect.penned <= _641)
    {
        c->flags |= 0x1;
        c->dist = isect.dist;
    }
    else
    {
        c->flags &= ~0x1;
    }
}

void xSphereHitsBound(const xSphere* o, const xBound* b, xCollis* c)
{
    switch (b->type)
    {
    case XBOUND_TYPE_SPHERE:
    {
        xSphereHitsSphere(o, &b->sph, c);
        break;
    }
    case XBOUND_TYPE_OBB:
    {
        xBoundSphereHitsOBB(o, &b->box.box, b->mat, c);
        break;
    }
    case XBOUND_TYPE_BOX:
    {
        xSphereHitsBox(o, &b->box.box, c);
        break;
    }
    }
}

void xVecHitsBound(const xVec3* v, const xBound* b, xCollis* c)
{
    xIsect isect;

    if (b->type == XBOUND_TYPE_SPHERE)
    {
        iSphereIsectVec(&b->sph, v, &isect);
    }
    else if (b->type == XBOUND_TYPE_OBB)
    {
        xVec3 lv;

        xMat4x3Tolocal(&lv, b->mat, v);
        iBoxIsectVec(&b->box.box, &lv, &isect);
    }
    else if (b->type == XBOUND_TYPE_BOX)
    {
        iBoxIsectVec(&b->box.box, v, &isect);
    }

    if (isect.penned <= _641)
    {
        c->flags |= 0x1;
    }
    else
    {
        c->flags &= ~0x1;
    }
}

void xDrawBox(const xBox*)
{
}

void xDrawOBB(const xBox*, const xMat4x3*)
{
}

void xDrawSphere2(const xSphere*, U32)
{
}

void xBoundDraw(const xBound* b)
{
    if (b->type == XBOUND_TYPE_SPHERE)
    {
        xDrawSphere2(&b->sph, 12);
    }
    else if (b->type == XBOUND_TYPE_OBB)
    {
        xDrawOBB(&b->box.box, b->mat);
    }
    else if (b->type == XBOUND_TYPE_BOX)
    {
        xDrawBox(&b->box.box);
    }
}

//                                      BFD / Async / Read

static U32 g_loadlock = 0xFFFFFF00;
static st_FILELOADINFO g_loadinst[8] = {};
static tag_xFile g_xfload[8] = {};
static st_BINIO_XTRADATA g_xtraload[8] = {};
static st_BINIO_XTRADATA* g_async_context = NULL;

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1

#ifdef GAMECUBE
#define ENDIAN BIG_ENDIAN
#else
#define ENDIAN LITTLE_ENDIAN
#endif

static void BFD_cb_GCP2_readasync(tag_xFile* bffp)
{
}

static S32 BFD_AsyncRead(st_FILELOADINFO* fli, S32 pos, void* data, S32 size, S32 n, S32 endian)
{
    tag_xFile* file = (tag_xFile*)fli->privdata;
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)fli->xtradata;

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    if (g_async_context)
    {
        return 0;
    }

    g_async_context = xtra;

    xtra->asyn_status = BINIO_ASYNC_INPROG;
    xtra->asyn_data = (char*)data;
    xtra->asyn_elesize = size;
    xtra->asyn_amt = n;
    xtra->asyn_ismot = 1;

    iFileSeek(file, pos, IFILE_SEEK_SET);
    xFileSetUserData(file, fli);

    S32 result = iFileReadAsync(file, data, size * n, BFD_cb_GCP2_readasync, 0);
    S32 rc;

    if (result < 0)
    {
        rc = 0;
    }
    else
    {
        xtra->gcaskey = result;
        rc = 1;
    }

    if (rc == 0)
    {
        g_async_context = NULL;
        xtra->asyn_status = BINIO_ASYNC_NOOP;
    }

    return 1;
}

static S32 BFD_read(void* data, S32 elesize, S32 elecnt, tag_xFile* bffp, void* xtradata)
{
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)xtradata;
    char* dest = (char*)data;
    S32 readbeg;
    S32 refill = 0;
    S32 remain;
    S32 actual;
    U32 holdpos;
    U32 numBytes = 0;
    U32 safety = 0;
    S32 r26 = elesize * elecnt;

    if (r26 == 0)
    {
        return 0;
    }

    if (!xtra->dbl_buf || xtra->dblbuf_size < 1 || r26 > xtra->dblbuf_size)
    {
        iFileSeek(bffp, xtra->fpos, IFILE_SEEK_SET);
        numBytes = iFileRead(bffp, dest, r26);

        xtra->fpos += numBytes;
    }
    else
    {
        remain = r26;

        while (true)
        {
            if (xtra->fpos < xtra->dbl_beg)
            {
                refill = 1;

                xtra->dbl_beg = xtra->fpos - (xtra->fpos % xtra->dblbuf_size);
                xtra->dbl_end = xtra->dbl_beg + xtra->dblbuf_size;
                xtra->dbl_amt = 0;
            }
            else if (xtra->fpos >= xtra->dbl_end)
            {
                refill = 1;

                xtra->dbl_beg = xtra->fpos - (xtra->fpos % xtra->dblbuf_size);
                xtra->dbl_end = xtra->dbl_beg + xtra->dblbuf_size;
                xtra->dbl_amt = 0;
            }

            if (refill)
            {
                holdpos = xtra->fpos;

                iFileSeek(bffp, xtra->dbl_beg, IFILE_SEEK_SET);

                xtra->fpos = holdpos;
                xtra->dbl_amt = iFileRead(bffp, xtra->dbl_buf, xtra->dblbuf_size);
            }

            readbeg = xtra->fpos - xtra->dbl_beg;

            if (xtra->dbl_amt > readbeg)
            {
                actual = MIN(remain, xtra->dbl_amt - readbeg);

                memcpy(dest + numBytes, xtra->dbl_buf + readbeg, actual);

                numBytes += actual;
                remain -= actual;

                xtra->fpos += actual;
            }

            if (numBytes == r26 || xtra->dbl_amt < 1 || safety++ > 60000)
            {
                break;
            }
        }
    }

    return (S32)numBytes / elesize;
}

static tag_xFile* BFD_open(const char* filename, const char* mode, U32 lockid, S32, void* xtradata,
                           S32)
{
    tag_xFile* bffp;
    U32 orc; // open return code
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)xtradata;
    S32 xfflg = IFILE_OPEN_READ;

    if (strcmp(mode, "rb") == 0)
    {
        bffp = &g_xfload[lockid];
        xfflg = IFILE_OPEN_READ;
    }
    else if (strcmp(mode, "wb") == 0)
    {
        bffp = NULL;
        xfflg = IFILE_OPEN_WRITE;
    }
    else
    {
        bffp = NULL;
    }

    if (bffp)
    {
        strncpy(bffp->relname, filename, sizeof(bffp->relname) - 1);
        bffp->relname[sizeof(bffp->relname) - 1] = '\0';

        orc = iFileOpen(filename, xfflg, bffp);

        if (orc != 0)
        {
            bffp = NULL;
        }
        else
        {
            xtra->fpos = 0;
            xtra->dbl_buf = NULL;
            xtra->dblbuf_size = 0;
            xtra->dbl_beg = 0;
            xtra->dbl_end = 0;
            xtra->dbl_amt = 0;
        }
    }

    return bffp;
}

static en_BIO_ASYNC_ERRCODES BFD_AsyncReadStatus(st_FILELOADINFO* fli)
{
    en_BIO_ASYNC_ERRCODES status;
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)fli->xtradata;

    if (!g_async_context)
    {
        status = BINIO_ASYNC_NOOP;
    }
    else if (xtra != g_async_context)
    {
        status = BINIO_ASYNC_INPROG;
    }
    else
    {
        if (xtra->asyn_status == BINIO_ASYNC_INPROG)
        {
            XFILE_READSECTOR_STATUS xrdstat;
            S32 amtsofar = 0;

            xrdstat = xFileReadAsyncStatus(xtra->gcaskey, &amtsofar);

            switch (xrdstat)
            {
            case XFILE_RDSTAT_INPROG:
            case XFILE_RDSTAT_QUEUED:
            {
                xtra->asyn_status = BINIO_ASYNC_INPROG;
                break;
            }
            case XFILE_RDSTAT_DONE:
            {
                xtra->asyn_status = BINIO_ASYNC_DONE;
                break;
            }
            case XFILE_RDSTAT_FAIL:
            case XFILE_RDSTAT_EXPIRED:
            default:
            {
                xtra->asyn_status = BINIO_ASYNC_FAIL;
                break;
            }
            }
        }

        status = xtra->asyn_status;

        if (status == BINIO_ASYNC_FAIL || status == BINIO_ASYNC_DONE)
        {
            xtra->asyn_status = BINIO_ASYNC_NOOP;
            g_async_context = NULL;
        }
    }

    return status;
}

static en_BIO_ASYNC_ERRCODES AsyncReadStatus(st_FILELOADINFO* fli)
{
    return BFD_AsyncReadStatus(fli);
}

static S32 AsyncIRead(st_FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n)
{
    return BFD_AsyncRead(fli, offset, data, size, n, LITTLE_ENDIAN);
}

static S32 AsyncMRead(st_FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n)
{
    return BFD_AsyncRead(fli, offset, data, size, n, BIG_ENDIAN);
}

static S32 ReadRaw(st_FILELOADINFO* fli, void* data, S32 size, S32 count)
{
    tag_xFile* file = (tag_xFile*)fli->privdata;

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    S32 amt = (size * count > fli->remain) ? fli->remain / size : count;

    if (amt)
    {
        S32 n = BFD_read(data, size, amt, file, fli->xtradata);

        if (n != amt)
        {
            fli->error = FIOERR_READFAIL;
        }

        fli->remain -= size * amt;
        fli->position += size * amt;
    }

    return amt;
}

static void Swap8(char* d, S32 n)
{
    char t;

    while (n--)
    {
        t = d[0];
        d[0] = d[7];
        d[7] = t;

        t = d[1];
        d[1] = d[6];
        d[6] = t;

        t = d[2];
        d[2] = d[5];
        d[5] = t;

        t = d[3];
        d[3] = d[4];
        d[4] = t;

        d += 8;
    }
}

static S32 ReadIDoubles(st_FILELOADINFO* fli, F64* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(F64), count);

#if ENDIAN == BIG_ENDIAN
    Swap8((char*)data, act);
#endif

    return act;
}

static void Swap4(char* d, S32 n)
{
    char t;

    while (n--)
    {
        t = d[0];
        d[0] = d[3];
        d[3] = t;

        t = d[1];
        d[1] = d[2];
        d[2] = t;

        d += 4;
    }
}

static S32 ReadIFloats(st_FILELOADINFO* fli, F32* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(F32), count);

#if ENDIAN == BIG_ENDIAN
    Swap4((char*)data, act);
#endif

    return act;
}

static S32 ReadILongs(st_FILELOADINFO* fli, S32* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(S32), count);

#if ENDIAN == BIG_ENDIAN
    Swap4((char*)data, act);
#endif

    return act;
}

static void Swap2(char* d, S32 n)
{
    char t;

    while (n--)
    {
        t = d[0];
        d[0] = d[1];
        d[1] = t;

        d += 2;
    }
}

static S32 ReadIShorts(st_FILELOADINFO* fli, S16* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(S16), count);

#if ENDIAN == BIG_ENDIAN
    Swap2((char*)data, act);
#endif

    return act;
}

static S32 ReadMDoubles(st_FILELOADINFO* fli, F64* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(F64), count);

#if ENDIAN == LITTLE_ENDIAN
    Swap8((char*)data, act);
#endif

    return act;
}

static S32 ReadMFloats(st_FILELOADINFO* fli, F32* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(F32), count);

#if ENDIAN == LITTLE_ENDIAN
    Swap4((char*)data, act);
#endif

    return act;
}

static S32 ReadMLongs(st_FILELOADINFO* fli, S32* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(S32), count);

#if ENDIAN == LITTLE_ENDIAN
    Swap4((char*)data, act);
#endif

    return act;
}

static S32 ReadMShorts(st_FILELOADINFO* fli, S16* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(S16), count);

#if ENDIAN == LITTLE_ENDIAN
    Swap2((char*)data, act);
#endif

    return act;
}

static S32 ReadBytes(st_FILELOADINFO* fli, char* data, S32 count)
{
    S32 act = ReadRaw(fli, data, sizeof(char), count);
    return act;
}

static void SetBuffer(st_FILELOADINFO* fli, char* dblbuffer, S32 bufsize)
{
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)fli->xtradata;

    if (xtra)
    {
        xtra->dbl_buf = dblbuffer;
        xtra->dblbuf_size = bufsize;
        xtra->dbl_beg = 0;
        xtra->dbl_end = 0;
        xtra->dbl_amt = 0;
    }
}

static void DiscardBuffer(st_FILELOADINFO* fli)
{
    SetBuffer(fli, NULL, 0);
}

static S32 BFD_seek(tag_xFile* bffp, S32 offset, S32 whence, void* xtradata)
{
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)xtradata;
    S32 pos = iFileSeek(bffp, offset, whence);

    xtra->fpos = offset;

    if (pos == -1)
    {
        return 1;
    }

    return 0;
}

static S32 ReadSeek(st_FILELOADINFO* fli, S32 pos)
{
    tag_xFile* file = (tag_xFile*)fli->privdata;

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    pos = MIN(pos, fli->filesize);

    if (BFD_seek(file, pos, IFILE_SEEK_SET, fli->xtradata) != 0)
    {
        fli->error = FIOERR_SEEKFAIL;
    }
    else
    {
        fli->position = pos;
        fli->remain = fli->filesize - pos;
    }

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    return 1;
}

static S32 SkipBytes(st_FILELOADINFO* fli, S32 fwd)
{
    tag_xFile* file = (tag_xFile*)fli->privdata;

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    if (fwd == 0)
    {
        return 1;
    }

    if (fwd < 0)
    {
        fwd = 0;
    }

    if (BFD_seek(file, fli->position + fwd, IFILE_SEEK_SET, fli->xtradata) != 0)
    {
        fli->error = FIOERR_SEEKFAIL;
    }
    else
    {
        fli->remain -= fwd;
        fli->position += fwd;
    }

    if (fli->error != FIOERR_NONE)
    {
        return 0;
    }

    return 1;
}

static void BFD_close(tag_xFile* bffp, void* xtradata)
{
    st_BINIO_XTRADATA* xtra = (st_BINIO_XTRADATA*)xtradata;

    iFileClose(bffp);

    xtra->fpos = 0;
    xtra->dbl_buf = NULL;
    xtra->dblbuf_size = 0;
    xtra->dbl_beg = 0;
    xtra->dbl_end = 0;
    xtra->dbl_amt = 0;
}

static void LoadDestroy(st_FILELOADINFO* fli)
{
    tag_xFile* fp = (tag_xFile*)fli->privdata;

    if (fp)
    {
        BFD_close(fp, fli->xtradata);
    }

    U32 lockid = fli->lockid;

    memset(fli, 0, sizeof(st_FILELOADINFO));

    g_loadlock &= ~(1 << lockid);
}

static S32 BFD_startSector(const char* filename)
{
    tag_xFile file = {};
    U32 addr = 0;
    U32 length = 0;

    if (!iFileFind(filename, 0, &file))
    {
        iFileGetInfo(&file, &addr, &length);
    }

    iFileClose(&file);

    return addr;
}

static S32 BFD_getLength(tag_xFile* bffp, void* xtradata)
{
    return iFileGetSize(bffp);
}

st_FILELOADINFO* xBinioLoadCreate(const char* filename, S32)
{
    st_FILELOADINFO* fli = NULL;
    tag_xFile* tmp_fp;
    S32 i;
    S32 uselock = -1;

    for (i = 0; i < 8; i++)
    {
        if (!(g_loadlock & (1 << i)))
        {
            g_loadlock |= 1 << i;

            uselock = i;
            fli = &g_loadinst[i];

            break;
        }
    }

    if (fli)
    {
        memset(fli, 0, sizeof(st_FILELOADINFO));

        fli->lockid = uselock & ~(uselock >> 31);
        fli->xtradata = &g_xtraload[uselock];
        fli->destroy = LoadDestroy;
        fli->readBytes = ReadBytes;
        fli->readMShorts = ReadMShorts;
        fli->readMLongs = ReadMLongs;
        fli->readMFloats = ReadMFloats;
        fli->readMDoubles = ReadMDoubles;
        fli->readIShorts = ReadIShorts;
        fli->readILongs = ReadILongs;
        fli->readIFloats = ReadIFloats;
        fli->readIDoubles = ReadIDoubles;
        fli->asyncIRead = AsyncIRead;
        fli->asyncMRead = AsyncMRead;
        fli->asyncReadStatus = AsyncReadStatus;
        fli->skipBytes = SkipBytes;
        fli->seekSpot = ReadSeek;
        fli->setDoubleBuf = SetBuffer;
        fli->discardDblBuf = DiscardBuffer;
        fli->error = FIOERR_NONE;
        fli->remain = 0;
        fli->basesector = 0;
        fli->filesize = 0;
        fli->position = 0;
        fli->basesector = 0;

        tmp_fp = BFD_open(filename, "rb", uselock, 0, fli->xtradata, 0);

        if (tmp_fp)
        {
            fli->basesector = BFD_startSector(filename);
            fli->privdata = tmp_fp;
            fli->filesize = BFD_getLength(tmp_fp, fli->xtradata);
            fli->remain = fli->filesize;
        }
        else
        {
            LoadDestroy(fli);
            fli = NULL;
        }
    }

    return fli;
}

//                                                  xGoal

extern S32 g_modinit_xBehaveMgr;
extern xBehaveMgr* g_behavmgr;

xFactory* xBehaveMgr_GoalFactory()
{
    return g_behavmgr->GetFactory();
}

void xPsyche::BrainEnd()
{
    xBehaveMgr_GoalFactory()->GrowDataDisable();
    this->psystat = PSY_STAT_THINK;
}

void xPsyche::BrainBegin()
{
    xFactory* factory = xBehaveMgr_GoalFactory();
    this->psystat = PSY_STAT_GROW;
    factory->GrowDataEnable(&this->fakebase, false);
}

void xBehaveMgr::UnSubscribe(xPsyche* psy)
{
    psy->KillBrain(this->goalFactory);
    XOrdRemove(&this->psylist, psy, -1);
}

xPsyche* xBehaveMgr::Subscribe(xBase* owner, S32 i)
{
    xPsyche* psyche = &this->psypool[this->psylist.cnt];
    XOrdAppend(&this->psylist, psyche);
    psyche->FreshWipe();
    psyche->SetOwner(owner, NULL);
    return psyche;
}

xBehaveMgr* xBehaveMgr_GetSelf()
{
    return g_behavmgr;
}

void xBehaveMgr_Shutdown()
{
    g_modinit_xBehaveMgr--;
    if (g_modinit_xBehaveMgr == 0)
    {
        if (g_behavmgr != NULL)
        {
            delete g_behavmgr;
        }
        g_behavmgr = NULL;
    }
}

void xBehaveMgr_Startup()
{
    if (g_modinit_xBehaveMgr++ == 0)
    {
        g_behavmgr = new ('BMGR', NULL) xBehaveMgr();
        g_behavmgr->Startup(0xfa, 0xfa);
    }
}

//                                            xGoalGeneric

S32 xGoalGeneric::SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam,
                           xBase* toParamWidget, S32* handled)
{
    if (this->fun_sysevent)
    {
        return this->fun_sysevent(this, this->usrData, from, to, toEvent, toParam, toParamWidget,
                                  handled);
    }

    return xGoal::SysEvent(from, to, toEvent, toParam, toParamWidget, handled);
}

S32 xGoalGeneric::Resume(F32 dt, void* updCtxt)
{
    if (this->fun_resume)
    {
        return this->fun_resume(this, this->usrData, dt, updCtxt);
    }

    return xGoal::Resume(dt, updCtxt);
}

S32 xGoalGeneric::Suspend(F32 dt, void* updCtxt)
{
    if (this->fun_suspend)
    {
        return this->fun_suspend(this, this->usrData, dt, updCtxt);
    }

    return xGoal::Suspend(dt, updCtxt);
}

S32 xGoalGeneric::Exit(F32 dt, void* updCtxt)
{
    if (this->fun_exit)
    {
        return this->fun_exit(this, this->usrData, dt, updCtxt);
    }

    return xGoal::Exit(dt, updCtxt);
}

S32 xGoalGeneric::Enter(F32 dt, void* updCtxt)
{
    if (this->fun_enter)
    {
        return this->fun_enter(this, this->usrData, dt, updCtxt);
    }

    return xGoal::Enter(dt, updCtxt);
}

void GOALDestroy_Generic(xFactoryInst* item)
{
    delete item;
}

xFactoryInst* GOALCreate_Generic(S32 who, RyzMemGrow* growCtxt, void*)
{
    xGoal* goal = NULL;

    switch (who)
    {
    case 'GSM\x00':
    {
        goal = new (who, growCtxt) xGoalGeneric(who);
        break;
    }
    case 'GSM\x01':
    {
        goal = new (who, growCtxt) xGoalEmpty(who);
        break;
    }
    }

    return goal;
}

//                                              xBase

void xBaseReset(xBase* xb, xBaseAsset* asset)
{
    xb->baseFlags = (xb->baseFlags & 0x10) | (asset->baseFlags & ~0x10);

    xBaseValidate(xb);
}

void xBaseLoad(xBase* ent, xSerial* s)
{
    S32 b = 0;
    s->Read_b1(&b);

    if (b)
    {
        xBaseEnable(ent);
    }
    else
    {
        xBaseDisable(ent);
    }
}

void xBaseSave(xBase* ent, xSerial* s)
{
    if (xBaseIsEnabled(ent))
    {
        s->Write_b1(1);
    }
    else
    {
        s->Write_b1(0);
    }
}

void xBaseSetup(xBase* xb)
{
    return;
}

void xBaseInit(xBase* xb, const xBaseAsset* asset)
{
    xb->id = asset->id;
    xb->baseType = asset->baseType;
    xb->baseFlags = asset->baseFlags;
    xb->linkCount = asset->linkCount;
    xb->link = NULL;

    xBaseValidate(xb);
}

//                                          xAntenna

//                                          xAnim

static xMemPool sxAnimTempTranPool;

U32 gxAnimUseGrowAlloc = 0;

static void EffectSingleStop(xAnimSingle* single)
{
    if (single->State == NULL || single->LastTime == -1.0f)
    {
        return;
    }

    for (U32 i = 0; single->ActiveCount > i && single->ActiveList[i].Effect != NULL; ++i)
    {
        if (single->ActiveList[i].Effect->Flags & 0x2)
        {
            single->ActiveList[i].Effect->Callback(3, &single->ActiveList[i], single,
                                                   single->Play->Object);
        }
    }

    single->ActiveList->Effect = NULL;
    for (xAnimEffect* effect = single->Effect; effect != NULL; effect = effect->Next)
    {
        xAnimActiveEffect tempActive;
        if ((effect->Flags & 0x9) == 0x9)
        {
            tempActive.Effect = effect;
            tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);

            if (effect->Flags & 0x2)
            {
                effect->Callback(3, &tempActive, single, single->Play->Object);
            }
        }
    }

    single->Effect = NULL;
}

void xAnimPoolFree(xAnimPlay* play)
{
    U32 i;

    for (i = 0; i < play->NumSingle; i++)
    {
        EffectSingleStop(&play->Single[i]);

        if (play->Single[i].Blend)
        {
            EffectSingleStop(play->Single[i].Blend);
        }
    }

    xMemPoolFree(play->Pool, play);
}

void xAnimPlaySetup(xAnimPlay* play, void* object, xAnimTable* table, xModelInstance* modelInst)
{
    play->BoneCount = modelInst->BoneCount;
    play->Object = object;
    play->Table = table;
    play->ModelInst = modelInst;

    modelInst->Anim = play;
    modelInst->Flags |= 0x104;

    if (table->MorphIndex != 0)
    {
        modelInst->Flags |= 0x80;
    }

    for (S32 i = 0; i < play->NumSingle; ++i)
    {
        play->Single[i].SingleFlags = (1 << i & table->MorphIndex) ? 0x8000 : 0x1;
        play->Single[i].State = NULL;
        play->Single[i].Tran = NULL;
        if (play->Single[i].Blend != NULL)
        {
            play->Single[i].Blend->State = NULL;
        }
    }
    xAnimPlaySetState(play->Single, table->StateList, 0.0f);
}

xAnimPlay* xAnimPoolAlloc(xMemPool* pool, void* object, xAnimTable* table,
                          xModelInstance* modelInst)
{
    xAnimPlay* play;

    play = (xAnimPlay*)xMemPoolAlloc(pool);

    xAnimPlaySetup(play, object, table, modelInst);

    return play;
}

void xAnimPoolCB(xMemPool* pool, void* data)
{
    S32 i;
    xAnimPlay* clone = (xAnimPlay*)data;
    xAnimPlay* play = (xAnimPlay*)pool->Buffer;
    xAnimSingle* clonesingle = (xAnimSingle*)(clone + 1);
    xAnimSingle* currsingle;
    xAnimActiveEffect* curract;

    clone->NumSingle = play->NumSingle;
    clone->Single = clonesingle;

    clonesingle += play->NumSingle;

    for (i = 0; i < play->NumSingle; i++)
    {
        clone->Single[i].Play = clone;

        if (play->Single[i].Blend)
        {
            clone->Single[i].Blend = clonesingle;

            clonesingle->Blend = NULL;
            clonesingle->Play = clone;

            clonesingle++;
        }
        else
        {
            clone->Single[i].Blend = NULL;
        }
    }

    curract = (xAnimActiveEffect*)clonesingle;

    for (i = 0; i < clone->NumSingle; i++)
    {
        clonesingle = &clone->Single[i];
        currsingle = &play->Single[i];

        while (clonesingle)
        {
            clonesingle->ActiveCount = currsingle->ActiveCount;

            if (currsingle->ActiveCount)
            {
                clonesingle->ActiveList = curract;

                curract += currsingle->ActiveCount;
            }
            else
            {
                clonesingle->ActiveList = NULL;
            }

            currsingle = currsingle->Blend;
            clonesingle = clonesingle->Blend;
        }
    }

    clone->Pool = pool;
}

#define ADD_4_BITS(x) (((x)&1) + (((x) >> 1) & 1) + (((x) >> 2) & 1) + (((x) >> 3) & 1))
void xAnimPoolInit(xMemPool* pool, U32 count, U32 singles, U32 blendFlags, U32 effectMax)
{
    effectMax += effectMax & 1;

    U32 size =
        (effectMax * sizeof(xAnimActiveEffect) + sizeof(xAnimSingle)) *
            (ADD_4_BITS((blendFlags & 0xffff) & ((int)(1 << singles) - 1 >> 0x0)) +
             ADD_4_BITS((blendFlags & 0xffff) & ((int)(1 << singles) - 1 >> 0x4)) +
             ADD_4_BITS((blendFlags & 0xffff) & ((int)(1 << singles) - 1 >> 0x8)) +
             ADD_4_BITS((blendFlags & 0xffff) & ((int)(1 << singles) - 1 >> 0xC)) + singles) +
        sizeof(xAnimPlay);

    U32 i;
    void* buffer = xMemAllocSize(count * size);

    xAnimPlay* play = (xAnimPlay*)buffer;
    play->NumSingle = singles;

    xAnimSingle* currsingle;
    play->Single = currsingle = (xAnimSingle*)((U32)play + sizeof(xAnimPlay));
    currsingle += singles;

    for (i = 0; i < singles; ++i)
    {
        if (blendFlags & (1 << i))
        {
            play->Single[i].Blend = currsingle;
            currsingle->Blend = NULL;
            currsingle++;
        }
        else
        {
            play->Single[i].Blend = NULL;
        }
    }

    xAnimActiveEffect* curract = (xAnimActiveEffect*)currsingle;
    for (i = 0; i < play->NumSingle; ++i)
    {
        currsingle = &play->Single[i];
        while (currsingle)
        {
            currsingle->ActiveCount = effectMax;
            if (effectMax != 0)
            {
                currsingle->ActiveList = curract;
                curract += effectMax;
            }
            else
            {
                currsingle->ActiveList = NULL;
            }

            currsingle = currsingle->Blend;
        }
    }

    play->Pool = pool;
    xMemPoolSetup(pool, buffer, 0, 1, xAnimPoolCB, size, count, count / 2);
}

void xAnimFileEval(xAnimFile* data, F32 time, F32* bilinear, U32 flags, xVec3* tran, xQuat* quat,
                   F32* arg6)
{
    S32 i;
    U32 numBones;
    F32 bilerp[2];
    U32 biindex[2];
    U32 biplus[2];
    xQuat* q0;
    xVec3* t0;
    xQuat* q1;
    xVec3* t1;

    time = xAnimFileRawTime(data, CLAMP(time, 0.0f, data->Duration));
    if (data->FileFlags & 0x8000)
    {
        return;
    }

    if (flags & 0x1)
    {
        numBones = 1;
    }
    else
    {
        numBones = data->BoneCount;
    }

    if (flags & 0x2)
    {
        numBones--;
    }

    if (numBones == 0)
    {
        return;
    }

    if (bilinear != NULL && data->FileFlags & 0x4000)
    {
        for (i = 0; i < 2; ++i)
        {
            F32 f30 = CLAMP(bilinear[i], 0.0f, data->NumAnims[i] - 1);
            f32 t = std::floorf(f30);
            bilerp[i] = f30 - t;
            biindex[i] = t;
            biplus[i] = MIN(biindex[i] + 1, data->NumAnims[i]);
        }

        q0 = (xQuat*)(giAnimScratch + 0x1560);
        t0 = (xVec3*)((U8*)q0 + 0x410);
        if (bilerp[0] && bilerp[1])
        {
            q1 = (xQuat*)(giAnimScratch + 0x1c80);
            t1 = (xVec3*)((U8*)q1 + 0x410);

            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], time, flags, tran,
                      quat);
            iAnimEval(data->RawData[biplus[0] + biindex[1] * data->NumAnims[0]], time, flags, t0,
                      q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);

            iAnimEval(data->RawData[biindex[0] + biplus[1] * data->NumAnims[0]], time, flags, t0,
                      q0);
            iAnimEval(data->RawData[biplus[0] + biplus[1] * data->NumAnims[0]], time, flags, t1,
                      q1);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, t0, q0, t1, q1, t0, q0);
            iAnimBlend(bilerp[1], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);
        }
        else if (bilerp[0])
        {
            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], time, flags, tran,
                      quat);
            iAnimEval(data->RawData[biplus[0] + biindex[1] * data->NumAnims[0]], time, flags, t0,
                      q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);
        }
        else if (bilerp[1])
        {
            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], time, flags, tran,
                      quat);
            iAnimEval(data->RawData[biindex[0] + biplus[1] * data->NumAnims[0]], time, flags, t0,
                      q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);
        }
        else
        {
            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], time, flags, tran,
                      quat);
        }
    }
    else
    {
        iAnimEval(data->RawData[0], time, flags, tran, quat);
    }
}

static void SingleEval(xAnimSingle* single, xVec3* tran, xQuat* quat)
{
    if (single->Play->BoneCount > 1)
    {
        xAnimFileEval(single->State->Data, single->Time, single->BilinearLerp, 0x2, tran + 1,
                      quat + 1, NULL);

        if (single->Blend && single->Blend->State)
        {
            xQuat* qbuf = (xQuat*)(giAnimScratch + 0xE40);
            xVec3* vbuf = (xVec3*)(qbuf + 0x41);

            xAnimFileEval(single->Blend->State->Data, single->Blend->Time,
                          single->Blend->BilinearLerp, 0x2, vbuf, qbuf, NULL);

            iAnimBlend(single->BlendFactor, single->Tran->BlendRecip, single->Tran->BlendOffset,
                       NULL, single->Play->BoneCount - 1, vbuf, qbuf, tran + 1, quat + 1, tran + 1,
                       quat + 1);
        }
    }
}

void xAnimPlayEval(xAnimPlay* play)
{
    U32 i;
    U32 bone;
    xQuat* quatresult = (xQuat*)giAnimScratch;
    xVec3* tranresult = (xVec3*)((U8*)quatresult + 0x410);

    if (play->BoneCount > 1)
    {
        xQuat* quatblend = (xQuat*)((U8*)quatresult + 0x720);
        xVec3* tranblend = (xVec3*)((U8*)quatblend + 0x410);
        SingleEval(play->Single, tranresult, (xQuat*)giAnimScratch);
        for (i = 1; i < play->NumSingle; ++i)
        {
            xAnimSingle* si = &play->Single[i];
            if (si->State == NULL || si->SingleFlags & 0x8000)
            {
                continue;
            }
            F32 blendF = 1.0f;
            F32 blendR = 1.0f;
            U16* blendO = NULL;
            SingleEval(si, tranblend, quatblend);

            if ((si->Blend == NULL || si->Blend->State == NULL) && si->BlendFactor)
            {
                if (si->Tran != NULL)
                {
                    blendF = si->BlendFactor;
                    blendR = si->Tran->BlendRecip;
                    blendO = si->Tran->BlendOffset;
                }
                else
                {
                    blendF = -si->BlendFactor;
                    blendR = si->State->FadeRecip;
                    blendO = si->State->FadeOffset;
                }
            }

            if ((si->SingleFlags & 0x3) == 2)
            {
                iAnimBlend(blendF, blendR, blendO, si->State->BoneBlend, play->BoneCount - 1,
                           tranblend + 1, quatblend + 1, NULL, NULL, tranblend + 1, quatblend + 1);

                for (bone = 1; bone < play->BoneCount; ++bone)
                {
                    tranresult[bone].x += tranblend[bone].x;
                    tranresult[bone].y += tranblend[bone].y;
                    tranresult[bone].z += tranblend[bone].z;
                    xQuatMul(&quatresult[bone], &quatresult[bone], &quatblend[bone]);
                }
            }
            else
            {
                iAnimBlend(blendF, blendR, blendO, si->State->BoneBlend, play->BoneCount - 1,
                           tranresult + 1, quatresult + 1, tranblend + 1, quatblend + 1,
                           tranresult + 1, quatresult + 1);
            }
        }
    }
    memset(tranresult, 0, sizeof(xVec3));
    memset(quatresult, 0, sizeof(xQuat));
    if (play->Single->State->BeforeAnimMatrices != NULL)
    {
        play->Single->State->BeforeAnimMatrices(play, quatresult, tranresult, play->BoneCount);
    }

    if (play->BeforeAnimMatrices != NULL)
    {
        play->BeforeAnimMatrices(play, quatresult, tranresult, play->BoneCount);
    }
    iModelAnimMatrices(play->ModelInst->Data, quatresult, tranresult, &play->ModelInst->Mat[1]);
}

static void EffectActiveInsert(xAnimSingle* single, xAnimActiveEffect* active)
{
    U32 index = 0;
    U32 count = single->ActiveCount;
    xAnimActiveEffect* curr = &single->ActiveList[0];

    while (index < count && curr->Effect)
    {
        curr++;
        index++;
    }

    *curr = *active;

    if (index < count - 1)
    {
        (curr + 1)->Effect = NULL;
    }
}

static void EffectSingleStart(xAnimSingle* single)
{
    xAnimEffect* effect = single->State->Effects;
    xAnimActiveEffect tempActive;
    F32 time = single->Time;
    U32 flags;

    while (effect && effect->StartTime < time)
    {
        flags = effect->Flags;

        if (flags & 0x1)
        {
            if (flags & 0x4)
            {
                tempActive.Effect = effect;
                tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);

                flags = effect->Flags;

                if (flags & 0x2)
                {
                    if (effect->EndTime <= time)
                    {
                        effect->Callback(3, &tempActive, single, single->Play->Object);
                    }
                    else
                    {
                        EffectActiveInsert(single, &tempActive);
                    }
                }
                else if (flags & 0x20)
                {
                    EffectActiveInsert(single, &tempActive);
                }
            }
            else if ((flags & 0x2) && !(flags & 0x10) && effect->EndTime > time)
            {
                tempActive.Effect = effect;
                tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);

                EffectActiveInsert(single, &tempActive);
            }
        }

        effect = effect->Next;
    }

    single->Effect = effect;
}

static void TransitionTimeInit(xAnimSingle* single, xAnimTransition* tran)
{
    if (tran->Flags & 0x20)
    {
        if ((tran->Dest->Data->FileFlags ^ single->State->Data->FileFlags) & 0x1000)
        {
            single->Time = tran->Dest->Data->Duration - single->Time;
        }
    }
    else
    {
        single->Time = ((tran->Dest->Flags & 0x100) && tran->DestTime == 0.0f) ?
                           single->State->Data->Duration * xurand() :
                           tran->DestTime;
    }
    single->LastTime = single->Time;
}

static void StopUpdate(xAnimSingle* single)
{
    if (single->Time > single->State->Data->Duration)
    {
        single->Time = single->State->Data->Duration;
        single->CurrentSpeed = 0.0f;
    }
}

static U32 EffectPlaying(xAnimSingle* single, xAnimEffect* effect)
{
    U32 index = 0;

    while (index < single->ActiveCount && single->ActiveList[index].Effect)
    {
        if (single->ActiveList[index].Effect == effect)
        {
            return TRUE;
        }

        index++;
    }

    return FALSE;
}

static void EffectSingleRun(xAnimSingle* single)
{
    xAnimEffect* effect = single->Effect;
    xAnimActiveEffect tempActive;
    F32 time = single->Time;
    U32 flags;

    while (effect && effect->StartTime <= time)
    {
        flags = effect->Flags;

        if ((flags & 0x1) && (!(flags & 0x20) || !EffectPlaying(single, effect)))
        {
            tempActive.Effect = effect;
            tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);

            if (flags & 0x2)
            {
                if (effect->EndTime <= time)
                {
                    effect->Callback(3, &tempActive, single, single->Play->Object);
                }
                else
                {
                    EffectActiveInsert(single, &tempActive);
                }
            }
            else if (flags & 0x20)
            {
                EffectActiveInsert(single, &tempActive);
            }
        }

        effect = effect->Next;
    }

    single->Effect = effect;
}

static void EffectActiveRemove(xAnimActiveEffect* active, U32 index, U32 count)
{
    count--;

    if (index == count || !(active + 1)->Effect)
    {
        active->Effect = NULL;
    }
    else
    {
        xAnimActiveEffect* curr = active;

        while (index < count && (curr + 1)->Effect)
        {
            curr++;
            index++;
        }

        *active = *curr;
        curr->Effect = NULL;
    }
}

static void EffectSingleLoop(xAnimSingle* single)
{
    EffectSingleRun(single);
    xAnimActiveEffect* alist = single->ActiveList;
    U32 index = 0;
    U32 count = single->ActiveCount;
    while (index < count && alist[index].Effect != NULL)
    {
        if (!(alist[index].Effect->Flags & 0x20))
        {
            alist[index].Effect->Callback(3, &alist[index], single, single->Play->Object);

            EffectActiveRemove(&alist[index], index, count);
        }
        else
        {
            index++;
        }
    }

    xAnimEffect* effect = single->State->Effects;
    while (effect != NULL && effect->StartTime < 0.0f)
    {
        effect = effect->Next;
    }
    single->Effect = effect;
}

static void LoopUpdate(xAnimSingle* single)
{
    F32 time = single->Time;
    F32 duration = single->State->Data->Duration;

    if (time > duration)
    {
        single->Time = duration;

        EffectSingleLoop(single);

        single->Time = time - duration;
    }
}

static void EffectSingleDuration(xAnimSingle* single)
{
    F32 time = single->Time;
    xAnimActiveEffect* alist = single->ActiveList;
    U32 index = 0;
    U32 count = single->ActiveCount;

    while (index < count && alist->Effect)
    {
        if (!(alist->Effect->Flags & 0x20) && alist->Effect->EndTime <= time)
        {
            alist->Effect->Callback(3, alist, single, single->Play->Object);

            EffectActiveRemove(alist, index, count);
        }
        else
        {
            alist->Effect->Callback(2, alist, single, single->Play->Object);

            alist++;
            index++;
        }
    }
}

static void SingleUpdate(xAnimSingle* single, F32 timeDelta)
{
    U32 foundBlendstop = 0;
    U32 firstStep = 1;
    xAnimTransition* foundTransition = NULL;
    xAnimSingle* bl = NULL;
    F32 tranDelta = 0.0f;
    F32 blendDelta = 0.0f;
    F32 singleTime;
    if (single->State == NULL)
    {
        return;
    }

    void* object = single->Play->Object;
    if (single->LastTime == -1.0f)
    {
        EffectSingleStart(single);
    }

    single->LastTime = single->Time;
    singleTime = timeDelta * single->CurrentSpeed + single->Time;

    if (single->Blend != NULL && single->Blend->State != NULL)
    {
        bl = single->Blend;
        if (bl->LastTime == -1.0f)
        {
            EffectSingleStart(bl);
        }
        bl->LastTime = bl->Time;
    }

    F32 duration = single->State->Data->Duration;
    if (single->Sync != NULL)
    {
        // FIXME: assignment in the loop seems unlikely but assigning at the
        // declaration swaps instructions.
        F32 timeCmp;
        if ((timeCmp = single->Sync->SrcTime) != 0.0f)
        {
            if (timeCmp > duration)
            {
                timeCmp = duration;
            }
            if (single->LastTime <= timeCmp && singleTime >= timeCmp)
            {
                foundTransition = single->Sync;
                tranDelta = (singleTime - timeCmp) / single->CurrentSpeed;
                timeDelta = timeDelta - tranDelta;
                if (timeDelta < 0.0f)
                {
                    timeDelta = 0.0f;
                }
            }
            else
            {
                timeCmp += duration;
                if (single->LastTime <= timeCmp && singleTime >= timeCmp)
                {
                    foundTransition = single->Sync;
                    tranDelta = (singleTime - timeCmp) / single->CurrentSpeed;
                    timeDelta = timeDelta - tranDelta;
                    if (timeDelta < 0.0f)
                    {
                        timeDelta = 0.0f;
                    }
                }
            }
        }
        else if (bl == NULL)
        {
            foundTransition = single->Sync;
            tranDelta = timeDelta;
            timeDelta = 0.0f;
        }
    }
    else
    {
        if ((single->State->Flags & 0x30) == 0x20)
        {
            F32 timeCmp = single->State->Default->T->SrcTime;

            if (timeCmp == 0.0f || timeCmp > duration)
            {
                timeCmp = duration;
            }

            if (singleTime >= timeCmp &&
                ((single->State->Default->T->Flags & 0x4) == 0 || bl == NULL))
            {
                xAnimTransitionList* curr = single->State->Default;
                while (curr != NULL && curr->T->Conditional != NULL &&
                       curr->T->Conditional(curr->T, single, object) == 0)
                {
                    curr = curr->Next;
                }

                if (curr == NULL)
                {
                    //fprintf(stderr, "State \"%s\" no default conditionals true!\n",
                    //        single->State->Name);
                    curr = single->State->Default;
                }

                foundTransition = curr->T;
                if (single->LastTime < timeCmp)
                {
                    tranDelta = (singleTime - timeCmp) / single->CurrentSpeed;
                    timeDelta = timeDelta - tranDelta;
                    if (timeDelta < 0.0f)
                    {
                        timeDelta = 0.0f;
                    }
                }
                else
                {
                    tranDelta = timeDelta;
                    timeDelta = 0.0f;
                }
            }
        }
    }

    if (single->BlendFactor != 0.0f)
    {
        F32 recip;
        if (single->Tran != NULL)
        {
            recip = single->Tran->BlendRecip;
        }
        else
        {
            recip = single->State->FadeRecip;
        }

        if (recip * (single->BlendFactor + timeDelta) > 1.0f)
        {
            foundBlendstop = 1;
            blendDelta = (single->BlendFactor + timeDelta) - (1.0f / recip);
            timeDelta = timeDelta - blendDelta;
            if (timeDelta < 0.0f)
            {
                timeDelta = 0.0f;
            }
            if (blendDelta < 0.0f)
            {
                blendDelta = 0.01f;
            }
        }
    }

    do
    {
        if (!firstStep)
        {
            if (foundBlendstop)
            {
                single->BlendFactor = 0.0f;
                if (bl != NULL)
                {
                    EffectSingleStop(bl);
                    bl->State = NULL;
                    bl = NULL;

                    if (single->Tran != NULL && single->Tran->Flags & 0x2)
                    {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = NULL;
                }
                else
                {
                    if (single->Tran != NULL)
                    {
                        if (single->Tran != NULL && single->Tran->Flags & 0x2)
                        {
                            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                        }
                        single->Tran = NULL;
                    }
                    else
                    {
                        EffectSingleStop(single);
                        single->State = NULL;
                        return;
                    }
                }
                timeDelta = blendDelta;
                foundBlendstop = FALSE;
            }
            else
            {
                if (bl != NULL)
                {
                    EffectSingleStop(bl);
                    bl->State = NULL;
                    bl = NULL;
                    single->BlendFactor = 0.0f;
                }
                if (foundTransition->BlendRecip == 0.0f || single->Blend == NULL)
                {
                    EffectSingleStop(single);
                    if (single->Tran != NULL && single->Tran->Flags & 0x2)
                    {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = NULL;
                }
                else
                {
                    bl = single->Blend;
                    bl->State = single->State;
                    bl->Time = single->Time;
                    bl->CurrentSpeed = single->CurrentSpeed;
                    bl->BilinearLerp[0] = single->BilinearLerp[0];
                    bl->BilinearLerp[1] = single->BilinearLerp[1];
                    bl->Effect = single->Effect;
                    bl->LastTime = single->LastTime;
                    memcpy(bl->ActiveList, single->ActiveList,
                           single->ActiveCount * sizeof(xAnimTransitionList));
                    single->ActiveList->Effect = NULL;

                    if (single->Tran != NULL && single->Tran->Flags & 0x2)
                    {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = foundTransition;
                    single->BlendFactor = 0.0000001f;
                }

                TransitionTimeInit(single, foundTransition);
                single->State = foundTransition->Dest;
                single->CurrentSpeed = single->State->Speed;
                single->BilinearLerp[0] = 0.0f;
                single->BilinearLerp[1] = 0.0f;
                single->Sync = NULL;
                EffectSingleStart(single);

                if (foundTransition->Dest->BeforeEnter != NULL)
                {
                    foundTransition->Dest->BeforeEnter(single->Play, foundTransition->Dest);
                }
                if (foundTransition->Callback != NULL)
                {
                    foundTransition->Callback(foundTransition, single, single->Play->Object);
                }
                timeDelta = tranDelta;
                foundTransition = NULL;
            }
        }

        single->Time = timeDelta * single->CurrentSpeed + single->Time;
        if (single->BlendFactor != 0.0f)
        {
            single->BlendFactor += timeDelta;
        }
        if ((single->State->Flags & 0x30) == 0x10)
        {
            LoopUpdate(single);
        }
        else
        {
            StopUpdate(single);
        }
        EffectSingleRun(single);

        if (bl != NULL)
        {
            if ((bl->State->Flags & 0x30) == 0x10)
            {
                LoopUpdate(bl);
            }
            else
            {
                StopUpdate(bl);
            }
            EffectSingleRun(bl);
        }

        firstStep = FALSE;
    } while (foundBlendstop || foundTransition != NULL);

    if (single->Tran == NULL && single->BlendFactor == 0.0f)
    {
        if ((single->State->Flags & 0x30) == 0x30)
        {
            if (single->State->Flags & 0x200)
            {
                if (single->Time >= duration)
                {
                    single->BlendFactor = 0.0000001f;
                }
            }
            else if (single->State->FadeRecip * (duration - single->Time) < 1.0f)
            {
                single->BlendFactor = 0.0000001f;
            }
        }
    }
    EffectSingleDuration(single);
    if (bl != NULL)
    {
        EffectSingleDuration(bl);
    }
}

void xAnimPlayUpdate(xAnimPlay* play, F32 timeDelta)
{
    U32 i;
    xAnimSingle* single;

    for (i = 0; i < play->NumSingle; i++)
    {
        single = &play->Single[i];

        SingleUpdate(single, timeDelta);

        if (single->State->StateCallback)
        {
            single->State->StateCallback(single->State, single, play->Object);
        }
    }
}

void xAnimPlayStartTransition(xAnimPlay* play, xAnimTransition* transition)
{
    xAnimSingle* single = &play->Single[transition->Dest->Flags & 0xf];
    xAnimSingle* bl = single->Blend;

    if (transition->SrcTime != 0.0f || (transition->Flags & 0x4 && bl != NULL && bl->State))
    {
        single->Sync = transition;
        return;
    }

    if (bl != NULL && bl->State != NULL)
    {
        EffectSingleStop(bl);
        bl->State = NULL;
        single->BlendFactor = 0.0f;
    }

    if (transition->BlendRecip == 0.0f || single->Blend == NULL)
    {
        EffectSingleStop(single);
        if (single->Tran != NULL && single->Tran->Flags & 0x2)
        {
            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
        }
        single->Tran = NULL;
    }
    else
    {
        if (single->State != NULL)
        {
            bl->State = single->State;
            bl->Time = single->Time;
            bl->CurrentSpeed = single->CurrentSpeed;
            bl->BilinearLerp[0] = single->BilinearLerp[0];
            bl->BilinearLerp[1] = single->BilinearLerp[1];
            bl->Effect = single->Effect;

            bl->LastTime = single->LastTime;

            memcpy(bl->ActiveList, single->ActiveList,
                   single->ActiveCount * sizeof(xAnimActiveEffect));
            single->ActiveList[0].Effect = NULL;
        }

        if (single->Tran != NULL && single->Tran->Flags & 0x2)
        {
            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
        }
        single->Tran = transition;
        single->BlendFactor = 0.0000001f;
    }

    TransitionTimeInit(single, transition);
    single->State = transition->Dest;
    single->CurrentSpeed = single->State->Speed;
    single->BilinearLerp[0] = 0.0f;
    single->BilinearLerp[1] = 0.0f;
    single->Sync = NULL;
    EffectSingleStart(single);

    if (transition->Dest->BeforeEnter != NULL)
    {
        transition->Dest->BeforeEnter(play, transition->Dest);
    }

    if (transition->Callback != NULL)
    {
        transition->Callback(transition, single, single->Play->Object);
    }
}

void xAnimPlayChooseTransition(xAnimPlay* play)
{
    U32 i;
    void* object = play->Object;
    xAnimTransition** list = (xAnimTransition**)giAnimScratch;
    xAnimTransition** found;
    xAnimTransitionList* curr;

    memset(list, 0, play->NumSingle * sizeof(xAnimTransition*));

    for (i = 0; i < play->NumSingle; i++)
    {
        if (play->Single[i].State)
        {
            curr = play->Single[i].State->List;

            if (curr && curr->T->Conditional)
            {
                while (curr)
                {
                    if (curr->T->Conditional(curr->T, &play->Single[i], object))
                    {
                        found = &list[curr->T->Dest->Flags & 0xF];

                        if (!*found || curr->T->Priority > (*found)->Priority)
                        {
                            *found = curr->T;
                        }
                    }

                    curr = curr->Next;
                }
            }
        }
    }

    for (i = 0; i < play->NumSingle; i++)
    {
        if (list[i] &&
            (!play->Single[i].Sync || list[i]->Priority > play->Single[i].Sync->QueuePriority))
        {
            xAnimPlayStartTransition(play, list[i]);
        }
    }
}

void xAnimPlaySetState(xAnimSingle* single, xAnimState* state, F32 startTime)
{
    EffectSingleStop(single);
    if (single->Blend)
    {
        EffectSingleStop(single->Blend);
        single->Blend->State = NULL;
    }

    single->State = state;
    if (state == NULL)
    {
        return;
    }

    single->Time =
        (state->Flags & 0x100 && startTime == 0.0f) ? state->Data->Duration * xurand() : startTime;
    single->CurrentSpeed = state->Speed;
    single->BilinearLerp[0] = 0.0f;
    single->BilinearLerp[1] = 0.0f;
    single->Effect = NULL;
    memset(single->ActiveList, 0, single->ActiveCount * sizeof(xAnimActiveEffect));
    single->LastTime = -1.0f;
    single->Sync = NULL;

    if (single->Tran != NULL && single->Tran->Flags & 0x2)
    {
        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
    }

    single->Tran = NULL;
    single->BlendFactor = 0.0f;
}

xAnimState* xAnimTableGetState(xAnimTable* table, const char* name)
{
    return xAnimTableGetStateID(table, xStrHash(name));
}

xAnimState* xAnimTableGetStateID(xAnimTable* table, U32 ID)
{
    xAnimState* curr;

    curr = table->StateList;

    while (curr)
    {
        if (curr->ID == ID)
        {
            return curr;
        }

        curr = curr->Next;
    }

    return NULL;
}

void xAnimTableAddFile(xAnimTable* table, xAnimFile* file, const char* states)
{
    U8* buffer = (U8*)giAnimScratch;
    char* stateName = xStrTokBuffer(states, " ,\t\n\r", buffer);
    while (stateName != NULL)
    {
        xAnimTableAddFileID(table, file, xStrHash(stateName), 0, 0);
        stateName = xStrTokBuffer(NULL, " ,\t\n\r", buffer);
    }
}

xAnimState* xAnimTableAddFileID(xAnimTable* table, xAnimFile* file, U32 stateID, U32 subStateID,
                                U32 subStateCount)
{
    xAnimState* state;

    state = xAnimTableGetStateID(table, stateID);

    if (state)
    {
        if (subStateID)
        {
            if (!state->MultiFile)
            {
                state->MultiFile =
                    (gxAnimUseGrowAlloc ? (xAnimMultiFile*)xMemGrowAllocSize(
                                              subStateCount * sizeof(xAnimMultiFileEntry) +
                                              sizeof(xAnimMultiFileBase)) :
                                          (xAnimMultiFile*)xMemAllocSize(
                                              subStateCount * sizeof(xAnimMultiFileEntry) +
                                              sizeof(xAnimMultiFileBase)));

                state->MultiFile->Count = 0;
            }

            U32 count = state->MultiFile->Count;

            state->MultiFile->Files[count].ID = subStateID;
            state->MultiFile->Files[count].File = file;

            state->MultiFile->Count++;
        }

        state->Data = file;

        if (file->FileFlags & 0x8000)
        {
            table->MorphIndex |= (1 << (state->Flags & 0xF));
        }
        else
        {
            table->AnimIndex |= (1 << (state->Flags & 0xF));
        }
    }

    return state;
}

float CalcRecipBlendMax(U16* arg0)
{
    float max = 0.0f;
    while (arg0[0] != 0xFFFF)
    {
        float f3;
        if (arg0[1] == 0)
        {
            f3 = 0.0f;
        }
        else
        {
            f3 = 1.0f / (0.0009765625f * arg0[1]);
        }

        f3 = 0.001f * arg0[0] + f3;
        if (f3 > max)
        {
            max = f3;
        }
        arg0 += 2;
    }

    if (max == 0.0f)
    {
        return 0.0f;
    }
    return 1.0f / max;
}

static bool _xCharIn(char ch, const char* str)
{
    while (*str != '\0')
    {
        if (ch == *str)
        {
            return true;
        }

        str++;
    }

    return false;
}

static bool _xSingleCompare(char a, char b)
{
    switch (b)
    {
    case '?':
    case '*':
    case '+':
    {
        return true;
    }
    case '#':
    {
        return (a >= '0' && a <= '9');
    }
    }

    return (a == b);
}

static bool _xSingleCompare(char ch, const char* str)
{
    while (*str != '\0')
    {
        if (_xSingleCompare(ch, *str))
        {
            return true;
        }

        str++;
    }

    return false;
}

static U8 _xCheckAnimNameInner(const char* name, const char* pattern, S32 patternSize, char* extra,
                               S32* nameOut, S32* extraOut)
{
    const char* startExtra = NULL;
    char* initialExtra = extra;
    S32 patternCurrent = 0;
    S32 nameCurrent = 0;
    while (patternCurrent < patternSize)
    {
        U32 check = pattern[patternCurrent];
        switch (check)
        {
        case '+':
        case '?':
            patternCurrent++;
            if (nameCurrent == 0)
            {
                return 0;
            }
            nameCurrent++;
            if (check != '?')
            {
            // Holy shit I hate this please tell me there's a better way
            case '*':
                check = patternCurrent + 1;
                char nextPattern[128] = { 0 };
                nextPattern[0] = pattern[check];
                while (nextPattern[0] == '{' || nextPattern[0] == '}' || nextPattern[0] == '<' ||
                       nextPattern[0] == '>')
                {
                    check++;
                    nextPattern[0] = pattern[check];
                }

                if (nextPattern[0] == '(')
                {
                    S32 nextPatternCount = 0;
                    U8 first = 1;
                    S32 parenCount = 0;

                    while (pattern[check] != NULL && parenCount > -1)
                    {
                        if (pattern[check] == '(')
                        {
                            parenCount++;
                        }
                        else if (pattern[check] == ')')
                        {
                            parenCount--;
                        }
                        else if (parenCount == 0)
                        {
                            const char* IGNORE_PATTERNS = "{}()<>";
                            if (pattern[check] == '|')
                            {
                                first = 1;
                            }
                            else if (first && _xCharIn(pattern[check], IGNORE_PATTERNS) == 0)
                            {
                                first = 0;
                                nextPattern[nextPatternCount++] = pattern[check];
                            }
                        }
                        check++;
                    }
                    nextPattern[nextPatternCount] = NULL;
                }

                while (name[nameCurrent] != NULL &&
                       _xSingleCompare(name[nameCurrent], nextPattern) == 0)
                {
                    nameCurrent++;
                }
                patternCurrent++;
            }
            break;
        case '#':
            if (name[nameCurrent] < '0' || name[nameCurrent] > '9')
            {
                return 0;
            }
            nameCurrent++;

            for (const char* x = &name[nameCurrent]; *x >= '0' && *x <= '9'; x++)
            {
                nameCurrent++;
            }
            patternCurrent++;
            break;
        case '{':
            startExtra = &name[nameCurrent];
            patternCurrent++;
            break;
        case '}':
            S32 length = &name[nameCurrent] - startExtra;
            if (extra != NULL)
            {
                memcpy(extra, startExtra, length);
                extra[length] = NULL;
                extra = extra + length;
                *++extra = '\x01';
            }
            startExtra = NULL;
            patternCurrent++;
            break;
        case '(':
            patternCurrent++;
            U8 done = 0;
            const char* current = &pattern[patternCurrent];
            while (*current != ')' && *current != NULL)
            {
                const char* startPattern = current;
                while (*startPattern != NULL && *startPattern != ')' && *startPattern != '|')
                {
                    if (*startPattern == '(')
                    {
                        S32 pc = 1;
                        while (*startPattern != NULL && pc > 0)
                        {
                            if (*startPattern == ')')
                            {
                                pc--;
                            }
                            else if (*startPattern == '(')
                            {
                                pc++;
                            }
                            startPattern++;
                        }
                        if (*startPattern != NULL)
                        {
                            startPattern++;
                        }
                    }
                    else
                    {
                        startPattern++;
                    }
                }

                if (startPattern != current)
                {
                    S32 nameOut;
                    S32 extraOut;

                    // NOTE (Square): There is no DWARF for an extra variable here and this programming pattern seems
                    // extremely wack. I think this "variable" is completely compiler generated for some reason
                    U8 wtfman = 0;
                    if (!done &&
                        _xCheckAnimNameInner(&name[nameCurrent], current, startPattern - current,
                                             extra, &nameOut, &extraOut))
                    {
                        wtfman = 1;
                    }

                    if (wtfman != 0)
                    {
                        done = 1;
                        nameCurrent += nameOut;
                        extra = extra + extraOut;
                    }
                    else if (extra != NULL)
                    {
                        *extra = '\x01';
                    }
                }

                current = startPattern;
                if (*startPattern == '|')
                {
                    current++;
                }
            }
            if (*current != NULL)
            {
                current++;
            }
            patternCurrent += current - pattern;
            if (!done)
            {
                return 0;
            }
            break;
        case '<':
        {
            patternCurrent++;
            const char* current = &pattern[patternCurrent];
            const char* positiveEnd = current;
            while (*positiveEnd != NULL && *positiveEnd != ';' && *positiveEnd != '>')
            {
                positiveEnd++;
            }

            const char* negative = NULL;
            const char* negativeEnd = NULL;

            if (*positiveEnd == ';')
            {
                negativeEnd = positiveEnd + 1;
                negative = negativeEnd;
                while (*negativeEnd != NULL && *negativeEnd != '>')
                {
                    negativeEnd++;
                };
                positiveEnd = negativeEnd;
            }
            S32 nameOut;
            S32 extraOut;
            U8 matched = _xCheckAnimNameInner(&name[nameCurrent], current, positiveEnd - current,
                                              extra, &nameOut, &extraOut);
            if (matched != 0)
            {
                if (negative != NULL &&
                    _xCheckAnimNameInner(&name[nameCurrent], negative, negativeEnd - negative, NULL,
                                         NULL, NULL) != 0)
                {
                    if (extra != NULL)
                    {
                        *extra = '\x01';
                    }
                    matched = 0;
                }
                else
                {
                    nameCurrent = nameCurrent + nameOut;
                    extra += extraOut;
                }
            }

            if (*positiveEnd != NULL)
            {
                positiveEnd++;
            }
            patternCurrent += &positiveEnd[patternCurrent] - &pattern[patternCurrent];
            if (matched == 0)
            {
                return 0;
            }
            break;
        }
        case NULL:
            return 0;
        case ')':
        default:
            if (name[nameCurrent] != pattern[patternCurrent])
            {
                return 0;
            }
            nameCurrent++;
            patternCurrent++;
            break;
        }
    }

    if (nameOut != NULL)
    {
        *nameOut = nameCurrent;
    }
    if (extraOut != NULL)
    {
        *extraOut = extra - initialExtra;
    }
    return 1;
}

static bool _xCheckAnimName(const char* name, const char* pattern, char* extra)
{
    S32 patternSize, nameOut;
    U8 rc;

    *extra = 1;
    patternSize = strlen(pattern);

    rc = _xCheckAnimNameInner(name, pattern, patternSize, extra, &nameOut, NULL);

    return (rc && name[nameOut] == '\0');
}

static U32 DefaultHasTransition(xAnimState* state, xAnimTransition* tran, U32* r5)
{
    U32 r7 = 1;
    xAnimTransitionList* curr = state->Default;

    while (curr)
    {
        if (curr->T == tran)
        {
            return TRUE;
        }

        if (curr->T->Conditional == tran->Conditional && curr->T->UserFlags == tran->UserFlags)
        {
            r7 = 0;
        }

        curr = curr->Next;
    }

    *r5 += r7;
    return FALSE;
}

static U32 StateHasTransition(xAnimState* state, xAnimTransition* tran)
{
    xAnimTransitionList* curr = state->List;

    while (curr)
    {
        if (curr->T == tran)
        {
            return TRUE;
        }

        curr = curr->Next;
    }

    return FALSE;
}

static void _xAnimTableAddTransitionHelper(xAnimState* state, xAnimTransition* tran,
                                           U32& stateCount, U32& allocCount, xAnimState** stateList)
{
    if (tran->Flags & 0x10)
    {
        if (!DefaultHasTransition(state, tran, &stateCount))
        {
            stateList[allocCount] = state;
            allocCount++;
        }
    }
    else
    {
        if (!StateHasTransition(state, tran))
        {
            stateList[allocCount] = state;
            allocCount++;
            stateCount++;
        }
    }
}

static U32 DefaultOverride(xAnimState* state, xAnimTransition* tran)
{
    xAnimTransitionList* list = state->Default;

    while (list)
    {
        if (list->T->Conditional == tran->Conditional && list->T->UserFlags == tran->UserFlags)
        {
            list->T = tran;
            return TRUE;
        }

        list = list->Next;
    }

    return FALSE;
}

void _xAnimTableAddTransition(xAnimTable* table, xAnimTransition* tran, const char* source,
                              const char* dest)
{
    U8* buffer = (U8*)giAnimScratch;
    xAnimState** stateList = (xAnimState**)(giAnimScratch + 0x400);
    S32 i;
    U32 stateCount = 0;
    U32 allocCount = 0;

    xAnimTransitionList* tlist;
    xAnimTransition* substTransitionList[32];

    char extra[128];
    char tempName[128];

    U8 bVar2 = false;
    U8 bVar1 = false;
    S32 iVar12 = 0;

    if (dest != NULL)
    {
        for (S32 i = 0; dest[i] != NULL; ++i)
        {
            if (dest[i] == '@' || dest[i] == '~')
            {
                bVar2 = true;
                break;
            }
        }
    }

    for (char* x = xStrTokBuffer(source, " ,\t\n\r", table); x != NULL;
         x = xStrTokBuffer(source, " ,\t\n\r", table))
    {
        bVar1 = dest != NULL;
        if (!bVar1)
        {
            for (char* it = x; *it != NULL; ++it)
            {
                if (_xCharIn(*it, "#+*?{}()<>|;") != 0)
                {
                    bVar1 = true;
                    break;
                }
            }
        }

        if (bVar1)
        {
            for (xAnimState* state = table->StateList; state != NULL; state = state->Next)
            {
                if (_xCheckAnimName(state->Name, x, tempName))
                {
                    if (bVar2)
                    {
                        for (const char* tempIterator = dest; *tempIterator != NULL; ++tempIterator)
                        {
                            if (*dest == '@' || *dest == '~')
                            {
                                bVar1 = *dest == '~';
                                U32 l = strlen(tempName);
                                strcpy(extra, tempName);
                            }
                            else
                            {
                                *extra = *dest;
                            }
                        }
                        *extra = NULL;
                        xAnimState* sp = xAnimTableGetState(table, extra);
                        if (bVar1 && sp == NULL)
                        {
                            continue;
                        }

                        xAnimTransition* duplicatedTransition = tran;
                        if (iVar12 != 0)
                        {
                            if (gxAnimUseGrowAlloc)
                            {
                                duplicatedTransition = (xAnimTransition*)xMemGrowAlloc(
                                    gActiveHeap, sizeof(xAnimTransition));
                            }
                            else
                            {
                                duplicatedTransition = (xAnimTransition*)xMemAlloc(
                                    gActiveHeap, sizeof(xAnimTransition), 0);
                            }
                            memcpy(duplicatedTransition, tran, sizeof(xAnimTransition));
                        }
                        duplicatedTransition->Dest = sp;
                        iVar12++;
                        substTransitionList[iVar12] = duplicatedTransition;
                    }
                    if (tran->Dest != state)
                    {
                        _xAnimTableAddTransitionHelper(state, tran, stateCount, allocCount,
                                                       stateList);
                    }
                }
            }
        }
        else
        {
            xAnimState* ssp = xAnimTableGetState(table, x);
            if (ssp != NULL && tran->Dest != ssp)
            {
                _xAnimTableAddTransitionHelper(ssp, tran, stateCount, allocCount, stateList);
            }
        }
    }

    xAnimTransitionList* curr;
    if (stateCount != 0)
    {
        if (gxAnimUseGrowAlloc)
        {
            curr = (xAnimTransitionList*)xMemGrowAlloc(gActiveHeap,
                                                       stateCount * sizeof(xAnimTransitionList));
        }
        else
        {
            curr = (xAnimTransitionList*)xMemAlloc(gActiveHeap,
                                                   stateCount * sizeof(xAnimTransitionList), 0);
        }
    }
    if (tran->Flags & 0x10)
    {
        for (S32 i = 0; i < allocCount; ++i)
        {
            if (DefaultOverride(stateList[i], tran) == 0)
            {
                if (tran->Conditional == NULL && stateList[i]->Default != NULL)
                {
                    curr->Next = NULL;
                    curr->T = bVar2 ? substTransitionList[i] : tran;
                }
            }
            else
            {
                curr->T = bVar2 ? substTransitionList[i] : tran;
                stateList[i]->Default = curr;
            }
        }
    }
    else
    {
        if (bVar2)
        {
            for (S32 i = 0; i < allocCount; ++i)
            {
                curr->T = substTransitionList[i];
                curr->Next = stateList[i]->List->Next;
            }
        }
        else
        {
            for (S32 i = 0; i < allocCount; ++i)
            {
                curr->T = tran;
                curr->Next = stateList[i]->List->Next;
            }
        }
    }
}

xAnimTransition* xAnimTableNewTransition(xAnimTable* table, const char* source, const char* dest,
                                         xAnimTransitionConditionalCallback conditional,
                                         xAnimTransitionCallback callback, U32 flags, U32 userFlags,
                                         F32 srcTime, F32 destTime, U16 priority, U16 queuePriority,
                                         F32 blendRecip, U16* blendOffset)
{
    xAnimTransition* tran;
    if (gxAnimUseGrowAlloc)
    {
        tran = (xAnimTransition*)xMemGrowAllocSize(sizeof(xAnimTransition));
    }
    else
    {
        tran = (xAnimTransition*)xMemAllocSize(sizeof(xAnimTransition));
    }

    if (blendRecip != 0.0f)
    {
        blendRecip = 1.0f / blendRecip;
    }

    tran->Next = table->TransitionList;
    table->TransitionList = tran;

    U8 isComplex = FALSE;
    if (dest == NULL || *dest == NULL)
    {
        tran->Dest = NULL;
    }
    else
    {
        for (S32 i = 0; dest[i] != NULL; ++i)
        {
            if (dest[i] == '@' || dest[i] == '~')
            {
                isComplex = true;
                break;
            }
        }
        tran->Dest = isComplex ? NULL : xAnimTableGetState(table, dest);
    }

    tran->Conditional = conditional;
    tran->Callback = callback;
    tran->Flags = flags;
    tran->UserFlags = userFlags;
    tran->SrcTime = srcTime;
    tran->DestTime = destTime;
    tran->Priority = priority;
    tran->QueuePriority = queuePriority;

    tran->BlendRecip = blendOffset != NULL ? CalcRecipBlendMax(blendOffset) : blendRecip;
    tran->BlendOffset = blendOffset;

    _xAnimTableAddTransition(table, tran, source, isComplex ? dest : NULL);

    return tran;
}

void xAnimTableAddTransition(xAnimTable* table, xAnimTransition* tran, const char* source)
{
    _xAnimTableAddTransition(table, tran, source, NULL);
}

xAnimState* xAnimTableNewState(xAnimTable* table, const char* name, U32 flags, U32 userFlags,
                               F32 speed, F32* boneBlend, F32* timeSnap, F32 fadeRecip,
                               U16* fadeOffset, void* callbackData,
                               xAnimStateBeforeEnterCallback beforeEnter,
                               xAnimStateCallback stateCallback,
                               xAnimStateBeforeAnimMatricesCallback beforeAnimMatrices)
{
    xAnimState* state;

    state = (gxAnimUseGrowAlloc ? (xAnimState*)xMemGrowAllocSize(sizeof(xAnimState)) :
                                  (xAnimState*)xMemAllocSize(sizeof(xAnimState)));

    if (!table->StateList)
    {
        state->Next = NULL;
        table->StateList = state;
    }
    else
    {
        state->Next = table->StateList->Next;
        table->StateList->Next = state;
    }

    state->Name = name;
    state->ID = xStrHash(name);
    state->Flags = flags;
    state->UserFlags = userFlags;
    state->Speed = speed;
    state->Data = NULL;
    state->Effects = NULL;
    state->Default = NULL;
    state->List = NULL;
    state->BoneBlend = boneBlend;
    state->TimeSnap = timeSnap;
    state->FadeRecip = (fadeOffset ? CalcRecipBlendMax(fadeOffset) : fadeRecip);
    state->FadeOffset = fadeOffset;
    state->MultiFile = NULL;
    state->CallbackData = callbackData;
    state->BeforeEnter = beforeEnter;
    state->StateCallback = stateCallback;
    state->BeforeAnimMatrices = beforeAnimMatrices;

    return state;
}

void xAnimDefaultBeforeEnter(xAnimPlay* play, xAnimState* state)
{
    U32 entry;

    if (state->MultiFile)
    {
        entry = rand() % state->MultiFile->Count;

        state->Data = state->MultiFile->Files[entry].File;
    }
}

xAnimTable* xAnimTableNew(const char* name, xAnimTable** linkedList, U32 userFlags)
{
    xAnimTable* table;

    table = (xAnimTable*)xMemAllocSize(sizeof(xAnimTable));

    if (linkedList)
    {
        table->Next = *linkedList;
        *linkedList = table;
    }
    else
    {
        table->Next = NULL;
    }

    table->Name = name;
    table->TransitionList = NULL;
    table->StateList = NULL;
    table->AnimIndex = NULL;
    table->MorphIndex = NULL;
    table->UserFlags = userFlags;

    return table;
}

xAnimEffect* xAnimStateNewEffect(xAnimState* state, U32 flags, F32 startTime, F32 endTime,
                                 xAnimEffectCallback callback, U32 userDataSize)
{
    xAnimEffect* curr;
    xAnimEffect** prev;
    xAnimEffect* effect;

    effect =
        (gxAnimUseGrowAlloc ? (xAnimEffect*)xMemGrowAllocSize(userDataSize + sizeof(xAnimEffect)) :
                              (xAnimEffect*)xMemAllocSize(userDataSize + sizeof(xAnimEffect)));

    effect->Flags = flags;
    effect->StartTime = startTime;
    effect->EndTime = endTime;
    effect->Callback = callback;

    prev = &state->Effects;
    curr = state->Effects;

    while (curr && startTime > curr->StartTime)
    {
        prev = &curr->Next;
        curr = curr->Next;
    }

    effect->Next = curr;
    *prev = effect;

    return effect;
}

void xAnimFileSetTime(xAnimFile* data, float duration, float timeOffset)
{
    F32 rawDuration;
    if (data->FileFlags & 0x8000)
    {
        rawDuration = xMorphSeqDuration(*(xMorphSeqFile**)data->RawData);
    }
    else
    {
        rawDuration = iAnimDuration(*data->RawData);
    }

    if (timeOffset > rawDuration - 0.1f)
    {
        timeOffset = rawDuration - 0.1f;
    }

    data->TimeOffset = timeOffset;
    if (duration > rawDuration - timeOffset)
    {
        duration = rawDuration - timeOffset;
    }

    data->Duration = (data->FileFlags & 0x2000) ? 2.0f * duration : duration;
}

xAnimFile* xAnimFileNewBilinear(void** rawData, const char* name, U32 flags, xAnimFile** linkedList,
                                U32 numX, U32 numY)
{
    xAnimFile* afile;
    if (gxAnimUseGrowAlloc)
    {
        afile = (xAnimFile*)xMemGrowAlloc(gActiveHeap, numX * numY * 4 + sizeof(xAnimFile));
    }
    else
    {
        afile = (xAnimFile*)xMemAlloc(gActiveHeap, numX * numY * 4 + sizeof(xAnimFile), 0);
    }

    if (numX > 1 || numY > 1)
    {
        flags |= 0x4000;
    }

    if (**(U32**)rawData == 'QSPM')
    {
        flags |= 0x8000;
    }

    if (linkedList != NULL)
    {
        afile->Next = *linkedList;
        *linkedList = afile;
    }
    else
    {
        afile->Next = NULL;
    }

    afile->RawData = (void**)&afile[1];
    for (S32 i = 0; i < (S32)numX * (S32)numY; ++i)
    {
        afile->RawData[i] = rawData[i];
    }

    afile->Name = name;
    afile->ID = xStrHash(name);
    afile->FileFlags = flags;

    F32 dur;
    if (afile->FileFlags & 0x2000)
    {
        if (afile->FileFlags & 0x8000)
        {
            dur = xMorphSeqDuration(*(xMorphSeqFile**)afile->RawData);
        }
        else
        {
            dur = iAnimDuration(*afile->RawData);
        }
        dur = dur * 2.0f;
    }
    else if (afile->FileFlags & 0x8000)
    {
        dur = xMorphSeqDuration(*(xMorphSeqFile**)afile->RawData);
    }
    else
    {
        dur = iAnimDuration(*afile->RawData);
    }
    afile->Duration = dur;
    afile->TimeOffset = 0.0f;

    afile->BoneCount = flags & 0x8000 ? 0 : iAnimBoneCount(*rawData);

    afile->NumAnims[0] = numX;
    afile->NumAnims[1] = numY;

    return afile;
}

xAnimFile* xAnimFileNew(void* rawData, const char* name, U32 flags, xAnimFile** linkedList)
{
    return xAnimFileNewBilinear(&rawData, name, flags, linkedList, 1, 1);
}

void xAnimInit()
{
    iAnimInit();
    memset(&sxAnimTempTranPool, 0, sizeof(xMemPool));
}

void xAnimTempTransitionInit(U32 count)
{
    xMemPoolSetup(&sxAnimTempTranPool, xMemAllocSize(count * sizeof(xAnimTransition)), 0, 0, NULL,
                  sizeof(xAnimTransition), count, count / 2);
}

//                                              iCamera

extern F32 sCameraNearClip;
extern F32 sCameraFarClip;
extern RwCamera* sMainGameCamera;

extern F32 _640_0;
extern F32 _706_0;
extern F32 _707_1;
extern F32 _708_3;
extern F32 _709_1;
extern F32 _741_3;
extern F32 _742_1;
extern F32 _743_1;
extern F64 _769_1;
extern F64 _826_0;

void iCameraSetFogRenderStates()
{
    RwCamera* pCamera;
    iFogParams* pFogParams;
    U32 bite_me;

    pCamera = RwCameraGetCurrentCamera();
    pFogParams = &xglobals->fog;

    if (pFogParams->type == rwFOGTYPENAFOGTYPE)
    {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);

        RwCameraSetFarClipPlane(pCamera, sCameraFarClip);
    }
    else
    {
        bite_me = (pFogParams->fogcolor.alpha << 24) | (pFogParams->fogcolor.red << 16) |
                  (pFogParams->fogcolor.green << 8) | pFogParams->fogcolor.blue;

        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)pFogParams->type);
        RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void*)bite_me);
        RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void*)&pFogParams->density);

        RwCameraSetFogDistance(pCamera, pFogParams->start);
        RwCameraSetFarClipPlane(pCamera, pFogParams->stop);
    }
}

void iCameraSetFogParams(iFogParams* fp, F32 time)
{
    if (!fp || fp->type == rwFOGTYPENAFOGTYPE)
    {
        xglobals->fog.type = rwFOGTYPENAFOGTYPE;
        xglobals->fogA.type = rwFOGTYPENAFOGTYPE;
    }
    else if (_742_1 == time || fp->type != xglobals->fogA.type)
    {
        xglobals->fog = *fp;
        xglobals->fogA = *fp;
        xglobals->fog_t0 = 0;
    }
    else
    {
        xglobals->fogA = xglobals->fog;
        xglobals->fogB = *fp;

        // todo
    }
}

void iCameraSetNearFarClip(F32 nearPlane, F32 farPlane)
{
    if (nearPlane <= *(const F32*)&_742_1)
    {
        nearPlane = _741_3;
    }

    sCameraNearClip = nearPlane;

    // non-matching: _742_1 is loaded too early

    if (farPlane <= *(const F32*)&_742_1)
    {
        farPlane = _743_1;
    }

    sCameraFarClip = farPlane;
}

void iCamGetViewMatrix(RwCamera* camera, xMat4x3* view_matrix)
{
    RwMatrix* rw_view;

    memset(view_matrix, 0, sizeof(xMat4x3));

    rw_view = RwCameraGetViewMatrix(camera);

    view_matrix->right.x = rw_view->right.x;
    view_matrix->right.y = rw_view->right.y;
    view_matrix->right.z = rw_view->right.z;
    view_matrix->up.x = rw_view->up.x;
    view_matrix->up.y = rw_view->up.y;
    view_matrix->up.z = rw_view->up.z;
    view_matrix->at.x = rw_view->at.x;
    view_matrix->at.y = rw_view->at.y;
    view_matrix->at.z = rw_view->at.z;
    view_matrix->pos.x = rw_view->pos.x;
    view_matrix->pos.y = rw_view->pos.y;
    view_matrix->pos.z = rw_view->pos.z;
}

void iCameraAssignEnv(RwCamera* camera, iEnv* env_geom)
{
    globalCamera = camera;

    RpWorldAddCamera(env_geom->world, camera);
    xShadowSetWorld(env_geom->world);
}

void iCameraSetFOV(RwCamera* cam, F32 fov)
{
    RwV2d vw;

    vw.x = itan(_706_0 * (_707_1 * fov) / _708_3);

    // non-matching: frsp instruction is here for some reason
    vw.y = _709_1 * vw.x;

    RwCameraSetViewWindow(cam, &vw);
}

void iCameraUpdatePos(RwCamera* cam, xMat4x3* pos)
{
    RwFrame* f;
    RwMatrix* m;

    f = RwCameraGetFrame(cam);
    m = RwFrameGetMatrix(f);

    xMat4x3Copy((xMat4x3*)m, pos);

    m = RwFrameGetLTM(f);

    xMat4x3Copy((xMat4x3*)m, pos);

    RwFrameOrthoNormalize(f);
    RwFrameUpdateObjects(f);
}

void iCameraFrustumPlanes(RwCamera* cam, xVec4* frustplane)
{
    RwFrustumPlane* rwPlane;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[0].x = rwPlane->plane.normal.x;
    frustplane[1].x = rwPlane->plane.normal.y;
    frustplane[2].x = rwPlane->plane.normal.z;
    frustplane[3].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[0].y = rwPlane->plane.normal.x;
    frustplane[1].y = rwPlane->plane.normal.y;
    frustplane[2].y = rwPlane->plane.normal.z;
    frustplane[3].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[5];
    frustplane[0].z = rwPlane->plane.normal.x;
    frustplane[1].z = rwPlane->plane.normal.y;
    frustplane[2].z = rwPlane->plane.normal.z;
    frustplane[3].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[3];
    frustplane[0].w = rwPlane->plane.normal.x;
    frustplane[1].w = rwPlane->plane.normal.y;
    frustplane[2].w = rwPlane->plane.normal.z;
    frustplane[3].w = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[1];
    frustplane[4].x = rwPlane->plane.normal.x;
    frustplane[5].x = rwPlane->plane.normal.y;
    frustplane[6].x = rwPlane->plane.normal.z;
    frustplane[7].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[0];
    frustplane[4].y = rwPlane->plane.normal.x;
    frustplane[5].y = rwPlane->plane.normal.y;
    frustplane[6].y = rwPlane->plane.normal.z;
    frustplane[7].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[4].z = rwPlane->plane.normal.x;
    frustplane[5].z = rwPlane->plane.normal.y;
    frustplane[6].z = rwPlane->plane.normal.z;
    frustplane[7].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[4].w = rwPlane->plane.normal.x;
    frustplane[5].w = rwPlane->plane.normal.y;
    frustplane[6].w = rwPlane->plane.normal.z;
    frustplane[7].w = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[8].x = rwPlane->plane.normal.x;
    frustplane[9].x = rwPlane->plane.normal.y;
    frustplane[10].x = rwPlane->plane.normal.z;
    frustplane[11].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[8].y = rwPlane->plane.normal.x;
    frustplane[9].y = rwPlane->plane.normal.y;
    frustplane[10].y = rwPlane->plane.normal.z;
    frustplane[11].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[5];
    frustplane[8].z = rwPlane->plane.normal.x;
    frustplane[9].z = rwPlane->plane.normal.y;
    frustplane[10].z = rwPlane->plane.normal.z;
    frustplane[11].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[3];
    frustplane[8].w = rwPlane->plane.normal.x;
    frustplane[9].w = rwPlane->plane.normal.y;
    frustplane[10].w = rwPlane->plane.normal.z;
    frustplane[11].w = rwPlane->plane.distance;
}

void iCameraShowRaster(RwCamera* cam)
{
    RwCameraShowRaster(cam, NULL, 0);
}

void iCameraEnd(RwCamera* cam)
{
    iScrFxCameraEndScene(cam);
    RwCameraEndUpdate(cam);
    iScrFxPostCameraEnd(cam);
}

void iCameraDestroy(RwCamera* camera)
{
    RpWorld* pWorld;
    RwRaster* raster;
    RwFrame* frame;

    _rwFrameSyncDirty();

    pWorld = RwCameraGetWorld(camera);

    if (pWorld)
    {
        RpWorldRemoveCamera(pWorld, camera);
    }

    if (camera == sMainGameCamera)
    {
        iScrFxCameraDestroyed(camera);
        sMainGameCamera = NULL;
    }

    if (camera)
    {
        frame = RwCameraGetFrame(camera);

        if (frame)
        {
            RwCameraSetFrame(camera, NULL);
            RwFrameDestroy(frame);
        }

        raster = RwCameraGetRaster(camera);

        if (raster)
        {
            RwRasterDestroy(raster);
            RwCameraSetRaster(camera, NULL);
        }

        raster = RwCameraGetZRaster(camera);

        if (raster)
        {
            RwRasterDestroy(raster);
            RwCameraSetZRaster(camera, NULL);
        }

        RwCameraDestroy(camera);
    }
}

RwCamera* iCameraCreate(S32 width, S32 height, S32 mainGameCamera)
{
    RwV2d vw;
    RwCamera* camera;

    camera = RwCameraCreate();

    RwCameraSetFrame(camera, RwFrameCreate());
    RwCameraSetRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPECAMERA));
    RwCameraSetZRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER));
    RwCameraSetFarClipPlane(camera, sCameraFarClip);
    RwCameraSetNearClipPlane(camera, sCameraNearClip);

    vw.x = _640_0;
    vw.y = _640_0;

    RwCameraSetViewWindow(camera, &vw);

    if (mainGameCamera)
    {
        iScrFxCameraCreated(camera);
        sMainGameCamera = camera;
    }

    return camera;
}
