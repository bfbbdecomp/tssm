
#include "xWad2.h"

//                                              xIni

const char* xIniGetString(xIniFile* ini, const char* tok, const char* def)
{
    S32 index = xIniGetIndex(ini, tok);
    if (index == -1)
    {
        return def;
    }
    return ini->Values[index].val;
}

F32 xIniGetFloat(xIniFile* ini, const char* tok, F32 def)
{
    S32 index = xIniGetIndex(ini, tok);
    if (index == -1)
    {
        return def;
    }
    return atof(ini->Values[index].val);
}

S32 xIniGetInt(xIniFile* ini, const char* tok, S32 def)
{
    S32 index = xIniGetIndex(ini, tok);
    if (index == -1)
    {
        return def;
    }
    return atoi(ini->Values[index].val);
}

void xIniDestroy(xIniFile* ini)
{
    RwFree(ini->mem);
    RwFree(ini);
}

char* TrimWhitespace(char* string)
{
    // Skip through any leading whitespace
    for (; *string == ' ' || *string == '\t'; string++)
    {
    }
    // Return early if there's no other characters to go through
    if (*string == '\0')
    {
        return string;
    }

    // Find first non-whitespace character at the end of the string
    char* substring = &string[strlen(string) - 1];
    for (; *substring == ' ' || *substring == '\t'; substring--)
    {
    }

    // Set the null-terminator accordingly
    substring[1] = '\0';

    return string;
}

xIniFile* xIniParse(char* buf, S32 len)
{
    S32 i;
    S32 ccr = 1;
    S32 clf = 1;
    S32 copen = 0;
    S32 lastCRLF = -1;
    for (i = 0; i < len; i++)
    {
        switch (buf[i])
        {
        case '\n':
            lastCRLF = i;
            clf++;
            break;
        case '\r':
            lastCRLF = i;
            ccr++;
            break;
        case '[':
            copen++;
            break;
        }
    }

    if (clf > ccr)
    {
        ccr = clf;
    }

    S32 sectionAlloc = copen;
    S32 valueAlloc = ccr;

    xIniFile* ini = (xIniFile*)RwMalloc(sizeof(xIniFile) + (valueAlloc * sizeof(xIniValue)) +
                                        (sectionAlloc * sizeof(xIniSection)) + (len - lastCRLF));
    ini->mem = NULL;
    ini->NumValues = 0;
    ini->NumSections = 0;
    ini->Values = (xIniValue*)(ini + 1);
    ini->Sections = (xIniSection*)(ini->Values + valueAlloc);

    char* lastLine = (char*)(ini->Sections + sectionAlloc);
    strncpy(lastLine, buf + (lastCRLF + 1), len - (lastCRLF + 1));
    lastLine[len - (lastCRLF + 1)] = '\0';

    if (lastCRLF >= 0)
    {
        buf[lastCRLF] = '\0';
    }
    else
    {
        buf[0] = '\0';
    }

    char* ltoken;
    char* line = xStrTok(buf, "\n\r", &ltoken);
    if (line == NULL)
    {
        line = xStrTok(lastLine, "\n\r", &ltoken);
        lastLine = NULL;
    }

    while (line != NULL)
    {
        line = TrimWhitespace(line);
        if (*line != '#' && *line != '\0')
        {
            if (*line == '[')
            {
                char* c = std::strstr(line, "]");
                if (c != NULL)
                {
                    *c = '\0';
                    c = TrimWhitespace(line + 1);
                    if (*c != '\0')
                    {
                        ini->Sections[ini->NumSections].sec = c;
                        ini->Sections[ini->NumSections].first = ini->NumValues;
                        ini->Sections[ini->NumSections].count = 0;
                        ini->NumSections++;
                    }
                }
            }
            else
            {
                char* c = std::strstr(line, "=");
                if (c != NULL)
                {
                    *c = '\0';
                    char* tok = TrimWhitespace(line);
                    if (*tok != '\0')
                    {
                        line = c + 1;
                        c = std::strstr(line, "#");
                        if (c != NULL)
                        {
                            *c = '\0';
                        }
                        char* val = TrimWhitespace(line);
                        ini->Values[ini->NumValues].tok = tok;
                        ini->Values[ini->NumValues].val = val;
                        ini->NumValues++;
                        if (ini->NumSections != 0)
                        {
                            ini->Sections[ini->NumSections - 1].count++;
                        }
                    }
                }
            }
        }

        line = xStrTok(NULL, "\n\r", &ltoken);
        if (line == NULL && lastLine != NULL)
        {
            line = xStrTok(lastLine, "\n\r", &ltoken);
            lastLine = NULL;
        }
    }

    return ini;
}

//                                           xhud::text_widget

void xhud::text_widget::render()
{
    F32 boundX = tb.bounds.x;
    F32 boundY = tb.bounds.y;
    U8 oldRed = tb.font.color.r;
    U8 oldGreen = tb.font.color.g;
    U8 oldBlue = tb.font.color.b;
    U8 oldAlpha = tb.font.color.a;

    U8 alpha;
    F32 fVar1 = (oldAlpha * 200.0f) / 255.0f;
    if (fVar1 > 255.0f)
    {
        alpha = 0xFF;
    }
    else if (fVar1 < 0.0f)
    {
        alpha = 0;
    }
    else
    {
        alpha = fVar1;
    }

    tb.bounds.x += 0.0075f;
    tb.bounds.y += 0.0075f;
    tb.font.color.r = 0x00;
    tb.font.color.g = 0x46;
    tb.font.color.b = 0x55;
    tb.font.color.a = alpha;
    tb.render(true);

    tb.bounds.x = boundX;
    tb.bounds.y = boundY;
    tb.font.color.r = oldRed;
    tb.font.color.g = oldGreen;
    tb.font.color.b = oldBlue;
    tb.font.color.a = oldAlpha;
    tb.render(true);
}

void xhud::text_widget::update(F32 dt)
{
    updater(dt);

    tb.set_text(text);
    tb.bounds.x = rc.loc.x;
    tb.bounds.y = rc.loc.y;
    tb.bounds.w = rc.size.x;
    tb.bounds.h = rc.size.y;
    tb.font.clip = tb.bounds;

    tb.font.color.r = (rc.r * 255.0f) + 0.5f;
    tb.font.color.g = (rc.g * 255.0f) + 0.5f;
    tb.font.color.b = (rc.b * 255.0f) + 0.5f;
    tb.font.color.a = (rc.a * 255.0f) + 0.5f;
}

bool xhud::text_widget::is(U32 id) const
{
    return id == text_widget::type() || widget::is(id);
}

U32 xhud::text_widget::type() const
{
    static U32 myid = xStrHash(text_asset::type_name());
    return myid;
}

void xhud::text_widget::destroy()
{
    destruct();
}

void xhud::text_widget::setup()
{
    presetup();
    text_asset* asset = (text_asset*)a;
    ztextbox* textbox = (ztextbox*)zSceneFindObject(asset->text_box);
    if (textbox == NULL)
    {
        tb = xtextbox::create();
        return;
    }

    textbox->refresh();
    tb = textbox->tb;
    textbox->get_text(text, sizeof(text));
    if (asset->text != 0)
    {
        xTextAsset* tasset = (xTextAsset*)xSTFindAsset(asset->text, NULL);
        if (tasset != NULL)
        {
            char* text_asset_text = xTextAssetGetText(tasset);
            U32 len = strlen(text_asset_text);
            if (len >= sizeof(text))
            {
                len = sizeof(text) - 1;
            }
            memcpy(text, text_asset_text, len);
            text[len] = '\0';
        }
    }

    rc.r = tb.font.color.r / 255.0f;
    rc.g = tb.font.color.g / 255.0f;
    rc.b = tb.font.color.b / 255.0f;
}

void xhud::text_widget::load(xBase& data, xDynAsset& asset, size_t)
{
    init_base(data, asset, sizeof(xBase) + sizeof(text_widget));
    text_widget* widget = (text_widget*)(&data + 1);
    new (widget) text_widget((text_asset&)asset);
}

//                                              xhud::model_widget

void xhud::model_widget::load(xBase& data, xDynAsset& asset, size_t)
{
    init_base(data, asset, sizeof(xBase) + sizeof(model_widget));
    model_widget* widget = (model_widget*)(&data + 1);
    new (widget) model_widget((model_asset&)asset);
}

void xhud::model_widget::destruct()
{
    widget::destruct();
}

void xhud::model_widget::destroy()
{
    destruct();
}

U32 xhud::model_widget::type() const
{
    static U32 myid = xStrHash(model_asset::type_name());
    return myid;
}

bool xhud::model_widget::is(U32 id) const
{
    return id == model_widget::type() || widget::is(id);
}

void xhud::model_widget::update(F32 dt)
{
    widget::updater(dt);

    if (!visible() || rc.a <= (0.5f / 255.0f))
    {
        return;
    }

    xModelUpdate(model, dt);
    xModelEval(model);
}

void xhud::model_widget::render()
{
    if (rc.a <= 0.0f)
    {
        return;
    }

    render_model(*model, rc);
}

void xhud::widget::clear_motives()
{
    activity = ACT_NONE;
    motive_node* node = _motive_top;
    while (node != NULL)
    {
        node->m.finish();
        _motive_top = node->next;
        motive_allocator()->free(node);
        node = _motive_top;
    }
}

void xhud::widget::add_motive(const motive& m)
{
    motive_node* node = (motive_node*)motive_allocator()->alloc();
    new (node) motive(m);

    if (_motive_temp_tail == NULL)
    {
        node->next = _motive_top;
        _motive_top = node;
    }
    else
    {
        if (_motive_temp == NULL)
        {
            _motive_temp_tail = &node->next;
        }
        node->next = _motive_temp;
        _motive_temp = node;
    }
}

void xhud::widget::hide()
{
    activity = ACT_HIDE;

    F32 fVar1 = start_rc.size.x;
    F32 fVar3 = start_rc.size.y;
    F32 fVar7 = (start_rc.loc.x - 0.5f) + 0.5f * fVar1;
    F32 fVar8 = (start_rc.loc.y - 0.5f) + 0.5f * fVar3;
    if (iabs(iabs(fVar7) + iabs(fVar8)) <= 0.0001f)
    {
        rc.a = 0.0f;
    }
    else
    {
        F32 fVar5;
        F32 fVar6;
        if (iabs(fVar7) > iabs(fVar8))
        {
            if (fVar8 >= 0.0f)
            {
                fVar6 = 0.5f + fVar3;
            }
            else
            {
                fVar6 = -0.5f - fVar3;
            }
            fVar5 = (fVar6 * fVar7) / fVar8;
        }
        else
        {
            if (fVar7 >= 0.5f)
            {
                fVar5 = 0.5f + fVar1;
            }
            else
            {
                fVar5 = -0.5f - fVar1;
            }
            fVar6 = (fVar5 * fVar8) / fVar7;
        }

        F32 dVar11 = 255.0f + (fVar6 - 0.5f * fVar3) - rc.loc.y;
        F32 dVar12 = 255.0f + (fVar5 - 0.5f * fVar1) - rc.loc.x;
        F32 dVar10 = xsqrt(dVar12 * dVar12 + dVar11 * dVar11);

        add_motive(
            motive(&rc.loc.x, 0.0f, dVar12, dVar12 * dVar10, accelerate_motive_update, NULL));

        add_motive(
            motive(&rc.loc.y, 0.0f, dVar11, dVar11 * dVar10, accelerate_motive_update, NULL));

        fVar1 = -rc.a;
        add_motive(motive(&rc.a, 0.4f * fVar1, dVar11, 0.0f, linear_motive_update, NULL));
    }
}

bool xhud::widget::is(U32 id) const
{
    return id == widget::type();
}

U32 xhud::widget::type() const
{
    static U32 myid = xStrHash(a->type_name());
    return myid;
}

void xhud::widget::dispatcher(xBase*, U32 event, const F32* toParam, xBase*, U32)
{
    switch (event)
    {
    case eEventEnable:
        enable();
        break;
    case eEventDisable:
        disable();
        break;
    case eEventVisible:
    case eEventFastVisible:
        flag.visible = 1;
        break;
    case eEventInvisible:
    case eEventFastInvisible:
        flag.visible = 0;
        break;
    case eEventDispatcher_ShowHud:
        show();
        break;
    case eEventDispatcher_HideHud:
        hide();
        break;
    }
}

void xhud::widget::presetup()
{
    activity = ACT_NONE;
}

void xhud::render()
{
    if (inited)
    {
        widget::render_all();
    }
}

//                                                   HIPL

static U32 g_loadlock = 0;

static st_HIPLOADDATA g_hiploadinst[8] = {};

static st_HIPLOADFUNCS g_map_HIPL_funcmap = { HIPLCreate,     HIPLDestroy,   HIPLBaseSector,
                                              HIPLBlockEnter, HIPLBlockExit, HIPLReadBytes,
                                              HIPLReadShorts, HIPLReadLongs, HIPLReadFloats,
                                              HIPLReadString, HIPLSetBypass, HIPLSetSpot,
                                              HIPLPollRead };

static S32 HIPLBlockRead(st_HIPLOADDATA* lddata, void* data, S32 cnt, S32 size)
{
    st_HIPLOADBLOCK* top;
    S32 got = 0;
    S32 left = 0;
    S32 head = 0;

    if (lddata->bypass)
    {
        return 0;
    }

    if (cnt == 0)
    {
        return 0;
    }

    if (lddata->top < 0)
    {
        top = NULL;
    }
    else
    {
        top = &lddata->stk[lddata->top];
        left = top->blk_remain / size;
    }

    if (cnt < 0)
    {
        cnt = -cnt;
        head = 1;

        if (top && cnt > left)
        {
            cnt = left;
        }
    }

    if (!head && left < cnt)
    {
        cnt = left;
    }

    if (cnt == 0)
    {
        got = 0;
    }
    else if (size == 1)
    {
        got = lddata->fli->readBytes(lddata->fli, (char*)data, cnt);
    }
    else if (size == 2)
    {
        got = lddata->fli->readMShorts(lddata->fli, (S16*)data, cnt);
    }
    else if (size == 4)
    {
        got = lddata->fli->readMLongs(lddata->fli, (S32*)data, cnt);
    }

    got *= size;

    lddata->pos += got;

    if (top)
    {
        top->blk_remain -= got;
    }

    return got;
}

static S32 HIPLReadAsync(st_HIPLOADDATA* lddata, S32 pos, char* data, S32 cnt, S32 size)
{
    S32 rc;

    lddata->asyn_stat = HIP_RDSTAT_NONE;

    rc = lddata->fli->asyncMRead(lddata->fli, pos, data, cnt, size);

    lddata->asyn_stat = HIP_RDSTAT_INPROG;

    return rc;
}

static S32 HIPLBypassRead(st_HIPLOADDATA* lddata, void* data, S32 cnt, S32 size)
{
    S32 got = 0;

    if (!lddata->bypass)
    {
        return 0;
    }

    if (lddata->use_async)
    {
        return HIPLReadAsync(lddata, lddata->pos, (char*)data, cnt, size);
    }

    if (cnt == 0)
    {
        return 0;
    }

    if (cnt == 0)
    {
        got = 0;
    }
    else if (size == 1)
    {
        got = lddata->fli->readBytes(lddata->fli, (char*)data, cnt);
    }
    else if (size == 2)
    {
        got = lddata->fli->readMShorts(lddata->fli, (S16*)data, cnt);
    }
    else if (size == 4)
    {
        got = lddata->fli->readMLongs(lddata->fli, (S32*)data, cnt);
    }

    return got * size;
}

