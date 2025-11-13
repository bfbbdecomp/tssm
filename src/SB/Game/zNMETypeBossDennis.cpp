#include "zNMETypeBossDennis.h"

S32 zNMEGoalDenDead::Exit(F32 dt, void* ctxt)
{
}

S32 zNMEGoalDenDamage::Exit(F32 dt, void* ctxt)
{
    zNMEDennis& npc = *(zNMEDennis*)this->psyche->clt_owner;
    npc.tmr_nextLeap = -1.0f;
    return 0;
}

zNMEGoalDenEvade::PathDirect(zNMEDennis*, F32 dt)
{
}

// Return type not in dwarf, Could be wrong
void zNMEDennis::MeterRender()
{
}

// Inlining issue
void zNMEDennis::RenderHud()
{
    zNMEDennis::MeterRender();
}

S32 zNMEDennis::DfltVulnFlags()
{
    return 0xc07b0007; // HACKED TOGETHER FOR TESTING PURPOSES
}
