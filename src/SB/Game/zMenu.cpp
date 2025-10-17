// For some reason including this breaks the bitshift in zMenuPause
//#include "zMenu.h"

void zMenuAllowAtract(bool allowAttract)
{
    sAllowAttract = allowAttract;
}

void zMenuPause(bool bPause)
{
    S64 time;
    U32 Utime;

    if (bPause == FALSE)
    {
        Utime = iTimeGet();
        time = iTimeDiffSec((U64)Utime >> 0x20);
        time_last = time - SECS_PER_VBLANK;
        Utime = iTimeGet();
    }

    sTimeLast = Utime >> 0x20;
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
