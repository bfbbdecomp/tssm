#include "xWad3.h"
#include "iModel.h"

//                                              XOrd

extern F32 lbl_803CCEE8; // 0.95f
extern F32 lbl_803CCEF0; // 176f

void XOrdSort(st_XORDEREDARRAY* array, S32 (*test)(void*, void*))
{
    void** list = array->list;
    S32 num = 1;
    // FIXME: cnt??
    // while (num <= cnt)
    // {
    //     num = num * 3 + 1;
    // }
    for (;;)
    {
        if (num == 1)
        {
            return;
        }
        num = num / 3 + (num >> 0x1f);
        num = num - (num >> 0x1f);
        S32 numPos = num * sizeof(void*);
        void** currItem = list + num;
        S32 i = num;
        // FIXME: cnt??
        // while (i < cnt)
        // {
        //     // TODO!!!
        // }
    }
}

S32 XOrdLookup(st_XORDEREDARRAY* array, const void* key, XOrdTestCallback test)
{
    S32 v, index, rightBound, leftBound;
    leftBound = 0;
    index = array->cnt;
    do
    {
        rightBound = index;
        if (rightBound <= leftBound)
        {
            return -1;
        }
        index = (leftBound + rightBound) / 2;
        v = test(key, array->list[index]);
        if (v == 0)
            break;
        if (0 < v)
        {
            leftBound = index + 1;
            index = rightBound;
        }
    } while (true);
    return index;
}

void* XOrdRemove(st_XORDEREDARRAY* array, void* elt, S32 index)
{
    if (elt == NULL)
    {
        if (index < 0)
        {
            return NULL;
        }
        if (index >= array->max)
        {
            return NULL;
        }
    }

    if ((index >= 0) && (index < array->max))
    {
        elt = array->list[index];
    }
    else if (elt != NULL)
    {
        index = -1;
        int iVar4 = 0;
        for (int i = array->cnt; i > 0; i--)
        {
            if (array->list[iVar4] == elt)
            {
                index = iVar4;
                break;
            }
            iVar4++;
        }
    }

    if (index < 0)
    {
        return NULL;
    }

    array->cnt--;
    for (; index < array->cnt; index++)
    {
        array->list[index] = array->list[index + 1];
    }

    return elt;
}

void XOrdInsert(st_XORDEREDARRAY* array, void* elt, XOrdCompareCallback compare)
{
    if (array->cnt < array->max)
    {
        array->cnt++;
        S32 pos = array->cnt - 1;
        void* currElement = (void*)(pos * sizeof(void*));
        while (pos > 0)
        {
            // FIXME: currElement
            // S32 score = compare(array->list + currElement - sizeof(void*), elt);
            // if (score <= 0)
            // {
            //     array->list[pos] = elt;
            //     return;
            // }
            // pos--;
            // void* tmp = (void*)(array->list + currElement);
            // currElement--;
            // *tmp = tmp[-1];
        }
        *array->list = elt;
    }
}

void XOrdAppend(st_XORDEREDARRAY* array, void* elt)
{
    if (array->cnt >= array->max)
    {
        return;
    }
    array->list[array->cnt++] = elt;
}

void XOrdDone(st_XORDEREDARRAY* array, S32 wasTempAlloc)
{
    if (array->max != 0 && wasTempAlloc != 0)
    {
        xMemPopTemp(array->list);
    }
    array->list = NULL;
    array->cnt = 0;
    array->max = 0;
    array->warnlvl = 0;
}

void XOrdReset(st_XORDEREDARRAY* array)
{
    array->cnt = 0;
}

void XOrdInit(st_XORDEREDARRAY* array, S32 size, S32 tempAlloc)
{
    U32 cnt = 1;
    if (size >= 1)
    {
        cnt = size;
    }
    if (tempAlloc)
    {
        array->list = (void**)xMemPushTemp(cnt << 2);
    }
    else
    {
        array->list = (void**)xMemAlloc(gActiveHeap, cnt << 2, 0);
    }
    array->cnt = 0;
    array->max = cnt;
    array->warnlvl = lbl_803CCEE8 * (cnt ^ 0x80000000);
    if (array->warnlvl == array->max)
    {
        cnt = array->max - 1;
        array->warnlvl = cnt & ~((S32)cnt >> 0x1f);
    }
}

//                                          xMove

extern float xMovePoint_float_0;

xVec3* xMovePointGetPos(const xMovePoint* m)
{
    return m->pos;
}

// If you uncomment the numPoints variable then this function is a perfect match
// minus ordering. In the original assembly some variable fetches are lifted to
// places earlier in the assembly listing than what this comiles to for some
// reason.
// The r27-30 registers used to hold the main varibles of the function are all
// perfect matches with this code.
extern float xMovePoint_float_0;
F32 xMovePointGetNext(const xMovePoint* m, const xMovePoint* prev, xMovePoint** next, xVec3* hdng)
{
    if (m->asset->numPoints < 1)
    {
        *next = NULL;
        return xMovePoint_float_0;
    }

    xMovePoint* previousOption = NULL;

    S32 rnd = xrand() % m->node_wt_sum;

    // The debug symbols don't show a dedicated numPoints var, but if it isn't
    // present, then getting numPoints isn't lifted outside of the loop, which
    // it is in the original assembly.
    U16 numPoints = m->asset->numPoints;

    for (U16 idx = 0; idx < m->asset->numPoints; ++idx)
    {
        *next = m->nodes[idx];
        rnd -= (*next)->asset->wt;
        if ((*next)->on == 0)
        {
            *next = NULL;
        }
        else
        {
            previousOption = *next;
            if (*next == prev)
            {
                *next = NULL;
            }
            else if (rnd < 0)
            {
                break;
            }
        }
    }

    if (*next == NULL)
    {
        if (previousOption != NULL)
        {
            *next = previousOption;
        }
        else
        {
            return xMovePoint_float_0;
        }
    }

    if (hdng)
    {
        return xVec3Hdng(hdng, m->pos, (*next)->pos);
    }
    else
    {
        return xMovePoint_float_0;
    }
}

void xMovePointSplineDestroy(xMovePoint* m)
{
    if (m->spl != NULL)
    {
        m->spl = NULL;
    }
}

void xMovePointSplineSetup(xMovePoint* m)
{
    xMovePoint *w0, *w1, *w2, *w3;
    xVec3 points[2];
    xVec3 p1, p2;

    if (m->asset->bezIndex != 1)
        return;
    if (m->spl)
        return;

    w0 = m->prev;
    w1 = m;
    w2 = m->nodes[0];

    points[0] = *w0->pos;
    if (w2->asset->bezIndex > 0)
    {
        w3 = w2->nodes[0];
        p1 = *w1->pos;
        p2 = *w2->pos;
        points[1] = *w3->pos;
    }
    else
    {
        p1.x = (1 / 3.f) * w0->pos->x + (2 / 3.f) * w1->pos->x;
        p1.y = (1 / 3.f) * w0->pos->y + (2 / 3.f) * w1->pos->y;
        p1.z = (1 / 3.f) * w0->pos->z + (2 / 3.f) * w1->pos->z;
        p2.x = (2 / 3.f) * w1->pos->x + (1 / 3.f) * w2->pos->x;
        p2.y = (2 / 3.f) * w1->pos->y + (1 / 3.f) * w2->pos->y;
        p2.z = (2 / 3.f) * w1->pos->z + (1 / 3.f) * w2->pos->z;
        points[1] = *w2->pos;
    }

    m->spl = xSpline3_Bezier(points, NULL, 2, 0, &p1, &p2);
    xSpline3_ArcInit(m->spl, 20);
}

