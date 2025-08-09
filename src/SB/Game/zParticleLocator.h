#ifndef ZPARTICLELOCATOR_H
#define ZPARTICLELOCATOR_H

#include "zParticleGenerator.h"
#include <PowerPC_EABI_Support/Runtime/MWCPlusPlusLib.h>

enum volume_type_enum
{
    VOLUME_POINT,
    VOLUME_SPHERE,
    VOLUME_SPHERE_EDGE,
    VOLUME_CIRCLE,
    VOLUME_CIRCLE_EDGE,
    VOLUME_LINE,
    VOLUME_CAPSULE,
    VOLUME_CAPSULE_EDGE,
    VOLUME_CYLINDER,
    VOLUME_CYLINDER_EDGE,
    VOLUME_MODEL_VERT,
    VOLUME_MODEL_SURFACE,
    MAX_VOLUME
};

struct volume_data_0
{
    struct _class_3
    {
        F32 ang_delta;
    };

    struct _class_5
    {
        F32 length;
        U32 cap_weight;
    };

    struct _class_7
    {
        S32 vertices[8];
        xModelInstance* models[8];
        S32 models_size;
        S32 total_vertices;
    };

    struct _class_0
    {
    };

    volume_type_enum type;
    F32 radius;
    union
    {
        _class_3 circle;
        _class_5 line;
        _class_7 model_vert;
        _class_0 model_surface;
    };
};

struct _class_2
{
    xMat4x3 mat;
    S32 bone;
};

struct attach_data_1
{
    _class_2 entity;
};

struct zParticleLocator;

struct activity_data
{
    zParticleLocator* owner;
    zParticleAsset* asset;
    void (*update_attach)(activity_data&, F32);
    void (*get_offset)(xVec3&, volume_data_0&);
    attach_data_1 attach;
    volume_data_0 volume;
    xEnt* attach_to;
    xMat4x3 buffer_mat;
    xMat4x3* attach_mat;
};

struct zParticleLocator
{
    activity_data* activity;

    void get_center(xVec3& center);
    xMat4x3& get_frame() const;
    void make_locations(void* data, S32 stride, S32 elements, S32 loc_offset);
    void update(F32 dt);
    void deactivate();
    U8 activate(const zParticleAsset& asset, xEnt* attach_to);
    void restart();
    void create();
    void scene_enter();
};

#endif
