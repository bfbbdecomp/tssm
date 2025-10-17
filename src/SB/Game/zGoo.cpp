#include "zGoo.h"

void zGooExit()
{
    zgoo_gps = 0;
    zgoo_ngps = 0;
    zgoo_nused = 0;
}

void zGooInit(S32 nobj)
{
    zgoo_gps = (zGooParams*)xMemAlloc(gActiveHeap, nobj * 0xc, 0);
    zgoo_ngps = nobj;
}