static S32 HIPLReadString(st_HIPLOADDATA* lddata, char* buf)
{
    S32 n = 0;
    char pad = 0;

    if (lddata->bypass)
    {
        while (HIPLBypassRead(lddata, &buf[n], 1, sizeof(char)))
        {
            if (buf[n] == '\0')
            {
                if (!(n & 1))
                {
                    HIPLBypassRead(lddata, &pad, 1, sizeof(char));
                }

                break;
            }

            n++;
        }
    }
    else
    {
        while (HIPLBlockRead(lddata, &buf[n], 1, sizeof(char)))
        {
            if (buf[n] == '\0')
            {
                if (!(n & 1))
                {
                    HIPLBlockRead(lddata, &pad, 1, sizeof(char));
                }

                break;
            }

            n++;
        }
    }

    return n;
}

static S32 HIPLReadFloats(st_HIPLOADDATA* lddata, F32* data, S32 cnt)
{
    S32 got;

    if (lddata->bypass)
    {
        got = HIPLBypassRead(lddata, data, cnt, sizeof(F32));
    }
    else
    {
        got = HIPLBlockRead(lddata, data, cnt, sizeof(F32));
    }

    got /= sizeof(F32);

    return got;
}

static S32 HIPLReadLongs(st_HIPLOADDATA* lddata, S32* data, S32 cnt)
{
    S32 got;

    if (lddata->bypass)
    {
        got = HIPLBypassRead(lddata, data, cnt, sizeof(S32));
    }
    else
    {
        got = HIPLBlockRead(lddata, data, cnt, sizeof(S32));
    }

    got /= sizeof(S32);

    return got;
}

static S32 HIPLReadShorts(st_HIPLOADDATA* lddata, S16* data, S32 cnt)
{
    S32 got;

    if (lddata->bypass)
    {
        got = HIPLBypassRead(lddata, data, cnt, sizeof(S16));
    }
    else
    {
        got = HIPLBlockRead(lddata, data, cnt, sizeof(S16));
    }

    got /= sizeof(S16);

    return got;
}

static S32 HIPLReadBytes(st_HIPLOADDATA* lddata, char* data, S32 cnt)
{
    if (lddata->bypass)
    {
        return HIPLBypassRead(lddata, data, cnt, sizeof(char));
    }
    else
    {
        return HIPLBlockRead(lddata, data, cnt, sizeof(char));
    }
}

static en_READ_ASYNC_STATUS HIPLPollRead(st_HIPLOADDATA* lddata)
{
    en_READ_ASYNC_STATUS rdstat = HIP_RDSTAT_INPROG;
    en_BIO_ASYNC_ERRCODES pollstat;

    if (!lddata->bypass)
    {
        return HIP_RDSTAT_NOBYPASS;
    }

    if (!lddata->use_async)
    {
        return HIP_RDSTAT_NOASYNC;
    }

    pollstat = lddata->fli->asyncReadStatus(lddata->fli);

    switch (pollstat)
    {
    case BINIO_ASYNC_INPROG:
    {
        rdstat = HIP_RDSTAT_INPROG;
        break;
    }
    case BINIO_ASYNC_DONE:
    {
        rdstat = HIP_RDSTAT_SUCCESS;
        lddata->asyn_stat = HIP_RDSTAT_NONE;
        break;
    }
    case BINIO_ASYNC_FAIL:
    {
        rdstat = HIP_RDSTAT_FAILED;
        lddata->asyn_stat = HIP_RDSTAT_NONE;
        break;
    }
    default:
    {
        lddata->asyn_stat = HIP_RDSTAT_NONE;
        break;
    }
    }

    return rdstat;
}

static void HIPLBlockExit(st_HIPLOADDATA* lddata)
{
    st_HIPLOADBLOCK* top;

    if (!lddata->bypass)
    {
        top = &lddata->stk[lddata->top--];

        lddata->fli->skipBytes(lddata->fli, top->endpos - lddata->pos);
        lddata->pos = top->endpos;
    }
}

static U32 HIPLBlockEnter(st_HIPLOADDATA* lddata)
{
    st_HIPLOADBLOCK* top;
    U32 cid = 0;
    S32 size = 0;

    if (lddata->bypass)
    {
        return 0;
    }

    if (lddata->top >= 0 && lddata->stk[lddata->top].blk_remain <= 0)
    {
        return 0;
    }

    if (HIPLReadLongs(lddata, (S32*)&cid, -1) == 0)
    {
        cid = 0;
    }
    else
    {
        HIPLReadLongs(lddata, &size, -1);

        if (lddata->top >= 0)
        {
            lddata->stk[lddata->top].blk_remain -= size;
        }

        top = &lddata->stk[++lddata->top];

        top->blk_id = cid;
        top->blk_remain = size;
        top->endpos = lddata->pos + top->blk_remain + (top->blk_remain & 1);
        top->flags = 0;
    }

    return cid;
}

static void HIPLSetSpot(st_HIPLOADDATA* lddata, S32 spot)
{
    if (lddata->bypass)
    {
        lddata->pos = spot;
        lddata->fli->seekSpot(lddata->fli, spot);
    }
}

static S32 HIPLSetBypass(st_HIPLOADDATA* lddata, S32 enable, S32 use_async)
{
    lddata->fli->discardDblBuf(lddata->fli);

    if (enable && lddata->bypass)
    {
        return 0;
    }

    if (!enable && !lddata->bypass)
    {
        return 0;
    }

    if (enable)
    {
        lddata->bypass = 1;
        lddata->use_async = use_async;
        lddata->bypass_recover = lddata->fli->position;
    }
    else
    {
        lddata->fli->seekSpot(lddata->fli, lddata->bypass_recover);
        lddata->bypass = 0;
        lddata->use_async = 0;
        lddata->bypass_recover = -1;
    }

    return 1;
}

static U32 HIPLBaseSector(st_HIPLOADDATA* lddata)
{
    return lddata->base_sector;
}

static void HIPLDestroy(st_HIPLOADDATA* lddata)
{
    S32 lockid;

    if (lddata)
    {
        if (lddata->fli)
        {
            lddata->fli->destroy(lddata->fli);
        }

        lockid = lddata->lockid;

        memset(lddata, 0, sizeof(st_HIPLOADDATA));

        g_loadlock &= ~(1 << lockid);
    }
}

static st_HIPLOADDATA* HIPLCreate(const char* filename, char* dblbuf, S32 bufsize, S32)
{
    st_HIPLOADDATA* lddata = NULL;
    st_FILELOADINFO* fli;
    st_HIPLOADBLOCK* tmp_blk;
    S32 i;
    S32 uselock = -1;

    for (i = 0; i < 8; i++)
    {
        if (!(g_loadlock & (1 << i)))
        {
            g_loadlock |= 1 << i;

            uselock = i;
            lddata = &g_hiploadinst[i];

            break;
        }
    }

    if (lddata)
    {
        memset(lddata, 0, sizeof(st_HIPLOADDATA));

        lddata->lockid = uselock;
        lddata->top = -1;
        lddata->base_sector = 0;
        lddata->use_async = 0;
        lddata->asyn_stat = HIP_RDSTAT_NONE;
        lddata->bypass = 0;
        lddata->bypass_recover = -1;
        lddata->pos = 0;
        lddata->readTop = 0;

        for (i = 0; i < 8; i++)
        {
            tmp_blk = &lddata->stk[i];

            tmp_blk->endpos = 0;
            tmp_blk->blk_id = 0;
            tmp_blk->blk_remain = 0;
            tmp_blk->flags = 0;
        }

        fli = xBinioLoadCreate(filename);

        if (fli)
        {
            lddata->fli = fli;
            lddata->base_sector = fli->basesector;

            if (dblbuf && bufsize > 0)
            {
                fli->setDoubleBuf(fli, dblbuf, bufsize);
            }
        }
        else
        {
            HIPLDestroy(lddata);
            lddata = NULL;
        }
    }

    return lddata;
}

st_HIPLOADFUNCS* get_HIPLFuncs()
{
    return &g_map_HIPL_funcmap;
}

//                                                   xGroup

U32 xGroup::get_any()
{
    U16 numItems = this->asset->itemCount;
    if (numItems == 0)
    {
        return NULL;
    }

    U32 last = ((U32*)((U8*)this->asset + sizeof(xGroupAsset)))[this->last_index];
    U32 cnt = this->last_index + 1;
    this->last_index = cnt % numItems;
    return last;
}

U32 xGroupGetItem(xGroup* g, U32 index)
{
    return (U32)((U32*)(g->asset + 1))[index];
}

xBase* xGroupGetItemPtr(xGroup* g, U32 index)
{
    if ((g->flg_group & 1) == 0)
    {
        xGroupSetup(g);
    }
    return g->item != NULL ? g->item[index] : NULL;
}

U32 xGroupGetCount(xGroup* g)
{
    return g->asset->itemCount;
}

S32 xGroupEventCB(xBase* to, xBase* from, U32 toEvent, const F32* toParam, xBase* toParamWidget,
                  U32)
{
    xGroup* g = (xGroup*)from;
    switch (toEvent)
    {
    case eEventReset:
        xGroupReset(g);
        break;
    case eEventDisableGroupContents:
        toEvent = eEventDisable;
        break;
    }
    S32 index = -1;
    if (g->asset->groupFlags & 1)
    {
        U32 rand = xrand();
        index = rand % g->asset->itemCount;
    }
    else if (g->asset->groupFlags & 2)
    {
        index = g->last_index;
        g->last_index = (g->last_index + 1) % g->asset->itemCount;
    }

    switch (toEvent)
    {
    case eEventFastVisible:
        for (S32 i = 0; i < g->asset->itemCount; i++)
        {
            if (!(index == -1 || index == i))
            {
                continue;
            }

            xBase* b = g->item[i];
            if (!b)
            {
                continue;
            }

            if (b->baseFlags & 0x20)
            {
                xEntShow((xEnt*)b);
            }
            else
            {
                zEntEvent(b, toEvent, toParam, toParamWidget);
            }
        }
        return 1;
    case eEventFastInvisible:
        for (S32 i = 0; i < g->asset->itemCount; i++)
        {
            if (!(index == -1 || index == i))
            {
                continue;
            }

            xBase* b = g->item[i];
            if (!b)
            {
                continue;
            }

            if (b->baseFlags & 0x20)
            {
                xEntHide((xEnt*)b);
            }
            else
            {
                zEntEvent(b, toEvent, toParam, toParamWidget);
            }
        }
        return 1;
    default:
        for (S32 i = 0; i < g->asset->itemCount; i++)
        {
            if (!(index == -1 || index == i))
            {
                continue;
            }

            xBase* b = g->item[i];
            if (!b)
            {
                continue;
            }

            zEntEvent(b, toEvent, toParam, toParamWidget);
        }
        return 1;
    }

    // Note (Square): This is unreachable. No idea what the default return was meant to bel
    // but presumably it's different from the good path.
    return 0;
}

void xGroupLoad(xGroup* ent, xSerial* s)
{
    xBaseLoad((xBase*)ent, s);
}

void xGroupSave(xGroup* ent, xSerial* s)
{
    xBaseSave((xBase*)ent, s);
}

void xGroupSetup(xGroup* g)
{
    if ((g->flg_group & 1) == 0)
    {
        U32 numItems = xGroupGetCount(g);
        U32 i = 0;
        U32 ind = 0;
        while (i < numItems)
        {
            xBase* item = xGroupFindItemPtr(g, i);
            i++;
            *(g->item + ind) = item;
            ind++;
        }
        g->flg_group |= 1;
    }
}

void xGroupInit(void* b, void* asset)
{
    xGroupInit((xBase*)b, (xGroupAsset*)asset);
}

//                                          xGrid

extern float xGrid_float_0p001;
extern float xGrid_float_one;
extern float xGrid_float_one_quarter;

void xGridIterClose(xGridIterator& it)
{
    if (it.listhead)
    {
        gGridIterActive--;

        if (it.delfound && !gGridIterActive)
        {
            xGridBound* cell = *it.listhead;
            xGridBound** head = it.listhead;

            while (cell)
            {
                if (cell->deleted)
                {
                    *head = cell->next;

                    cell->next = NULL;
                    cell->head = NULL;
                    cell->deleted = 0;
                    cell->gx = 0xFFFF;
                    cell->gz = 0xFFFF;

                    cell = *head;
                }
                else
                {
                    head = &cell->next;
                    cell = cell->next;
                }
            }
        }

        it.listhead = NULL;
        it.curcell = NULL;
        it.delfound = 0;
    }
}

xGridBound* xGridIterFirstCell(xGrid* grid, F32 posx, F32 posy, F32 posz, S32& grx, S32& grz,
                               xGridIterator& iter)
{
    xGridGetCell(grid, posx, posy, posz, grx, grz);
    return xGridIterFirstCell(grid, grx, grz, iter);
}

void xGridGetCell(xGrid* grid, F32 x, F32 y, F32 z, S32& grx, S32& grz)
{
    F32 pgridx = (x - grid->minx) * grid->inv_csizex;
    F32 pgridz = (z - grid->minz) * grid->inv_csizez;

    grx = MIN(F32((grid->nx - 1) ^ 0x8000), MAX(0, pgridx));
    grz = MIN(F32((grid->nz - 1) ^ 0x8000), MAX(0, pgridx));
}

void xGridUpdate(xGrid* grid, xEnt* ent)
{
}

S32 xGridRemove(xGridBound* bound)
{
    if (bound->head)
    {
        if (gGridIterActive)
        {
            bound->deleted = 1;
            return 0;
        }
        else
        {
            xGridBound* curr = bound->head[0];
            xGridBound** prev = bound->head;
            while (curr && curr != bound)
            {
                prev = &curr->next;
                curr = curr->next;
            }

            *prev = curr->next;
            curr->next = NULL;
            curr->head = NULL;
            curr->deleted = 0;
            curr->gx = -1;
            curr->gz = -1;
        }
    }
    return 1;
}

void xGridKill(xGrid* grid)
{
    xGridEmpty(grid);
    grid->cells = NULL;
}

void xGridInit(xGrid* grid, const xBox* bounds, U16 nx, U16 nz, U8 ingrid_id)
{
    grid->ingrid_id = ingrid_id;
    grid->nx = nx;
    grid->nz = nz;
    grid->minx = bounds->upper.x;
    grid->minz = bounds->upper.z;
    grid->maxx = bounds->lower.x;
    grid->maxz = bounds->lower.z;
    F32 gsizex = grid->maxx - grid->minx;
    F32 gsizez = grid->maxz - grid->minz;
    grid->csizex = gsizex / nx;
    grid->csizez = gsizex / nz;

    if (__fabs(gsizex) <= xGrid_float_0p001)
    {
        grid->inv_csizex = xGrid_float_one;
    }
    else
    {
        grid->inv_csizex = nx / gsizex;
    }

    if (__fabs(gsizez) <= xGrid_float_0p001)
    {
        grid->inv_csizez = xGrid_float_one;
    }
    else
    {
        grid->inv_csizez = nz / gsizez;
    }

    grid->maxr = xGrid_float_one_quarter * MAX(grid->csizex, grid->csizez);
    grid->cells = (xGridBound**)xMemAllocSize(nx * nz * sizeof(xGridBound*));
    memset(grid->cells, 0, sizeof(xGridBound*) * (nz * nx));
}

//                                              xFX

/* boot.HIP texture IDs */
#define ID_gloss_edge 0xB8C2351E
#define ID_rainbowfilm_smooth32 0x741B0566

extern const char _stringBase0_7[];

