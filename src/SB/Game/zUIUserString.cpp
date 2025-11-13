void zUIUserString_Init(zUIUserString*, zUIUserStringAsset*)
{
}

// Inlining issue
void zUIUserString_Init(xBase&, xDynAsset&, unsigned long)
{
    zUIUserString* zUIString = 0;
    zUIUserStringAsset* zUIStringAsset = 0;
    zUIUserString_Init(zUIString, zUIStringAsset);
}

U32 zUIUserString::GetSortKey() const
{
    return 0;
}
