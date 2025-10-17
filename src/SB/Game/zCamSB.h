#ifndef ZCAMSB_H
#define ZCAMSB_H

#include "xCamera.h"

struct zCamSB : xCam
{
    void update(F32 dt);
    void start();
    void destroy();
    void create();
};

#endif