extern F32 _957_0;
extern F32 _958;
extern F32 _995;
extern F32 _1132;
extern F32 _1171;

RpAtomicCallBackRender gAtomicRenderCallBack = NULL;
F32 EnvMapShininess = 1.0f;
RpLight* MainLight = NULL;

static U32 num_fx_atomics = 0;

static U32 xfx_initted = 0;

static void LightResetFrame(RpLight* light);

static void DrawRing(xFXRing* m)
{
    // todo: uses int-to-float conversion
}

// Inlining issue
void xFXRingRender()
{
    S32 i;
    xFXRing* ring = &ringlist[0];

    for (i = 0; i < RING_COUNT; i++, ring++)
    {
        if (ring->time > _957_0)
        {
            DrawRing(ring);
        }
    }
}

void xFXInit()
{
    if (!xfx_initted)
    {
        xfx_initted = 1;

        RpLight* light = RpLightCreate(rpLIGHTDIRECTIONAL);

        if (light)
        {
            RwFrame* frame = RwFrameCreate();

            if (frame)
            {
                RpLightSetFrame(light, frame);
                LightResetFrame(light);

                MainLight = light;
            }
            else
            {
                RpLightDestroy(light);
            }
        }

        xFXanimUVCreate();
        xFXAuraInit();
    }
}

xFXRing* xFXRingCreate(const xVec3* pos, const xFXRing* params)
{
    xFXRing* ring = &ringlist[0];

    if (!pos || !params)
    {
        return NULL;
    }

    for (S32 i = 0; i < RING_COUNT; i++, ring++)
    {
        if (ring->time <= _957_0)
        {
            // non-matching: _958 is only loaded once

            memcpy(ring, params, sizeof(xFXRing));

            ring->time = _995;
            ring->pos = *pos;
            ring->ring_radius_delta *= _958 / ring->lifetime;
            ring->ring_height_delta *= _958 / ring->lifetime;
            ring->ring_tilt_delta *= _958 / ring->lifetime;

            return ring;
        }
    }

    return NULL;
}

static RpMaterial* MaterialSetEnvMap(RpMaterial* material, void* data);
static RpMaterial* MaterialSetBumpMap(RpMaterial* material, void* data);
static RpMaterial* MaterialSetBumpEnvMap(RpMaterial* material, RwTexture* env, F32 shininess,
                                         RwTexture* bump, F32 bumpiness);

void xFX_SceneEnter(RpWorld* world)
{
    S32 i;
    S32 num = RpWorldGetNumMaterials(world);

    for (i = 0; i < num; i++)
    {
        xSurface* sp = zSurfaceGetSurface(i);
        zSurfaceProps* pp = (zSurfaceProps*)sp->moprops;

        if (pp && pp->asset)
        {
            zSurfMatFX* fxp = &pp->asset->matfx;

            if (fxp->flags)
            {
                if (fxp->flags == 0x10)
                {
                    fxp->flags |= 0x1;
                }

                RpMaterial* mp = RpWorldGetMaterial(world, i);

                if (RpMaterialGetTexture(mp))
                {
                    gFXSurfaceFlags = fxp->flags;

                    if (fxp->flags & 0x1)
                    {
                        RwTexture* env = (RwTexture*)xSTFindAsset(fxp->envmapID, NULL);

                        if (!env)
                        {
                            continue;
                        }

                        MaterialSetEnvMap(mp, env);
                        RpMatFXMaterialSetEnvMapCoefficient(mp, _1132 * fxp->shininess);
                    }

                    if (fxp->flags & 0x2)
                    {
                        RwTexture* bump = (RwTexture*)xSTFindAsset(fxp->bumpmapID, NULL);

                        if (!bump)
                        {
                            continue;
                        }

                        MaterialSetBumpMap(mp, bump);
                        RpMatFXMaterialSetBumpMapCoefficient(mp, fxp->bumpiness);
                    }

                    if (fxp->flags & 0x4)
                    {
                        RwTexture* env = (RwTexture*)xSTFindAsset(fxp->envmapID, NULL);
                        RwTexture* bump = (RwTexture*)xSTFindAsset(fxp->bumpmapID, NULL);

                        if (!env || !bump)
                        {
                            continue;
                        }

                        MaterialSetBumpEnvMap(mp, env, fxp->shininess, bump, fxp->bumpiness);
                    }
                }
            }
        }
    }

    zScene* sc = globals.sceneCur;

    for (i = 0; i < sc->num_act_ents; i++)
    {
        xEnt* ent = sc->act_ents[i];

        if (!gAtomicRenderCallBack && ent->model)
        {
            RpAtomicCallBackRender tmp = RpAtomicGetRenderCallBack(ent->model->Data);

            RpAtomicSetRenderCallBack(ent->model->Data, NULL);

            gAtomicRenderCallBack = RpAtomicGetRenderCallBack(ent->model->Data);

            RpAtomicSetRenderCallBack(ent->model->Data, tmp);
        }

        if (ent->model)
        {
            // non-matching: strings are getting cached for some reason

            U32 bubble;

            bubble = (ent->id == xStrHash(_stringBase0_7));
            bubble |= (ent->id == xStrHash(_stringBase0_7 + 13));
            bubble |= (ent->id == xStrHash(_stringBase0_7 + 28));
            bubble |= (ent->id == xStrHash(_stringBase0_7 + 42));
            bubble |= (ent->id == xStrHash(_stringBase0_7 + 62));
            bubble |= (ent->id == xStrHash(_stringBase0_7 + 75));

            if (bubble)
            {
                xSTAssetName(ent->id);

                RpAtomicSetRenderCallBack(ent->model->Data, xFXBubbleRender);
            }
        }
    }

    num_fx_atomics = 0;
}

void xFX_SceneExit(RpWorld*)
{
}

void xFXUpdate(F32 dt)
{
    xFXRibbonUpdate(dt);
    xFXAuraUpdate(dt);
}

static RpMaterial* MaterialDisableMatFX(RpMaterial* material, void*)
{
    RpMatFXMaterialSetEffects(material, rpMATFXEFFECTNULL);
    return material;
}

RpAtomic* AtomicDisableMatFX(RpAtomic* atomic)
{
    RpMatFXAtomicEnableEffects(atomic);

    RpGeometry* geometry = RpAtomicGetGeometry(atomic);

    if (geometry)
    {
        RpGeometryForAllMaterials(geometry, MaterialDisableMatFX, NULL);
    }

    return atomic;
}

RpMaterial* MaterialSetShininess(RpMaterial* material, void*)
{
    RpMatFXMaterialFlags flags = RpMatFXMaterialGetEffects(material);

    if (flags == rpMATFXEFFECTENVMAP || flags == rpMATFXEFFECTBUMPENVMAP)
    {
        RpMatFXMaterialSetEnvMapCoefficient(material, EnvMapShininess);
    }

    return material;
}

//                                      xtextbox::layout

#define FLAG_UNK40 0x40
#define FLAG_UNK80 0x80

#define WRAP_WORD 0x0
#define WRAP_CHAR 0x10
#define WRAP_NONE 0x20
#define WRAP_MASK (WRAP_WORD | WRAP_CHAR | WRAP_NONE)

#define XJUSTIFY_LEFT 0x0
#define XJUSTIFY_RIGHT 0x1
#define XJUSTIFY_CENTER 0x2
#define XJUSTIFY_MASK (XJUSTIFY_LEFT | XJUSTIFY_RIGHT | XJUSTIFY_CENTER)

#define YJUSTIFY_TOP 0x0
#define YJUSTIFY_BOTTOM 0x4
#define YJUSTIFY_CENTER 0x8
#define YJUSTIFY_MASK (YJUSTIFY_TOP | YJUSTIFY_BOTTOM | YJUSTIFY_CENTER)

extern substr _1615;
extern substr _1616;
extern xtextbox::tag_entry_list _1642;

static const basic_rect<F32> screen_bounds = { 0, 0, 1, 1 };

extern substr text_delims;

xModelInstance* load_model(U32 id);

namespace
{

    size_t parse_split_tag(xtextbox::split_tag& ti)
    {
        ti.value.size = 0;
        ti.action.size = 0;
        ti.name.size = 0;

        // non-matching: scheduling

        substr s;

        s.text = ti.tag.text;
        s.size = ti.tag.size;

        s.text++;
        s.size--;

        ti.name.text = skip_ws(s);
        s.text = find_char(s, text_delims);

        if (!s.text)
        {
            return 0;
        }

        ti.name.size = s.text - ti.name.text;
        s.size -= ti.name.size;
        ti.action.text = skip_ws(s);

        if (!s.size)
        {
            return 0;
        }

        char c = s.text[0];

        if (c == '\0' || c == '{')
        {
            return 0;
        }

        s.text++;
        s.size--;

        if (c == '}')
        {
            return ti.tag.size - s.size;
        }

        ti.action.size = 1;
        ti.value.text = skip_ws(s);
        s.text = find_char(s, '}');
        s.size -= s.text - ti.value.text;

        if (!s.text)
        {
            return 0;
        }

        ti.value.size = s.text - ti.value.text;

        rskip_ws(ti.value);

        s.text++;
        s.size--;

        return ti.tag.size - s.size;
    }

    const char* parse_next_tag_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb,
                                   const char* text, size_t text_size)
    {
        xtextbox::split_tag ti = {};
        ti.tag.text = text;
        ti.tag.size = text_size;

        size_t size = parse_split_tag(ti);

        if (!size)
        {
            return NULL;
        }

        a.s.text = text;
        a.s.size = size;
        a.flag.invisible = a.flag.ethereal = true;

        if (icompare(ti.name, substr::create("~", 1)) == 0 ||
            icompare(ti.name, substr::create("reset", 5)) == 0)
        {
            a.tag = xtextbox::find_format_tag(ti.value);

            if (a.tag && a.tag->reset_tag)
            {
                a.tag->reset_tag(a, tb, ctb, ti);
            }
        }
        else
        {
            a.tag = xtextbox::find_format_tag(ti.name);

            if (a.tag && a.tag->parse_tag)
            {
                a.tag->parse_tag(a, tb, ctb, ti);
            }
        }

        return text + size;
    }

    const char* parse_next_text_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb,
                                    const char* text, size_t text_size)
    {
        char c = text[0];

        a.s.text = text;
        a.s.size = 1;
        a.flag.merge = true;

        if (c == '\n')
        {
            a.flag.line_break = true;
        }
        else if (c == '\t')
        {
            a.flag.tab = true;
        }
        else if (c == '-')
        {
            a.flag.word_end = true;
        }

        if (is_ws(c))
        {
            a.flag.invisible = a.flag.word_break = true;
        }

        a.bounds = tb.font.bounds(c);
        a.cb = &xtextbox::text_cb;
        a.context = NULL;
        a.context_size = 0;

        return a.s.text + a.s.size;
    }

    const char* parse_next_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb,
                               const char* text, size_t text_size)
    {
        const char* ret;

        if (text[0] != '{' || !(ret = parse_next_tag_jot(a, tb, ctb, text, text_size)))
        {
            ret = parse_next_text_jot(a, tb, ctb, text, text_size);
        }

        a.flag.merge = a.flag.merge && !(tb.flags & FLAG_UNK80);
        return ret;
    }

    struct tex_args
    {
        RwRaster* raster;
        F32 rot;
        basic_rect<F32> src;
        basic_rect<F32> dst;
        xVec2 off;
        enum
        {
            SCALE_FONT,
            SCALE_SCREEN,
            SCALE_SIZE,
            SCALE_FONT_WIDTH,
            SCALE_FONT_HEIGHT,
            SCALE_SCREEN_WIDTH,
            SCALE_SCREEN_HEIGHT
        } scale;
    };

    struct model_args
    {
        xModelInstance* model;
        xVec3 rot;
        basic_rect<F32> dst;
        xVec2 off;
        enum
        {
            SCALE_FONT,
            SCALE_SCREEN,
            SCALE_SIZE
        } scale;
    };

    tex_args def_tex_args;
    model_args def_model_args;

    void reset_tex_args(tex_args& ta)
    {
        ta.raster = NULL;
        ta.rot = 0.0f;
        ta.src = ta.dst = basic_rect<F32>::m_Unit;
        ta.off.x = ta.off.y = 1.0f;
        ta.scale = tex_args::SCALE_FONT;
    }

    void load_tex_args(tex_args& ta, const substr& s)
    {
        xtextbox::tag_entry_list el = xtextbox::read_tag(s);
        xtextbox::tag_entry* e;

        if (el.size == 0)
        {
            return;
        }

        e = &el.entries[0];

        if (e->op == ':' || (e->args_size == 1 && e->args[0].size))
        {
            substr& name = e->args[0];
            U32 id;

            if (name.size == 10 && imemcmp(name.text, "0x", 2) == 0)
            {
                id = atox(substr::create(name.text + 2, 8));
            }
            else
            {
                id = xStrHash(name.text, name.size);
            }

            RwTexture* texture = (RwTexture*)xSTFindAsset(id, NULL);

            if (texture && texture->raster && texture->raster->width > 0 &&
                texture->raster->height > 0 && texture->raster->width <= 4096 &&
                texture->raster->height <= 4096)
            {
                RwTextureSetFilterMode(texture, rwFILTERLINEAR);
                ta.raster = texture->raster;
            }
        }

        el.entries++;
        el.size--;

        e = xtextbox::find_entry(el, substr::create("rot", 3));

        if (e && e->op == '=' && e->args_size == 1)
        {
            xtextbox::read_list(*e, &ta.rot, 1);
        }

        e = xtextbox::find_entry(el, substr::create("src", 3));

        if (e && e->op == '=' && e->args_size == 4)
        {
            xtextbox::read_list(*e, (F32*)&ta.src, 4);
        }

        e = xtextbox::find_entry(el, substr::create("dst", 3));

        if (e && e->op == '=' && e->args_size == 4)
        {
            xtextbox::read_list(*e, (F32*)&ta.dst, 4);
        }

        e = xtextbox::find_entry(el, substr::create("off", 3));

        if (e && e->op == '=' && e->args_size == 2)
        {
            xtextbox::read_list(*e, (F32*)&ta.off, 2);
        }

        e = xtextbox::find_entry(el, substr::create("scale", 5));

        if (e && e->op == '=' && e->args_size == 1)
        {
            if (icompare(e->args[0], substr::create("font", 4)) == 0)
            {
                ta.scale = tex_args::SCALE_FONT;
            }
            else if (icompare(e->args[0], substr::create("screen", 6)) == 0)
            {
                ta.scale = tex_args::SCALE_SCREEN;
            }
            else if (icompare(e->args[0], substr::create("size", 4)) == 0)
            {
                ta.scale = tex_args::SCALE_SIZE;
            }
            else if (icompare(e->args[0], substr::create("font_width", 10)) == 0)
            {
                ta.scale = tex_args::SCALE_FONT_WIDTH;
            }
            else if (icompare(e->args[0], substr::create("font_height", 11)) == 0)
            {
                ta.scale = tex_args::SCALE_FONT_HEIGHT;
            }
            else if (icompare(e->args[0], substr::create("screen_width", 12)) == 0)
            {
                ta.scale = tex_args::SCALE_SCREEN_WIDTH;
            }
#if 1 // needed until .sdata2 is decomped
            else if (icompare(e->args[0], substr::create("screen_height\0=:+*;}\0,;}", 13)) ==
#else
            else if (icompare(e->args[0], substr::create("screen_height", 13)) ==
#endif
                     0) // screen_height
            {
                ta.scale = tex_args::SCALE_SCREEN_HEIGHT;
            }
            else
            {
                ta.scale = tex_args::SCALE_FONT;
            }
        }
    }

    void reset_model_args(model_args& ma)
    {
        ma.model = NULL;
        ma.rot = xVec3::m_Null;
        ma.dst = basic_rect<F32>::m_Unit;
        ma.off.x = ma.off.y = 1.0f;
        ma.scale = model_args::SCALE_FONT;
    }

    void load_model_args(model_args& ma, const substr& s)
    {
        xtextbox::tag_entry_list el = xtextbox::read_tag(s);
        xtextbox::tag_entry* e;

        if (!el.size)
        {
            return;
        }

        e = &el.entries[0];

        if (e->op == ':' || (e->args_size == 1 && e->args[0].size))
        {
            substr& name = e->args[0];

            ma.model = load_model(xStrHash(name.text, name.size));
        }

        el.entries++;
        el.size--;

        e = xtextbox::find_entry(el, substr::create("rot", 3));

        if (e && e->op == '=' && e->args_size == 3)
        {
            xtextbox::read_list(*e, (F32*)&ma.rot, 3);
        }

        e = xtextbox::find_entry(el, substr::create("dst", 3));

        if (e && e->op == '=' && e->args_size == 4)
        {
            xtextbox::read_list(*e, (F32*)&ma.dst, 4);
        }

        e = xtextbox::find_entry(el, substr::create("off", 3));

        if (e && e->op == '=' && e->args_size == 2)
        {
            xtextbox::read_list(*e, (F32*)&ma.off, 2);
        }

        e = xtextbox::find_entry(el, substr::create("scale", 5));

        if (e && e->op == '=' && e->args_size == 1)
        {
            if (icompare(e->args[0], substr::create("screen", 6)) == 0)
            {
                ma.scale = model_args::SCALE_SCREEN;
            }
            else
            {
                ma.scale = model_args::SCALE_FONT;
            }
        }
    }

    void start_layout(const xtextbox&)
    {
        reset_tex_args(def_tex_args);
        reset_model_args(def_model_args);
    }

    void stop_layout(const xtextbox&)
    {
    }

    void start_render(const xtextbox& tb)
    {
        tb.font.start_render();
    }

    void stop_render(const xtextbox& tb)
    {
        tb.font.stop_render();
    }
} // namespace

