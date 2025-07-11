#ifndef ZGRID_H
#define ZGRID_H

#include "zScene.h"

#include "xEnt.h"
#include "xGrid.h"

extern xGrid colls_grid;
extern xGrid colls_oso_grid;
extern xGrid npcs_grid;

void zGridUpdateEnt(xEnt* ent);
void zGridExit(zScene* s);
void zGridInit(zScene* s);
void zGridReset(zScene* s);

void xGridEmpty(xGrid* grid);
void xGridKill(xGrid* grid);

#endif
