#ifndef XWAD2_H
#define XWAD2_H

#include <PowerPC_EABI_Support\MSL_C\PPC_EABI\math_ppc.h>
#include "cmath.h"

#include "zGlobals.h"
#include <types.h>
#include <rpworld.h>
#include <libc/stdlib.h>
#include <stdlib.h>
#include "xString.h"
#include "string.h"
#include <cstring>
#include "xIni.h"
#include "xHudText.h"
#include "xTextAsset.h"
#include "xstransvc.h"
#include "zScene.h"
#include "zTextBox.h"
#include "new.h"
#include "xHudModel.h"
#include "iMath.h"
#include "xEvent.h"
#include "xhipio.h"
#include "xGroup.h"
#include "xGrid.h"
#include "zGrid.h"
#include "zFX.h"
#include "rpworld.h"
#include "zSurface.h"
#include "rpmatfx.h"
#include "xFont.h"
#include "xfog.h"
#include "xFFX.h"
#include "xFactory.h"
#include "xEntMotion.h"
#include "xEntBoulder.h"
#include "iCollide.h"
#include "zEntDestructObj.h"

static st_HIPLOADDATA* HIPLCreate(const char* filename, char* dblbuf, S32 bufsize);
static void HIPLDestroy(st_HIPLOADDATA* lddata);
static U32 HIPLBaseSector(st_HIPLOADDATA* lddata);
static S32 HIPLSetBypass(st_HIPLOADDATA* lddata, S32 enable, S32 use_async);
static void HIPLSetSpot(st_HIPLOADDATA* lddata, S32 spot);
static U32 HIPLBlockEnter(st_HIPLOADDATA* lddata);
static void HIPLBlockExit(st_HIPLOADDATA* lddata);
static S32 HIPLBlockRead(st_HIPLOADDATA* lddata, void* data, S32 cnt, S32 size);
static S32 HIPLBypassRead(st_HIPLOADDATA* lddata, void* data, S32 cnt, S32 size);
static S32 HIPLReadAsync(st_HIPLOADDATA* lddata, S32 pos, char* data, S32 cnt, S32 size);
static en_READ_ASYNC_STATUS HIPLPollRead(st_HIPLOADDATA* lddata);
static S32 HIPLReadBytes(st_HIPLOADDATA* lddata, char* data, S32 cnt);
static S32 HIPLReadShorts(st_HIPLOADDATA* lddata, S16* data, S32 cnt);
static S32 HIPLReadLongs(st_HIPLOADDATA* lddata, S32* data, S32 cnt);
static S32 HIPLReadFloats(st_HIPLOADDATA* lddata, F32* data, S32 cnt);
static S32 HIPLReadString(st_HIPLOADDATA* lddata, char* buf);

#endif
