#ifndef XWAD1_H
#define XWAD1_H

#include <PowerPC_EABI_Support\MSL_C\PPC_EABI\math_ppc.h>

#include "zGlobals.h"
#include "xCutscene.h"
#include "iCutscene.h"
#include "xModel.h"
#include "xSnd.h"
#include "xCounter.h"
#include "xEvent.h"
#include "xCM.h"
#include "xClimate.h"
#include "zParEmitter.h"
#include "xString.h"
#include "xCamera.h"
#include "xBound.h"
#include "xbinio.h"
#include "xBehaviour.h"
#include "xBehaveMgr.h"
#include "xBehaveGoalSimple.h"
#include "xAnim.h"
#include "iAnim.h"
#include "iModel.h"
#include "stdlib.h"
#include "xMorph.h"
#include "iCamera.h"
#include "iScrFX.h"
#include "xShadow.h"

struct st_BINIO_XTRADATA
{
    char* dbl_buf;
    S32 dblbuf_size;
    S32 dbl_beg;
    S32 dbl_end;
    S32 dbl_amt;
    U32 fpos;
    char* asyn_data;
    S32 asyn_amt;
    S32 asyn_elesize;
    S32 asyn_ismot;
    en_BIO_ASYNC_ERRCODES asyn_status;
    U32 pad[3];
    S32 gcaskey;
};

#endif
