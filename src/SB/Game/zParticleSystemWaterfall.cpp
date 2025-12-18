
#include "zParticleSystemWaterfall.h"

void zParticleGeneratorWaterfallSplash::deactivate()
{
    zParticleGenerator* pGen;
    pGen->deactivate();
    locator.deactivate();
}

void zParticleGeneratorWaterfallSplash::activate()
{
    zParticleGenerator* pGen;
    pGen->activate();
    locator.activate(*asset, attach_to); // Make const
}

void zParticleGeneratorWaterfallSplash::reset()
{
    zParticleGenerator::reset();
    emitted = 0;
    locator.restart();
    locator.deactivate();
    if (flags & ~0xFFFFFFFE)
    {
        locator.activate(*asset, attach_to);
    }
}

S32 zParticleSystemWaterfallSplash::get_asset_size() const
{
    return 56; // Calls for 0x38
}

void zParticleSystemWaterfallSplash::scene_enter()
{
    batch_group.create();
}

void zParticleGeneratorWaterfallMist::deactivate()
{
    zParticleGenerator* pGen;
    pGen->deactivate();
    locator.deactivate();
}

void zParticleGeneratorWaterfallMist::activate()
{
    zParticleGenerator* pGen;
    pGen->activate();
    locator.activate(*asset, attach_to); // Make const
}

void zParticleGeneratorWaterfallMist::reset()
{
    zParticleGenerator::reset();
    emitted = 0;
    locator.restart();
    locator.deactivate();
    if (flags & ~0xFFFFFFFE)
    {
        locator.activate(*asset, attach_to);
    }
}

S32 zParticleSystemWaterfallMist::get_asset_size() const
{
    return 52; // Calls for 0x34
}

void zParticleSystemWaterfallMist::scene_enter()
{
    batch_group.create();
}

void zParticleGeneratorWaterfall::deactivate()
{
    zParticleGenerator* pGen;
    pGen->deactivate();
    locator.deactivate();
}

void zParticleGeneratorWaterfall::activate()
{
    zParticleGenerator* pGen;
    pGen->activate();
    locator.activate(*asset, attach_to); // Make const
}

void zParticleGeneratorWaterfall::reset() // 87%
{
    zParticleGenerator::reset();
    emitted = 0;
    locator.restart();
    locator.deactivate();
    if (flags & ~0xFFFFFFFE)
    {
        locator.activate(*asset, attach_to);
    }
}

S32 zParticleSystemWaterfall::get_asset_size() const
{
    return 76; // Calls for 0x4c
}

void zParticleSystemWaterfall::scene_enter()
{
    batch_group.create();
}
