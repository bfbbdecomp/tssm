#include "zUI.h"

// I believe this is correct?
// Aren't these initialized to 0 if no initialization is given?

zUIController* UIControl = 0;
zUIControllerAsset* UIControlAsset = 0;
zUIUserString* zUIString = 0;
zUIUserStringAsset* zUIStringAsset = 0;

#include "zUICustom.cpp"
#include "zUIController.cpp"
#include "zUIUserString.cpp"
