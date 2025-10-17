#include "zNMETypeCritBasic.h"

void zNMECritBasic::SelfSetup()
{
    xBehaveMgr* bmgr;
    xPsyche* psy;
    bmgr = xBehaveMgr_GetSelf();
    bmgr->Subscribe(this, 0);
    psy = psy_self;
    psy->BrainBegin();
    psy->AddGoal('NGC\0', NULL);
    psy->AddGoal('NGC\1', NULL);
    psy->AddGoal('NGC\4', NULL);
    psy->AddGoal('NGC\2', NULL);
    psy->AddGoal('NGC\3', NULL);
    psy->BrainEnd();
    //*(undefined4 *)(this_01 + 0x44) = 0x4e474300;
}
