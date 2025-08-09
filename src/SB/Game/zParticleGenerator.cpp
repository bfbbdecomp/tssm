#include "zParticleGenerator.h"

namespace
{
    zParticleSystem** system_handler_set[7];
    S32 system_handler_set_size[7];

    S32 compare_generator_systems_function(const void* e1, const void* e2)
    {
        zParticleGenerator* g1;
        zParticleGenerator* g2;
        return 0;
    }
} // namespace

void zParticleGenerator::perturb_dir(xVec3& dir, F32 angle)
{
    F32 rand_scale;
    F32 ang_min;
    xVec3 ang;
    xMat3x3 mat;
}

xEnt* zParticleGenerator::get_attach_entity(const zParticleAsset& asset)
{
    U32 id;
    xEnt* ent;

    // asset.attach_type 0x1c ???
    return 0;
}

void zParticleGenerator::event_handler(xBase* to, xBase* from, U32 event, const F32* param,
                                       xBase* paramWidget, U32 paramWidgetID)
{
}

void zParticleGenerator::deactivate()
{
    flags &= ~0x1;
}

void zParticleGenerator::activate()
{
    flags |= 0x1;
}

void zParticleGenerator::reset()
{
    if ((asset->flags & 1) != 0)
    {
        flags |= 0x1;
        return;
    }

    flags &= 0xfffffffe;
}

static void zSceneObjHashtableAdd(U32 id, xBase* base);

void zParticleGenerator::create(zParticleSystem& system, zParticleAsset& asset)
{
    // This isn't even close to being correct, but its the best that i can
    // do at this moment. A lot of what i try isnt working so its hacked together
    zParticleGenerator* parGen;

    int iVarl;

    xBaseInit(parGen, &asset);
    parGen->asset = &asset;
    parGen->link->param[4] = 0x7c;
    flags = 0;
    parGen->eventFunc = event_handler;
    if (link->param[5] != 0)
    {
        iVarl = (int)system.generators;
        parGen = (zParticleGenerator*)&asset + iVarl;
    }
    zSceneObjHashtableAdd(*(uint*)parGen, (xBase*)parGen);
}

void zParticleRender()
{
    // Also not really close, too tired to decomp effectively ig
    zParticleSystem** set;
    zParticleSystem** end_set;

    set = system_handler_set[0];
    end_set = (zParticleSystem**)system_handler_set_size[0];

    for (end_set = (zParticleSystem**)system_handler_set; end_set != set; end_set++)
    {
        set + 0x28;
    }
}