bool xtextbox::layout::changed(const xtextbox& ctb)
{
    U32 flags1 = tb.flags & (WRAP_MASK | FLAG_UNK40);
    U32 flags2 = ctb.flags & (WRAP_MASK | FLAG_UNK40);

    if (tb.text_hash != ctb.text_hash || tb.font.id != ctb.font.id ||
        tb.font.width != ctb.font.width || tb.font.height != ctb.font.height ||
        tb.font.space != ctb.font.space || tb.bounds.w != ctb.bounds.w || flags1 != flags2 ||
        tb.line_space != ctb.line_space)
    {
        return true;
    }

    S32 i = dynamics_size;

    while (i > 0)
    {
        i--;

        jot& j = _jots[dynamics[i]];
        U32 oldval = xStrHash((char*)j.context, j.context_size);

        parse_next_jot(j, tb, ctb, j.s.text, j.s.size);

        U32 val = xStrHash((char*)j.context, j.context_size);

        if (val != oldval)
        {
            return true;
        }
    }

    return false;
}

#define min(a, b) ((a) >= (b) ? (b) : (a))

F32 xtextbox::layout::yextent(F32 max, S32& size, S32 begin_jot, S32 end_jot) const
{
    size = 0;

    if (begin_jot < 0)
    {
        begin_jot = 0;
    }

    if (end_jot < begin_jot)
    {
        end_jot = _jots_size;
    }

    if (begin_jot >= end_jot)
    {
        return 0.0f;
    }

    S32 begin_line = 0;

    while (true)
    {
        if (begin_line >= (S32)_lines_size)
        {
            return 0.0f;
        }

        if ((S32)_lines[begin_line].last > begin_jot)
        {
            break;
        }

        begin_line++;
    }

    // non-matching: wrong float registers
    F32 top = _lines[begin_line].bounds.y;
    S32 i = begin_line;

    while (true)
    {
        if (i == (S32)_lines_size)
        {
            break;
        }

        // non-matching: r11 missing
        const jot_line& line = _lines[i];

        if (line.bounds.y + line.bounds.h > max + top)
        {
            i--;
            break;
        }

        if ((S32)line.last >= end_jot)
        {
            break;
        }

        if (line.page_break)
        {
            break;
        }

        i++;
    }

    if (i < begin_line)
    {
        return 0.0f;
    }

    const jot_line& line = _lines[i];

    size = min((S32)line.last, end_jot) - begin_jot;

    return line.bounds.y + line.bounds.h - top;
}

void xtextbox::layout::render(const xtextbox& ctb, S32 begin_jot, S32 end_jot)
{
    if (begin_jot < 0)
    {
        begin_jot = 0;
    }

    if (end_jot < begin_jot)
    {
        end_jot = _jots_size;
    }

    if (begin_jot >= end_jot)
    {
        return;
    }

    tb = ctb;
    start_render(ctb);

    S32 begin_line = 0;

    while (true)
    {
        if (begin_line >= (S32)_lines_size)
        {
            stop_render(ctb);
            return;
        }

        if ((S32)_lines[begin_line].last > begin_jot)
        {
            break;
        }

        begin_line++;
    }

    for (S32 i = 0; i < begin_jot; i++)
    {
        jot& j = _jots[i];

        if (j.cb && j.cb->render_update)
        {
            j.cb->render_update(j, tb, ctb);
        }
    }

    F32 top = _lines[begin_line].bounds.y;
    U32 li = begin_line - 1;
    S32 line_last = -1;
    F32 x, y;

    for (S32 i = begin_jot; i < end_jot; i++)
    {
        if (i >= line_last)
        {
            li++;
            jot_line& line = _lines[li];

            line_last = line.last;

            x = tb.bounds.x + line.bounds.x;
            y = tb.bounds.y + line.bounds.y + line.baseline - top;

            U32 xj = tb.flags & XJUSTIFY_MASK;

            if (xj == XJUSTIFY_CENTER)
            {
                x += 0.5f * (tb.bounds.w - line.bounds.w);
            }
            else if (xj == XJUSTIFY_RIGHT)
            {
                x += tb.bounds.w - line.bounds.w;
            }

            if (line.page_break && end_jot > line_last)
            {
                end_jot = line_last;
            }
        }

        jot& j = _jots[i];

        if (j.cb)
        {
            if (j.cb->render_update)
            {
                j.cb->render_update(j, tb, ctb);
            }

            if (!j.flag.ethereal && !j.flag.invisible && j.cb->render)
            {
                j.cb->render(j, tb, x + j.bounds.x, y);
            }
        }
    }

    stop_render(ctb);
}

void xtextbox::layout::calc(const xtextbox& ctb, size_t start_text)
{
    if (!start_text)
    {
        dynamics_size = 0;
        context_buffer_size = 0;
        _lines_size = 0;
        _jots_size = 0;
    }

    if (!tb.texts_size)
    {
        return;
    }

    start_layout(ctb);

    jot_line& first_line = _lines[_lines_size];
    first_line.first = 0;
    first_line.bounds.w = 0.0f;
    first_line.bounds.x = 0.0f;
    first_line.bounds.y = 0.0f;
    first_line.baseline = 0.0f;

    struct
    {
        const char* s;
        const char* end;
    } text_stack[16];

    size_t text_stack_size = 0;
    size_t size = ((!tb.text_sizes) ? 0x40000000 : tb.text_sizes[start_text]);
    size_t text_index = start_text + 1;
    const char* s = tb.texts[start_text];
    const char* end = s + size;
    const char* r25;

    while (true)
    {
        jot& a = _jots[_jots_size];
        jot_line& line = _lines[_lines_size];

        a.context = &context_buffer[context_buffer_size];
        a.context_size = 0;
        a.reset_flags();
        a.cb = NULL;
        a.tag = NULL;

        if (s == end || *s == '\0')
        {
            if (text_stack_size)
            {
                text_stack_size--;
                s = text_stack[text_stack_size].s;
                end = text_stack[text_stack_size].end;
            }
            else if (text_index < tb.texts_size)
            {
                size = ((!tb.text_sizes) ? 0x40000000 : tb.text_sizes[text_index]);

                s = tb.texts[text_index];
                end = s + size;
                text_index++;
            }
            else
            {
                break;
            }

            a.flag.invisible = a.flag.ethereal = true;
            a.s = substr::create(NULL, 0);

            _jots_size++;
        }
        else
        {
            r25 = parse_next_jot(a, tb, ctb, s, end - s);

            if (a.context == &context_buffer[context_buffer_size])
            {
                context_buffer_size += ALIGN(a.context_size, 4);
            }

            _jots_size++;

            if (a.cb && a.cb->layout_update)
            {
                a.cb->layout_update(a, tb, ctb);
            }

            if (a.flag.stop)
            {
                break;
            }

            if (!a.flag.ethereal)
            {
                a.bounds.x += line.bounds.w;
                line.bounds.w += a.bounds.w;

                if (line.bounds.w >= tb.bounds.w)
                {
                    line.last = _jots_size;

                    if (!fit_line())
                    {
                        break;
                    }

                    next_line();
                }
            }

            if (a.flag.line_break || a.flag.page_break)
            {
                line.last = _jots_size;

                if (!fit_line())
                {
                    break;
                }

                next_line();
            }

            s = r25;

            if (a.flag.insert)
            {
                s = (const char*)a.context;
                text_stack[text_stack_size].s = r25;
                text_stack[text_stack_size].end = end;
                text_stack_size++;
                end = s + a.context_size;
            }
        }
    }

    jot_line& last_line = _lines[_lines_size];

    if (last_line.first < _jots_size)
    {
        last_line.last = _jots_size;

        if (fit_line())
        {
            _lines_size++;
        }
    }

    for (size_t i = 0; i < _jots_size; i++)
    {
        if (_jots[i].flag.dynamic)
        {
            dynamics[dynamics_size++] = i;
        }
    }

    stop_layout(ctb);
}

bool xtextbox::layout::fit_line(bool)
{
    jot_line& line = _lines[_lines_size];

    if (line.bounds.w > tb.bounds.w)
    {
        switch (tb.flags & WRAP_MASK)
        {
        case WRAP_CHAR:
        {
            if (line.last > line.first + 1)
            {
                line.last--;
            }

            break;
        }
        case WRAP_NONE:
        {
            return false;
        }
        default:
        {
            for (S32 i = (S32)line.last - 1; i > (S32)line.first; i--)
            {
                if (_jots[i].flag.word_break)
                {
                    line.last = i + 1;
                    break;
                }
                else if (_jots[i - 1].flag.word_end)
                {
                    line.last = i;
                    break;
                }
            }

            if (line.last <= line.first)
            {
                line.last = line.first + 1;
            }

            trim_line(line);
            break;
        }
        }
    }

    merge_line(line);
    bound_line(line);

    return true;
}

void xtextbox::layout::merge_line(jot_line& line)
{
    size_t d = line.first;
    size_t i = line.first + 1;

    while (i != line.last)
    {
        jot& a1 = _jots[d];
        jot& a2 = _jots[i];

        if (!a1.flag.ethereal && !a2.flag.ethereal && a1.flag.merge && a2.flag.merge &&
            a1.cb == a2.cb)
        {
            a1.s.size = a2.s.text - a1.s.text + a2.s.size;
            a1.bounds |= a2.bounds;
            a1.intersect_flags(a2);
        }
        else
        {
            d++;

            if (d != i)
            {
                _jots[d] = a2;
            }
        }

        i++;
    }

    erase_jots(d + 1, line.last);
    line.last = d + 1;
}

void xtextbox::layout::trim_line(jot_line& line)
{
    // non-matching: mtctr and bdnz not generated

    for (S32 i = line.last - 1; i >= line.first; i--)
    {
        jot& a = _jots[i];

        if (!a.flag.ethereal)
        {
            if (a.flag.invisible)
            {
                erase_jots(i, i + 1);
                line.last--;
            }

            break;
        }
    }

    for (S32 i = line.first; i < line.last; i++)
    {
        jot& a = _jots[i];

        if (!a.flag.ethereal)
        {
            if (a.flag.invisible)
            {
                erase_jots(i, i + 1);
            }

            break;
        }
    }
}

void xtextbox::layout::refresh_end(const xtextbox& tb)
{
    size_t texts_size = this->tb.texts_size;

    if (texts_size > tb.texts_size)
    {
        this->tb = tb;
        calc(tb, texts_size);
    }
}

void xtextbox::layout::clear()
{
    dynamics_size = 0;
    context_buffer_size = 0;
    _lines_size = 0;
    _jots_size = 0;
    tb = xtextbox::create();
}

void xtextbox::layout::refresh(const xtextbox& tb, bool force)
{
    if (force || changed(tb))
    {
        this->tb = tb;
        calc(tb, 0);
    }
}

void xfont::irender(const char* text, F32 x, F32 y) const
{
    irender(text, 0x40000000, x, y);
}

//                                            xFog

void xFogUpdate(xBase* ent, xScene* sc, F32 dt)
{
}

S32 xFogEventCB(xBase* to, xBase* from, U32 toEvent, const F32* toParam, xBase* b3, U32)
{
    switch (toEvent)
    {
    case eEventOn:
        iFogParams fog;
        fog.type = rwFOGTYPELINEAR;
        xFogAsset* fromFog = ((_xFog*)from)->tasset;
        fog.start = fromFog->fogStart;
        fog.stop = fromFog->fogStop;
        fog.density = fromFog->fogDensity;
        fog.fogcolor.red = fromFog->fogColor[0];
        fog.fogcolor.green = fromFog->fogColor[1];
        fog.fogcolor.blue = fromFog->fogColor[2];
        fog.fogcolor.alpha = fromFog->fogColor[3];
        fog.bgcolor.red = fromFog->bkgndColor[0];
        fog.bgcolor.green = fromFog->bkgndColor[1];
        fog.bgcolor.blue = fromFog->bkgndColor[2];
        fog.bgcolor.alpha = fromFog->bkgndColor[3];
        fog.table = NULL;
        iCameraSetFogParams(&fog, fromFog->transitionTime);
        break;
    case eEventOff:
        iCameraSetFogParams(NULL, 0.0f);
        break;
    case eEventReset:
        xFogReset((_xFog*)from);
        break;
    }
    return eEventEnable;
}

void xFogLoad(_xFog* ent, xSerial* s)
{
    xBaseLoad((xBase*)ent, s);
}

void xFogSave(_xFog* ent, xSerial* s)
{
    xBaseSave((xBase*)ent, s);
}

void xFogInit(void* b, void* tasset)
{
    xFogInit((xBase*)b, (xFogAsset*)tasset);
}

void xFogInit(xBase* ent, xFogAsset* tasset)
{
    xBaseInit(ent, (xBaseAsset*)tasset);
    //ent->eventFunc = xFogEventCB;
    ((_xFog*)ent)->tasset = tasset;
    if (ent->linkCount != 0)
    {
        ent->link = (xLinkAsset*)(((_xFog*)ent)->tasset + 1);
    }
    else
    {
        ent->link = NULL;
    }
}

void xFogClearFog()
{
    iCameraSetFogParams(NULL, 0.0f);
}

//                                               xFFX

extern xFFX* alist;
extern xFFXShakeState* shake_alist;
extern U32 rot_match_psize;
extern xFFXRotMatchState* rot_match_pool;
extern xFFXRotMatchState* rot_match_alist;

