#include "zGameState.h"

void zGameStatePauseUnpauseSnd(bool pause)
{
    xSndMgrPauseSounds(0, pause, TRUE);
    xSndMgrPauseSounds(1, pause, TRUE);
    xSndMgrPauseSounds(2, pause, FALSE);
}

void SB04FMVPauseSoundCB(bool doPause)
{
    bool tempBool;

    if (gGameMode == 8)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 7)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 6)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 2)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 4)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 6)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 1)
    {
        tempBool = TRUE;
    }
    else if (gGameMode == 0)
    {
        tempBool = TRUE;
    }
    else
    {
        tempBool = FALSE;
    }

    if (tempBool)
    {
        xSndMgrPauseSounds(3, doPause, TRUE);
    }
    else if (doPause != 0)
    {
        xSndMgrPauseSounds(-3, TRUE, TRUE);
    }
    else
    {
        xSndMgrPauseSounds(-3, FALSE, TRUE);
    }
}

void zGameSetOstrich(_GameOstrich o)
{
    gGameOstrich = o;
}

_GameOstrich zGameGetOstrich()
{
    return gGameOstrich;
}

eGameMode zGameModeGet()
{
    return gGameMode;
}

S32 zGameStateGet()
{
    return gGameState;
}
