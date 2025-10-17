S32 xSGT_LoadPrefsCB(void* vp, st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, U32 ui,
                     S32 i)
{
    U32 stereo;

    xSGReadData(xsgdata, rctxt, &stereo, 1);
    xSGReadData(xsgdata, rctxt, &gSnd.categoryVolFader[2], 1);
    xSGReadData(xsgdata, rctxt, &gSnd.categoryVolFader[0], 1);
    xSGReadData(xsgdata, rctxt, &globals.option_vibration, 1);
    if (globals.option_vibration != 0)
    {
        xPadRumbleEnable(globals.currentActivePad, 1);
    }
    else
    {
        xPadRumbleEnable(globals.currentActivePad, 0);
    }
    return 1;
}

S32 xSGT_LoadLoadCB(void* vp, st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_READCONTEXT* rctxt, U32 ui,
                    S32 i)
{
    char bigbuf[32] = {};
    S32 compdiff = 0;

    xSGReadData(xsgdata, rctxt, bigbuf, 1, strlen(currSceneStr));
    if (strlen(currSceneStr) != strlen(bigbuf))
    {
        compdiff = 1;
    }
    if (compdiff == 0)
    {
        strcpy(sceneRead, bigbuf);
    }

    return compdiff == 0;
}

S32 xSGT_SaveProcPrefsCB(void* vp, st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt)
{
    int sum = 0;
    sum += xSGWriteData(xsgdata, wctxt, &gSnd.stereo, 1);
    sum += xSGWriteData(xsgdata, wctxt, &gSnd.categoryVolFader[2], 1);
    sum += xSGWriteData(xsgdata, wctxt, &gSnd.categoryVolFader[0], 1);
    sum += xSGWriteData(xsgdata, wctxt, &globals.option_vibration, 1);
    return sum + 1;
}

S32 xSGT_SaveInfoPrefsCB(void* p1, st_XSAVEGAME_DATA* data, S32* i, S32* j)
{
    *i = 16;
    *j = *i * 2;
    return 1;
}

S32 xSGT_SaveProcCB(void* vp, st_XSAVEGAME_DATA* xsgdata, st_XSAVEGAME_WRITECONTEXT* wctxt)
{
    if (globals.sceneCur->sceneID == 'PG12')
    {
        strcpy(currSceneStr, xUtil_idtag2string('HB01', 0));
    }
    else
    {
        strcpy(currSceneStr, xUtil_idtag2string(globals.sceneCur->sceneID, 0));
    }
    return xSGWriteData(xsgdata, wctxt, currSceneStr, 1, strlen(currSceneStr)) + 1;
}

S32 xSGT_SaveInfoCB(void* vp, st_XSAVEGAME_DATA* xsgdata, S32* need, S32* most)
{
    *need = xSGWriteStrLen(currSceneStr);
    *most = *need << 1;
    return 1;
}
