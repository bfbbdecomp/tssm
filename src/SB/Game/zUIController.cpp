#include "zUIController.h"

void zUIController_Init(zUIController*, zUIControllerAsset*)
{
}

// Inlining issue
void zUIController_Init(xBase&, xDynAsset&, unsigned long)
{
    zUIController* UIControl = 0;
    zUIControllerAsset* UIControlAsset = 0;
    zUIController_Init(UIControl, UIControlAsset);
}

void zUIController::DoRender() const
{
}
