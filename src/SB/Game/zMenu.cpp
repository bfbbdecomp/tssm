#include "iTime.h"
#include "zGame.h"
#include "zMain.h"
#include "zMenu.h"

//bool menu_fmv_played;
S32 sFirstBoot = 1;
//S32 logoTmr;
F32 time_elapsed = 0.01f;
F32 time_last;
F32 time_current;
F32 sAttractMode_timer;
F32 sOneLiner_timer;
//S32 promptSel;
S32 card;
//S32 var;
//S32 fullCard;
S32 sInMenu;
//F32 ONELINER_WAITTIME;
F32 holdTmr = 10.0f;
U8 sAllowAttract;

void zMenuAllowAtract(bool allowAttract)
{
    sAllowAttract = allowAttract;
}

void zMenuPause(bool bPause)
{
    if (bPause == FALSE)
    {
        time_last = iTimeDiffSec(iTimeGet()) - SECS_PER_VBLANK;
        sTimeLast = iTimeGet();
    }
}

S32 zMenuIsFirstBoot()
{
    return sFirstBoot;
}

U32 zMenuGetBadCard()
{
    return card + 1;
}

void zMenuExit()
{
}

S32 zMenuRunning()
{
    return sInMenu;
}