void xMovePointSetup(xMovePoint* m, xScene* sc)
{
    m->node_wt_sum = 0;
    U32* pointIds = (U32*)(m->asset + 1);
    for (U16 idx = 0; idx < m->asset->numPoints; ++idx)
    {
        U32 id = pointIds[idx];
        m->nodes[idx] = (xMovePoint*)xSceneResolvID(sc, id);
        m->node_wt_sum += m->nodes[idx]->asset->wt;
        m->nodes[idx]->prev = m;
    }
}

void xMovePointReset(xMovePoint* m)
{
    xBaseReset(m, m->asset);

    m->on = m->asset->on;
    m->delay = m->asset->delay;
}

void xMovePointLoad(xMovePoint* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void xMovePointSave(xMovePoint* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void xMovePointInit(xMovePoint* ent, xMovePointAsset* asset)
{
    xBaseInit(ent, asset);
    ent->asset = asset;

    // They didn't use `&asset->pos` here as that doesn't generate a 100% match.
    ent->pos = &ent->asset->pos;
    ent->on = asset->on;
    ent->delay = asset->delay;
    ent->spl = NULL;

    if (asset->numPoints != 0)
    {
        ent->nodes = (xMovePoint**)xMemAllocSize(asset->numPoints * sizeof(xMovePoint*));
    }
    else
    {
        ent->nodes = NULL;
    }
}

//                                          xMorph

F32 xMorphSeqDuration(xMorphSeqFile* seq)
{
    return ((F32*)(seq + 1))[seq->TimeCount - 1];
}

xMorphSeqFile* xMorphSeqSetup(void* data, xMorphFindAssetCallback FindAssetCB)
{
    S32 i, j;
    RpAtomic* model;
    xMorphTargetFile* mphtgt;
    xMorphSeqFile* header;
    F32* timelist;
    xMorphFrame* framelist;
    U32* assetlist;
    char* namelist;
    void* assetPtr;
    U32 skipsize;

    header = (xMorphSeqFile*)data;
    timelist = (F32*)(header + 1);
    framelist = (xMorphFrame*)(timelist + header->TimeCount);
    assetlist = (U32*)(framelist + header->TimeCount);
    namelist = (char*)(assetlist + header->ModelCount * 2);

    if (header->Flags & 0x80000000)
    {
        return header;
    }

    header->Flags |= 0x80000000;
    if (!(header->Flags & 0x1))
    {
        namelist = NULL;
    }

    for (i = 0; i < (S32)header->ModelCount * 2; i++)
    {
        if (assetlist[i])
        {
            assetPtr = FindAssetCB(assetlist[i], namelist);
            assetlist[i] = (U32)assetPtr;
        }
        if (namelist)
        {
            namelist = strlen(namelist) + 1 + namelist;
        }
    }

    for (i = 0; i < (S32)header->ModelCount; i++)
    {
        if (assetlist[i * 2 + 1])
        {
            model = (RpAtomic*)assetlist[i * 2];
            mphtgt = (xMorphTargetFile*)assetlist[i * 2 + 1];
            iMorphOptimize(model, mphtgt->Flags & 0x1);
        }
    }

    for (i = 0; i < (S32)header->TimeCount; i++)
    {
        model = (RpAtomic*)assetlist[(U32)framelist[i].Model * 2];
        mphtgt = (xMorphTargetFile*)assetlist[(U32)framelist[i].Model * 2 + 1];
        framelist[i].Model = model;
        for (j = 0; j < 4; j++)
        {
            if ((U32)framelist[i].Targets[j] + 0x10000 == 0xFFFF)
            {
                framelist[i].Targets[j] = NULL;
            }
            else
            {
                skipsize = (mphtgt->NumVerts * 3 + 7) & ~7;
                if (mphtgt->Flags & 0x1)
                {
                    skipsize *= 2;
                }
                if (j == 0)
                {
                    framelist[i].Flags = mphtgt->Flags;
                    framelist[i].NumVerts = mphtgt->NumVerts;
                    framelist[i].Scale *= mphtgt->Scale;
                }
                framelist[i].Targets[j] =
                    (S16*)((U8*)(mphtgt + 1) + skipsize * (U32)framelist[i].Targets[j] * 2);
            }
        }
    }

    return header;
}

//                                           xModelBucket

//                                               xModel

static xModelPool* sxModelPoolList;
static RwCamera* subcamera;
S32 xModelBucketEnabled;

static RwCamera* CameraCreate(int a, int b, int c);
void CameraDestroy(RwCamera* cam);

void xModelAnimCollStart(xModelInstance& model)
{
    model.Flags = (model.Flags & ~0x1000) | 0x800;
    if (model.anim_coll.verts == NULL)
    {
        U32 size = iModelVertCount(model.Data);
        if (size != 0)
        {
            model.anim_coll.verts = (xVec3*)xMemAlloc(gActiveHeap, size * sizeof(xVec3), 0);
        }
    }
}

void xModel_SceneExit(RpWorld* world)
{
    RpWorldRemoveCamera(world, subcamera);
}

void xModel_SceneEnter(RpWorld* world)
{
    RpWorldAddCamera(world, subcamera);
}

void xModelRender(xModelInstance* model)
{
    while (model != NULL)
    {
        if (xModelBucketEnabled)
        {
            xModelBucket_Add(model);
        }
        else
        {
            xModelRenderSingle(model);
        }
        model = model->Next;
    }
}

void xModelEvalSingle(xModelInstance* model);

void xModelEval(xModelInstance* model)
{
    while (model != NULL)
    {
        xModelEvalSingle(model);
        model = model->Next;
    }
}

void xModelUpdate(xModelInstance* modelInst, F32 timeDelta)
{
    for (; modelInst != NULL; modelInst = modelInst->Next)
    {
        if (modelInst->Anim != NULL)
        {
            if ((modelInst->Flags & 4) != 0)
            {
                if ((modelInst->Flags & 0x100) != 0)
                {
                    xAnimPlayChooseTransition(modelInst->Anim);
                }
                xAnimPlayUpdate(modelInst->Anim, timeDelta);
            }
        }
    }
}

void xModelInstanceAttach(xModelInstance* inst, xModelInstance* parent)
{
    xModelInstance* curr = parent;
    while (curr->Next != NULL)
    {
        curr = curr->Next;
    }
    curr->Next = inst;
    inst->Parent = parent;
    if ((inst->Flags & 0x2000) != 0)
    {
        inst->Mat = parent->Mat;
    }
}

static xModelInstance* FindChild(xModelInstance* model)
{
    xModelInstance* child = model->Next;
    while (child != NULL)
    {
        if (child->Parent == model)
        {
            return child;
        }
        child = child->Next;
    }
    return NULL;
}

void xModelInstanceFree(xModelInstance* model)
{
    xModelInstance* prev;
    xModelInstance* curr;

    while (prev = FindChild(model), prev != NULL)
    {
        xModelInstanceFree(prev);
    }
    prev = model->Parent;
    if (prev != NULL)
    {
        for (curr = prev->Next; (curr != NULL) && (curr != model); curr = curr->Next)
        {
            prev = curr;
        }
        prev->Next = curr->Next;
    }
    if (model->Anim != NULL)
    {
        xAnimPoolFree(model->Anim);
        model->Anim = NULL;
    }
    if (model->Pool != NULL)
    {
        model->Next = model->Pool->List;
        model->Pool->List = model;
    }
}

// Equivalent (regalloc)
void xModelInit()
{
    iModelInit();
    sxModelPoolList = NULL;
    if (subcamera == NULL)
    {
        subcamera = CameraCreate(0, 0, 1);
    }
    xModelPipeNumTables = 0;
}

//                                      xMemoryManagerGeneral

//                                      xMemoryManagerFixed

//                                      xMemoryManager

//                                          xMem

extern xMemInfo_tag gMemInfo;
extern xMemHeap_tag gxHeap[3];
extern xMemHeap gxxHeap[3];
extern void (*sMemBaseNotifyFunc)();

void xMemPoolFree(xMemPool* pool, void* data)
{
    void** prev;
    void* freeList;
    U32 next;
    void* curr;

    if (data == NULL)
    {
        return;
    }

    freeList = pool->FreeList;
    next = pool->NextOffset;
    if (pool->Flags & 1)
    {
        prev = &pool->UsedList;
        curr = pool->UsedList;

        while (curr != NULL && curr != data)
        {
            prev = (void**)((U32)curr + next);
            curr = *prev;
        }

        if (curr != NULL)
        {
            *prev = *(void**)((U32)curr + next);
        }
    }

    *(void**)((U32)data + next) = freeList;
    pool->FreeList = data;
}

void* xMemPoolAlloc(xMemPool* pool)
{
    void* retval = pool->FreeList;
    U32 next = pool->NextOffset;
    U32 flags = pool->Flags;

    if (retval == NULL)
    {
        xMemPoolAddElements(pool, xMemAlloc(gActiveHeap, pool->NumRealloc * pool->Size, 0),
                            pool->NumRealloc);
        retval = pool->FreeList;
    }

    pool->FreeList = *(void**)((U32)retval + next);
    if (flags & 1)
    {
        *(void**)((U32)retval + next) = pool->UsedList;
        pool->UsedList = retval;
    }

    return retval;
}

void xMemPoolSetup(xMemPool* pool, void* buffer, U32 nextOffset, U32 flags, xMemPoolInitCB initCB,
                   U32 size, U32 count, U32 numRealloc)
{
    pool->FreeList = NULL;
    pool->NextOffset = nextOffset;
    pool->Flags = flags;
    pool->UsedList = NULL;
    pool->InitCB = initCB;
    pool->Buffer = buffer;
    pool->Size = size;
    pool->NumRealloc = numRealloc;
    pool->Total = 0;
    xMemPoolAddElements(pool, buffer, count);
}

S32 xMemGetBase()
{
    return xMemGetBase(gActiveHeap);
}

void xMemRegisterBaseNotifyFunc(void (*func)())
{
    sMemBaseNotifyFunc = func;
}

S32 xMemPopBase(S32 depth)
{
    return xMemPopBase(gActiveHeap, depth);
}

S32 xMemPushBase()
{
    return xMemPushBase(gActiveHeap);
}

void* xMemAlloc(U32 heapID, U32 size, S32 align)
{
    // This variable not in DWARF
    S32 true_align;

    xMemHeap_tag* heap;
    xMemBlock_tag* hdr;
    xHeapState_tag* sp;

    heap = &gxHeap[heapID];
    sp = &heap->state[heap->state_idx];

    true_align = 1 << (heap->flags >> 9 & 0x1f);
    if (align > true_align)
    {
        true_align = align;
    }

    if (size == 0)
    {
        return (void*)0xDEADBEEF;
    }

    xMemBlkInfo_tag info;
    U32 total = xMemGetBlockInfo(heap, size, true_align, &info);
    hdr = info.header;

    if (sp->used + total > heap->size)
    {
        return NULL;
    }

    sp->curr = info.curr;
    sp->blk_ct += 1;
    if (heap->flags & 0x10000)
    {
        return NULL;
    }

    hdr->addr = info.block;
    hdr->size = size;
    hdr->align = true_align;
    sp->used += total;
    sp->wasted += info.waste;

    memset((void*)hdr->addr, 0, size);
    heap->lastblk = hdr;
    return (void*)hdr->addr;
}

#if 0
// Not particularly close. Not 100% confident that I understand what's going on,
// in particular the bit that goes:
// (heap->lastblk->size + size) - heap->lastblk->size;
// It really seems like the assembly code is adding and then immediately
// subtracting again, and I don't see why it would want to do that.
#undef xMemGrowAlloc
void* xMemGrowAlloc(U32 heapID, U32 size)
{
    size = (size + 3) & 0xFFFFFFFC;
    xMemHeap_tag* heap = &gxHeap[heapID];
    xHeapState_tag* sp = &heap->state[heap->state_idx];
    // pad size to multiple of 4 bytes
    xMemBlock_tag* hdr = heap->lastblk;
    U32 anotherVar = (heap->lastblk->size + size) - heap->lastblk->size;
    if (sp->used + anotherVar > heap->size)
    {
        return 0;
    }
    else
    {
        void* memptr;
        if (heap->flags & 0x100)
        {
            memptr = (void*)(hdr->addr - size);
            hdr->addr = (U32)memptr;
            sp->curr -= anotherVar;
        }
        else
        {
            sp->curr += anotherVar;
            memptr = (void*)(hdr->addr + heap->lastblk->size);
        }
        sp->used += anotherVar;
        hdr->size += size;
        memset(memptr, 0, size);
        return memptr;
    }
}
#endif

#if 0
// Not particularly close. This function is a nightmare, it has so many
// variables to untangle.
void xMemGetBlockInfo(xMemHeap_tag* heap, U32 size, S32 align, xMemBlkInfo_tag* info)
{
    S32 hdr = 1;
    xHeapState_tag* sp = &heap->state[heap->state_idx];
    if (heap->flags & 0x100)
    {
        hdr = -1;
    }

    S32 remainder = 0xc & -(heap->flags & 0x10000);
    S32 pre;
    S32 post;
    S32 block;
    S32 r8;
    if (heap->flags & 0x100)
    {
        block = -remainder;
        pre = -(remainder + size + ((sp->curr - remainder - size) & (align - 1)));
        post = pre + size;
        r8 = -pre;
    }
    else
    {
        block = 0;
        if (r8 = (align - 1) & (sp->curr + remainder))
        {
            r8 = align;
        }
        pre = (align + remainder) - r8;
        post = pre + size;
        r8 = post;
    }

    S32 total = (r8 + 3) & 0xFFFFFFFE;
    if (heap->flags & 0x10000)
    {
        info->header = &heap->blk[sp->curr];
    }
    else
    {
        info->header = (xMemBlock_tag*)(sp->curr + block);
    }
    info->pre = sp->curr + pre;
    info->block = sp->curr + pre;
    info->post = sp->curr + post;
    info->curr = sp->curr + hdr * total;
    info->waste = total - (remainder + size);
    info->total = total;
}
#endif

void xMemInitHeap(xMemHeap* heap, U32 base, U32 size, U32 flags)
{
    U32 old_base = base;
    S32 align = 1 << ((flags >> 9) & 0x1F);
    if (flags & 0x100)
    {
        base = base & -align;
        size -= (old_base - base);
    }
    else
    {
        base = (old_base + align - 1) & -align;
        size -= (base - old_base);
    }

    heap->state_idx = 0;
    heap->hard_base = base;
    heap->size = size;
    heap->flags = flags;
    heap->lastblk = NULL;
    xHeapState* sp = &heap->state[heap->state_idx];
    sp->curr = base;
    sp->blk_ct = 0;

    if (flags & 0x10000)
    {
        heap->max_blks = (1 << ((flags >> 14) & 0x1));
        heap->blk = (xMemBlock*)malloc(heap->max_blks * sizeof(xMemBlock_tag));
        memset(heap->blk, 0, heap->max_blks * sizeof(xMemBlock_tag));
    }
    else
    {
        heap->max_blks = -1;
        heap->blk = 0;
    }
    heap->opp_heap[0] = -1;
    heap->opp_heap[1] = -1;
}

void iMemExit();

void xMemExit()
{
    iMemExit();
}

void iMemInit();

// The instructions putting 0/1/2 into registers happen in the wrong order.
void xMemInit()
{
    iMemInit();
    xMemInitHeap(&gxHeap[0], gMemInfo.DRAM.addr, gMemInfo.DRAM.size, 0x8826);
    xMemInitHeap(&gxHeap[1], gMemInfo.DRAM.addr + gMemInfo.DRAM.size, gMemInfo.DRAM.size, 0x8925);
    xMemInitHeap(&gxHeap[2], gMemInfo.DRAM.addr + gMemInfo.DRAM.size, gMemInfo.DRAM.size, 0x892A);
    gxHeap[0].opp_heap[0] = 1;
    gxHeap[0].opp_heap[1] = 2;
    gxHeap[1].opp_heap[0] = 0;
    gxHeap[1].opp_heap[1] = 2;
    gxHeap[2].opp_heap[0] = 0;
    gxHeap[2].opp_heap[1] = 1;
    gActiveHeap = 0;
}

void xMemDebug_SoakLog(const char*)
{
}

//                                      xQuat / xMat3x3 / xMat4x3

/* xQuatDiff (xQuat *, xQuat const *, xQuat const *) */
void xQuatDiff(xQuat* o, const xQuat* a, const xQuat* b)
{
    xQuatConj(o, a);
    xQuatMul(o, o, b);
    if (o->s < 0.0f)
    {
        xQuatFlip(o, o);
    }
}

/* xQuatMul (xQuat *, xQuat const *, xQuat const *) */
void xQuatMul(xQuat* o, const xQuat* a, const xQuat* b)
{
    F32 temp_f10;
    F32 temp_f11;
    F32 temp_f4;
    F32 temp_f5;
    F32 temp_f6;
    F32 temp_f7;
    F32 temp_f8;
    F32 temp_f9;

    temp_f4 = b->s;
    temp_f11 = a->v.x;
    temp_f5 = a->v.y;
    temp_f8 = b->v.x;
    temp_f7 = a->s;
    temp_f9 = a->v.z;
    temp_f6 = b->v.y;
    temp_f10 = b->v.z;
    o->v.x = -((temp_f9 * temp_f6) -
               ((temp_f5 * temp_f10) + ((temp_f7 * temp_f8) + (temp_f11 * temp_f4))));
    o->v.y = -((temp_f11 * temp_f10) -
               ((temp_f9 * temp_f8) + ((temp_f7 * temp_f6) + (temp_f5 * temp_f4))));
    o->v.z = -((temp_f5 * temp_f8) -
               ((temp_f11 * temp_f6) + ((temp_f7 * temp_f10) + (temp_f9 * temp_f4))));
    o->s = -((temp_f9 * temp_f10) -
             -((temp_f5 * temp_f6) - ((temp_f7 * temp_f4) - (temp_f11 * temp_f8))));
    xQuatNormalize(o, o);
}

F32 xacos(F32 x)
{
    return std::acosf(x);
}

/* xQuatSlerp (xQuat *, xQuat const *, xQuat const *, float) */
void xQuatSlerp(xQuat* q, const xQuat* a, const xQuat* b, F32 t)
{
    xQuat sp28;
    xQuat sp18;
    F32 sp14;
    F32 sp10;
    F32 spC;
    F32 sp8;
    F32 temp_f0;
    F32 temp_f1;
    F32 temp_f28;
    F32 temp_f2;
    F32 temp_f3;
    F32 temp_f4;
    F32 var_f1;
    F32 var_f29;
    F32 var_f30;
    const xQuat* var_r31;

    var_r31 = b;
    var_f1 = xQuatDot(a, var_r31);
    if (var_f1 < 0.0f)
    {
        var_f1 = -var_f1;
        temp_f4 = -var_r31->v.x;
        temp_f0 = var_r31->s;
        temp_f3 = -var_r31->v.y;
        temp_f2 = -var_r31->v.z;
        var_r31 = (xQuat*)&sp8;
        sp8 = temp_f4;
        spC = temp_f3;
        sp10 = temp_f2;
        sp14 = -temp_f0;
    }
    //M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
    if (var_f1 == 0.999f)
    {
        var_f30 = t;
        var_f29 = 1.0f - t;
    }
    else
    {
        temp_f1 = xacos(var_f1);
        temp_f28 = 1.0f / isin(temp_f1);
        var_f29 = temp_f28 * isin((1.0f - t) * temp_f1);
        var_f30 = temp_f28 * isin(t * temp_f1);
    }
    xQuatSMul(&sp28, a, var_f29);
    xQuatSMul(&sp18, var_r31, var_f30);
    xQuatAdd(q, &sp28, &sp18);
    xQuatNormalize(q, q);
}

void xQuatToAxisAngle(const xQuat* q, xVec3* a, F32* t)
{
    *t = 2.0f * xacos(q->s);
    xVec3Normalize(a, &q->v);
}

/* xQuatToMat (xQuat const *, xMat3x3 *) */
void xQuatToMat(const xQuat* q, xMat3x3* m)
{
    F32 tx = (2.0f * q->v.x);
    F32 ty = (2.0f * q->v.y);
    F32 tz = (2.0f * q->v.z);
    F32 tsx = tx * q->s;
    F32 tsy = ty * q->s;
    F32 tsz = tz * q->s;
    F32 txx = tx * q->v.x;
    F32 txy = ty * q->v.x;
    F32 txz = tz * q->v.x;
    F32 tyy = ty * q->v.y;
    F32 tyz = tz * q->v.y;
    F32 tzz = tz * q->v.z;

    m->right.x = (1.0f - tyy) - tzz;
    m->right.y = txy - tsz;
    m->right.z = txz + tsy;

    m->up.x = txy + tsz;
    m->up.y = (1.0f - tzz) - txx;
    m->up.z = tyz - tsx;

    m->at.x = txz - tsy;
    m->at.y = tyz + tsx;
    m->at.z = (1.0f - txx) - tyy;

    m->flags = 0;
}

void xQuatFromAxisAngle(xQuat* q, const xVec3* a, F32 t)
{
    F32 t_2;

    if (t == 0.0f)
    {
        xQuatCopy(q, &g_IQ);
    }
    else
    {
        t_2 = isin(t * 0.5f);
        q->s = icos((t * 0.5f));
        xVec3SMul(&q->v, a, t_2);
    }
}

/* xQuatFromMat (xQuat *, xMat3x3 const *) */
void xQuatFromMat(xQuat* q, const xMat3x3* m)
{
    static S32 nxt[3] = { 1, 2, 0 };

    F32 temp_f1;
    F32 temp_f1_2;
    F32 temp_f1_3;
    F32 temp_f5;
    F32 temp_f6;
    S32 temp_r25;
    F32 temp_r28;
    F32 temp_r30;
    F32 temp_r31;
    F32 temp_r8;
    S32 var_r29;

    temp_f1 = m->at.z + (m->right.x + m->up.y);
    if (temp_f1 > 0.0f)
    {
        temp_f1_2 = xsqrt(1.0f + temp_f1);
        q->s = 0.5f * temp_f1_2;
        temp_f5 = 0.5f / temp_f1_2;
        q->v.x = temp_f5 * (m->at.y - m->up.z);
        q->v.y = temp_f5 * (m->right.z - m->at.x);
        q->v.z = temp_f5 * (m->up.x - m->right.y);
        return;
    }
    var_r29 = 0;
    if (m->up.y > m->right.x)
    {
        var_r29 = 1;
    }
    // if (m->at.z > F32 (*(m + (var_r29 * 0x14)))) {
    //     var_r29 = 2;
    // }
    // temp_r31 = var_r29 * 4;
    // temp_r25 = nxt[var_r29];
    // temp_r30 = temp_r25 * 4;
    // temp_r28 = nxt[temp_r25];
    // temp_f1_3 = xsqrt(1.0f + ((*(m + (var_r29 * 0x14)) - *(m + (temp_r25 * 0x14))) - *(m + (temp_r28 * 0x14))));
    // if ((F32) fabs(temp_f1_3) < 0.00001f) {
    //     xQuatCopy(q, &g_IQ);
    //     return;
    // }
    // temp_r8 = temp_r28 * 4;
    // *(q + temp_r31) = 0.5f * temp_f1_3;
    // temp_f6 = 0.5f / temp_f1_3;
    // q->unkC = temp_f6 * (*(m + ((temp_r25 + temp_r8) * 4)) - *(m + ((temp_r28 + temp_r30) * 4)));
    // *(q + temp_r30) = temp_f6 * (*(m + ((var_r29 + temp_r30) * 4)) + *(m + ((temp_r25 + temp_r31) * 4)));
    // *(q + temp_r8) = temp_f6 * (*(m + ((var_r29 + temp_r8) * 4)) + *(m + ((temp_r28 + temp_r31) * 4)));
    // if (q->unkC < 0.0f) {
    //     xQuatFlip(q, q);
    // }
}

/* xMat4x3Mul (xMat4x3 *, xMat4x3 const *, xMat4x3 const *) */
void xMat4x3Mul(xMat4x3* o, const xMat4x3* a, const xMat4x3* b)
{
    xVec3 sp8;

    xMat4x3Toworld(&sp8, b, &a->pos);
    xMat3x3Mul((xMat3x3*)o, (xMat3x3*)a, (xMat3x3*)b);
    xVec3Copy(&o->pos, &sp8);
}

void xMat4x3Rot(xMat4x3* m, const xVec3* a, F32 t, const xVec3* p)
{
    xMat4x3 temp;

    xMat3x3RotC(m, a->x, a->y, a->z, t);
    xVec3Copy(&m->pos, p);
    xMat3x3Identity(&temp);
    xVec3Inv(&temp.pos, p);
    xMat4x3Mul(m, &temp, m);
}

/* xMat3x3Tolocal (xVec3 *, xMat3x3 const *, xVec3 const *) */
void xMat3x3Tolocal(xVec3* o, const xMat3x3* m, const xVec3* v)
{
    F32 temp_f0;
    F32 temp_f0_2;
    F32 temp_f1;
    F32 temp_f1_2;
    F32 temp_f29;
    F32 temp_f2;
    F32 temp_f30;
    F32 temp_f31;
    F32 temp_f6;
    F32 temp_f7;
    F32 temp_f8;
    F32 temp_f8_2;

    temp_f1 = m->right.x;
    temp_f0 = m->right.y;
    temp_f2 = m->up.x;
    temp_f1_2 = m->up.y;
    temp_f8 = m->at.x;
    temp_f0_2 = m->at.y;
    temp_f6 = m->right.z;
    temp_f7 = m->up.z;
    temp_f8_2 = m->at.z;
    temp_f31 = (temp_f6 * temp_f6) + ((temp_f1 * temp_f1) + (temp_f0 * temp_f0));
    temp_f30 = (temp_f7 * temp_f7) + ((temp_f2 * temp_f2) + (temp_f1_2 * temp_f1_2));
    temp_f29 = (temp_f8_2 * temp_f8_2) + ((temp_f8 * temp_f8) + (temp_f0_2 * temp_f0_2));
    xMat3x3LMulVec(o, m, v);
    o->x /= temp_f31;
    o->y /= temp_f30;
    o->z /= temp_f29;
}

void xMat3x3LMulVec(xVec3* o, const xMat3x3* m, const xVec3* v)
{
}

/* xMat3x3Mul (xMat3x3 *, xMat3x3 const *, xMat3x3 const *) */
void xMat3x3Mul(xMat3x3* o, const xMat3x3* a, const xMat3x3* b)
{
    xMat3x3 sp8;
    F32 temp_f0;
    F32 temp_f0_2;
    F32 temp_f10;
    F32 temp_f10_2;
    F32 temp_f12;
    F32 temp_f13;
    F32 temp_f1;
    F32 temp_f25;
    F32 temp_f26;
    F32 temp_f27;
    F32 temp_f29;
    F32 temp_f2;
    F32 temp_f30;
    F32 temp_f3;
    F32 temp_f4;
    F32 temp_f4_2;
    F32 temp_f5;
    F32 temp_f9;
    U8 var_r0;
    xMat3x3* var_r6;

    var_r0 = 0;
    if ((o == a) || (o == b))
    {
        var_r0 = 1;
    }
    if (var_r0 != 0)
    {
        var_r6 = &sp8;
    }
    else
    {
        var_r6 = o;
    }
    temp_f4 = a->right.y;
    temp_f3 = b->up.x;
    temp_f2 = b->up.y;
    temp_f1 = a->up.y;
    temp_f10 = b->up.z;
    temp_f0 = a->at.y;
    temp_f26 = a->right.x;
    temp_f27 = b->right.x;
    temp_f30 = b->right.y;
    temp_f9 = a->up.x;
    temp_f12 = b->right.z;
    temp_f4_2 = a->at.x;
    temp_f25 = a->right.z;
    temp_f0_2 = b->at.x;
    temp_f29 = b->at.y;
    temp_f13 = b->at.z;
    temp_f10_2 = a->up.z;
    temp_f5 = a->at.z;
    var_r6->right.x = (temp_f25 * temp_f0_2) + ((temp_f26 * temp_f27) + (temp_f4 * temp_f3));
    var_r6->flags = 0;
    var_r6->right.y = (temp_f25 * temp_f29) + ((temp_f26 * temp_f30) + (temp_f4 * temp_f2));
    var_r6->right.z = (temp_f25 * temp_f13) + ((temp_f26 * temp_f12) + (temp_f4 * temp_f10));
    var_r6->up.x = (temp_f10_2 * temp_f0_2) + ((temp_f9 * temp_f27) + (temp_f1 * temp_f3));
    var_r6->up.y = (temp_f10_2 * temp_f29) + ((temp_f9 * temp_f30) + (temp_f1 * temp_f2));
    var_r6->up.z = (temp_f10_2 * temp_f13) + ((temp_f9 * temp_f12) + (temp_f1 * temp_f10));
    var_r6->at.x = (temp_f5 * temp_f0_2) + ((temp_f4_2 * temp_f27) + (temp_f0 * temp_f3));
    var_r6->at.y = (temp_f5 * temp_f29) + ((temp_f4_2 * temp_f30) + (temp_f0 * temp_f2));
    var_r6->at.z = (temp_f5 * temp_f13) + ((temp_f4_2 * temp_f12) + (temp_f0 * temp_f10));
    // if (var_r0 != 0) {
    //     xMat3x3Copy__FP7xMat3x3PC7xMat3x3(o, var_r6);
    // }
}

void xMat3x3Transpose(xMat3x3* o, const xMat3x3* m)
{
    F32 temp;

    if (o == m)
    {
        temp = o->right.y;
        o->right.y = o->up.x;
        o->up.x = temp;

        temp = o->right.z;
        o->right.z = o->at.x;
        o->at.x = temp;

        temp = o->up.z;
        o->up.z = o->at.y;
        o->at.y = temp;

        return;
    }

    o->right.x = m->right.x;
    o->right.y = m->up.x;
    o->right.z = m->at.x;

    o->up.x = m->right.y;
    o->up.y = m->up.y;
    o->up.z = m->at.y;

    o->at.x = m->right.z;
    o->at.y = m->up.z;
    o->at.z = m->at.z;

    o->flags = 0;
}

void xMat3x3RMulRotY(xMat3x3* o, const xMat3x3* m, F32 t)
{
    F32 cos = icos(t);
    F32 sin = isin(t);
    if (o == m)
    {
        F32 temp = o->right.z;
        o->right.z = ((cos * o->right.z) - (sin * o->right.x));
        o->right.x = ((cos * o->right.x) + (sin * temp));

        temp = o->up.z;
        o->up.z = ((cos * temp) - (sin * o->up.x));
        o->up.x = ((cos * o->up.x) + (sin * temp));

        temp = o->at.z;
        o->at.z = ((cos * temp) - (sin * o->at.x));
        o->at.x = ((cos * o->at.x) + (sin * temp));
    }
    else
    {
        o->right.x = (cos * m->right.x + (sin * m->right.z));
        o->right.y = m->right.y;
        o->right.z = (cos * m->right.z - (sin * m->right.x));

        o->up.x = (cos * m->up.x + (sin * m->up.z));
        o->up.y = m->up.y;
        o->up.z = (cos * m->up.z - (sin * m->up.x));

        o->at.x = (cos * m->at.x + (sin * m->at.z));
        o->at.y = m->at.y;
        o->at.z = (cos * m->at.z - (sin * m->at.x));

        o->flags = 0;
    }
}

/* xMat3x3ScaleC (xMat3x3 *, float, float, float) */
void xMat3x3ScaleC(xMat3x3* m, F32 x, F32 y, F32 z)
{
    xVec3Init((xVec3*)m, x, 0.0f, 0.0f);
    xVec3Init(&m->up, 0.0f, y, 0.0f);
    xVec3Init(&m->at, 0.0f, 0.0f, z);
    m->flags = 0;
}

void xMat3x3RotZ(xMat3x3* m, F32 t)
{
    F32 cos = icos(t);
    F32 sin = isin(t);

    xVec3Init(&m->right, cos, sin, 0.0f);
    xVec3Init(&m->up, -sin, cos, 0.0f);
    xVec3Copy(&m->at, &g_Z3);
    m->flags = 0;
}

void xMat3x3RotY(xMat3x3* m, F32 t)
{
    F32 temp_f31;
    F32 temp_f1;

    temp_f31 = icos(t);
    temp_f1 = isin(t);
    xVec3Init((xVec3*)m, temp_f31, 0.0f, -temp_f1);
    xVec3Copy(&m->up, &g_Y3);
    xVec3Init(&m->at, temp_f1, 0.0f, temp_f31);
    m->flags = 0;
}

void xMat3x3RotX(xMat3x3* m, F32 t)
{
    F32 cos = icos(t);
    F32 sin = isin(t);

    xVec3Copy(&m->right, &g_X3);
    xVec3Init(&m->up, 0.0f, cos, sin);
    xVec3Init(&m->at, 0.0f, -sin, cos);
    m->flags = 0;
}

/* xMat3x3RotC (xMat3x3 *, float, float, float, float) */
void xMat3x3RotC(xMat3x3* m, F32 _x, F32 _y, F32 _z, F32 t)
{
    F32 temp_f0;
    F32 temp_f0_2;
    F32 temp_f1;
    F32 temp_f28;
    F32 temp_f2;
    F32 temp_f3;
    F32 temp_f4;
    F32 temp_f5;
    F32 temp_f6;
    F32 temp_f7;
    F32 temp_f8;

    if (t == 0.0f)
    {
        xMat3x3Identity(m);
        return;
    }
    temp_f28 = icos(t);
    temp_f1 = isin(t);
    temp_f5 = -temp_f1;
    temp_f2 = 1.0f - temp_f28;
    temp_f0 = temp_f2 * _x;
    temp_f8 = temp_f2 * _z;
    temp_f4 = temp_f2 * _y;
    temp_f3 = _y * temp_f0;
    temp_f6 = _x * temp_f8;
    temp_f0_2 = (temp_f1 * _z) + temp_f3;
    m->right.x = (_x * temp_f0) + temp_f28;
    temp_f7 = _z * temp_f4;
    m->right.y = temp_f0_2;
    m->right.z = (temp_f5 * _y) + temp_f6;
    m->up.x = (temp_f5 * _z) + temp_f3;
    m->up.y = (_y * temp_f4) + temp_f28;
    m->up.z = (temp_f1 * _x) + temp_f7;
    m->at.x = (temp_f1 * _y) + temp_f6;
    m->at.y = (temp_f5 * _x) + temp_f7;
    m->at.z = (_z * temp_f8) + temp_f28;
    m->flags = 0;
}

/* xMat3x3Euler (xMat3x3 *, xVec3 const *) */
void xMat3x3Euler(xMat3x3* m, const xVec3* ypr)
{
    xMat3x3Euler(m, ypr->x, ypr->y, ypr->z);
}

/* xMat3x3Euler (xMat3x3 *, float, float, float) */
void xMat3x3Euler(xMat3x3* m, F32 yaw, F32 pitch, F32 roll)
{
    F32 temp_f0;
    F32 temp_f1;
    F32 temp_f27;
    F32 temp_f28;
    F32 temp_f29;
    F32 temp_f30;
    F32 temp_f31;
    F32 temp_f7;

    temp_f27 = isin(yaw);
    temp_f28 = icos(yaw);
    temp_f29 = isin(pitch);
    temp_f30 = icos(pitch);
    temp_f31 = isin(roll);
    temp_f1 = icos(roll);
    temp_f7 = temp_f27 * temp_f29;
    temp_f0 = temp_f28 * temp_f29;
    m->right.x = (temp_f28 * temp_f1) + (temp_f31 * temp_f7);
    m->right.y = temp_f30 * temp_f31;
    m->right.z = (-temp_f27 * temp_f1) + (temp_f31 * temp_f0);
    m->up.x = (-temp_f28 * temp_f31) + (temp_f1 * temp_f7);
    m->up.y = temp_f30 * temp_f1;
    m->up.z = (temp_f27 * temp_f31) + (temp_f1 * temp_f0);
    m->at.x = temp_f27 * temp_f30;
    m->at.y = -temp_f29;
    m->at.z = temp_f28 * temp_f30;
    m->flags = 0;
}

F32 xasin(F32 x)
{
    return std::asinf(x);
}

/* xMat3x3GetEuler (xMat3x3 const *, xVec3 *) */
void xMat3x3GetEuler(const xMat3x3* m, xVec3* a)
{
    F32 yaw = -xasin(m->at.y);

    F32 roll;
    F32 pitch;

    if (yaw < (PI / 2))
    {
        if (yaw > -(PI / 2))
        {
            pitch = xatan2(m->at.x, m->at.z);
            roll = xatan2(m->right.y, m->up.y);
        }
        else
        {
            pitch = -xatan2(-m->up.x, m->right.x);
            roll = 0.0f;
        }
    }
    else
    {
        pitch = xatan2(-m->up.x, m->right.x);
        roll = 0.0f;
    }

    a->x = pitch;
    a->y = yaw;
    a->z = roll;
}

F32 xMat3x3LookVec(xMat3x3* m, const xVec3* at)
{
    F32 temp_f2;
    F32 temp_f31;
    xVec3* temp_r3;

    temp_f31 = xVec3Normalize(&m->at, at);
    temp_r3 = &m->at;
    xVec3Inv(temp_r3, temp_r3);
    if ((F32)__fabs(1.0f - m->at.y) < 0.00001f)
    {
        m->right.x = 1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 0.0f;
        m->up.z = 1.0f;
        m->at.x = 0.0f;
        m->at.y = -1.0f;
        m->at.z = 0.0f;
        return temp_f31;
    }
    if ((F32)__fabs(1.0f + m->at.y) < 0.00001f)
    {
        m->right.x = -1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 0.0f;
        m->up.z = -1.0f;
        m->at.x = 0.0f;
        m->at.y = 1.0f;
        m->at.z = 0.0f;
        return temp_f31;
    }
    if (((F32)__fabs(at->z) < 0.00001f) && ((F32)__fabs(at->x) < 0.00001f))
    {
        m->right.x = 1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 1.0f;
        m->up.z = 0.0f;
        m->at.x = 0.0f;
        m->at.y = 0.0f;
        m->at.z = 1.0f;
        return 0.0f;
    }
    m->right.x = m->at.z;
    m->right.y = 0.0f;
    m->right.z = -m->at.x;
    xVec3Normalize((xVec3*)m, (xVec3*)m);
    xVec3Cross(&m->up, &m->at, (xVec3*)m);
    xVec3Cross((xVec3*)m, &m->up, &m->at);
    m->flags = 0;
    return temp_f31;
}

/* xMat4x3MoveLocalAt (xMat4x3 *, float) */
void xMat4x3MoveLocalAt(xMat4x3* m, F32 mag)
{
    m->pos.x += m->at.x * mag;
    m->pos.y += m->at.y * mag;
    m->pos.z += m->at.z * mag;
}

void xMat4x3MoveLocalUp(xMat4x3* m, F32 mag)
{
    m->pos.x += m->up.x * mag;
    m->pos.y += m->up.y * mag;
    m->pos.z += m->up.z * mag;
}

void xMat4x3MoveLocalRight(xMat4x3* m, F32 mag)
{
    m->pos.x += m->right.x * mag;
    m->pos.y += m->right.y * mag;
    m->pos.z += m->right.z * mag;
}

void xMat3x3Normalize(xMat3x3* o, const xMat3x3* m)
{
    xVec3Normalize((xVec3*)o, (xVec3*)m);
    xVec3Normalize(&o->up, &m->up);
    xVec3Normalize(&o->at, &m->at);
}

void xBoxInitBoundOBB(xBox* o, const xBox* b, const xMat4x3* m)
{
}

S32 xPointInBox(const xBox* b, const xVec3* p)
{
    S32 ret = 0;
    if ((p->x >= b->lower.x) && (p->x <= b->upper.x))
    {
        if ((p->y >= b->lower.y) && (p->y <= b->upper.y))
        {
            if ((p->z >= b->lower.z) && (p->z <= b->upper.z))
            {
                ret = 1;
            }
        }
    }
    return (char)ret;
}

//                                   xResponse / xAccel / xAngle / xMath

extern S32 xmath_inited;
extern S32 xmath_exited;
extern volatile U32 rndseed; // made this volatile so xrand() matches

F32 xDangleClamp(F32 a)
{
    F32 rad360 = 2 * PI;
    F32 rem = fmod(a, 2 * PI_DOUBLE);

    if (rem >= PI)
    {
        return rem - rad360;
    }

    if (rem < -PI)
    {
        return rem + rad360;
    }

    return rem;
}

F32 xAngleClampFast(F32 a)
{
    F32 rad360 = (2 * PI);

    if (a < 0.0f)
    {
        return a + rad360;
    }

    if (a >= rad360)
    {
        return a - rad360;
    }

    return a;
}

F32 xAngleClamp(F32 a)
{
    F32 rad360 = 2 * PI;
    F32 rem = fmod(a, 2 * PI_DOUBLE);

    if (rem < 0.0f)
    {
        return rem + rad360;
    }

    return rem;
}

U32 xMathSolveQuadratic(F32 a, F32 b, F32 c, F32* x1, F32* x2)
{
    F32 d;
    F32 dx;
    F32 p;

    if (a == 0.0f)
    {
        if (b == 0.0f)
        {
            return 0;
        }

        *x1 = -c / b;
        return 1;
    }

    d = b * b - (4.0f * a * c);

    if (d < 0.0f)
    {
        return 0;
    }

    dx = 1.0f / (2.0f * a);

    *x1 = -b * dx;

    if (d == 0.0f)
    {
        return 1;
    }

    *x2 = *x1;

    p = dx * xsqrt(d);

    if (a > 0.0f)
    {
        *x1 -= p;
        *x2 += p;
    }
    else
    {
        *x1 += p;
        *x2 -= p;
    }

    return 2;
}

F32 xatof(const char* x)
{
    return atof(x);
}

void xMathExit()
{
    if (!xmath_exited)
    {
        xmath_exited = 1;
    }
}

void xMathInit()
{
    if (!xmath_inited)
    {
        xmath_inited = 1;
        rndseed = 0;
    }
}

//                                        xLightVolume

//                                          xLightKit

S32 iModelHack_DisablePrelight;
xLightKit* gLastLightKit;

void xLightKit_Destroy(xLightKit* lkit)
{
    if (lkit == NULL)
    {
        return;
    }

    int i;
    xLightKitLight* currLight = lkit->lightList;

    for (i = 0; i < lkit->lightCount; currLight++, i++)
    {
        if (currLight->platLight != NULL)
        {
            _rwFrameSyncDirty();
            RwFrame* tframe = (RwFrame*)(currLight->platLight->object).object.parent;
            if (tframe != NULL)
            {
                _rwObjectHasFrameSetFrame(currLight->platLight, 0);
                RwFrameDestroy(tframe);
            }
            RpLightDestroy(currLight->platLight);
            currLight->platLight = 0;
        }
    }
}

xLightKit* xLightKit_GetCurrent(RpWorld* world)
{
    return gLastLightKit;
}

void xLightKit_Enable(xLightKit* lkit, RpWorld* world)
{
    if (lkit != gLastLightKit)
    {
        int i;
        if (gLastLightKit != NULL)
        {
            for (i = 0; i < gLastLightKit->lightCount; i++)
            {
                RpWorldRemoveLight(world, gLastLightKit->lightList[i].platLight);
            }
        }
        gLastLightKit = lkit;
        if (lkit != NULL)
        {
            iModelHack_DisablePrelight = 1;
            for (i = 0; i < lkit->lightCount; i++)
            {
                RpWorldAddLight(world, lkit->lightList[i].platLight);
            }
        }
        else
        {
            iModelHack_DisablePrelight = 0;
        }
    }
}

xLightKit* xLightKit_Prepare(void* data)
{
    xLightKit* lkit = (xLightKit*)data;
    lkit->lightList = (xLightKitLight*)((int*)data + 4);
    xLightKitLight* currlight = (xLightKitLight*)((int*)data + 4);

    for (int i = 0; i < lkit->lightCount; currlight++, i++)
    {
        if (currlight->platLight != NULL)
        {
            return lkit;
        }

        // If any of the colors is greater than 1.0, normalize back to 0-1
        if (currlight->color.red > 1.0f || currlight->color.green > 1.0f ||
            currlight->color.blue > 1.0f)
        {
            F32 s;
            s = MAX(MAX(currlight->color.red, currlight->color.green), currlight->color.blue);
            s = MAX(s, 0.00001f);
            s = 1.0f / s;
            currlight->color.red *= s;
            currlight->color.green *= s;
            currlight->color.blue *= s;
        }

        switch (currlight->type)
        {
        case 1:
            currlight->platLight = RpLightCreate(2);
            break;
        case 2:
            currlight->platLight = RpLightCreate(1);
            break;
        case 3:
            currlight->platLight = RpLightCreate(128);
            break;
        case 4:
            currlight->platLight = RpLightCreate(130);
            break;
        default:
            break;
        }
        RpLightSetColor(currlight->platLight, &currlight->color);
        if (currlight->type >= 2)
        {
            RwFrame* frame = RwFrameCreate();
            RwMatrixTag tmpmat;

            memset(&tmpmat, 0, 64);
            tmpmat.right.x = -currlight->matrix[0];
            tmpmat.right.y = -currlight->matrix[1];
            tmpmat.right.z = -currlight->matrix[2];
            tmpmat.up.x = currlight->matrix[4];
            tmpmat.up.y = currlight->matrix[5];
            tmpmat.up.z = currlight->matrix[6];
            tmpmat.at.x = -currlight->matrix[8];
            tmpmat.at.y = -currlight->matrix[9];
            tmpmat.at.z = -currlight->matrix[10];
            tmpmat.pos.x = currlight->matrix[12];
            tmpmat.pos.y = currlight->matrix[13];
            tmpmat.pos.z = currlight->matrix[14];
            RwV3dNormalize(&tmpmat.right, &tmpmat.right);
            RwV3dNormalize(&tmpmat.up, &tmpmat.up);
            RwV3dNormalize(&tmpmat.at, &tmpmat.at);
            RwFrameTransform(frame, &tmpmat, rwCOMBINEREPLACE);
            _rwObjectHasFrameSetFrame(currlight->platLight, frame);
        }
        if (currlight->type >= 3)
        {
            RpLightSetRadius(currlight->platLight, currlight->radius);
        }
        if (currlight->type >= 4)
        {
            RpLightSetConeAngle(currlight->platLight, currlight->angle);
        }
    }

    return (xLightKit*)data;
}

//                                      xMisc / Miscellaneous
