#include "zCar.h"

// Refrain from doing these functions until the zCar struct is fixed.

#define Car 1
#define Idle01 2
#define Drive 3
#define DriveSlippy 4
#define Boosting 5
#define Reversing 6
#define Crash 7
#define SpinLeft 8
#define SpinRight 9
#define Success 10
#define Failed 11
#define Death 12
#define Jump 13
#define JumpLandSquash 14

#define ANIM_COUNT 78
extern U32 g_hash_caranim[ANIM_COUNT];
const char* g_strz_caranim[ANIM_COUNT];

// Needs the functions calls updated
// NewState and DefaultBeforeEnter do not have correct amount of params yet

xAnimTable* CreateAnimTable()
{
    S32 ourAnims[32] = {
        // clang-format off
        Car,
        Idle01,
        Drive,
        DriveSlippy,
        Boosting,
        Reversing,
        Crash,
        SpinLeft,
        SpinRight,
        Success,
        Failed,
        Death,
        Jump,
        JumpLandSquash,

        // clang-format on
    };

    xAnimTable* table = xAnimTableNew("stop0", 0);

    xAnimTableNewState(table, "Car", 0x10, 0, 1.0f, NULL, NULL, 0.0f, NULL, NULL,
                       xAnimDefaultBeforeEnter, NULL, NULL);
}

U32 zCar::JumpEndedCB(xAnimTransition* unk0, xAnimSingle* anim, void* unk3)
{
    zCar* pCar = zEntPlayerCarGetCar();
}

U32 zCar::AnimDefaultCB(xAnimTransition* unk0, xAnimSingle* anim, void*)
{
    zCar* pCar = zEntPlayerCarGetCar();
}
