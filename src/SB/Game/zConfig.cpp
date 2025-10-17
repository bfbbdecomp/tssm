#include "zConfig.h"

#include <types.h>

U8 zConfig_isPlayerPatrick(ePlayerEnum playerEnum)
{
    U8 isPatrick = 0;
    if ((playerEnum >= 10) && (playerEnum <= 0x15))
    {
        isPatrick = 1;
    }
    return isPatrick;
}

U8 zConfig_isPlayerSpongebob(ePlayerEnum playerEnum)
{
    U8 isSpongebob = 0;
    if ((playerEnum >= 0) && (playerEnum <= 10))
    {
        isSpongebob = 1;
    }
    return isSpongebob;
}
