
#ifndef ZPARTICLESYSTEMWATERFALL_H
#define ZPARTICLESYSTEMWATERFALL_H

#include "zParticleGenerator.h"
#include "zParticleLocator.h"

struct relative_ordering
{
    S32 other;
    U8 before_other;
};

struct xParticleBatchGroup
{
    U8* elements;
    S32 elements_size;

    void create();
};

struct zParticleSystemWaterfall : zParticleSystem
{
    S32 unk;
    xParticleBatchGroup batch_group;

    S32 get_asset_size() const;
    void update(F32 dt);
    void scene_enter();
    zParticleGenerator* create_generator(zParticleAsset& asset);
};

struct zParticleGeneratorWaterfall : zParticleGenerator
{
    S32 unk;
    zParticleSystemWaterfall* system;
    xEnt* attach_to;
    F32 emitted; //0x24
    zParticleLocator locator;
    S32 batch_id;
    xVec3 vel_dir;

    void deactivate();
    void activate();
    void reset();
    void create(zParticleSystemWaterfall& system, zParticleAsset& asset);
    void update(F32 dt);
};

struct zParticleSystemWaterfallMist : zParticleSystem
{
    S32 unk;
    xParticleBatchGroup batch_group;

    S32 get_asset_size() const;
    void update(F32 dt);
    void scene_enter();
    zParticleGenerator* create_generator(zParticleAsset& asset);
};

struct zParticleGeneratorWaterfallMist : zParticleGenerator
{
    S32 unk;
    zParticleSystemWaterfallMist* system;
    xEnt* attach_to;
    F32 emitted;
    zParticleLocator locator;
    S32 batch_id;

    void deactivate();
    void activate();
    void reset();
    void create(zParticleSystemWaterfallMist& system, zParticleAsset& asset);
    void update(F32 dt);
};

struct zParticleSystemWaterfallSplash : zParticleSystem
{
    S32 unk;
    xParticleBatchGroup batch_group;

    S32 get_asset_size() const;
    void update(F32 dt);
    void scene_enter();
    zParticleGenerator* create_generator(zParticleAsset& asset);
};

struct zParticleGeneratorWaterfallSplash : zParticleGenerator
{
    S32 unk;
    zParticleSystemWaterfallSplash* system;
    xEnt* attach_to;
    F32 emitted;
    zParticleLocator locator;
    S32 batch_id;

    void deactivate();
    void activate();
    void reset();
    void create(zParticleSystemWaterfallSplash& system, zParticleAsset& asset);
    void update(F32 dt);
};

#endif
