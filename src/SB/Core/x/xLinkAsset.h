#ifndef XLINKASSET_H
#define XLINKASSET_H

#include <types.h>

struct xLinkAsset
{
    U16 srcEvent; // 0x0
    U16 dstEvent; // 0x2
    U32 dstAssetID; // 0x4
    F32 param[4]; // 0x8
    U32 paramWidgetAssetID; // 0x18
    U32 chkAssetID; // 0x1C
};

#endif
