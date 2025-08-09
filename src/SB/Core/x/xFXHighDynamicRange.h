#ifndef XFXHIGHDYNAMICRANGE_H
#define XFXHIGHDYNAMICRANGE_H

#include "xFX.h"
#include "xGlobals.h"
#include "xModelBlur.h"
#include "xFX.h"

struct xFXHighDynamicRangeConfiguration
{
    S32 glow;
    S32 darken;
    S32 downsamples;
    F32 overbrighten;
    F32 overbrighten_decay;
    bool blur_faster;
};

struct downsample_pass_info
{
    xVec2* offset;
    S32 resamples;
};

struct interpolator
{
    S32 value;
    F32 start;
    F32 end;
    F32 current;
    F32 t;
    F32 trate;
};

#endif
