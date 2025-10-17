#include "zCutsceneMgr.h"

static S32 gCutsceneSkipOK;
static S32 donpcfx;
static U8 ents_hidden;
static S32 s_atomicNumber;

void zCutsceneMgrUpdate(xBase* to, xScene* sc, F32 dt)
{
    zCutsceneMgr* t = (zCutsceneMgr*)to;
    if (t->csn->ShutDownWait)
    {
        if (donpcfx)
        {
            zNPCFXCutsceneDone(sc, dt, t);
        }
        donpcfx = 0;
        if (xCutscene_Destroy(t->csn))
        {
            zCutsceneMgrKillFX(t);
            t->csn = NULL;
            globals.cmgr = NULL;
            xCameraSetFOV(&globals.oldSkoolCamera, t->oldfov);
        }
    }
    else
    {
        if (t->csn->Ready)
        {
            if (!donpcfx)
            {
                donpcfx = zNPCFXCutscenePrep(sc, dt, t);
            }
            if (donpcfx)
            {
                zNPCFXCutscene(sc, dt, t);
            }

            zCutsceneMgrUpdateFX(t);
            if (t->stop || !xCutscene_Update(t->csn, dt))
            {
                if (!t->stop)
                {
                    zEntEvent(to, to, 0x13);
                }

                t->csn->PlaybackSpeed = 0.0f;
                //zEntEvent(&globals.player.ent, 9);
                //zEntEvent(&globals.player.ent, 3);
                zEntEvent(to, to, 0xbf);
                iCameraSetNearFarClip(0.0f, 0.0f);

                if ((globals.sceneCur->pendingPortal))
                {
                    //zEntEvent(&globals.player.ent, 8);
                    //zEntEvent(&globals.player.ent, 4);
                    xSndStopAll(0xffff);
                    return;
                }

                if (donpcfx)
                {
                    zNPCFXCutsceneDone(sc, dt, t);
                }
                donpcfx = 0;

                if (xCutscene_Destroy(t->csn))
                {
                    zCutsceneMgrKillFX(t);
                    t->csn = NULL;
                    globals.cmgr = NULL;
                    xCameraSetFOV(&globals.oldSkoolCamera, t->oldfov);
                }

                zMusicUnpause(0);
            }
            else
            {
                if (gCutsceneSkipOK && t->csn->Time > gSkipTimeCutscene &&
                    globals.pad0->pressed & 0x50000)
                {
                    zEntEvent(to, to, 0x13);
                }
            }
        }
        else
        {
            iCSLoadStep(t->csn);
            if (t->csn->Ready)
            {
                zCutsceneMgrPlayStart(t);
                zEntEvent(to, to, 0x18);
                //zEntEvent(&globals.player.ent, 8);
                //zEntEvent(&globals.player.ent, 4);
                donpcfx = zNPCFXCutscenePrep(sc, dt, t);
            }
        }
    }
    check_hide_entities();
}

void zCutsceneMgrFinishExit(xBase* to)
{
    zCutsceneMgr* t = (zCutsceneMgr*)to;

    while (1)
    {
        iFileAsyncService();
        xSndUpdate();
        if (t->csn->Waiting == 0)
            break;
        iVSync();
    }
    if (donpcfx != 0)
    {
        zNPCFXCutsceneDone(globals.sceneCur, 0.0f, t);
    }
    donpcfx = 0;
    if (xCutscene_Destroy(t->csn) == 0)
    {
        return;
    }

    //zEntEvent(&globals.player.ent, 9);
    //zEntEvent(&globals.player.ent, 3);
    zCutsceneMgrKillFX(t);
    t->csn = NULL;
    globals.cmgr = NULL;
}

void zCutsceneMgrFinishLoad(xBase* to)
{
    zCutsceneMgr* t = (zCutsceneMgr*)to;

    while (true)
    {
        iFileAsyncService();
        xSndUpdate();
        iCSLoadStep(t->csn);
        iTRCDisk::CheckDVDAndResetState();
        if (t->csn->Ready)
        {
            break;
        }
        iVSync();
    }
    zCutsceneMgrPlayStart(t);
    zEntEvent(to, to, 0x18);
    //zEntEvent(&globals.player.ent, 8);
    //zEntEvent(&globals.player.ent, 4);
    return;
}

void zCutsceneMgrLoad(zCutsceneMgr*, xSerial* s)
{
}

void zCutsceneMgrSave(zCutsceneMgr*, xSerial* s)
{
}