xFFXRotMatchState* xFFXRotMatchAlloc()
{
    xFFXRotMatchState* alist2;
    alist2 = rot_match_alist;
    if (rot_match_alist == NULL)
    {
        return NULL;
    }
    rot_match_alist = rot_match_alist->next;
    return alist2;
}

// Some instructions are in the wrong order.
void xFFXRotMatchPoolInit(U32 num)
{
    rot_match_psize = num;
    rot_match_pool = (xFFXRotMatchState*)xMemAllocSize(num * sizeof(xFFXRotMatchState));
    U32 i = 1;
    rot_match_pool->next = NULL;
    S32 ind = sizeof(xFFXRotMatchState);
    while (i < rot_match_psize)
    {
        S32 nextAddr = ind - 1;
        i++;
        *(xFFXRotMatchState**)((S32)&rot_match_pool->next + ind) = rot_match_pool + nextAddr;
        ind += sizeof(xFFXRotMatchState);
    }
    rot_match_alist = rot_match_pool + (rot_match_psize - 1);
}

void xFFXShakeFree(xFFXShakeState* s)
{
    s->next = shake_alist;
    shake_alist = s;
}

xFFXShakeState* xFFXShakeAlloc()
{
    xFFXShakeState* alist2;
    alist2 = shake_alist;
    if (shake_alist == NULL)
    {
        return NULL;
    }
    shake_alist = shake_alist->next;
    return alist2;
}

void xFFXShakePoolInit(U32 num)
{
    rot_match_psize = num;
    rot_match_pool = (xFFXRotMatchState*)xMemAllocSize(num * sizeof(xFFXRotMatchState));
    U32 i = 1;
    rot_match_pool->next = NULL;
    S32 ind = sizeof(xFFXRotMatchState);
    while (i < rot_match_psize)
    {
        S32 nextAddr = ind - 1;
        i++;
        *(xFFXRotMatchState**)((S32)&rot_match_pool->next + ind) = rot_match_pool + nextAddr;
        ind += sizeof(xFFXRotMatchState);
    }
    rot_match_alist = rot_match_pool + (rot_match_psize - 1);
}

void xFFXApply(xEnt* ent, xScene* sc, F32 dt)
{
    if ((xFFX*)ent->ffx != NULL)
    {
        xFFXApplyOne((xFFX*)ent->ffx, ent, sc, dt);
    }
}

void xFFXApplyOne(xFFX* ffx, xEnt* ent, xScene* sc, F32 dt)
{
    if (ffx->next != NULL)
    {
        xFFXApplyOne(ffx->next, ent, sc, dt);
    }
    if ((ffx->flags & 1) != 0)
    {
        ffx->doEffect(ent, sc, dt, ffx->fdata);
    }
}

S16 xFFXAddEffect(xEnt* ent, void (*dof)(xEnt*, xScene*, F32, void*), void* fd)
{
    xFFX* f = (xFFX*)xFFXAlloc();
    S16 effectID;
    if (f == NULL)
    {
        effectID = -1;
    }
    else
    {
        f->doEffect = dof;
        f->fdata = fd;
        effectID = xFFXAddEffect(ent, f);
    }
    return effectID;
}

S16 xFFXAddEffect(xEnt* ent, xFFX* f)
{
    U8 numFFX;
    f->next = (xFFX*)ent->ffx;
    f->flags = f->flags | 1;
    *(xFFX**)&ent->ffx = f;
    numFFX = ent->num_ffx;
    ent->num_ffx = numFFX + 1;
    return numFFX;
}

xFFX* xFFXAlloc()
{
    xFFX* alist2;
    alist2 = alist;
    if (alist == NULL)
    {
        return NULL;
    }
    alist = alist->next;
    return alist2;
}

void xFFXPoolInit(U32 num_ffx)
{
    rot_match_psize = num_ffx;
    rot_match_pool = (xFFXRotMatchState*)xMemAllocSize(num_ffx * sizeof(xFFXRotMatchState));
    U32 i = 1;
    rot_match_pool->next = NULL;
    S32 ind = sizeof(xFFXRotMatchState);
    while (i < rot_match_psize)
    {
        S32 nextAddr = ind - 1;
        i++;
        *(xFFXRotMatchState**)((S32)&rot_match_pool->next + ind) = rot_match_pool + nextAddr;
        ind += sizeof(xFFXRotMatchState);
    }
    rot_match_alist = rot_match_pool + (rot_match_psize - 1);
}

//                                      xFactory

static S32 OrdComp_infotype(void* vkey, void* vitem)
{
    S32 rc;

    if (((XGOFTypeInfo*)vkey)->tid < ((XGOFTypeInfo*)vitem)->tid)
    {
        rc = -1;
    }
    else if (((XGOFTypeInfo*)vkey)->tid > ((XGOFTypeInfo*)vitem)->tid)
    {
        rc = 1;
    }
    else
    {
        rc = 0;
    }

    return rc;
}

static S32 OrdTest_infotype(const void* vkey, void* vitem)
{
    S32 rc;

    if (((XGOFTypeInfo*)vkey)->tid < ((XGOFTypeInfo*)vitem)->tid)
    {
        rc = -1;
    }
    else if (((XGOFTypeInfo*)vkey)->tid > ((XGOFTypeInfo*)vitem)->tid)
    {
        rc = 1;
    }
    else
    {
        rc = 0;
    }

    return rc;
}

void xFactory::DestroyItem(xFactoryInst* item)
{
    S32 idx;
    XGOFTypeInfo pattern = {};

    pattern.tid = item->itemType;

    if (item)
    {
        if (products == item)
        {
            products = item->nextprod;

            if (products)
            {
                products->prevprod = NULL;
            }
        }

        if (item->prevprod)
        {
            item->prevprod->nextprod = item->nextprod;
        }

        if (item->nextprod)
        {
            item->nextprod->prevprod = item->prevprod;
        }

        item->prevprod = NULL;
        item->nextprod = NULL;

        idx = XOrdLookup(&infolist, &pattern, OrdTest_infotype);

        ((XGOFTypeInfo*)infolist.list[idx])->destroyer(item);
    }
}

void xFactory::DestroyAll()
{
    while (products)
    {
        DestroyItem(products);
    }
}

xFactoryInst* xFactory::CreateItem(S32 typeID, void* userdata, RyzMemGrow* callerzgrow)
{
    S32 idx;
    xFactoryInst* item;
    XGOFTypeInfo pattern = {};
    XGOFTypeInfo* darec = NULL;
    RyzMemGrow* grow = callerzgrow;

    pattern.tid = typeID;

    idx = XOrdLookup(&infolist, &pattern, OrdTest_infotype);

    if (idx >= 0)
    {
        darec = (XGOFTypeInfo*)infolist.list[idx];
    }

    if (!darec)
    {
        return NULL;
    }

    if (!darec->creator)
    {
        return NULL;
    }

    if (!darec->destroyer)
    {
        return NULL;
    }

    if (!grow && growContextData.IsEnabled())
    {
        grow = &growContextData;
    }

    item = darec->creator(darec->tid, grow, userdata);

    if (!item)
    {
        return item;
    }

    item->itemType = darec->tid;
    item->prevprod = NULL;
    item->nextprod = NULL;

    if (products)
    {
        item->nextprod = products;
        products->prevprod = item;
        products = item;
    }
    else
    {
        products = item;
    }

    return item;
}

void xFactory::GrowDataDisable()
{
    growContextData.Done();
}

void xFactory::GrowDataEnable(xBase* user, S32 isResume)
{
    if (isResume)
    {
        growContextData.Resume(user);
    }
    else
    {
        growContextData.Init(user);
    }
}

S32 xFactory::RegItemType(S32 tid, XGOFTypeInfoCreator create, XGOFTypeInfoDestroyer destroy)
{
    XGOFTypeInfo typerec[2] = {};

    typerec[0].tid = tid;
    typerec[0].creator = create;
    typerec[0].destroyer = destroy;

    return RegItemType(typerec);
}

S32 xFactory::RegItemType(XGOFTypeInfo* info)
{
    S32 rc = 0;
    XGOFTypeInfo* tptr = info;
    XGOFTypeInfo* nextrec;

    while (tptr->tid)
    {
        rc = 1;

        if (!tptr->creator)
        {
            rc = 0;
            break;
        }

        if (!tptr->destroyer)
        {
            rc = 0;
            break;
        }

        if (infolist.cnt >= infolist.max)
        {
            rc = -2;
            break;
        }

        S32 idx = XOrdLookup(&infolist, tptr, OrdTest_infotype);

        if (idx >= 0)
        {
            rc = -1;
            break;
        }

        nextrec = &infopool[infolist.cnt];
        nextrec->tid = tptr->tid;
        nextrec->creator = tptr->creator;
        nextrec->destroyer = tptr->destroyer;

        XOrdInsert(&infolist, nextrec, OrdComp_infotype);

        tptr++;
    }

    return rc;
}

xFactory::~xFactory()
{
    this->infopool = NULL;
    XOrdDone(&this->infolist, 0);
}

xFactory::xFactory(S32 maxTypes)
{
    U32 amt = maxTypes * sizeof(XGOFTypeInfo);
    this->infopool = (XGOFTypeInfo*)xMemAllocSize(amt);
    memset(this->infopool, 0, amt);
    XOrdInit(&this->infolist, maxTypes, 0);
}

//                                              zEntEvent

void zEntEvent(U32 toID, U32 toEvent, F32 toParam0, F32 toParam1, F32 toParam2, F32 toParam3)
{
    xBase* sendTo;
    F32 toParam[4];

    toParam[0] = toParam0;
    toParam[1] = toParam1;
    toParam[2] = toParam2;
    toParam[3] = toParam3;

    sendTo = zSceneFindObject(toID);

    if (sendTo)
    {
        zEntEvent(sendTo, toEvent, toParam);
    }
}

void zEntEvent(U32 toID, U32 toEvent)
{
    xBase* sendTo = zSceneFindObject(toID);

    if (sendTo)
    {
        zEntEvent(sendTo, toEvent);
    }
}

void zEntEvent(char* to, U32 toEvent)
{
    U32 id = xStrHash(to);
    xBase* sendTo = zSceneFindObject(id);

    if (sendTo)
    {
        zEntEvent(sendTo, toEvent);
    }
}

//                                          xEnv

xEnv* gCurXEnv;

void xEnvRender(xEnv* env, bool)
{
    if (env->geom != NULL)
    {
        iEnvRender(env->geom, false);
    }
}

void xEnvFree(xEnv* env)
{
    if (env->geom != NULL)
    {
        iEnvFree(env->geom);
        env->geom = NULL;
    }
}

void xEnvSetup(xEnv* env)
{
    iEnvDefaultLighting(env->geom);
    env->lightKit = NULL;
    gCurXEnv = env;
}

//                                 xEntMotionTranslate

// Artificial
enum en_MOTIONTYPE
{
    MOTION_TYPE_EXRT,
    MOTION_TYPE_ORBT,
    MOTION_TYPE_SPLN,
    MOTION_TYPE_MVPT,
    MOTION_TYPE_MECH,
    MOTION_TYPE_PEND
};

void xEntMechReverse(xEntMotion* motion)
{
    xEntMotionMechData* mech = &(motion->asset->mech);
    xEntMotionAsset* mkasst = motion->asset;

    xEntMotionRun(motion);

    if (motion->mech.state == 0)
    {
        motion->mech.ss = -motion->mech.ss;
        motion->mech.sr = -motion->mech.sr;
        motion->tmr = mkasst->mech.sld_tm - motion->tmr;
        motion->mech.state = 3;
    }
    else if (motion->mech.state == 1)
    {
        motion->mech.ss = -motion->mech.ss;
        motion->mech.sr = -motion->mech.sr;
        motion->tmr = mkasst->mech.rot_tm - motion->tmr;
        motion->mech.state = 4;
    }
    else if ((motion->mech.state != 2) && (motion->mech.state != 3) && (motion->mech.state != 4) &&
             (motion->mech.state != 5) && (motion->mech.state == 6))
    {
        motion->mech.ss = -motion->mech.ss;
        motion->mech.sr = -motion->mech.sr;
        motion->tmr = 0.0f;

        if ((mech->type == 0) || (mech->type == 2) || (mech->type == 4))
        {
            motion->mech.state = 3;
        }
        else
        {
            motion->mech.state = 4;
        }
    }
}

void xEntMechForward(xEntMotion* motion)
{
    xEntMotionMechData* mech = &(motion->asset->mech);
    xEntMotionAsset* mkasst = motion->asset;

    xEntMotionRun(motion);

    if ((motion->mech.state != 0) && (motion->mech.state != 1) && (motion->mech.state != 2))
    {
        if (motion->mech.state == 3)
        {
            motion->mech.ss = -motion->mech.ss;
            motion->mech.sr = -motion->mech.sr;
            motion->tmr = mkasst->mech.sld_tm - motion->tmr;
            motion->mech.state = 0;
        }
        else if (motion->mech.state == 4)
        {
            motion->mech.ss = -motion->mech.ss;
            motion->mech.sr = -motion->mech.sr;
            motion->tmr = mkasst->mech.rot_tm - motion->tmr;
            motion->mech.state = 1;
        }
        else if ((motion->mech.state != 5) && (motion->mech.state != 6) &&
                 (motion->mech.state == 7))
        {
            motion->mech.ss = -motion->mech.ss;
            motion->mech.sr = -motion->mech.sr;
            motion->tmr = 0.0f;

            if ((mech->type == 0) || (mech->type == 2) || (mech->type == 4))
            {
                motion->mech.state = 0;
            }
            else
            {
                motion->mech.state = 1;
            }
        }
    }
}

void xEntMotionInit(xEntMotion* a, xEnt* b, xEntMotionAsset* c)
{
    a->asset = c;
    a->type = c->type;
    a->flags = c->flags;

    if (a->type == MOTION_TYPE_EXRT)
    {
        xVec3Copy(&a->er.a, &c->er.ret_pos);
        xVec3Add(&a->er.b, &c->er.ret_pos, &c->er.ext_dpos);

        a->er.et = c->er.ext_tm;
        a->er.wet = c->er.ext_wait_tm;
        a->er.rt = c->er.ret_tm;
        a->er.wrt = c->er.ret_wait_tm;

        if (a->er.p <= 0)
        {
            a->er.p = 10.0f;
        }

        a->er.brt = a->er.et + a->er.wet;
        a->er.ert = a->er.brt + a->er.rt;
        a->er.p = a->er.ert + a->er.wrt;
    }
    else if (a->type == MOTION_TYPE_ORBT)
    {
        xVec3Copy((xVec3*)(&a->er.b), &c->er.ret_pos);

        a->orb.a = c->orb.w;
        a->orb.b = c->orb.h;

        if (c->orb.period <= 0.0f)
        {
            c->orb.period = 10.0f;
        }

        a->orb.p = c->orb.period;
        a->orb.w = (2 * PI) / c->orb.period;
    }
    else if (a->type == MOTION_TYPE_MVPT)
    {
        // literally nothing
    }
    else if (a->type == MOTION_TYPE_PEND)
    {
        if (c->pen.period <= 1e-5f)
        {
            c->pen.period = 0;
        }

        a->pen.w = (2 * PI) / c->pen.period;
    }
    else if (a->type == MOTION_TYPE_MECH)
    {
        if (c->mp.speed < 1e-5f)
        {
            c->mp.speed = 0;
        }

        if (c->mech.sld_acc_tm + c->mech.sld_dec_tm > c->mech.sld_tm)
        {
            c->mech.sld_dec_tm = c->mech.sld_acc_tm = c->mech.sld_tm * 0.5f;
        }

        if (c->mech.rot_tm < 3.0f)
        {
            c->mech.rot_tm = 1.0f;
        }

        if (c->mech.type == 2)
        {
            if (c->mech.rot_tm != c->mech.sld_tm)
            {
                c->mech.rot_tm = c->mech.sld_tm;
            }
        }
        if (c->mech.rot_acc_tm + c->mech.rot_dec_tm > c->mech.rot_tm)
        {
            c->mech.rot_dec_tm = c->mech.rot_acc_tm = c->mech.rot_tm * 0.5f;
        }
    }

    a->owner = b;
    a->target = NULL;

    xEntMotionDebugAdd(a);
}

