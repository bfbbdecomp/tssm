#ifndef ZPLAYERSB_H
#define ZPLAYERSB_H

#include "zPlayer.h"

// not everything is here?
struct zPlayerSB : zPlayer
{
    // padded to get  GetTreasureChestHud to match.
    // All structs that are used here are updated to be correct.
    // Assumably the dwarf is simply wrong
    U64 padding;
    U32 padding2;
    U8 inited;
    ManlinessMeter manlinessMeter;
    KrabbyPattyMeter krabbyPattyMeter;
    TreasureChest treasureChest;
    GoofyGooberMeter goofyGooberMeter;
    NitroMeter nitroMeter;

    TreasureChest* GetTreasureChestHud();
    void SetCamera();
    void LoadCheckPoint();
    void StoreCheckPoint();
    void HandleEvent(xBase* from, U32 thisEvent, F32* toParam, xBase* toParamWidget,
                     U32 toParamWidgetID);
    void RenderEffects();
    void Render();
    void Update(F32 dt);
    void Reset();
    void Init(xEntAsset* asset);
    void Init(xEntAsset* asset, S32 playerIdx);
};

#endif
