#include "Object/CNPCObject.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Object/cltNPC_Object.h"
#include "Info/cltNPCInfo.h"
#include "Logic/cltNPCManager.h"
#include "global.h"
#include <cstring>

extern cltNPCInfo g_clNPCInfo;
extern cltNPCManager g_clNPCManager;
extern CObjectManager g_ObjectManager;

CNPCObject g_clNPCObject;

// -------------------------------------------------------------------------
// 前向宣告 — cltNPC_Taxi (計程車 NPC，尚未還原)
// -------------------------------------------------------------------------
// cltNPC_Taxi 是 cltNPC_Object 的衍生類別，當 NPC 為計程車類型時使用。
// 目前尚未還原，AddNPC 中走一般 cltNPC_Object 路徑。
// 當 cltNPC_Taxi 被還原後，取消下方的條件判斷即可。

// 前向宣告 — cltQuestMark / cltRewardMark (任務/獎勵標記物件，尚未還原)
// 這兩個類別繼承自 CBaseObject，分別顯示任務和獎勵的標記圖示。
// AddQuestMark / AddRewardMark 會建立這些物件並加入到 ObjectManager。
// 由於目前缺少完整定義，相關功能暫時以 stub 實作。

// -------------------------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------------------------
CNPCObject::CNPCObject()
{
}

CNPCObject::~CNPCObject()
{
}

// -------------------------------------------------------------------------
// AddNPC — 建立 NPC 物件並加入到 ObjectManager
// -------------------------------------------------------------------------
void CNPCObject::AddNPC(std::uint16_t npcID, unsigned int resourceID, std::uint16_t maxFrames,
                         float posX, float posY,
                         std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                         int flipFlag, int toggleFlag)
{
    stNPCInfo* pInfo = g_clNPCInfo.GetNPCInfoByID(npcID);
    if (!pInfo)
        return;

    // Ground truth: 檢查 NPC 是否為計程車類型
    // if (pInfo 的 byte+105 == 1 || pInfo 的 WORD+1 == 26759)
    //   建立 cltNPC_Taxi
    // else
    //   建立 cltNPC_Object

    // 目前統一建立 cltNPC_Object
    cltNPC_Object* pNPC = new cltNPC_Object();
    // chatTextIDs = (unsigned __int16*)pInfo + 20 = byte offset 40 from stNPCInfo
    std::uint16_t* chatTextIDs = (std::uint16_t*)((char*)pInfo + 40);
    pNPC->Initialize(posX, posY, nameTextCode, titleTextCode,
                     resourceID, maxFrames, flipFlag, toggleFlag,
                     chatTextIDs);

    g_ObjectManager.AddObject(pNPC);
}

// -------------------------------------------------------------------------
// AddQuestMark — 為 NPC 加入任務標記
// -------------------------------------------------------------------------
void CNPCObject::AddQuestMark(std::uint16_t npcID)
{
    // 檢查是否已存在相同 NPC 的任務標記
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 101)
            return;  // 已存在，不重複新增
    }

    // TODO: 建立 cltQuestMark 物件
    // 當 cltQuestMark 被還原後實作：
    // cltQuestMark* pMark = new cltQuestMark();
    // float posY = (float)cltNPCManager::GetPosYByID(&g_clNPCManager, npcID);
    // float posX = (float)cltNPCManager::GetPosXByID(&g_clNPCManager, npcID);
    // pMark->Initialize(npcID, posX, posY, 0x65);
    // g_ObjectManager.AddObject(pMark);
}

// -------------------------------------------------------------------------
// AddRewardMark — 為 NPC 加入獎勵標記
// -------------------------------------------------------------------------
void CNPCObject::AddRewardMark(std::uint16_t npcID)
{
    // 檢查是否已存在相同 NPC 的獎勵標記
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 101)
            return;  // 已存在
    }

    // TODO: 建立 cltRewardMark 物件
    // 當 cltRewardMark 被還原後實作：
    // cltRewardMark* pMark = new cltRewardMark();
    // float posY = (float)cltNPCManager::GetPosYByID(&g_clNPCManager, npcID);
    // float posX = (float)cltNPCManager::GetPosXByID(&g_clNPCManager, npcID);
    // pMark->Initialize(npcID, posX, posY, 0x66);
    // g_ObjectManager.AddObject(pMark);
}

// -------------------------------------------------------------------------
// DelQuestMark — 刪除 NPC 的任務標記
// -------------------------------------------------------------------------
void CNPCObject::DelQuestMark(std::uint16_t npcID)
{
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 101)
        {
            pObj->m_dwField3 = 0;  // DWORD offset 6 = 0
            g_ObjectManager.DelObject(i);
            return;
        }
    }
}

// -------------------------------------------------------------------------
// DelRewardMark — 刪除 NPC 的獎勵標記
// -------------------------------------------------------------------------
void CNPCObject::DelRewardMark(std::uint16_t npcID)
{
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 102)
        {
            pObj->m_dwField3 = 0;  // DWORD offset 6 = 0
            g_ObjectManager.DelObject(i);
            return;
        }
    }
}
