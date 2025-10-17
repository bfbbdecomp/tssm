#include "zGame.h"

void DoTheResetinator()
{
}

void zGameStall()
{
    bool tempGameBool;
    if (gGameMode == 8)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 7)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 6)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 2)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 4)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 6)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 1)
    {
        tempGameBool = TRUE;
    }
    else if (gGameMode == 0)
    {
        tempGameBool = TRUE;
    }
    else
    {
        tempGameBool = FALSE;
    }

    if (!tempGameBool) // chain of if elses
    {
        zGameModeSwitch(eGameMode_Stall);
        xSndMgrPauseSounds(0, TRUE, TRUE);
        xSndMgrPauseSounds(1, TRUE, TRUE);
        xSndMgrPauseSounds(2, TRUE, FALSE);
        iPadStopRumble(globals.pad[1]);
        zEntEvent("techbutton6_click", 24);
    }
}
