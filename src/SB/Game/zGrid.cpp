#include "zGrid.h"

void zGridExit(zScene* s)
{
    xGridKill(&colls_grid);
    xGridKill(&colls_oso_grid);
    xGridKill(&npcs_grid);
    xGridKill(&grabbable_grid);
    zGridInitted = 0;
}
