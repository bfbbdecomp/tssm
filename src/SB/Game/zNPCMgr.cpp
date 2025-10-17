#include "zNPCMgr.h"

struct NPCMTypeTable //     ported from bfbb
{
    en_npctyp useNPCType;
    char* modelName;
    unsigned int hashOfName;
} g_tbltype[250];

S32 zNPCMgr_OrdComp_npcid(void* vkey, void* vitem)
{
    S32 rc;
    U32 item;
    U32 key;

    key = *(U32*)vkey;
    item = *(U32*)vitem;
    if (key < item)
    {
        rc = -1;
    }
    else if (key > item)
    {
        rc = 1;
    }
    else
    {
        rc = 0;
    }

    return rc;
}

// S32 zNPCMgr_OrdTest_npcid(void* vkey, void* vitem)
// {
//     S32 rc;
//     void* key = *(void**)(vitem);

//     if (vkey < key)
//     {
//         rc = -1;
//     }
//     else if (vkey > key)
//     {
//         rc = 1;
//     }
//     else
//     {
//         rc = 0;
//     }

//     return rc;
// }

void zNPCMgr::Shutdown()
{
    if (this->npcFactory != NULL)
    {
        delete this->npcFactory;
    }
    this->npcFactory = NULL;
}

void zNPCMgr_scenePostInit()
{
}

void zNPCMgr_sceneReset()
{
}

void zNPCMgr_sceneFinish()
{
}

void zNPCMgr_scenePrepare(S32 npccnt) // npccnt may not be right. code was ported
{
}

void zNPCMgr_Shutdown()
{
    g_modinit--;
    if (g_modinit == 0)
    {
        zNPCMgr* mgr = zNPCMgrSelf();
        if (mgr != NULL)
        {
            mgr->Shutdown();
            delete mgr;
            g_npcmgr = NULL;
        }
        zNPCMgr_Shutdown();
        xBehaveMgr_Shutdown();
    }
}

void zNPCMgr_Startup()
{
    if (g_modinit++ == 0)
    {
        xBehaveMgr_Startup();
        zNPCMgr* npc = new ('NPCM', NULL) zNPCMgr();
        g_npcmgr = npc;
        npc->Startup();
    }
}

zNPCMgr* zNPCMgr_GetNPCList()
{
    return g_npcmgr;
}

zNPCMgr* zNPCMgrSelf()
{
    return g_npcmgr;
}
