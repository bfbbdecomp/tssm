#ifndef ZTALKBOX_H
#define ZTALKBOX_H

#include "zEntPlayerOOBState.h"
#include "zTextBox.h"

#include "xScene.h"
#include "zNMECommon.h"
#include "xBase.h"
#include "zFX.h"

enum query_enum
{
    Q_SKIP,
    Q_YESNO
};

struct xEntNPCAssetIN : xEntNPCAsset
{
    U32 navigation_mesh_id;
    U32 settings;
};

struct ztalkbox : xBase
{
    enum answer_enum
    {
        ANSWER_CONTINUE,
        ANSWER_YES,
        ANSWER_NO
    };

    struct _class_8
    {
        U8 visible : 1;
    };

    struct asset_type_1 : xDynAsset
    {
        struct _class_3
        {
            struct _class_4
            {
                U8 time : 8;
                U8 prompt : 8;
                U8 sound : 8;
                U8 event : 8;
            };
            _class_4 type;
            F32 delay;
            S32 which_event;
        };

        struct _class_11
        {
            U32 skip;
            U32 noskip;
            U32 quit;
            U32 noquit;
            U32 yesno;
        };

        U32 dialog_box;
        U32 prompt_box;
        U32 quit_box;
        U8 trap : 8;
        U8 pause : 8;
        U8 allow_quit : 8;
        U8 trigger_pads : 8;
        U8 page : 8;
        U8 show : 8;
        U8 hide : 8;
        U8 audio_effect : 8;
        U32 teleport;
        _class_3 auto_wait;
        _class_11 prompt;
    };

    struct _class_16
    {
        char* skip; // originally int8*
        char* noskip;
        char* quit;
        char* noquit;
        char* yesno;
    };

    struct callback
    {
    };

    _class_8 flag;
    asset_type_1* asset;
    ztextbox* dialog_box;
    ztextbox* prompt_box;
    ztextbox* quit_box;
    _class_16 prompt;
    U32 triggerPads;
    base* npc;

    void permit(U32, U32);
    ztalkbox* get_active();
    void reset_all();
    void MasterLoveSlave(xBase* slave, S32 starting);
    void hide();
    void show();
    void stop_talk();
    void start_talk(U32 text_id, callback* cb, base* npc);
    void start_talk(char* s, callback* cb, base* npc);
    void set_text(U32 id);
    void set_text(char* s);
    void load(asset_type_1& a);
};

struct state_type
{
    state_enum type;

    void start();
};

struct jot;

struct callback
{
    void (*render)(jot&, xtextbox&, F32, F32);
    void (*layout_update)(jot&, xtextbox&, xtextbox&);
    void (*render_update)(jot&, xtextbox&, xtextbox&);
};

struct split_tag
{
    substr tag;
    substr name;
    substr action;
    substr value;
};

struct tag_type
{
    substr name;
    void (*parse_tag)(jot&, xtextbox&, xtextbox&, split_tag&);
    void (*reset_tag)(jot&, xtextbox&, xtextbox&, split_tag&);
    void* context;
};

struct jot
{
    substr s;

    struct
    {
        // Offset: 0x8
        bool invisible : 1; // bit 24
        bool ethereal : 1; // bit 25
        bool merge : 1; // bit 26
        bool word_break : 1; // bit 27
        bool word_end : 1; // bit 28
        bool line_break : 1; // bit 29
        bool stop : 1; // bit 30
        bool tab : 1; // bit 31

        // Offset: 0x9
        bool insert : 1; // bit 24
        bool dynamic : 1; // bit 25
        bool page_break : 1; // bit 26
        bool stateful : 1; // bit 27
        U16 dummy : 4; // bits 28-31
    } flag;
    // Offset: 0xC
    U16 context_size;

    // Offset: 0x10
    void* context;
    basic_rect<F32> bounds;
    basic_rect<F32> render_bounds;
    callback* cb;
    tag_type* tag;

    void intersect_flags(const jot& other);
    void reset_flags();
};

struct jot_line
{
    basic_rect<F32> bounds;
    F32 baseline;
    U32 first;
    U32 last;
    U8 page_break;
};

struct layout
{
    xtextbox tb;
    jot _jots[512];
    U32 _jots_size;
    jot_line _lines[128];
    U32 _lines_size;
    U8 context_buffer[1024];
    U32 context_buffer_size;
    U16 dynamics[64];
    U32 dynamics_size;
};

struct _class_7
{
    struct
    {
        U8 time : 1;
        U8 prompt : 1;
        U8 sound : 1;
        U8 event : 1;
        U32 pad : 28;
    };
};

struct wait_context
{
    _class_7 type;
    U8 need;
    F32 delay;
    U32 event_mask;
    query_enum query;
};

struct sound_queue
{
    iSndHandle _playing[5];
    S32 head;
    S32 tail;
};

struct trigger_pair
{
    ztalkbox* origin;
    U32 event;
};

struct fixed_queue
{
    U32 _first;
    U32 _last;
    trigger_pair _buffer[33];
};

struct shared_type
{
    S32 flags;
    U32 permit;
    ztalkbox* active;
    state_type* state;
    state_type* states[5];
    layout lt;
    S32 begin_jot;
    S32 end_jot;
    S32 page_end_jot;
    wait_context wait;
    wait_context auto_wait;
    U32 wait_event_mask;
    //sound_queue<> sounds;
    // Needs something similar to line "fixed_queue<trigger_pair, 33> triggered;
    U8 allow_quit;
    U8 quitting;
    U8 delay_events;
    callback* cb;
    fixed_queue<trigger_pair, 33> triggered; // This line is just a guess. Could be incorect.
    F32 volume;
    base* speak_npc;
    U32 speak_player;
};

#endif
