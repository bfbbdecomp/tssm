#include "zTalkBox.h"

#include <types.h>

namespace
{

    shared_type shared;
} // namespace

void ztalkbox::permit(U32 add_flags, U32 remove_flags)
{
    shared.permit &= ~add_flags;
    shared.permit |= (U32)this;
}

ztalkbox* ztalkbox::get_active()
{
    return shared.active;
}

void ztalkbox::reset_all()
{
}

namespace
{

    struct state_type
    {
        state_enum type;

        void start();
        void stop();
    };

    void state_type::start()
    {
    }

    void stop()
    {
    }

    void state_type::stop()
    {
    }

} // namespace

void ztalkbox::hide()
{
    flag.visible = 0;
    if (prompt_box)
    {
        prompt_box->deactivate();
    }
    if (quit_box)
    {
        quit_box->deactivate();
    }
}

void ztalkbox::stop_talk()
{
    if ((ztalkbox*)shared.active->flag.visible == this)
    {
        stop();
    }
}

namespace
{
}
