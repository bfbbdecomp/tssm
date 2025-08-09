#include "zParticleLocator.h"

extern U32 gActiveHeap;

namespace
{
    activity_data* activities;
    S32 activities_used;
} // namespace

xMat4x3& zParticleLocator::get_frame() const
{
    return (xMat4x3&)activity->attach_mat->right.x;
}

void zParticleLocator::update(F32 dt)
{
}

void zParticleLocator::restart()
{
}

void zParticleLocator::create()
{
    activity = 0;
}

void zParticleLocator::scene_enter()
{
    S32 tmpBlock;

    tmpBlock = (S32)xMemAlloc(gActiveHeap, 0x8810, 0);
    activities = (activity_data*)__construct_new_array((void*)tmpBlock, 0x0, 0x0, 0x110, 0x80);
    activities_used = 0;
}

namespace
{

    struct volume_data
    {
    };

    void get_offset_model_vert_expand(xVec3&, volume_data&)
    {
    }
} // namespace
