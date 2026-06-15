#ifndef XVEC3INLINES_H
#define XVEC3INLINES_H

#include "xVec3.h"
#include "fake_tgmath.h"

inline void xVec3Sub(xVec3* o, const xVec3* a, const xVec3* b)
{
    o->x = a->x - b->x;
    o->y = a->y - b->y;
    o->z = a->z - b->z;
}
void xVec3Cross(xVec3* o, const xVec3* a, const xVec3* b);
void xVec3Inv(xVec3* o, const xVec3* v);
void xVec3Copy(xVec3* o, const xVec3* v);
inline F32 xVec3Length(const xVec3* v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
inline void xVec3SMul(xVec3* o, const xVec3* v, F32 s)
{
    o->x = s * v->x;
    o->y = s * v->y;
    o->z = s * v->z;
}
inline void xVec3Add(xVec3* o, const xVec3* a, const xVec3* b)
{
    o->x = a->x + b->x;
    o->y = a->y + b->y;
    o->z = a->z + b->z;
}
void xVec3Init(xVec3* v, F32 _x, F32 _y, F32 _z);
void xVec3AddTo(xVec3* o, const xVec3* v);
void xVec3Lerp(xVec3* o, const xVec3* a, const xVec3* b, F32 t);
void xVec3ScaleC(xVec3* o, const xVec3* v, F32 x, F32 y, F32 z);
F32 xVec3Dist(const xVec3* a, const xVec3* b);
F32 xVec3Dist2(const xVec3* vecA, const xVec3* vecB);
F32 xVec3Length2(const xVec3* vec);
F32 xVec3LengthFast(F32 x, F32 y, F32 z);
void xVec3AddScaled(xVec3* o, const xVec3* v, F32 s);

inline void xVec3SMulBy(xVec3* v, F32 s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

inline void xVec3SubFrom(xVec3* o, const xVec3* v)
{
    o->x -= v->x;
    o->y -= v->y;
    o->z -= v->z;
}

#endif
