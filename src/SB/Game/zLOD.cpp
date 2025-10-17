#include "zLOD.h"

zLODTable* zLOD_Get(xEnt* ent)
{
    if (!ent->model)
    {
        return 0;
    }

    for (int i = 0; i < sLODTableCount; i++)
    {
        if (sLODTableList[i].baseBucket != NULL)
        {
            if ((*sLODTableList[i].baseBucket)->OriginalData == ent->model->Data)
            {
                return &sLODTableList[i];
            }
        }
    }
    return 0;
}
