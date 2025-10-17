#include "zLightEffect.h"

void zLightEffectInitCauldron(_zLight* zlight)
{
    *zlight->reg = 0.0f;
    zlight->flags = zlight->flags | 1;
}

void zLightEffectRandomColFast(_zLight* zlight, F32)
{
}

void zLightEffectRandomCol(_zLight* zlight, F32)
{
}

void zLightEffectRandomColSlow(_zLight* zlight, F32)
{
}

void zLightEffectHalfDimFast(_zLight* zlight, F32)
{
}

void zLightEffectHalfDim(_zLight* zlight, F32)
{
}

void zLightEffectHalfDimSlow(_zLight* zlight, F32)
{
}

void zLightEffectDimFast(_zLight* zlight, F32)
{
}

void zLightEffectDim(_zLight* zlight, F32)
{
}

void zLightEffectDimSlow(_zLight* zlight, F32)
{
}

void zLightEffectStrobeFast(_zLight* zlight, F32)
{
}

void zLightEffectStrobe(_zLight* zlight, F32)
{
}

void zLightEffectStrobeSlow(_zLight* zlight, F32)
{
}

void zLightEffectInitFlicker(_zLight* zlight)
{
    *zlight->reg = 0.0f;
    zlight->flags = zlight->flags | 1;
}

void zLightEffectInitRandomCol(_zLight*)
{
}

void zLightEffectInitHalfDim(_zLight*)
{
}

void zLightEffectInitDim(_zLight*)
{
}

void zLightEffectInitStrobe(_zLight*)
{
}
