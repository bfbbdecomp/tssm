#include "iScrFX.h"
#include "rwcore.h"
#include "rwplcore.h"

extern RwRaster* g_rast_gctapdance;
extern S32 g_alreadyTriedAlloc;
extern _iMotionBlurData sMBD;
extern U32 sMotionBlurEnabled;

void iScrFxLensFlare_Reset()
{
}

void RenderRect(RwRect* rect, F32 unknown)
{
}

S32 iScrFxCameraDestroyed(RwCamera* pCamera)
{
    if (sMBD.motionBlurFrontBuffer != NULL)
    {
        RwRasterDestroy(sMBD.motionBlurFrontBuffer);
        sMBD.motionBlurFrontBuffer = NULL;
        return 1;
    }
    return 0;
}

void iScrFxCameraEndScene(RwCamera* pCamera)
{
}

void iScrFxCameraCreated(RwCamera* pCamera)
{
    sMBD.motionBlurAlpha = 0x90;
    sMBD.motionBlurFrontBuffer = NULL;
    // for some reason these are 0x40 ahead of target?
    sMBD.index[0] = 0;
    sMBD.index[1] = 1;
    sMBD.index[2] = 2;
    sMBD.index[3] = 0;
    sMBD.index[4] = 2;
    sMBD.index[5] = 3;
}

void iScrFxEnd()
{
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE,
                                                   (void*)rwFOGTYPENAFOGTYPE);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDZERO);
}

void iScrFxBegin()
{
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER,
                                                   (void*)rwFILTERNEAREST);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)true);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND,
                                                   (void*)rwBLENDINVSRCALPHA);
    RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATECULLMODE,
                                                   (void*)rwCULLMODECULLNONE);
}

void iScrFxInit()
{
}
