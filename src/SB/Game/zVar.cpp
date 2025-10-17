#include "zVar.h"

#include <PowerPC_EABI_Support\MSL_C\MSL_Common/printf.h>

#include "zGlobals.h"
#include "xutil.h"

char* var_text(const substr& str);
U32 zVarEntryCB_SndFXVol(void*);
U32 zVarEntryCB_SndMusicVol(void*);

extern const char zVar_strings[];

S32 zVarEntryCB_IsVisible(void* context)
{
    return 0;
}

S32 zVarEntryCB_IsEnabled(void* arg)
{
    return 0;
}

S32 zVarEntryCB_CounterValue(void* arg)
{
    // Given the void argument type, no idea what type this actually is.
    // What we do know that at an offset 5 words in there's a signed S16.
    struct something
    {
        U32 stuff[5];
        S16 theValue;
    };
    return ((something*)arg)->theValue;
}

S32 zVarEntryCB_Minute(void*)
{
    return iGetMinute();
}

S32 zVarEntryCB_Hour(void*)
{
    return iGetHour();
}

S32 zVarEntryCB_Date(void*)
{
    return iGetDay() + iGetMonth() * 0x64;
}

S32 zVarEntryCB_CurrentRoom(void*)
{
    char buffer[16];
    sprintf(buffer, &zVar_strings[0xbaa] /*"%s"*/,
            xUtil_idtag2string(globals.sceneCur->sceneID, 0));

    S32 room = (buffer[2] - '0') * 10 + (buffer[3] - '0');

    // Convert the most significant char to uppercase
    // The lowercase variable doesn't exist in the ps4 symbols but I can't
    // figure out how to get a match otherwise.
    U32 mostSignificantChar = buffer[0];
    bool lowercase = false;
    if (mostSignificantChar >= 'a' && mostSignificantChar <= 'z')
    {
        lowercase = true;
    }
    if (lowercase)
    {
        mostSignificantChar -= 0x20;
    }

    return room + ((mostSignificantChar - 'A') + 1) * 100;
}

S32 zVarEntryCB_CurrentSceneLetter(void*)
{
    char buffer[16];
    sprintf(buffer, &zVar_strings[0x10] /*"%s"*/, xUtil_idtag2string(globals.sceneCur->sceneID, 0));

    // Convert the most significant char to uppercase
    // The lowercase variable doesn't exist in the ps4 symbols but I can't
    // figure out how to get a match otherwise.
    U32 mostSignificantChar = buffer[0];
    bool lowercase = false;
    if (mostSignificantChar >= 'a' && mostSignificantChar <= 'z')
    {
        lowercase = true;
    }
    if (lowercase)
    {
        mostSignificantChar -= 0x20;
    }

    return (mostSignificantChar - 'A') + 1;
}

U32 zVarEntryCB_VibrationOn(void* arg)
{
    // I'm pretty sure they actually and'd with a bit mask here, I couldn't find
    // another way (such as casting to bool) to get the right instructions.
    return globals.option_vibration & 0x1;
}

U32 zVarEntryCB_MCAvailable(void* arg)
{
    if (xSGGameIsEmpty(NULL, 0) == 1)
    {
        if (xSGGameIsEmpty(NULL, 1) == 1)
        {
            return 0;
        }
        if (xSGGameIsEmpty(NULL, 1) == 0)
        {
            return 2;
        }
    }
    else if (xSGGameIsEmpty(NULL, 0) == 0)
    {
        if (xSGGameIsEmpty(NULL, 1) == 1)
        {
            return 1;
        }
        if (xSGGameIsEmpty(NULL, 1) == 0)
        {
            return 3;
        }
    }

    return 0;
}

U32 zVarEntryCB_SndFXVol(void* arg)
{
    F64 f0 = 10.0;
    F64 f1 = 0.5;
    F64 f2 = gSnd.categoryVolFader[0];

    f2 *= f1;
    f2 = f0 + f2;

    return f2;
}

U32 zVarEntryCB_SndMusicVol(void* arg)
{
    F64 f0 = 10.0;
    F64 f1 = 0.5;
    F64 f2 = gSnd.categoryVolFader[2];

    f2 *= f1;
    f2 = f0 + f2;

    return f2;
}

U32 zVarEntryCB_SndMode(void* arg)
{
    return gSnd.stereo;
}

void zVarNewGame()
{
}
