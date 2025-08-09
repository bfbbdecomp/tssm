#ifndef IWAD_H
#define IWAD_H

#include "../src/sb/game/zGlobals.h"
#include <PowerPC_EABI_Support\MSL_C\MSL_Common/printf.h>
#include <PowerPC_EABI_Support\MSL_C\MSL_Common\cmath>

#include "iAnim.h"
#include "iAnimSKB.h"
#include "iARAMTmp.h"
#include "iCollide.h"
#include "iCollideFast.h"
#include "iColor.h"
#include "iCutscene.h"
#include "iDraw.h"
#include "iEnv.h"
#include "iFile.h"
#include "iFMV.h"
#include "iFX.h"
#include "iLight.h"
#include "iMath.h"
#include "iMath3.h"
#include "iMemMgr.h"
#include "iMix.h"
#include "iModel.h"
#include "iMorph.h"
#include "iPad.h"
#include "iParMgr.h"
#include "isavegame.h"
#include "iScrFX.h"
#include "iSnd.h"
#include "iSystem.h"
#include "iTime.h"
#include "iTRC.h"
#include "ngcrad3d.h"
#include "xDebug.h"
#include "xPad.h"
#include "xTRC.h"
#include "stdlib.h"
#include "../include/dolphin/dvd.h"
#include "xstransvc.h"
#include <limits.h>
#include <rwcore.h>
#include <rtslerp.h>
#include <dolphin/os.h>

struct ThreadParam
{
    S32 status;
    void (*entry)(void*);
    void* stack;
    S32 stackSize;
    void* gpReg;
    S32 initPriority;
    S32 currentPriority;
    U32 attr;
    U32 option;
    S32 waitType;
    S32 waitId;
    S32 wakeupCount;
};

void iFuncProfileFuncs(int, int, float);

#endif
