#include "xFXHighDynamicRange.h"
#include "xDebug.h"

U8 HDR_brightening;

namespace
{
    S32 background_glow_normal;
    S32 background_darken_normal;
    interpolator background_glow;
    interpolator background_darken;

    namespace tweak
    {
        F32 overbrighten = 3.0f;
        F32 overbrighten_decay = 0.3f;
        S32 downsamples = 4;
        bool blur_faster = true;
    }; // namespace tweak

    bool enabled = true;
} // namespace

S32 hdr_dump_efb;
S32 hdr_down_dst;

void xFXHighDynamicRangeResetGlowClamp()
{
}

void xFXHighDynamicRangeSetGlowClamp(S32 glow_min, bool dest_test)
{
    S32 unk0;

    unk0 = 0xff;
    if (glow_min < 0xff)
    {
        unk0 = glow_min;
    }
    if (unk0 < 0)
    {
        unk0 = 0;
    }
    else
    {
        unk0 = 0xff;
        if (glow_min < 0xff)
        {
            unk0 = glow_min;
        }
    }
    RwGameCubeSetAlphaCompare(7, 0, 1, 6, (char)unk0);
    _rwDlRenderStateSetZCompLoc(FALSE);
}

void xFXHighDynamicRangeGetConfiguration(xFXHighDynamicRangeConfiguration& configuration)
{
    configuration.glow = background_glow.value;
    configuration.darken = background_darken.value;
    configuration.overbrighten = tweak::overbrighten;
    configuration.overbrighten_decay = tweak::overbrighten_decay;
    configuration.downsamples = tweak::downsamples;
    configuration.blur_faster = tweak::blur_faster;
}

void xFXHighDynamicRangeSetConfiguration(const xFXHighDynamicRangeConfiguration& configuration)
{
    background_glow.value = configuration.glow;
    background_darken.value = configuration.darken;
    tweak::overbrighten = configuration.overbrighten;
    tweak::overbrighten_decay = configuration.overbrighten_decay;
    tweak::downsamples = configuration.downsamples;
    tweak::blur_faster = configuration.blur_faster;
}

S32 xFXHighDynamicRangeGetBackgroundGlow()
{
    return background_glow.value;
}

bool xFXHighDynamicRangeEnable(bool tmpEnable)
{
    return enabled = tmpEnable;
}

void xFXHighDynamicRangeTunePalette(RwRaster*, F32, bool)
{
}

void xFXHighDynamicRangeRestorePtank(RpAtomic*, U32)
{
}

U32 xFXHighDynamicRangeBrightenPtank(RpAtomic* ptank, bool bright)
{
    return 0;
}

void xFXHighDynamicRangeReset()
{
    // Stopped working on this function because the ghidra output is weird and objdiff is all over the place.
    // Could be a compiler quirk or something. Will revisit later.
    // FIXME

    // background_glow.value =
    //     ((0x43300000, background_glow_normal ^ 0x80000000) - 4503601774854144.0);

    // background_darken.value = background_darken_normal;
    // enabled = true;

    // background_darken.value =
    //     ((0x43300000, background_darken_normal ^ 0x80000000) - 4503601774854144.0);

    // background_glow.value = background_glow_normal;
    // //   DAT_80458194 = DAT_80458190;
    // background_glow_normal = background_glow.value;
    // //   DAT_8045818c = DAT_80458190;
    // //   DAT_8045819c = @1180;
    // //   DAT_80458198 = @1180;
    // //   DAT_804581ac = DAT_804581a8;
    // //   DAT_804581a4 = DAT_804581a8;
    // //   DAT_804581b4 = @1180;
    // //   DAT_804581b0 = @1180;
}

void xFXHighDynamicRangeSceneExit()
{
}

void xFXHighDynamicRangeSceneEnter()
{
}

void xFXHighDynamicRangeInit()
{
    background_glow.value = 10.0f;
    background_darken.value = 0x50;
    background_glow_normal = 10;
    background_darken_normal = 0x50;
    enabled = 1;

    background_glow.value = 10.0f;
    background_darken.value = 10.0f;
}
