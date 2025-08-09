#ifndef ZFMV_H
#define ZFMV_H

#include <types.h>

enum eFMVFile
{
    eFMVFile_NoMovie,
    eFMVFile_HeavyIronLOGO,
    eFMVFile_NickLOGO = 0x4,
    eFMVFile_THQLOGO,
    eFMVFile_AttractMode,
    eFMVFile_NickMovieLOGO,
    eFMVFile_OXMESRB,
    eFMVFile_Pwr_SB_Bash1 = 0x14,
    eFMVFile_Pwr_SB_Bowl1,
    eFMVFile_Pwr_SB_Guitar1,
    eFMVFile_Pwr_Pat_Cartwheel1,
    eFMVFile_Pwr_Pat_Bellyflop1,
    eFMVFile_Pwr_Pat_Throw1,
    eFMVFile_Pwr_SB_Karate2,
    eFMVFile_Pwr_SB_Bash2,
    eFMVFile_Pwr_SB_Bowl2,
    eFMVFile_Pwr_SB_Guitar2,
    eFMVFile_Pwr_SB_Health4,
    eFMVFile_Pwr_SB_Health5,
    eFMVFile_Pwr_SB_Health6,
    eFMVFile_Pwr_Pat_Spin2,
    eFMVFile_Pwr_Pat_Cartwheel2,
    eFMVFile_Pwr_Pat_Bellyflop2,
    eFMVFile_Pwr_Pat_Throw2,
    eFMVFile_Pwr_Pat_Health4,
    eFMVFile_Pwr_Pat_Health5,
    eFMVFile_Pwr_Pat_Health6,
    eFMVFile_Story1 = 0x32,
    eFMVFile_Story2,
    eFMVFile_Story3,
    eFMVFile_Story4,
    eFMVFile_Story5,
    eFMVFile_Story6,
    eFMVFile_Story7,
    eFMVFile_Story8,
    eFMVFile_Story9,
    eFMVFile_Story10,
    eFMVFile_Story11,
    eFMVFile_Story12,
    eFMVFile_Promo1 = 0x46,
    eFMVFile_Promo2,
    eFMVFile_Promo3,
    eFMVFile_Promo4,
    eFMVFile_Promo5,
    eFMVFile_Promo6,
    eFMVFile_Making_Of = 0x50,
    eFMVFile_Trailer1,
    eFMVFile_Trailer2,
    eFMVFile_Trailer3,
    eFMVFile_Pwr_Wagon = 0x64,
    eFMVFileCount
};

struct zFMVFile
{
    eFMVFile fmvCode;
    char* fileName;
    U8 happensBeforePortal;
};

U32 zFMVPlay(char* filename, U32 buttons, F32 time, U32 unk0, bool skippable, bool lockController);
char* zFMVFileGetName(eFMVFile fileEnum);
zFMVFile* zFMVFileGetFile(eFMVFile fmvId);

#endif
