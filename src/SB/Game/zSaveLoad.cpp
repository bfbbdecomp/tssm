#include "zSaveLoad.h"

#include "iTRC.h"

void zSaveLoad_DispatchCB(U32 dispatchEvent, const F32* toParam)
{
    switch (dispatchEvent)
    {
    case 0xa8:
        promptSel = 4;
        break;
    case 0xaa:
        promptSel = 3;
        break;
    case 0xab:
        currentCard = (int)*toParam;
        en_SAVEGAME_MODE mode = XSG_MODE_LOAD;
        if (gGameMode == eGameMode_Save)
        {
            mode = XSG_MODE_SAVE;
        }
        st_XSAVEGAME_DATA* data = xSGInit(mode);
        zSaveLoad_CardCheckSpaceSingle_doCheck(data, currentCard);
        xSGDone(data);
        break;
    case 0xac:
        currentGame = (int)*toParam;
        break;
    case 0xad:
        promptSel = 3;
        break;
    }
}

// Scheduling meme on the return
U32 zSaveLoad_SaveLoop()
{
    zSaveLoadInit();
    S32 state = 0;
    saveSuccess = 0;

    while (state != 6)
    {
        switch (state)
        {
        case 0:
            while (zSaveLoad_CardCount() == 0)
            {
                zSaveLoad_CardPrompt(0);
            }
            state = 1;
            break;
        case 1:
            switch (zSaveLoad_CardPick(0))
            {
            case 1:
                state = 2;
                break;
            case 4:
                if (gGameState != 0)
                {
                    gGameMode = eGameMode_Pause;
                    state = 6;

                    xBase* sendTo = zSceneFindObject(xStrHash("PAUSE OPTIONS BKG GROUP"));
                    zEntEvent(sendTo, eEventUIFocusOn_Select);

                    sendTo = zSceneFindObject(xStrHash("PAUSE OPTIONS GROUP"));
                    zEntEvent(sendTo, eEventUIFocusOn);

                    sendTo = zSceneFindObject(xStrHash("PAUSE OPTION MGR UIF"));
                    zEntEvent(sendTo, eEventUIFocusOn_Select);

                    sendTo = zSceneFindObject(xStrHash("PAUSE OPTION SAVE UIF"));
                    zEntEvent(sendTo, eEventUIFocusOn_Select);
                }
                else
                {
                    gGameMode = eGameMode_Title;
                    state = 6;

                    xBase* sendTo = zSceneFindObject(xStrHash("MNU3 START CREATE NEW GAME NO"));
                    zEntEvent(sendTo, eEventUIFocusOn);

                    sendTo = zSceneFindObject(xStrHash("MNU3 START CREATE NEW GAME NO"));
                    zEntEvent(sendTo, eEventUIUnselect);

                    sendTo = zSceneFindObject(xStrHash("MNU3 START CREATE NEW GAME YES"));
                    zEntEvent(sendTo, eEventUIFocusOn_Select);

                    sendTo = zSceneFindObject(xStrHash("MNU3 START CREATE NEW GAME"));
                    zEntEvent(sendTo, eEventUIFocusOn);

                    sendTo = zSceneFindObject(xStrHash("BLUE ALPHA 1 UI"));
                    zEntEvent(sendTo, eEventInvisible);
                }
                break;
            case 11:
                state = 10;
                currentGame = 0;
                break;
            default:
                state = 0;
            }
            break;
        case 2:
            sAccessType = 2;
            switch (zSaveLoad_GameSelect(0))
            {
            case 2:
            case 4:
            case 5:
            case 6:
            case 7:
            case 9:
                state = 1;
                break;
            case 1:
                state = 10;
                break;
            case -1:
                zSaveLoad_ErrorPrompt(0);
                state = 1;
                break;
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        case 11:
            zGameModeSwitch(eGameMode_Game);
            zGameStateSwitch(1);
            state = 6;
            break;
        case 10:
            S32 tmp = zSaveLoad_SaveGame();
            if (tmp == 1)
            {
                zGameModeSwitch(eGameMode_Game);
                zGameStateSwitch(1);
                saveSuccess = 1;
                state = 6;

                xBase* sendTo = zSceneFindObject(xStrHash("MNU4 SAVE COMPLETED"));
                zEntEvent(sendTo, eEventVisible);
            }
            else
            {
                switch (tmp)
                {
                case 7:
                    zSaveLoad_SaveDamagedErrorPrompt(0);
                    break;
                case 8:
                    zSaveLoad_CardYankedErrorPrompt(0);
                    break;
                case 10:
                    zSaveLoad_CardPromptSpace(0);
                    break;
                default:
                    zSaveLoad_ErrorPrompt(0);
                    break;
                }
                state = 1;
            }
            break;
        }
    }

    sAccessType = 0;
    return saveSuccess;
}

U32 zSaveLoad_LoadLoop()
{
    zSaveLoadInit();
    sAccessType = 1;
    S32 state = 0;
    while (state != 6)
    {
        switch (state)
        {
        case 0:
            while (zSaveLoad_CardCount() == 0)
            {
                zSaveLoad_CardPrompt(1);
            }
            zSaveLoad_UIEvent(0x14, eEventUIFocusOff_Unselect);
            state = 1;
            break;
        case 1:
            switch (zSaveLoad_CardPick(1))
            {
            case 1:
                state = 2;
                break;
            case 4:
                state = 8;
                break;
            case -1:
                state = 0;
                break;
            case 2:
                break;
            }
            break;
        case 2:
            switch (zSaveLoad_GameSelect(1))
            {
            case 2:
            case 4:
            case 5:
            case 6:
            case 7:
            case 9:
                state = 1;
                break;
            case 1:
                state = 9;
                break;
            case -1:
                zSaveLoad_ErrorPrompt(1);
                state = 1;
                break;
            }
            break;
        case 3:
            break;
        case 8:
            zGameModeSwitch(eGameMode_Title);
            zGameStateSwitch(0);
            state = 6;
            break;
        case 9:
            switch (zSaveLoad_LoadGame())
            {
            case 1:
                zGameModeSwitch(eGameMode_Game);
                zGameStateSwitch(0);
                state = 6;
                globals.autoSaveFeature = 1;
                break;
            case 7:
                zSaveLoad_DamagedSaveGameErrorPrompt(1);
                state = 1;
                break;
            default:
                zSaveLoad_ErrorPrompt(1);
                state = 1;
            }
        }
    }

    zSendEventToThumbIcon(eEventInvisible);
    return (U32)sceneRead[0] << 0x18 | (U32)sceneRead[1] << 0x10 | (U32)sceneRead[2] << 0x8 |
           (U32)sceneRead[3];
}

// Reordering, causing different register use at the end
S32 zSaveLoad_LoadGame()
{
    S32 success = false;
    S32 teststat = true;
    S32 rc;
    en_XSGASYNC_STATUS asstat = XSG_ASTAT_NOOP;
    S32 use_tgt = CardtoTgt(currentCard);
    autoSaveCard = currentCard;

    st_XSAVEGAME_DATA* xsgdata = zSaveLoadSGInit(XSG_MODE_LOAD);
    xSGTgtSelect(xsgdata, use_tgt);
    xSGGameSet(xsgdata, currentGame);

    if (xsgdata == NULL)
    {
        teststat = false;
    }

    xSGAddLoadClient(xsgdata, 'ROOM', NULL, xSGT_LoadLoadCB);
    xSGAddLoadClient(xsgdata, 'PREF', NULL, xSGT_LoadPrefsCB);
    xSerial_svgame_register(xsgdata, XSG_MODE_LOAD);

    if (!xSGSetup(xsgdata))
    {
        teststat = false;
    }

    en_XSG_WHYFAIL whyFail = XSG_WHYERR_NONE;
    if (teststat)
    {
        rc = xSGProcess(xsgdata);
        if (rc)
        {
            asstat = xSGAsyncStatus(xsgdata, 1, &whyFail, NULL);
        }
        if (!rc)
        {
            teststat = false;
        }
        else
        {
            switch (asstat)
            {
            case XSG_ASTAT_SUCCESS:
                success = true;
                break;
            case XSG_ASTAT_FAILED:
                success = false;
                break;
            case XSG_ASTAT_NOOP:
            case XSG_ASTAT_INPROG:
                break;
            }
        }
    }

    if (teststat)
    {
        rc = xSGWrapup(xsgdata);
        if (rc < 0)
        {
            teststat = false;
            whyFail = XSG_WHYERR_OTHER;
        }
        else if (rc == 0)
        {
            teststat = false;
        }
    }

    if (!zSaveLoadSGDone(xsgdata))
    {
        teststat = false;
    }

    XSGAutoData* asg = xSGAutoSave_GetCache();
    S32 use_game = currentCard;
    if (success && teststat)
    {
        S32 idx = xSGTgtPhysSlotIdx(xsgdata, use_tgt);
        asg->SetCache(use_tgt, use_game, idx);
        globals.autoSaveFeature = 1;
        return 1;
    }
    asg->Discard();

    switch (whyFail)
    {
    case XSG_WHYERR_OTHER:
        return 7;
    default:
        return -1;
    }
}

// Reordering at beginning
S32 zSaveLoad_SaveGame()
{
    S32 success = false;
    S32 teststat = true;
    en_XSGASYNC_STATUS asstat = XSG_ASTAT_NOOP;
    S32 use_tgt = CardtoTgt(currentCard);
    S32 use_game = currentGame;
    autoSaveCard = currentCard;

    st_XSAVEGAME_DATA* xsgdata = zSaveLoadSGInit(XSG_MODE_SAVE);
    if (xSGCheckMemoryCard(xsgdata, currentCard) == 0)
    {
        zSaveLoadSGDone(xsgdata);
        return 8;
    }

    xSGTgtSelect(xsgdata, use_tgt);
    xSGGameSet(xsgdata, use_game);
    if (xsgdata == NULL)
    {
        teststat = false;
    }

    zSceneSave(globals.sceneCur, NULL);

    xSGAddSaveClient(xsgdata, 'ROOM', NULL, xSGT_SaveInfoCB, xSGT_SaveProcCB);
    xSGAddSaveClient(xsgdata, 'PREF', NULL, xSGT_SaveInfoPrefsCB, xSGT_SaveProcPrefsCB);

    xSerial_svgame_register(xsgdata, XSG_MODE_SAVE);

    U32 progress = zSceneCalcProgress();
    char label[64];
    const char* area;

    if (globals.sceneCur->sceneID == 'PG12')
    {
        area = zSceneGetLevelName('HB01');
    }
    else
    {
        area = zSceneGetLevelName(globals.sceneCur->sceneID);
    }

    strncpy(label, area, sizeof(label));

    if (!xSGSetup(xsgdata, use_game, label, progress, 0, zSceneGetLevelIndex()))
    {
        teststat = false;
    }

    en_XSG_WHYFAIL whyFail = XSG_WHYERR_NONE;
    if (teststat)
    {
        S32 rc = xSGProcess(xsgdata);
        if (rc)
        {
            asstat = xSGAsyncStatus(xsgdata, 1, &whyFail, NULL);
        }

        xSGGameIsEmpty(xsgdata, use_game);
        xSGTgtHasGameDir(xsgdata, use_tgt);
        if (!rc)
        {
            teststat = false;
        }
        else
        {
            switch (asstat)
            {
            case XSG_ASTAT_SUCCESS:
                success = true;
                break;
            case XSG_ASTAT_FAILED:
                success = false;
                break;
            case XSG_ASTAT_NOOP:
            case XSG_ASTAT_INPROG:
                break;
            }
        }
    }

    if (teststat && !xSGWrapup(xsgdata))
    {
        teststat = false;
    }

    if (!xSGCheckMemoryCard(xsgdata, currentCard))
    {
        zSaveLoadSGDone(xsgdata);
        return 8;
    }

    if (!zSaveLoadSGDone(xsgdata))
    {
        teststat = false;
    }

    XSGAutoData* asg = xSGAutoSave_GetCache();
    if (success && teststat)
    {
        S32 idx = xSGTgtPhysSlotIdx(xsgdata, use_tgt);
        asg->SetCache(use_tgt, use_game, idx);
        globals.autoSaveFeature = 1;
        return 1;
    }
    asg->Discard();

    switch (whyFail)
    {
    case XSG_WHYERR_DAMAGE:
    case XSG_WHYERR_OTHER:
        return 7;
    case XSG_WHYERR_NOCARD:
    case XSG_WHYERR_CARDYANKED:
        return 8;
    default:
        return -1;
    }
}

S32 zSaveLoad_DoAutoSave()
{
    S32 success = 0;
    S32 teststat = 1;
    en_XSGASYNC_STATUS asstat = XSG_ASTAT_NOOP;
    st_XSAVEGAME_DATA* svinst;

    XSGAutoData* autodata = xSGAutoSave_GetCache();
    if (autodata == NULL)
    {
        return -1;
    }
    if (!autodata->IsValid())
    {
        return -1;
    }

    S32 use_tgt = CardtoTgt(autodata->LastTarget());
    S32 lastGame = autodata->LastGame();
    autodata->Discard();
    svinst = xSGInit(XSG_MODE_SAVE);
    xSGTgtSelect(svinst, use_tgt);
    xSGGameSet(svinst, lastGame);

    if (svinst == NULL)
    {
        teststat = 0;
    }

    zSceneSave(globals.sceneCur, 0);

    xSGAddSaveClient(svinst, 'ROOM', 0, xSGT_SaveInfoCB, xSGT_SaveProcCB);
    xSGAddSaveClient(svinst, 'PREF', 0, xSGT_SaveInfoPrefsCB, xSGT_SaveProcPrefsCB);

    xSerial_svgame_register(svinst, XSG_MODE_SAVE);
    U32 progress = zSceneCalcProgress();

    const char* area;
    if (globals.sceneCur->sceneID == 'PG12')
    {
        area = zSceneGetLevelName('HB01');
    }
    else
    {
        area = zSceneGetLevelName(globals.sceneCur->sceneID);
    }

    char label[64];
    strncpy(label, area, sizeof(label));
    if (!xSGSetup(svinst, lastGame, label, progress, 0, zSceneGetLevelIndex()))
    {
        teststat = 0;
    }

    if (teststat != 0)
    {
        S32 iprocess = xSGProcess(svinst);
        if (iprocess != 0)
        {
            asstat = xSGAsyncStatus(svinst, 1, 0, 0);
        }
        xSGGameIsEmpty(svinst, lastGame);
        xSGTgtHasGameDir(svinst, use_tgt);
        if (iprocess == 0)
        {
            teststat = false;
        }
        else
        {
            switch (asstat)
            {
            case XSG_ASTAT_INPROG:
                break;
            case XSG_ASTAT_SUCCESS:
                success = true;
                break;
            case XSG_ASTAT_FAILED:
                success = false;
                break;
            }
        }
    }

    if (teststat && xSGWrapup(svinst) == 0)
    {
        teststat = false;
    }

    if (xSGDone(svinst) == 0)
    {
        teststat = false;
    }

    if ((success) && (teststat))
    {
        if (autodata != NULL)
        {
            S32 idx = xSGTgtPhysSlotIdx(svinst, use_tgt);
            autodata->SetCache(use_tgt, lastGame, idx);
            globals.autoSaveFeature = 1;
        }
        return 1;
    }
    else
    {
        if (autodata != NULL)
        {
            autodata->Discard();
        }
        globals.autoSaveFeature = 0;
        return -1;
    }
}

void zSaveLoadAutoSaveUpdate()
{
    xBase* obj;
    s32 out1, out2;

    if (globals.autoSaveFeature == 0 || gGameMode == eGameMode_Pause)
    {
        return;
    }
    if (preAutoSaving == 0)
    {
        return;
    }

    S32 physicalSlot = xSGAutoSave_GetCache()->LastPhysicalSlot();
    if (physicalSlot >= 0)
    {
        autoSaveCard = physicalSlot;
        switch (CARDProbeEx(physicalSlot, &out1, &out2))
        {
        case 0:
        case -1:
            obj = zSceneFindObject(xStrHash("SAVING GAME ICON UI")); //"SAVING GAME ICON UI"
            if (obj != NULL)
            {
                zEntEvent(obj, eEventVisible);
            }
            break;
        default:
            obj = zSceneFindObject(xStrHash("SAVING GAME ICON UI")); //"SAVING GAME ICON UI"
            if (obj != NULL)
            {
                zEntEvent(obj, eEventInvisible);
            }

            obj = zSceneFindObject(xStrHash("MNU4 AUTO SAVE FAILED")); //"MNU4 AUTO SAVE FAILED"
            if (obj != NULL)
            {
                zEntEvent(obj, eEventVisible);
            }
            globals.autoSaveFeature = 0;
            zSaveLoadPreAutoSave(0);
            break;
        }
    }
}

void zSaveLoadPreAutoSave(bool onOff)
{
    preAutoSaving = onOff;
    if (onOff)
    {
        ResetButton::DisableReset();
    }
    else
    {
        ResetButton::EnableReset();
    }
}

S32 zSaveLoad_CardCheckFormatted(int, int);

S32 zSaveLoad_GameSelect(S32 mode)
{
    S32 done = 0;
    S32 i;
    st_XSAVEGAME_DATA* svinst;
    S32 use_tgt;

    badCard = 1;
    while (badCard != 0)
    {
        currentGame = -1;
        promptSel = -1;
        use_tgt = CardtoTgt(currentCard);

        if (mode == 1)
        {
            zSaveLoad_UIEvent(0, eEventEnable);
            zSaveLoad_UIEvent(0, eEventUIFocusOn);
            zSaveLoad_UIEvent(0x33, eEventUIFocusOn);
        }
        else
        {
            zSaveLoad_UIEvent(0x34, eEventUIFocusOn);
        }

        for (i = 0; i < 3; i++)
        {
            svinst = xSGInit(XSG_MODE_LOAD);
            xSGTgtSelect(svinst, use_tgt);
            if (xSGGameIsEmpty(svinst, i))
            {
                if (mode == 1)
                {
                    zSaveLoad_UIEvent(i + 6, eEventDisable);
                }
                strcpy(zSaveLoadGameTable[i].label, "Empty");
                strcpy(zSaveLoadGameTable[i].date, "Empty");
                zSaveLoadGameTable[i].progress = 0;
                zSaveLoadGameTable[i].size = 0;
                zSaveLoadGameTable[i].thumbIconIndex = -1;
            }
            else
            {
                strcpy(zSaveLoadGameTable[i].label, xSGGameLabel(svinst, i));
                if (strcmpi(zSaveLoadGameTable[i].label, zSceneGetLevelName('MNU3')) == 0)
                {
                    strcpy(zSaveLoadGameTable[i].label, zSceneGetLevelName('HB01'));
                }
                strcpy(zSaveLoadGameTable[i].date, xSGGameModDate(svinst, i));
                zSaveLoadGameTable[i].progress = xSGGameProgress(svinst, i);
                zSaveLoadGameTable[i].size = xSGGameSize(svinst, i);
                zSaveLoadGameTable[i].size = zSaveLoadGameTable[i].size + 0x1fff & 0xffffe000;
                zSaveLoadGameTable[i].size >>= 0xd;
                zSaveLoadGameTable[i].size += 3;

                zSendEventToThumbIcon(3);
                zSaveLoadGameTable[i].thumbIconIndex = xSGGameThumbIndex(svinst, i);
                if (strcmpi(zSaveLoadGameTable[i].label, "") == 0)
                {
                    zSaveLoadGameTable[i].thumbIconIndex = -1;
                }
            }
            xSGDone(svinst);
        }

        if (mode == 1)
        {
            for (S32 emptyCount = 6; emptyCount <= 8; emptyCount++)
            {
                if (zSaveLoad_slotIsEmpty(emptyCount - 6) == 0)
                {
                    zSaveLoad_UIEvent(emptyCount, eEventUISelect);
                    break;
                }
            }
        }
        else
        {
            zSaveLoad_UIEvent(0x15, eEventUIFocusOn);
            zSaveLoad_UIEvent(0x19, eEventUISelect);
        }
        badCard = 0;

        while (!done && promptSel == -1)
        {
            zSaveLoad_Tick();
            zSaveLoad_poll(3);
            if (promptSel == 4)
            {
                done = promptSel;
            }

            if (currentGame == -1)
            {
                continue;
            }

            done = zSaveLoad_CardCheck(currentCard, mode);
            if (done != 1)
            {
                currentGame = -1;
                badCard = 0;

                if (done == 9)
                {
                    zSaveLoad_CardWrongDeviceErrorPrompt(mode);
                    done = 9;
                }
                break;
            }

            done = zSaveLoad_CardCheckFormatted(currentCard, mode);
            if (done != 1)
            {
                switch (done)
                {
                case 5:
                    done = 0;
                    promptSel = -1;
                    continue;
                case 7:
                    zSaveLoad_CardDamagedErrorPrompt(mode);
                    promptSel = -1;
                    done = 7;
                    badCard = 0;
                    continue;
                case 2:
                    currentGame = -1;
                    badCard = 0;
                    continue;
                default:
                    continue;
                }
            }

            done = zSaveLoad_CardCheckValid(currentCard, mode);
            if (done != 1)
            {
                switch (done)
                {
                case 5:
                case 6:
                    done = 0;
                    promptSel = -1;
                    continue;
                case 2:
                    promptSel = -1;
                    currentGame = -1;
                    badCard = 0;
                    continue;
                default:
                    break;
                }
            }

            if (badCard == 0)
            {
                done = zSaveLoad_CardCheckGameSlot(currentCard, currentGame, mode);
                if (done != 1)
                {
                    switch (done)
                    {
                    case -1:
                    case 5:
                        done = 0;
                        promptSel = -1;
                        break;
                    case 2:
                        currentGame = -1;
                        break;
                    }
                }
            }
            else
            {
                done = 0;
                currentGame = -1;
            }
        }
    }

    if (mode == 1)
    {
        zSaveLoad_UIEvent(0, eEventEnable);
    }

    S32 index = 0x15;
    if (mode == 1)
    {
        index = 0;
    }
    zSaveLoad_UIEvent(index, eEventUIFocusOff_Unselect);
    return done;
}

void zSaveLoad_BuildName(char* name_txt, S32 idx)
{
    char desired[128];
    char current_name[128];

    BuildIt(desired, idx);

    S32 counter = 0;
    for (S32 i = 0; i < idx; i++)
    {
        BuildIt(current_name, i);
        if (strcmp(zSaveLoadGameTable[i].label, zSaveLoadGameTable[idx].label) == 0 &&
            strcmp(current_name, "Corrupt Save File\n\n") != 0)
        {
            counter++;
        }
    }

    if (counter > 0)
    {
        sprintf(name_txt, "%s (%d)", desired, counter);
        name_txt[0x3f] = NULL;
    }
    else
    {
        strcpy(name_txt, desired);
    }
}

S32 zSaveLoad_CardPick(S32 mode)
{
    S32 done = 0;
    S32 formatDone = 0x16;

    currentCard = -1;
    promptSel = -1;

    if (mode == 1)
    {
        formatDone = 1;
    }

    zSaveLoad_UIEvent(formatDone, eEventUIFocusOn);
    formatDone = 0x17;
    if (mode == 1)
    {
        formatDone = 4;
    }

    zSaveLoad_UIEvent(formatDone, eEventUISelect);

    while (!done && promptSel == -1)
    {
        if (currentCard != -1)
        {
            done = zSaveLoad_CardCheck(currentCard, mode);
            switch (done)
            {
            case 2:
                currentCard = -1;
                promptSel = -1;
                continue;
            case -1:
                done = 0;
                promptSel = -1;
                continue;
            case 9:
                zSaveLoad_CardWrongDeviceErrorPrompt(mode);
                done = -1;
                promptSel = -1;
                continue;
            }

            done = zSaveLoad_CardCheckFormatted(currentCard, mode);
            switch (done)
            {
            case 5:
                done = 0;
                if (promptSel == 3)
                {
                    promptSel = -1;
                    continue;
                }
                if (promptSel == 4)
                {
                    currentCard = -1;
                    return -1;
                }
                break;
            case 6:
                done = 0;
                promptSel = -1;
                continue;
            case 7:
                zSaveLoad_CardDamagedErrorPrompt(mode);
                done = -1;
                promptSel = -1;
                continue;
            case 2:
            case 4:
                currentCard = -1;
                promptSel = -1;
                continue;
            case -1:
                done = 0;
                promptSel = -1;
                continue;
            case 8:
            case 9:
            case 10:
            case 11:

            default:
                break;
            }

            done = zSaveLoad_CardCheckValid(currentCard, mode);
            switch (done)
            {
            case 5:
            case 6:
                done = 0;
                promptSel = -1;
                continue;
            case 2:
            case 4:
                promptSel = -1;
                currentGame = -1;
                break;
            case 0:
            case 1:
                break;
            case -1:
                done = 0;
                promptSel = -1;
                continue;
            case 3:
                break;
            }
        }

        zSaveLoad_Tick();
    }

    formatDone = 0x16;
    if (mode == 1)
    {
        formatDone = 1;
    }

    zSaveLoad_UIEvent(formatDone, eEventUIFocusOff_Unselect);

    if (done == 1)
    {
        return done;
    }
    if (promptSel != -1)
    {
        return promptSel;
    }
    return done;
}

S32 zSaveLoad_CardCheckSlotOverwrite(S32 cardNumber, S32 gameNumber)
{
    // TODO: Figure out what this number means.
    S32 iVar1 = zSaveLoad_CardCheckSlotOverwrite_Free(cardNumber, gameNumber);
    // NOTE (Square): I'm not sure that this is supposed to be a loop. It doesn't make
    // sense to just break at the end and the condition feels like it should be an early return
    // but this matches and I don't know how else to generate the `b` instruction at 0x18
    while (iVar1 != 1 || iVar1 == 10)
    {
        if (iVar1 == -1 || iVar1 == 10)
        {
            return iVar1;
        }

        if (IsValidName(zSaveLoadGameTable[gameNumber].label))
        {
            iVar1 = zSaveLoad_CardPromptOverwrite();
        }
        else
        {
            iVar1 = zSaveLoad_CardPromptOverwriteDamaged();
        }

        if (iVar1 == 5)
        {
            return iVar1;
        }

        if (iVar1 == 2 || iVar1 == 4)
        {
            return 2;
        }
        break;
    }

    return 1;
}

S32 zSaveLoad_CardCheckSlotEmpty(S32 cardNumber, S32 gameNumber)
{
    S32 i = zSaveLoad_CardCheckSlotEmpty_hasGame(cardNumber, gameNumber);
    while (i != 1)
    {
        i = zSaveLoad_CardCheckSlotEmpty_hasGame(cardNumber, gameNumber);
        if (i == -1 || i == 6)
        {
            return i;
        }

        i = zSaveLoad_CardPromptGameSlotEmpty();
        if (i == 2 || i == 4)
        {
            return 2;
        }
    }
    return 1;
}

S32 zSaveLoad_CardCheckGameSlot(S32 cardNumber, S32 gameNumber, S32 mode)
{
    if (mode == 1)
    {
        return zSaveLoad_CardCheckSlotEmpty(cardNumber, gameNumber);
    }
    else
    {
        return zSaveLoad_CardCheckSlotOverwrite(cardNumber, gameNumber);
    }
}

S32 zSaveLoad_CardCheckGames(S32 cardNumber, S32 mode)
{
    S32 result = zSaveLoad_CardCheckGamesSingle(cardNumber);
    while (result != 1)
    {
        badCard = 1;

        result = zSaveLoad_CardCheckGamesSingle(cardNumber);
        if (result == -1 || result == 6 || result == 5)
        {
            return result;
        }

        result = zSaveLoad_CardPromptGames(mode);
        if (result == 2 || result == 4)
        {
            return 2;
        }
    }
    return 1;
}

S32 zSaveLoad_CardCheckSpace(S32 cardNumber, S32 mode)
{
    S32 result = zSaveLoad_CardCheckSpaceSingle(cardNumber);
    while (result != 1)
    {
        badCard = 1;

        result = zSaveLoad_CardCheckSpaceSingle(cardNumber);
        if (result == -1 || result == 6 || result == 5)
        {
            return result;
        }

        result = zSaveLoad_CardPromptSpace(mode);
        if (result == 2 || result == 4)
        {
            return 2;
        }
    }
    return 1;
}

S32 zSaveLoad_CardCheckValid(S32 cardNumber, S32 mode)
{
    if (mode == 1)
    {
        return zSaveLoad_CardCheckGames(cardNumber, mode);
    }
    else
    {
        return zSaveLoad_CardCheckSpace(cardNumber, mode);
    }
}

S32 zSaveLoad_CardCheckFormatted(S32 cardNumber, S32 mode)
{
    S32 result;
    while (result = zSaveLoad_CardCheckFormattedSingle(cardNumber), result != 1)
    {
        badCard = 1;

        result = zSaveLoad_CardCheckFormattedSingle(cardNumber);
        if (result == -1)
        {
            return -1;
        }

        if (result == 7)
        {
            return 7;
        }

        result = zSaveLoad_CardPromptFormat(mode);
        if (result == 2 || result == 4)
        {
            return 2;
        }

        sAccessType = 3;
        S32 tmp = format(cardNumber, mode);
        sAccessType = 2;
        return tmp != 1 ? tmp : 11;
    }
    return 1;
}

S32 zSaveLoad_CardCheck(S32 cardNumber, S32 mode)
{
    S32 cardResult = zSaveLoad_CardCheckSingle(cardNumber);
    while (cardResult == 0)
    {
        badCard = 1;

        cardResult = zSaveLoad_CardPrompt(mode);
        if (cardResult == 2 || cardResult == 4)
        {
            return 2;
        }
        cardResult = zSaveLoad_CardCheckSingle(cardNumber);
    }
    return cardResult == 9 ? 9 : 1;
}

S32 zSaveLoad_CardCheckFormattedSingle(S32 cardNumber)
{
    S32 rc;
    st_XSAVEGAME_DATA* ldinst = xSGInit(XSG_MODE_LOAD);
    S32 tgtmax;

    rc = 0;
    switch (xSGTgtCount(ldinst, &tgtmax))
    {
    case 2:
        switch (xSGTgtIsFormat(ldinst, cardNumber, NULL))
        {
        case -1:
            rc = 7;
            break;
        case 1:
            rc = 1;
            break;
        case 0:
        default:
            rc = 0;
            break;
        }
        break;
    case 1:
        S32 idx = xSGTgtPhysSlotIdx(ldinst, 0);
        if (idx != cardNumber)
        {
            rc = -1;
        }
        else if (!(idx ^ cardNumber))
        {
            switch (xSGTgtIsFormat(ldinst, 0, NULL))
            {
            case -1:
                rc = 7;
                break;
            case 1:
                rc = 1;
                break;
            case 0:
            default:
                rc = 0;
                break;
            }
        }
        break;
    case 0:
        rc = -1;
        break;
    }

    xSGDone(ldinst);
    return rc;
}

S32 zSaveLoad_CardPromptOverwriteDamaged()
{
    zSaveLoad_UIEvent(0x15, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x23, eEventUIFocusOn_Select);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
        zSaveLoad_poll(5);
    }
    zSaveLoad_UIEvent(0x23, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPromptOverwrite()
{
    zSaveLoad_UIEvent(0x15, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x22, eEventUIFocusOn_Select);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
        zSaveLoad_poll(5);
    }
    zSaveLoad_UIEvent(0x22, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPromptGameSlotEmpty()
{
    zSaveLoad_UIEvent(0, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x13, eEventUIFocusOn_Select);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
    }
    zSaveLoad_UIEvent(0x13, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPromptGames(S32 mode)
{
    S32 i = 0x15;
    if (mode == 1)
    {
        i = 0;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(1, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x12, eEventUIFocusOn_Select);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
        zSaveLoad_poll(3);
    }
    zSaveLoad_UIEvent(0x12, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPromptSpace(S32 mode)
{
    S32 i = 0x15;
    if (mode == 1)
    {
        i = 0;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x16, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x25, eEventUIFocusOn_Select);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
        zSaveLoad_poll(3);
    }
    zSaveLoad_UIEvent(0x25, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPromptFormat(S32 mode)
{
    S32 i = 0x15;
    if (mode == 1)
    {
        i = 0;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);

    i = 0x16;
    if (mode == 1)
    {
        i = 1;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);
    zSaveLoad_UIEvent(0x32, eEventUIFocusOn);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
        zSaveLoad_poll(3);
    }
    zSaveLoad_UIEvent(0x32, eEventUIFocusOff_Unselect);
    return promptSel;
}

S32 zSaveLoad_CardPrompt(S32 cardNumber)
{
    S32 i = 0x15;
    if (cardNumber == 1)
    {
        i = 0;
    }
    zSaveLoad_UIEvent(i, eEventEnable);

    i = 0x15;
    if (cardNumber == 1)
    {
        i = 0;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);

    i = 0x24;
    if (cardNumber == 1)
    {
        i = 0x11;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOn_Select);

    i = 0x16;
    if (cardNumber == 1)
    {
        i = 1;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);

    promptSel = -1;
    while (promptSel == -1)
    {
        zSaveLoad_Tick();
    }

    i = 0x24;
    if (cardNumber == 1)
    {
        i = 0x11;
    }
    zSaveLoad_UIEvent(i, eEventUIFocusOff_Unselect);

    if (cardNumber == 1)
    {
        zSaveLoad_UIEvent(0, eEventDisable);
    }
    return promptSel;
}

S32 format(S32 num, S32 mode)
{
    st_XSAVEGAME_DATA* data = zSaveLoadSGInit(XSG_MODE_LOAD);
    S32 tgtmax;

    S32 rc = 0;
    switch (xSGTgtCount(data, &tgtmax))
    {
    case 2:
        xSGTgtSelect(data, num);
        rc = xSGTgtFormatTgt(data, num, 0);
        zSaveLoadSGDone(data);
        if (rc == -1)
        {
            zSaveLoad_ErrorFormatCardYankedPrompt(mode);
            rc = 5;
        }
        else
        {
            if (rc == 0)
            {
                zSaveLoad_ErrorFormatPrompt(mode);
            }
            if (rc != 0)
            {
                rc = 1;
            }
        }
        break;
    case 1:
        S32 idx = xSGTgtPhysSlotIdx(data, 0);
        if (idx != num)
        {
            zSaveLoadSGDone(data);
            rc = 5;
        }
        else
        {
            if (idx ^ num)
            {
                zSaveLoadSGDone(data);
            }
            else
            {
                xSGTgtSelect(data, 0);
                rc = xSGTgtFormatTgt(data, num, 0);
                zSaveLoadSGDone(data);
                if (rc == 0)
                {
                    zSaveLoad_ErrorFormatPrompt(mode);
                }
            }
        }
        break;
    case 0:
        rc = 5;
        zSaveLoadSGDone(data);
        break;
    }
    return rc;
}

void zSaveLoad_Tick(bool)
{
    // REFERENCE BFBB
}