//                                              xEntDrive

// DONT FORGET, THESE ARE COPIED FROM BFBB
// THEY WILL NOT BE 100% CORRECT OR EVEN CLOSE

void xEntDriveUpdate(xEntDrive* drv, xScene* s, F32 dt, const xCollis* coll)
{
    if ((drv != NULL && (drv->odriver != NULL || drv->driver != NULL)))
    {
        if (drv->driven == NULL || drv->driven->frame == NULL ||
            (drv->odriver != NULL && drv->odriver->frame == NULL))
        {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        if (drv->otmr > 0.0f)
        {
            drv->otmr -= dt;
            if (drv->otmr <= 0.0f)
            {
                drv->os = 0.0f;
                drv->otmr = 0.0f;
            }
            else
            {
                drv->os = drv->otmr / drv->otm;
            }
        }

        if (drv->tmr > 0.0f)
        {
            drv->tmr -= dt;
            if (drv->tmr <= 0.0f)
            {
                drv->s = 1.0f;
                drv->tmr = 0.0f;
            }
            else
            {
                drv->s = 1.0f - drv->tmr / drv->tm;
            }
        }

        if (!drv->os && !drv->s)
        {
            return;
        }

        xVec3 euler;
        xMat3x3 rot;
        xMat3x3 a_descaled;
        if (drv->s && drv->flags & 1)
        {
            if (drv->driver == NULL || drv->driver->frame == NULL)
            {
                xEntDriveInit(drv, drv->driven);
                return;
            }

            {
                F32 len2 = SQR(drv->driver->frame->mat.right.x) +
                           SQR(drv->driver->frame->mat.right.y) +
                           SQR(drv->driver->frame->mat.right.z);
                if (xabs(len2 - 1) <= 0.00001f)
                {
                    a_descaled.right.x = drv->driver->frame->mat.right.x;
                    a_descaled.right.y = drv->driver->frame->mat.right.y;
                    a_descaled.right.z = drv->driver->frame->mat.right.z;
                }
                else if (xabs(len2) <= 0.00001f)
                {
                    a_descaled.right.x = 0.0f;
                    a_descaled.right.y = 1.0f;
                    a_descaled.right.z = 0.0f;
                }
                else
                {
                    F32 len_inv = 1.0f / xsqrt(len2);
                    a_descaled.right.x = drv->driver->frame->mat.right.x * len_inv;
                    a_descaled.right.y = drv->driver->frame->mat.right.y * len_inv;
                    a_descaled.right.z = drv->driver->frame->mat.right.z * len_inv;
                }
            }

            {
                F32 len2 = SQR(drv->driver->frame->mat.up.x) + SQR(drv->driver->frame->mat.up.y) +
                           SQR(drv->driver->frame->mat.up.z);
                if (xabs(len2 - 1.0f) <= 0.00001f)
                {
                    a_descaled.up.x = drv->driver->frame->mat.up.x;
                    a_descaled.up.y = drv->driver->frame->mat.up.y;
                    a_descaled.up.z = drv->driver->frame->mat.up.z;
                }
                else if (xabs(len2) <= 0.00001f)
                {
                    a_descaled.up.x = 0.0f;
                    a_descaled.up.y = 1.0f;
                    a_descaled.up.z = 0.0f;
                }
                else
                {
                    F32 len_inv = 1.0f / xsqrt(len2);
                    a_descaled.up.x = drv->driver->frame->mat.up.x * len_inv;
                    a_descaled.up.y = drv->driver->frame->mat.up.y * len_inv;
                    a_descaled.up.z = drv->driver->frame->mat.up.z * len_inv;
                }
            }

            {
                F32 len2 = SQR(drv->driver->frame->mat.at.x) + SQR(drv->driver->frame->mat.at.y) +
                           SQR(drv->driver->frame->mat.at.z);
                if (xabs(len2 - 1.0f) <= 0.00001f)
                {
                    a_descaled.at.x = drv->driver->frame->mat.at.x;
                    a_descaled.at.y = drv->driver->frame->mat.at.y;
                    a_descaled.at.z = drv->driver->frame->mat.at.z;
                }
                else if (xabs(len2) <= 0.00001f)
                {
                    a_descaled.at.x = 0.0f;
                    a_descaled.at.y = 1.0f;
                    a_descaled.at.z = 0.0f;
                }
                else
                {
                    F32 len_inv = 1.0f / xsqrt(len2);
                    a_descaled.at.x = drv->driver->frame->mat.at.x * len_inv;
                    a_descaled.at.y = drv->driver->frame->mat.at.y * len_inv;
                    a_descaled.at.z = drv->driver->frame->mat.at.z * len_inv;
                }
            }
            xMat3x3GetEuler(&a_descaled, &euler);
            xMat3x3RotY(&rot, drv->s * (euler.x - drv->yaw));
            xMat3x3Mul(&drv->driven->frame->mat, &drv->driven->frame->mat, &rot);
            drv->yaw = euler.x;
        }

        drv->dloc = 0.0f;
        xVec3 newq;
        if (drv->os)
        {
            if (drv->odriver == NULL || drv->odriver->frame == NULL)
            {
                xEntDriveInit(drv, drv->driven);
                return;
            }

            xMat4x3Toworld(&newq, &drv->odriver->frame->mat, &drv->op);
            xVec3Sub(&drv->driven->frame->dpos, &newq, &drv->q);
            xVec3SMulBy(&drv->driven->frame->dpos, drv->os);
            xVec3AddTo(&drv->driven->frame->mat.pos, &drv->driven->frame->dpos);
            drv->dloc += drv->driven->frame->dpos;
        }

        if (drv->s)
        {
            if (drv->driver == NULL || drv->driver->frame == NULL)
            {
                xEntDriveInit(drv, drv->driven);
                return;
            }

            if (drv->flags & 0x2)
            {
                xModelInstance& m = *drv->driver->model;
                if (xModelAnimCollDirty(m))
                {
                    xModelAnimCollRefresh(m);
                }
                xVec3 world_loc;
                xMat4x3Toworld(&world_loc, &drv->driver->frame->mat, &drv->tri.loc);

                xVec3 new_loc;
                new_loc = xCollisTriHit(drv->tri, m);
                drv->driven->frame->dpos = new_loc - world_loc;

                if (drv->tri.index != drv->tri.coll->tri.index ||
                    !(xabs(drv->tri.r - drv->tri.coll->tri.r) <= 0.1f) ||
                    !(xabs(drv->tri.d - drv->tri.coll->tri.d) <= 0.1f))
                {
                    *(xCollis::tri_data*)&drv->tri = drv->tri.coll->tri;
                }

                xMat4x3 oldmat = *(xMat4x3*)m.Mat;
                *(xMat4x3*)m.Mat = drv->driver->frame->mat;

                drv->tri.loc = xCollisTriHit(drv->tri, m);
                xMat4x3Tolocal(&drv->tri.loc, &drv->driver->frame->mat, &drv->tri.loc);
                *(xMat4x3*)m.Mat = oldmat;
            }
            else
            {
                xMat4x3Toworld(&newq, &drv->driver->frame->mat, &drv->p);
                xVec3Sub(&drv->driven->frame->dpos, &newq, &drv->q);
            }

            drv->driven->frame->dpos *= drv->s;
            drv->dloc += drv->driven->frame->dpos;
            drv->driven->frame->mat.pos += drv->driven->frame->dpos;
            if (drv->driven->model != NULL)
            {
                *(xVec3*)&drv->driven->model->Mat->pos = drv->driven->frame->mat.pos;
            }
        }

        xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);
        if (drv->os)
        {
            if (drv->odriver == NULL || drv->odriver->frame == NULL)
            {
                xEntDriveInit(drv, drv->driven);
                return;
            }
            xMat4x3Tolocal(&drv->op, &drv->odriver->frame->mat, &drv->q);
        }
        if (drv->s)
        {
            if (drv->driver == NULL || drv->driver->frame == NULL)
            {
                xEntDriveInit(drv, drv->driven);
                return;
            }
            xMat4x3Tolocal(&drv->p, &drv->driver->frame->mat, &drv->q);
        }
    }
}

void xEntDriveDismount(xEntDrive* drv, F32 dmt)
{
    if (drv == NULL)
    {
        return;
    }

    if (drv->driver == NULL)
    {
        return;
    }

    if (drv->driven == NULL || drv->driven->frame == NULL || drv->driver->frame == NULL)
    {
        xEntDriveInit(drv, drv->driven);
        return;
    }
    drv->odriver = drv->driver;
    drv->os = drv->s;
    drv->otm = dmt;
    drv->otmr = dmt * drv->os;
    if (drv->driver != NULL)
    {
        drv->driver->driving_count--;
    }

    drv->driver = NULL;
    drv->s = 0.0f;
    drv->tm = 0.0f;
    drv->tmr = 0.0f;
    drv->flags &= ~2;
    xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);
    xMat4x3Tolocal(&drv->op, &drv->odriver->frame->mat, &drv->q);
}

void xEntDriveMount(xEntDrive* drv, xEnt* driver, F32 mt, const xCollis* coll)
{
    if (drv->driven == NULL || drv->driven->frame == NULL || driver == NULL ||
        driver->frame == NULL)
    {
        xEntDriveInit(drv, drv->driven);
        return;
    }
    drv->dloc = 0.0f;
    if (driver == drv->odriver && drv->os)
    {
        drv->driver = driver;
        driver->driving_count++;
        if (mt < 0.0f)
        {
            drv->s = 1.0f;
            drv->tmr = 0.0f;
        }
        else
        {
            drv->s = drv->os;
            drv->tmr = mt * (1.0f - drv->s);
        }

        drv->tm = mt;
        drv->odriver = NULL;
        drv->os = 0.0f;
        drv->otm = 0.0f;
        drv->otmr = 0.0f;
    }
    else
    {
        drv->driver = driver;
        driver->driving_count++;
        if (mt < 0.0f)
        {
            drv->s = 1.0f;
            drv->tmr = 0.0f;
        }
        else
        {
            drv->s = 0.0f;
            drv->tmr = mt;
        }
        drv->tm = mt;
    }

    xVec3 euler;
    xMat3x3 a_descaled;
    if (drv->flags & 1)
    {
        {
            F32 len2 = SQR(drv->driver->frame->mat.right.x) + SQR(drv->driver->frame->mat.right.y) +
                       SQR(drv->driver->frame->mat.right.z);
            if (xabs(len2 - 1) <= 0.00001f)
            {
                a_descaled.right.x = drv->driver->frame->mat.right.x;
                a_descaled.right.y = drv->driver->frame->mat.right.y;
                a_descaled.right.z = drv->driver->frame->mat.right.z;
            }
            else if (xabs(len2) <= 0.00001f)
            {
                a_descaled.right.x = 0.0f;
                a_descaled.right.y = 1.0f;
                a_descaled.right.z = 0.0f;
            }
            else
            {
                F32 len_inv = 1.0f / xsqrt(len2);
                a_descaled.right.x = drv->driver->frame->mat.right.x * len_inv;
                a_descaled.right.y = drv->driver->frame->mat.right.y * len_inv;
                a_descaled.right.z = drv->driver->frame->mat.right.z * len_inv;
            }
        }

        {
            F32 len2 = SQR(drv->driver->frame->mat.up.x) + SQR(drv->driver->frame->mat.up.y) +
                       SQR(drv->driver->frame->mat.up.z);
            if (xabs(len2 - 1.0f) <= 0.00001f)
            {
                a_descaled.up.x = drv->driver->frame->mat.up.x;
                a_descaled.up.y = drv->driver->frame->mat.up.y;
                a_descaled.up.z = drv->driver->frame->mat.up.z;
            }
            else if (xabs(len2) <= 0.00001f)
            {
                a_descaled.up.x = 0.0f;
                a_descaled.up.y = 1.0f;
                a_descaled.up.z = 0.0f;
            }
            else
            {
                F32 len_inv = 1.0f / xsqrt(len2);
                a_descaled.up.x = drv->driver->frame->mat.up.x * len_inv;
                a_descaled.up.y = drv->driver->frame->mat.up.y * len_inv;
                a_descaled.up.z = drv->driver->frame->mat.up.z * len_inv;
            }
        }

        {
            F32 len2 = SQR(drv->driver->frame->mat.at.x) + SQR(drv->driver->frame->mat.at.y) +
                       SQR(drv->driver->frame->mat.at.z);
            if (xabs(len2 - 1.0f) <= 0.00001f)
            {
                a_descaled.at.x = drv->driver->frame->mat.at.x;
                a_descaled.at.y = drv->driver->frame->mat.at.y;
                a_descaled.at.z = drv->driver->frame->mat.at.z;
            }
            else if (xabs(len2) <= 0.00001f)
            {
                a_descaled.at.x = 0.0f;
                a_descaled.at.y = 1.0f;
                a_descaled.at.z = 0.0f;
            }
            else
            {
                F32 len_inv = 1.0f / xsqrt(len2);
                a_descaled.at.x = drv->driver->frame->mat.at.x * len_inv;
                a_descaled.at.y = drv->driver->frame->mat.at.y * len_inv;
                a_descaled.at.z = drv->driver->frame->mat.at.z * len_inv;
            }
        }
        xMat3x3GetEuler(&a_descaled, &euler);
        drv->yaw = euler.x;
    }

    if (coll != NULL && coll->flags & 0x2000)
    {
        drv->flags |= 0x2;

        *(xCollis::tri_data*)&drv->tri = coll->tri;
        drv->tri.loc = xCollisTriHit(drv->tri, *driver->model);
        xMat4x3Tolocal(&drv->tri.loc, &drv->driver->frame->mat, &drv->tri.loc);
        drv->tri.coll = coll;
    }

    xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);
    xMat4x3Tolocal(&drv->p, &drv->driver->frame->mat, &drv->q);
}

void xEntDriveInit(xEntDrive* drv, xEnt* driven)
{
    if (drv == NULL)
    {
        return;
    }

    drv->flags = 0;
    drv->driven = driven;
    drv->driver = NULL;
    drv->s = 0.0f;
    drv->tm = 0.0f;
    drv->tmr = 0.0f;
    drv->odriver = NULL;
    drv->os = 0.0f;
    drv->otm = 0.0f;
    drv->otmr = 0.0f;
}

//                                        xBoulder / xEntBoulder

void xEntBoulder_Setup(xEntBoulder* ent)
{
    ent->asset->redMult = 0.0f;
    ent->asset->greenMult = 0.0f;
    ent->asset->blueMult = 0.0f;

    if (ent->model != NULL)
    {
        ent->asset->seeThru = ent->model->Alpha;
    }
    else
    {
        ent->asset->seeThru = 1.0f;
    }

    xEntSetup(ent);
}

void xEntBoulder_BUpdate(xEnt*, xVec3*)
{
}

void xEntBoulder_AddForce(xEntBoulder* ent, xVec3* force)
{
    if (ent->update != 0)
    {
        xVec3* frcptr = &ent->force;
        xVec3AddTo(frcptr, force);
    }
}

