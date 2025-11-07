#pragma inline_depth(0)

#include "zTaskBox.h"

#include "xEvent.h"
#include "xstransvc.h"

#include "zBase.h"
#include "zScene.h"
#include "zTalkBox.h"

#include <types.h>

// CODE PORTED DIRECTLY FROM BFBB

namespace {
    ztaskbox* shared;
}

void ztaskbox::load(const asset_type& a)
{
    xBaseInit((xBase*)this, &(xBaseAsset)a);
    this->baseType = eBaseTypeTaskBox;
    // FIXME: can't force const to non-const?
    // this->asset = &a;

    // FIXME: Define cb_dispatch
    // this->eventFunc = cb_dispatch;
    if (this->linkCount != 0)
    {
        this->link = (xLinkAsset*)(&a + 1);
    }
    bool enabled = a.enable;
    this->state = STATE_INVALID;
    if (!enabled)
    {
        this->flag.enabled = false;
        this->current = NULL;
    }
    else
    {
        this->flag.enabled = true;
        this->set_state(STATE_BEGIN);
        this->current = this;
    }
    if (a.persistent)
    {
        this->baseFlags |= 2;
    }
}

void ztaskbox::read(xSerial& s)
{
    U8 data[4];
    data[0] = (U8)this->state;
    s.Read(data);
    set_state((ztaskbox::state_enum)data[0]);
}

void ztaskbox::write(xSerial& s)
{
    s.Write((U8)this->state);
}

// WIP.
void ztaskbox::start_talk(zNPCCommon* npc)
{
    ztaskbox* curr = this->current;
    if (curr != NULL)
    {
        if (curr == this)
        {
            if (this->flag.enabled && this->state != STATE_INVALID)
            {
                //TODO!!!
            }
        }
        else
        {
            curr->set_callback(this->cb);
            this->current->start_talk(NULL);
        }
    }
}

void ztaskbox::talk_callback::reset(ztaskbox& task)
{
    this->task = &task;
    this->answer = ANSWER_CONTINUE;
}

// This function matches better with `-inline auto`
// but most functions in this file prefer it off,
// though `auto` behaves with an inline_depth of 4.
#pragma inline_depth(5)
void ztaskbox::stop_talk()
{
    ztaskbox* curr = this->current;

    if (curr == NULL)
    {
        return;
    }

    if (curr != this)
    {
        curr->stop_talk();
        return;
    }

    if (!this->flag.enabled || this->state == STATE_INVALID)
    {
        return;
    }

    if (shared != this)
    {
        return;
    }

    ztalkbox* other = (ztalkbox*)zSceneFindObject(this->asset->talk_box);

    if (other)
    {
        other->stop_talk();
        shared = NULL;
    }
}
#pragma inline_depth(0)

void ztaskbox::enable()
{
    if (!this->flag.enabled)
    {
        this->flag.enabled = true;
        set_state(STATE_BEGIN);
    }
}

void ztaskbox::disable()
{
    if (this->flag.enabled)
    {
        stop_talk();
        this->flag.enabled = false;
        set_state(STATE_INVALID);
    }
}

void ztaskbox::reset()
{
    disable();
    if (this->asset->enable)
    {
        enable();
    }
}

void ztaskbox::initiate()
{
    if (this->state == STATE_BEGIN)
    {
        set_state(STATE_DESCRIPTION);
    }
}

void ztaskbox::succeed()
{
    if (state == STATE_BEGIN || state == STATE_DESCRIPTION || state == STATE_REMINDER)
    {
        set_state(STATE_SUCCESS);
    }
}

void ztaskbox::fail()
{
    if (state == STATE_BEGIN || state == STATE_DESCRIPTION || state == STATE_REMINDER)
    {
        set_state(STATE_FAILURE);
    }
}

void ztaskbox::complete()
{
    if (this->flag.enabled)
    {
        this->state = STATE_INVALID;
        this->flag.enabled = false;
        zEntEvent(this, 0, this, eEventTaskBox_OnComplete, 0, NULL, 0, 1);
        this->current = (ztaskbox*)zSceneFindObject(this->asset->next_task);

        // Bruh
        if (this->current != NULL)
        {
            return;
        }
    }
}

void ztaskbox::set_callback(callback* cb)
{
    this->cb = cb;
}

// WIP.
void ztaskbox::init()
{
    shared = NULL;
    // STUFF.
}

void ztaskbox::load(xBase& data, xDynAsset& asset, size_t num)
{
    ((ztaskbox&)data).load((asset_type&)asset);
}

bool ztaskbox::exists(state_enum stage)
{
    U32 state = this->asset->stages[stage];
    return state != STATE_BEGIN && xSTFindAsset(state, NULL);
}

void ztaskbox::set_state(state_enum stage)
{
    this->state = stage;
    this->current = this;

    // todo
}

void ztaskbox::cb_dispatch(xBase*, xBase* to, U32 event, F32*, xBase*, U32)
{
    ztaskbox& taskbox = *(ztaskbox*)to;

    switch (event)
    {
    case eEventReset:
        taskbox.reset();
        break;
    case eEventStartConversation:
        taskbox.start_talk(NULL);
        break;
    case eEventEndConversation:
        taskbox.stop_talk();
        break;
    case eEventTaskBox_Initiate:
        taskbox.initiate();
        break;
    case eEventTaskBox_SetSuccess:
        taskbox.succeed();
        break;
    case eEventTaskBox_SetFailure:
        taskbox.fail();
        break;
    case eEventTaskBox_OnAccept:
    case eEventTaskBox_OnDecline:
    case eEventTaskBox_OnComplete:
        break;
    }
}

void ztaskbox::talk_callback::on_start()
{
    this->task->flag.unk_11 = 1;
    if (this->task->cb != nullptr)
    {
        this->task->cb->on_talk_start();
    }
}

void ztaskbox::talk_callback::on_stop()
{
    this->task->on_talk_stop(answer);
}

void ztaskbox::talk_callback::on_answer(answer_enum answer)
{
    this->answer = answer;
}
