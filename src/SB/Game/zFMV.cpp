#include "zFMV.h"
#include <stdio.h>
#include "iFMV.h"
#include "xSnd.h"
#include "zFMV.h"
#include "zGameState.h"

// Ported All Code from BFBB
// Updated FileTable and FMVFile Enum to be closer to correct

// Filetable may have reloc errors.
// Remove "", before 0 to see

zFMVFile zFMVFileTable[] = {
    { eFMVFile_NoMovie, "", 0 }, //
    { eFMVFile_NickLOGO, "", 0 }, //
    { eFMVFile_THQLOGO, "", 0 }, //
    { eFMVFile_HeavyIronLOGO, "", 0 }, //
    { eFMVFile_NickMovieLOGO, "", 0 }, //
    { eFMVFile_AttractMode, "", 0 }, //
    //{ eFMVFile_OXMESRB, "", 0 }, ////
    { eFMVFile_Pwr_SB_Karate2, "", 0 }, //
    { eFMVFile_Pwr_SB_Bash1, "", 0 }, //
    { eFMVFile_Pwr_SB_Bash2, "", 0 }, //
    { eFMVFile_Pwr_SB_Bowl1, "", 0 }, //
    { eFMVFile_Pwr_SB_Bowl2, "", 0 }, //
    { eFMVFile_Pwr_SB_Guitar1, "", 0 }, //
    { eFMVFile_Pwr_SB_Guitar2, "", 0 }, //
    { eFMVFile_Pwr_SB_Health4, "", 0 }, //
    { eFMVFile_Pwr_SB_Health5, "", 0 }, //
    { eFMVFile_Pwr_SB_Health6, "", 0 }, //
    { eFMVFile_Pwr_Pat_Spin2, "", 0 }, //
    { eFMVFile_Pwr_Pat_Cartwheel1, "", 0 }, //
    { eFMVFile_Pwr_Pat_Cartwheel2, "", 0 }, //
    { eFMVFile_Pwr_Pat_Bellyflop1, "", 0 }, //
    { eFMVFile_Pwr_Pat_Bellyflop2, "", 0 }, //
    { eFMVFile_Pwr_Pat_Throw1, "", 0 }, //
    { eFMVFile_Pwr_Pat_Throw2, "", 0 }, //
    { eFMVFile_Pwr_Pat_Health4, "", 0 }, //
    { eFMVFile_Pwr_Pat_Health5, "", 0 }, //
    { eFMVFile_Pwr_Pat_Health6, "", 0 }, //
    { eFMVFile_Story1, "", 0 }, //
    { eFMVFile_Story2, "", 1 }, //
    { eFMVFile_Story3, "", 1 }, //
    { eFMVFile_Story4, "", 1 }, //
    { eFMVFile_Story5, "", 1 }, //
    { eFMVFile_Story6, "", 1 }, //
    { eFMVFile_Story7, "", 1 }, //
    { eFMVFile_Story8, "", 1 }, //
    { eFMVFile_Story9, "", 1 }, //
    { eFMVFile_Story10, "", 1 }, //
    { eFMVFile_Story11, "", 1 }, //
    { eFMVFile_Story12, "", 0 }, //
    { eFMVFile_Promo1, "", 0 }, //
    { eFMVFile_Promo2, "", 0 }, //
    { eFMVFile_Promo3, "", 0 }, //
    { eFMVFile_Promo4, "", 0 }, //
    { eFMVFile_Promo5, "", 0 }, //
    { eFMVFile_Promo6, "", 0 }, //
    { eFMVFile_Making_Of, "", 0 }, //
    { eFMVFile_Trailer1, "", 0 }, //
    { eFMVFile_Trailer2, "", 0 }, //
    { eFMVFile_Trailer3, "", 0 }, //
    { eFMVFile_Pwr_Wagon, "", 0 }, //
    //{ eFMVFileCount, "", 0 }, ////
};
U32 zFMVPlay(const char* name, U32 buttons, F32 time, U32 uSubtitlesAID, bool skippable,
             bool lockController)
{
    U32 retval = 0;
    //xSndEffect currentEffect;

    //iFMVPlay(fullname, buttons, time, skippable, lockController);

    return retval;
}

char* zFMVFileGetName(eFMVFile fileEnum)
{
    for (S32 i = 0; i < eFMVFileCount; i++)
    {
        if (fileEnum == zFMVFileTable[i].fmvCode)
        {
            return zFMVFileTable[i].fileName;
        }
    }

    return NULL;
}

zFMVFile* zFMVFileGetFile(eFMVFile fmvId)
{
    // eFMVFileCount is currently 101, ASM says this should be 49
    // Turning this into a magic number for now
    S32 i;
    for (S32 i = 0; i < 49; i++)
    {
        if (fmvId == zFMVFileTable[i].fmvCode)
        {
            return (zFMVFile*)zFMVFileTable[i].fileName;
        }
    }

    return NULL;
}