void xEntBoulder_AddInstantForce(xEntBoulder* ent, xVec3* force)
{
    if (ent->update != 0)
    {
        xVec3* frcptr = &ent->instForce;
        xVec3AddTo(frcptr, force);
    }
}

void xEntBoulder_Init(void* ent, void* asset)
{
    xEntBoulder_Init((xEntBoulder*)ent, (xEntAsset*)asset);
}

//                                          xEnt

static F32 nsn_angle = DEG2RAD(30);
static F32 sEntityTimePassed;
static xBox all_ents_box;
static S32 all_ents_box_init;

static S32 setMaterialTextureRestore;
S32 sSetPipeline;
static RxPipeline* oldPipe;

void xEntInitShadow(xEnt& ent, xEntShadow& shadow)
{
    ent.entShadow = &shadow;

    shadow.vec.assign(0.0f, 1.0f, 0.0f);
    shadow.pos = ent.asset->pos;
    shadow.shadowModel = NULL;
    shadow.dst_cast = -1.0f;
    shadow.radius[0] = -1.0f;
    shadow.radius[1] = -1.0f;
}

bool xEntValidType(U8 type)
{
    return type == eBaseTypeTrigger || type == eBaseTypeVillain || type == eBaseTypePlayer ||
           type == eBaseTypePickup || type == eBaseTypePlatform || type == eBaseTypeDoor ||
           type == eBaseTypeSavePoint || type == eBaseTypeItem || type == eBaseTypeStatic ||
           type == eBaseTypeDynamic || type == eBaseTypeBubble || type == eBaseTypePendulum ||
           type == eBaseTypeHangable || type == eBaseTypeButton || type == eBaseTypeProjectile ||
           type == eBaseTypeDestructObj || type == eBaseTypeUI || type == eBaseTypeUIFont ||
           type == eBaseTypeProjectileType || type == eBaseTypeEGenerator || type == eBaseTypeNPC ||
           type == eBaseTypeBoulder || type == eBaseTypeTeleportBox || type == eBaseTypeZipLine;
}

xBox* xEntGetAllEntsBox()
{
    return &all_ents_box;
}

void xEntSetNostepNormAngle(F32 angle)
{
    nsn_angle = angle;
}

void xEntCollideWalls(xEnt* p, xScene* sc, F32 dt)
{
    xCollis* coll;
    xEnt* cent;
    U8 idx, sidx;
    F32 sbr;

    if (p->bound.type == XBOUND_TYPE_SPHERE)
    {
        sbr = p->bound.sph.r;
    }
    else
    {
        sbr = 0.7f;
    }

    if (p->collis->pen & 0x8)
    {
        idx = p->collis->npc_eidx;
        sidx = p->collis->npc_sidx;

        while (sidx < idx)
        {
            coll = &p->collis->colls[sidx];
            cent = (xEnt*)coll->optr;

            if (!(coll->flags & 0x2) && coll->dist < sbr &&
                ((p->collis->depenq) ? p->collis->depenq(p, cent, sc, dt, coll) :
                                       cent->penby & p->collType))
            {
                if (0.0f != coll->depen.x || 0.0f != coll->depen.z)
                {
                    coll->depen.y = 0.0f;
                }

                xEntAddToPos(p, &coll->depen);
            }

            sidx++;
        }
    }

    if (p->collis->pen & 0x4)
    {
        idx = p->collis->dyn_eidx;
        sidx = p->collis->dyn_sidx;

        while (sidx < idx)
        {
            coll = &p->collis->colls[sidx];
            cent = (xEnt*)coll->optr;

            if (!(coll->flags & 0x2) && coll->dist < sbr &&
                ((p->collis->depenq) ? p->collis->depenq(p, cent, sc, dt, coll) :
                                       cent->penby & p->collType))
            {
                coll->depen.y = 0.0f;

                xEntAddToPos(p, &coll->depen);
            }

            sidx++;
        }
    }

    if (p->collis->pen & 0x2)
    {
        idx = p->collis->stat_eidx;
        sidx = p->collis->stat_sidx;

        while (sidx < idx)
        {
            coll = &p->collis->colls[sidx];
            cent = (xEnt*)coll->optr;

            if (!(coll->flags & 0x2) && coll->dist < sbr &&
                ((p->collis->depenq) ? p->collis->depenq(p, cent, sc, dt, coll) :
                                       cent->penby & p->collType))
            {
                coll->depen.y = 0.0f;

                xEntAddToPos(p, &coll->depen);
            }

            sidx++;
        }
    }

    if (p->collis->pen & 0x20)
    {
        idx = p->collis->env_eidx;
        sidx = p->collis->env_sidx;

        while (sidx < idx)
        {
            coll = &p->collis->colls[sidx];
            cent = (xEnt*)coll->optr;

            if (!(coll->flags & 0x2) && coll->dist < sbr)
            {
                coll->depen.y = 0.0f;

                xEntAddToPos(p, &coll->depen);
            }

            sidx++;
        }
    }
}

void xEntCollideFloor(xEnt* p, xScene* sc, F32 dt)
{
    xCollis* coll = &p->collis->colls[0];
    U8 idx;
    xCollis* ml = coll;
    xVec3 motion;
    F32 mlen;
    S32 stepping = 0;
    F32 sbr;

    if (p->bound.type == XBOUND_TYPE_SPHERE)
    {
        sbr = p->bound.sph.r;
    }
    else
    {
        sbr = 0.7f;
    }

    xVec3Copy(&motion, &p->frame->mat.pos);
    xVec3SubFrom(&motion, &p->frame->oldmat.pos);

    motion.y = 0.0f;
    mlen = xVec3Length(&motion);

    for (idx = 6; idx < p->collis->idx; idx++)
    {
        xCollis* mf = &p->collis->colls[idx];

        if (mf->flags & 0x1)
        {
            xEnt* fent = (xEnt*)mf->optr;

            if (fent)
            {
                if ((fent->collType == XENT_COLLTYPE_DYN || fent->collType == XENT_COLLTYPE_STAT) ||
                    (fent->pflags & 0x20 && 0.0f == mf->hdng.x && 0.0f == mf->hdng.z))
                {
                    if (!((p->collis->depenq) ?
                              p->collis->depenq(p, fent, sc, dt, mf) :
                              (fent->collType & p->collis->pen && fent->penby & p->collType)))
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else if (!(p->collis->pen & 0x20))
            {
                continue;
            }

            if (mf->dist < ml->dist)
            {
                if (mf->hdng.y < -icos(PI / 3) &&
                    (mf->norm.y > icos(nsn_angle) ||
                     p->frame->oldmat.pos.y > dt * (sc->gravity * dt) + p->frame->mat.pos.y))
                {
                    ml = mf;
                    stepping = 0;
                }
                else if (mlen > 0.001f && mf->hdng.y < 0.65f / sbr - 1.0f &&
                         mf->norm.y > icos(nsn_angle))
                {
                    stepping = 1;
                    ml = mf;
                }
            }
        }
    }

    if (ml != coll)
    {
        F32 flr_dist = ml->dist * (F32)iabs(ml->hdng.y);
        xEnt* fent; // unused

        *coll = *ml;

        if (flr_dist < sbr)
        {
            ml->flags |= 0x6;

            if (stepping)
            {
                p->frame->mat.pos.y += 1.5f * dt;
                p->frame->mat.pos.x += ml->depen.x;
                p->frame->mat.pos.z += ml->depen.z;
            }
            else
            {
                p->frame->mat.pos.y += ml->depen.y;
            }

            p->frame->vel.y = 0.0f;
        }
    }
}

S32 xent_entent = 0;
xEnt* xEntCollCheckOneEntNoDepen(xEnt* ent, xScene* sc, void* data)
{
    xent_entent = 1;

    xEnt* p = (xEnt*)data;
    xCollis* coll;
    U32 modl_coll = 0;

    if (p->collis->idx >= 15)
    {
        xent_entent = 0;
        return NULL;
    }

    if ((ent->chkby & p->collType) == 0)
    {
        xent_entent = 0;
        return ent;
    }

    if (ent->id == p->id && (ent == p || ent->baseType != eBaseTypeBoulder))
    {
        xent_entent = 0;
        return ent;
    }

    coll = &p->collis->colls[p->collis->idx];

    if (ent->collLev == 5 && p->collType & (XENT_COLLTYPE_NPC | XENT_COLLTYPE_PLYR))
    {
        modl_coll = 1;
    }

    if (modl_coll)
    {
        coll->flags = 0;
    }
    else
    {
        coll->flags = 0x1F00;
    }

    xBoundHitsBound(&p->bound, &ent->bound, coll);

    if (coll->flags & 0x1)
    {
        if (modl_coll)
        {
            xBound tmp;
            xBound* bptr; // unused
            U8 ncolls;
            xVec3 *upper, *lower;
            U8 idx;

            coll->flags = 0x1F00;

            if (p->bound.type == XBOUND_TYPE_SPHERE)
            {
                xModelInstance* r4 = (ent->collModel) ? ent->collModel : ent->model;
                ncolls = 15 - p->collis->idx;
                idx = iSphereHitsModel3(&p->bound.sph, r4, coll, ncolls, 0.78539819f);
            }
            else if (p->bound.type == XBOUND_TYPE_BOX)
            {
                upper = &p->bound.box.box.upper;
                lower = &p->bound.box.box.lower;

                tmp.type = XBOUND_TYPE_SPHERE;

                xVec3Add(&tmp.sph.center, upper, lower);
                xVec3SMulBy(&tmp.sph.center, 0.5f);

                tmp.sph.r =
                    0.167f * (upper->x + upper->y + upper->z - lower->x - lower->y - lower->z);

                // none of the code above is used for anything... maybe debug stuff

                xModelInstance* r4 = (ent->collModel) ? ent->collModel : ent->model;
                ncolls = 15 - p->collis->idx;
                idx = iSphereHitsModel3(&p->bound.sph, r4, coll, ncolls, 0.78539819f);
            }

            // idx might be undefined here...!

            for (U8 i = 0; i < idx; i++)
            {
                coll[i].optr = ent;
                coll[i].mptr = ent->model;

                p->collis->idx++;
            }

            xent_entent = 0;
            return ent;
        }
        else
        {
            coll->oid = ent->id;
            coll->optr = ent;
            coll->mptr = ent->model;

            p->collis->idx++;

            if (ent->pflags & 0x20 && ent->bound.type == XBOUND_TYPE_SPHERE &&
                p->bound.type == XBOUND_TYPE_SPHERE && coll->hdng.y < -0.866025f)
            {
                F32 rsum = p->bound.sph.r + ent->bound.sph.r;
                F32 dx = p->bound.sph.center.x - ent->bound.sph.center.x;
                F32 dy = p->bound.sph.center.y - ent->bound.sph.center.y;
                F32 dz = p->bound.sph.center.z - ent->bound.sph.center.z;

                F32 hsqr = SQR(rsum) - (SQR(dx) + SQR(dz));

                if (hsqr >= 0.0f)
                {
                    coll->depen.x = 0.0f;
                    coll->depen.y = xsqrt(hsqr) - dy;
                    coll->depen.z = 0.0f;
                    coll->dist = 0.7f - coll->depen.y;
                    coll->hdng.x = 0.0f;
                    coll->hdng.y = -1.0f;
                    coll->hdng.z = 0.0f;
                }
            }
        }
    }

    xent_entent = 0;
    return ent;
}

void xEntCollCheckNPCs(xEnt* p, xScene* sc, xEnt* (*hitIt)(xEnt*, xScene*, void*))
{
    p->collis->npc_sidx = p->collis->idx;

    xSceneForAllNPCs(sc, hitIt, p);

    p->collis->npc_eidx = p->collis->idx;
}

void xEntCollCheckDyns(xEnt* p, xScene* sc, xEnt* (*hitIt)(xEnt*, xScene*, void*))
{
    p->collis->dyn_sidx = p->collis->idx;

    xSceneForAllDynamics(sc, hitIt, p);

    p->collis->dyn_eidx = p->collis->idx;
}

void xEntCollCheckStats(xEnt* p, xScene* sc, xEnt* (*hitIt)(xEnt*, xScene*, void*))
{
    p->collis->stat_sidx = p->collis->idx;

    xSceneForAllStatics(sc, hitIt, p);

    p->collis->stat_eidx = p->collis->idx;
}

void xEntCollCheckNPCsByGrid(xEnt* p, xScene* sc, xEnt* (*hitIt)(xEnt*, xScene*, void*))
{
    p->collis->npc_sidx = p->collis->idx;

    // npcs_grid has problem
    //xEntCollCheckOneGrid(p, sc, hitIt, &npcs_grid);

    p->collis->npc_eidx = p->collis->idx;
}

static void xEntCollCheckOneGrid(xEnt* p, xScene* sc, xEnt* (*hitIt)(xEnt*, xScene*, void*),
                                 xGrid* grid)
{
    xVec3* r26 = xEntGetCenter(p);
    xGridIterator it;
    S32 px, pz;

    xGridBound* cell = xGridIterFirstCell(grid, r26->x, r26->y, r26->z, px, pz, it);

    while (cell)
    {
        if (xQuickCullIsects(&p->bound.qcd, (xQCData*)(cell + 1)))
        {
            hitIt((xEnt*)cell->data, sc, p);
        }

        cell = xGridIterNextCell(it);
    }

    // non-matching: float scheduling

    F32 halfsizez = (grid->csizez * 0.5f);
    F32 halfsizex = (grid->csizex * 0.5f);

    F32 clcenterx = grid->csizex * px;
    F32 clcenterz = grid->csizez * pz;

    clcenterx += grid->minx;
    clcenterx += halfsizex;

    clcenterz += halfsizez;
    clcenterz += grid->minz;

    static S32 k;

    if (r26->x < clcenterx)
    {
        if (r26->z < clcenterz)
        {
            k = 0;
        }
        else
        {
            k = 1;
        }
    }
    else
    {
        if (r26->z < clcenterz)
        {
            k = 3;
        }
        else
        {
            k = 2;
        }
    }

    static S32 offs[4][3][2] = {
        { { -1, 0 }, { -1, -1 }, { 0, -1 } },
        { { 0, -1 }, { 1, -1 }, { 1, 0 } },
        { { 1, 0 }, { 1, 1 }, { 0, 1 } },
        { { 0, 1 }, { -1, 1 }, { -1, 0 } },
    };

    for (S32 i = 0; i < 3; i++)
    {
        S32 _x = px + offs[k][i][1];
        S32 _z = pz + offs[k][i][0];

        cell = xGridIterFirstCell(grid, _x, _z, it);

        while (cell)
        {
            if (xQuickCullIsects(&p->bound.qcd, (xQCData*)(cell + 1)))
            {
                hitIt((xEnt*)cell->data, sc, p);
            }

            cell = xGridIterNextCell(it);
        }
    }

    cell = xGridIterFirstCell(&grid->other, it);

    while (cell)
    {
        if (xQuickCullIsects(&p->bound.qcd, (xQCData*)(cell + 1)))
        {
            hitIt((xEnt*)cell->data, sc, p);
        }

        cell = xGridIterNextCell(it);
    }
}

void xEntCollCheckEnv(xEnt* p, xScene* sc)
{
    xCollis* coll;
    U8 ncolls;

    p->collis->env_sidx = p->collis->idx;

    coll = &p->collis->colls[p->collis->idx];
    coll->flags = 0x1F00;

    ncolls = 18 - p->collis->idx;

    p->collis->idx += (U8)iSphereHitsEnv3(&p->bound.sph, sc->env, coll, ncolls, 0.78539819f);
    p->collis->env_eidx = p->collis->idx;
}

void xEntEndCollide(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->collis->post)
    {
        ent->collis->post(ent, sc, dt, ent->collis);
    }
}

void xEntBeginCollide(xEnt* ent, xScene*, F32)
{
    U8 idx;
    xCollis* coll;

    if (ent->bupdate)
    {
        ent->bupdate(ent, &ent->frame->mat.pos);
    }

    for (idx = 0; idx < 18; idx++)
    {
        coll = &ent->collis->colls[idx];

        coll->flags = 0x1F00;
        coll->optr = NULL;
        coll->mptr = NULL;
        coll->dist = FLOAT_MAX;
    }

    ent->collis->idx = 6;
    ent->collis->stat_sidx = 6;
    ent->collis->stat_eidx = 6;
    ent->collis->dyn_sidx = 6;
    ent->collis->dyn_eidx = 6;
    ent->collis->npc_sidx = 6;
    ent->collis->npc_eidx = 6;
    ent->collis->env_sidx = 6;
    ent->collis->env_eidx = 6;
}

void xEntCollide(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->model)
    {
        if (ent->collis->chk & 0x2E)
        {
            xEntBeginCollide(ent, sc, dt);
        }

        if (ent->collis->chk & 0x8)
        {
            xEntCollCheckNPCsByGrid(ent, sc, xEntCollCheckOneEntNoDepen);
        }

        if (ent->collis->chk & 0x6)
        {
            xEntCollCheckByGrid(ent, sc, xEntCollCheckOneEntNoDepen);
        }

        if (ent->collis->chk & 0x20)
        {
            xEntCollCheckEnv(ent, sc);
        }

        xCollis* coll = &ent->collis->colls[0];

        if (ent->collis->chk & 0x2E)
        {
            F32 h_dot_n;

            if (ent->bound.type == XBOUND_TYPE_SPHERE)
            {
                h_dot_n = ent->bound.sph.r;
            }
            else
            {
                h_dot_n = 0.7f;
            }

            if (ent->pflags & 0x80 && coll->flags & 0x1)
            {
                F32 depen_len = xVec3Dot(&coll->hdng, &coll->norm);

                if (depen_len > 0.0f)
                {
                    xVec3Inv(&coll->norm, &coll->norm);
                    depen_len = -depen_len;
                }

                depen_len = depen_len * coll->dist + h_dot_n;

                if (depen_len < 0.0f || depen_len > h_dot_n)
                {
                    depen_len = CLAMP(depen_len, 0.0f, h_dot_n);
                }

                xVec3SMul(&coll->depen, &coll->norm, depen_len);
            }

            if (ent->frame->vel.y <= 0.0f)
            {
                xEntCollideFloor(ent, sc, dt);
            }
            else
            {
                xEntCollideCeiling(ent, sc, dt);
            }

            xEntCollideWalls(ent, sc, dt);
            xEntEndCollide(ent, sc, dt);
        }
    }
}

