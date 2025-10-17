#include "zGameExtras.h"

void zGameExtras_Load(xSerial* xser)
{
    S32 keepers[2];
    keepers[0] = 0;
    xser->Read(keepers);
    g_flg_chEnabled |= keepers[0];
}

void zGameExtras_Save(xSerial* xser)
{
    xser->Write(g_flg_chEnabled & 0xFFFF);
}

S32 zGameExtras_CheatFlags()
{
    return g_flg_chEnabled;
}

S32 EGG_check_ExtrasFlags(EGGItem*)
{
    switch (g_currMonth)
    {
    case JANUARY:
        if (g_currDay == 1) // New Year's Day
            g_gameExtraFlags |= 0b000000001;
        break;

    case FEBRUARY:
        if (g_currDay == 2)
            g_gameExtraFlags |= 0b100000000;
        break;

    case MARCH:
        if (g_currDay == 15)
            g_gameExtraFlags |= 0b000001000;
        if (g_currDay == 17) // St. Patrick's Day
            g_gameExtraFlags |= 0b000000100;
        if (g_currDay == 18)
            g_gameExtraFlags |= 0b100000000;
        if (g_currDay == 21)
            g_gameExtraFlags |= 0b100000000;
        if (g_currDay == 22)
            g_gameExtraFlags |= 0b100000000;
        break;

    case APRIL:
        if (g_currDay == 1) // April Fools' Day
            g_gameExtraFlags |= 0b010000000;
        break;

    case MAY:
        break;

    case JUNE:
        if (g_currDay == 6)
            g_gameExtraFlags |= 0b001000000;
        break;

    case JULY:
        if (g_currDay == 4) // Independence Day
            g_gameExtraFlags |= 0b000000010;
        break;

    case AUGUST:
        if (g_currDay == 8)
            g_gameExtraFlags |= 0b100000000;
        break;

    case SEPTEMBER:
        if (g_currDay == 8)
            g_gameExtraFlags |= 0b000000010;
        break;

    case OCTOBER:
        if (g_currDay == 5)
            g_gameExtraFlags |= 0b100000000;
        if (g_currDay == 14)
            g_gameExtraFlags |= 0b100000000;
        if (g_currDay == 22)
            g_gameExtraFlags |= 0b100000000;
        if (g_currDay == 31) // Halloween
            g_gameExtraFlags |= 0b000100000;
        break;

    case NOVEMBER:
        if (g_currDay == 5)
            g_gameExtraFlags |= 0b100000000;
        break;

    case DECEMBER:
        break;
    }

    return 0;
}

void zGameExtras_SceneReset()
{
    if (!g_enableGameExtras)
    {
        return;
    }

    EGGItem* egg_next = g_eggBasket;

    while (egg_next->fun_check)
    {
        EGGItem* egg = egg_next++;

        if (egg->enabled)
        {
            if (egg->funcs->fun_reset)
            {
                egg->funcs->fun_reset(egg);
            }
        }
    }
}

void zGameExtras_MoDay(S32* month, S32* day)
{
    *month = g_currMonth;
    *day = g_currDay;
}

S32 zGameExtras_ExtrasFlags()
{
    return g_gameExtraFlags;
}
