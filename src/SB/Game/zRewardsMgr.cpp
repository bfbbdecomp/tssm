#include "zRewardsMgr.h"

#include "xHudMeter.h"

U32 upgradeSlotsPoints[14];

void zRewardsMgr::SetPlayerDead()
{
    playerDead = TRUE;
}

U32 zRewardsMgr::GetUpgradeRangeUpper() const
{
}

void zRewardsMgr::ResetLastComboIndex()
{
    lastComboIndex = 0;
}

U32 zRewardsMgr::GetLastComboIndex() const
{
    return lastComboIndex;
}

void zRewardsMgr::ResetLastComboManlinessPoints()
{
    lastComboManlinessPoints = 0;
}

U32 zRewardsMgr::GetLastComboManlinessPoints() const
{
    return lastComboManlinessPoints;
}