void xEntApplyPhysics(xEnt* ent, xScene* sc, F32 dt)
{
    xVec3 dposvel = { 0, 0, 0 };

    if (ent->pflags & 0x4 && sc->flags & 0x1)
    {
        ent->frame->vel.y += sc->gravity * dt;
    }

    if (ent->pflags & 0x10 && sc->flags & 0x2)
    {
        F32 tfric = -(sc->friction * dt - 1.0f);
        xVec3SMulBy(&ent->frame->vel, tfric);
    }

    if (ent->pflags & 0x8 && sc->flags & 0x4)
    {
        F32 tdrag = -(sc->drag * dt - 1.0f);
        xVec3SMulBy(&ent->frame->vel, tdrag);
    }

    xVec3Add(&dposvel, &ent->frame->vel, &ent->frame->oldvel);
    xVec3SMulBy(&dposvel, 0.5f * dt);

    if (dposvel.y < 0.0f)
    {
        F32 dposXZ = xsqrt(SQR(dposvel.x) + SQR(dposvel.z));
        F32 scaleXZ = (dposXZ > 0.00001f) ? ((0.63f * (30.0f * dt)) / dposXZ) : 0.0f;
        F32 scaleY = (0.63f * (30.0f * dt)) / (F32)iabs(dposvel.y);

        if (scaleXZ < 1.0f)
        {
            dposvel.x *= scaleXZ;
            dposvel.z *= scaleXZ;
        }

        if (scaleY < 1.0f)
        {
            dposvel.y *= scaleY;
        }
    }

    xEntAddToPos(ent, &dposvel);
}

void xEntMove(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->moreFlags & 0x20)
    {
        //anim_coll::pre_move(*ent);
    }

    ent->move(ent, sc, dt, ent->frame);

    xEntMotionToMatrix(ent, ent->frame);

    if (ent->driver)
    {
        xEntFrame* dframe = ent->driver->frame;

        if (ent->driveMode == 0)
        {
            xVec3 dpos;
            xVec3Sub(&dpos, &dframe->mat.pos, &dframe->oldmat.pos);

            ent->transl(ent, &dpos, NULL);
        }
        else if (ent->driveMode == 1)
        {
            RwMatrixUpdate((RwMatrix*)&dframe->oldmat);

            xMat4x3 invOldmat;
            RwMatrixInvert((RwMatrix*)&invOldmat, (RwMatrix*)&dframe->oldmat);

            xMat4x3 deltaMat;
            xMat4x3Mul(&deltaMat, &invOldmat, &dframe->mat);

            ent->transl(ent, NULL, &deltaMat);
        }
    }

    if (ent->moreFlags & 0x20)
    {
        //anim_coll::post_move(*ent);
    }
}

void xEntMotionToMatrix(xEnt* ent, xEntFrame* frame)
{
    if (frame->mode & 0x1000)
    {
    }

    if (frame->mode & 0x2)
    {
        if (frame->mode & 0x800)
        {
            xMat3x3RMulVec(&frame->dpos, &frame->mat, &frame->dpos);
        }

        xEntAddToPos(ent, &frame->dpos);
    }

    if (frame->mode & 0x8)
    {
        if (frame->mode & 0x800)
        {
            xMat3x3RMulVec(&frame->dvel, &frame->mat, &frame->dvel);
        }

        xVec3AddTo(&frame->vel, &frame->dvel);
    }

    if (!(frame->mode & 0x1000))
    {
    }
}

void xEntDefaultTranslate(xEnt* ent, xVec3* dpos, xMat4x3* dmat)
{
    if (dmat)
    {
        if (ent->model)
        {
            xMat4x3Mul((xMat4x3*)ent->model->Mat, (xMat4x3*)ent->model->Mat, dmat);
        }

        if (ent->frame)
        {
            xMat4x3Mul(&ent->frame->mat, &ent->frame->mat, dmat);
        }

        xMat4x3Toworld(xEntGetCenter(ent), dmat, xEntGetCenter(ent));
    }
    else
    {
        if (ent->model)
        {
            xVec3AddTo(xEntGetPos(ent), dpos);
        }

        if (ent->frame)
        {
            xVec3AddTo(&ent->frame->mat.pos, dpos);
        }

        xVec3AddTo(xEntGetCenter(ent), dpos);
    }
}

void xEntDefaultBoundUpdate(xEnt* ent, xVec3* pos)
{
    xBound* bound = &ent->bound;

    if (bound->type == XBOUND_TYPE_SPHERE)
    {
        xVec3Copy(&bound->sph.center, pos);

        bound->sph.center.y += 0.7f;
        bound->sph.r = 0.7f;
    }

    xBoundUpdate(bound);
    zGridUpdateEnt(ent);
}

void xEntEndUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    //ent->num_updates++;

    if (ent->model)
    {
        if (ent->frame)
        {
            if (!(ent->frame->mode & 0x20000))
            {
                xMat3x3Copy((xMat4x3*)ent->model->Mat, &ent->frame->mat);
            }

            if (!(ent->frame->mode & 0x10000))
            {
                xVec3* mpos = (xVec3*)&ent->model->Mat->pos;
                xVec3Copy(mpos, &ent->frame->mat.pos);
            }
        }

        if (ent->bupdate)
        {
            xVec3* upos = (xVec3*)&ent->model->Mat->pos;
            ent->bupdate(ent, upos);
        }

        xModelEval(ent->model);

        if (ent->moreFlags & 0x20)
        {
            //anim_coll::refresh(*ent);
        }

        if (ent->endUpdate)
        {
            ent->endUpdate(ent, sc, dt);
        }
    }
}

void xEntBeginUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->model)
    {
        xModelUpdate(ent->model, dt);

        if (ent->frame)
        {
            xVec3Copy(&ent->frame->oldvel, &ent->frame->vel);

            ent->frame->oldmat = ent->frame->mat;

            xRotCopy(&ent->frame->oldrot, &ent->frame->rot);
            xMat4x3Copy(&ent->frame->mat, xModelGetFrame(ent->model));

            ent->frame->mode = 0;
        }
    }
}

namespace
{
    F32 get_lower_extent(const xBound& bound)
    {
        switch (bound.type)
        {
        case XBOUND_TYPE_SPHERE:
        {
            return bound.sph.r;
        }
        case XBOUND_TYPE_BOX:
        {
            return bound.box.center.y - bound.box.box.lower.y;
        }
        case XBOUND_TYPE_OBB:
        {
            if (0.0f == bound.mat->up.x && 0.0f == bound.mat->up.z)
            {
                return bound.box.center.y -
                       (bound.mat->up.y * bound.box.box.lower.y + bound.mat->pos.y);
            }
            else
            {
                xBox box;
                xBoundGetBox(box, bound);

                return bound.box.center.y - box.lower.y;
            }
        }
        case XBOUND_TYPE_CYL:
        default:
        {
            return 0.0f;
        }
        }
    }

    bool collide_downward(xVec3& loc, xEnt*& hit, xScene& s, xEnt& ent, F32 max_dist)
    {
        xRay3 ray;
        U8 old_bound_type;
        F32 old_bound_radius;
        xCollis coll;

        F32 f31 = get_lower_extent(ent.bound);
        xVec3* r30 = xBoundCenter(&ent.bound);

        ray.origin = *r30;
        ray.dir.assign(0.0f, -1.0f, 0.0f);
        ray.min_t = 0.0f;
        ray.max_t = max_dist + f31;
        ray.flags = 0xC00;

        old_bound_type = ent.bound.type;
        old_bound_radius = ent.bound.sph.r;

        ent.bound.type = XBOUND_TYPE_SPHERE;
        ent.bound.sph.r = 0.0f;

        r30->y = FLOAT_MAX;

        coll.flags = 0x100;

        xRayHitsSceneFlags(&s, &ray, &coll, XENT_COLLTYPE_PLYR, 0x26);

        ent.bound.type = old_bound_type;
        ent.bound.sph.r = old_bound_radius;

        r30->y = ray.origin.y;

        if (!(coll.flags & 0x1))
        {
            return false;
        }

        loc = *(xVec3*)&ent.model->Mat->pos;
        loc.y -= coll.dist - f31;

        hit = (xEnt*)coll.optr;

        return true;
    }

    void drop_stacked_entity(xEnt&);

    void stacked_owner_destroyed(zEntDestructObj&, void* context)
    {
        drop_stacked_entity(*(xEnt*)context);
    }

    void mount_stacked_entity(xEnt& ent, xEnt& driver)
    {
        if (driver.collType == XENT_COLLTYPE_DYN)
        {
            if (ent.driver)
            {
                ent.driver->driving_count--;
            }

            ent.driver = &driver;

            driver.driving_count++;

            if (driver.baseType == eBaseTypeDestructObj)
            {
                zEntDestructObj* d = (zEntDestructObj*)&driver;
                d->destroy_notify = stacked_owner_destroyed;
                d->notify_context = &ent;
            }
        }
    }

    void dismount_stacked_entity(xEnt& ent)
    {
        if (ent.driver)
        {
            if (ent.driver->baseType == eBaseTypeDestructObj)
            {
                zEntDestructObj* d = (zEntDestructObj*)ent.driver;
                d->destroy_notify = NULL;
                d->notify_context = NULL;
            }

            ent.driver = NULL;
        }
    }

    void setup_stacked_entity(xEnt& ent)
    {
        ent.pflags = 0x4;
    }

    // Inlining issue
    void drop_stacked_entity(xEnt& ent)
    {
        ent.pflags = 0x4;

        dismount_stacked_entity(ent);
    }

    void stop_stacked_entity(xEnt& ent)
    {
        ent.pflags = 0;
    }

    void update_stacked_entity(xScene& sc, xEnt& ent, F32 dt)
    {
        xEntApplyPhysics(&ent, &sc, dt);
        xEntMove(&ent, &sc, dt);

        F32 dist = ent.model->Mat->pos.y - ent.frame->mat.pos.y;
        xVec3 loc;
        xEnt* hit;

        if (!(dist <= 0.0f) && collide_downward(loc, hit, sc, ent, dist))
        {
            ent.frame->mat.pos.y = loc.y;
            stop_stacked_entity(ent);

            if (hit)
            {
                mount_stacked_entity(ent, *hit);
            }
        }
    }
} // namespace

// Inlining issue
void xEntUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    xEntBeginUpdate(ent, sc, dt);

    if (ent->pflags & 0x2)
    {
        xEntApplyPhysics(ent, sc, dt);
    }

    if (ent->pflags & 0x1)
    {
        xEntMove(ent, sc, dt);
    }

    xFFXApply(ent, sc, dt);

    if (ent->collis)
    {
        xEntCollide(ent, sc, dt);
    }

    if (ent->flags & 0x2 && ent->pflags & 0x4)
    {
        update_stacked_entity(*sc, *ent, dt);
    }

    xEntEndUpdate(ent, sc, dt);
}

// void xEntRender(xEnt* ent)
// {
//     S32 shadowOutside;
//     xVec3 shadVec;

//     if (ent->model == NULL || !xEntIsVisible(ent) || ent->model->Flags & 0x400)
//     {
//         return;
//     }

//     ent->isCulled = 0;

//     if (ent->baseType == eBaseTypePlayer || (ent->baseType == eBaseTypeNPC && !(ent->flags & 0x40)))
//     {
//         // non-matching: 10.0f is loaded too early

//         shadVec.x = ent->model->Mat->pos.x;
//         shadVec.y = ent->model->Mat->pos.y - 10.0f;
//         shadVec.z = ent->model->Mat->pos.z;

//         if (iModelCullPlusShadow(ent->model->Data, ent->model->Mat, &shadVec, &shadowOutside))
//         {
//             if (shadowOutside)
//             {
//                 ent->isCulled = 1;
//                 return;
//             }
//             else
//             {
//                 goto postrender;
//             }
//         }
//     }
//     else
//     {
//         if (iModelCull(ent->model->Data, ent->model->Mat))
//         {
//             ent->isCulled = 1;
//             return;
//         }
//     }

//     xModelRender(ent->model);

// postrender:

//     if ((ent->baseType == eBaseTypeNPC && !(ent->flags & 0x40)) || ent->baseType == eBaseTypePlayer)
//     {
//         zLightAddLocal(ent);
//         xShadow_ListAdd(ent);
//     }
// }

void xEntRestorePipeline(xSurface*, RpAtomic* model)
{
    if (setMaterialTextureRestore)
    {
        //iModelResetMaterial(model);
        setMaterialTextureRestore = 0;
    }

    if (sSetPipeline)
    {
        model->pipeline = oldPipe;
        sSetPipeline = 0;
    }
}

void xEntRestorePipeline(xModelInstance* model)
{
    xEntRestorePipeline(model->Surf, model->Data);
}
