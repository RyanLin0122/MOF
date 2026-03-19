#include "Object/CNPCObject.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Object/cltNPC_Object.h"
#include "Object/cltNPC_Taxi.h"
#include "Object/cltQuestMark.h"
#include "Info/cltNPCInfo.h"
#include "Logic/cltNPCManager.h"
#include "global.h"
#include <cstring>

extern cltNPCInfo g_clNPCInfo;
extern cltNPCManager g_clNPCManager;
extern CObjectManager g_ObjectManager;

CNPCObject g_clNPCObject;

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
// Ground truth: 根據 stNPCInfo 判斷是否為 Taxi NPC
// -------------------------------------------------------------------------
void CNPCObject::AddNPC(std::uint16_t npcID, unsigned int resourceID, std::uint16_t maxFrames,
                         float posX, float posY,
                         std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                         int flipFlag, int toggleFlag)
{
    stNPCInfo* pInfo = g_clNPCInfo.GetNPCInfoByID(npcID);
    if (!pInfo)
        return;

    CBaseObject* pNPC = nullptr;

    // Ground truth: 檢查 NPC 是否為計程車類型
    // byte +105 == 1 (在 _reserved 區域) 或 WORD +1 == 26759
    unsigned char byteAt105 = *((unsigned char*)pInfo + 105);
    std::uint16_t wordAt1 = ((std::uint16_t*)pInfo)[1];

    if (byteAt105 == 1 || wordAt1 == 26759)
    {
        // Taxi NPC — 使用 cltNPC_Taxi
        cltNPC_Taxi* pTaxi = new cltNPC_Taxi();
        // Taxi 的 Initialize 參數組合不同: 沒有 maxFrames 和 toggleFlag
        std::uint16_t* chatTextIDs = (std::uint16_t*)((char*)pInfo + 40);
        pTaxi->Initialize(posX, posY, nameTextCode, titleTextCode,
                          resourceID, flipFlag, chatTextIDs);
        pNPC = pTaxi;
    }
    else
    {
        // 一般 NPC — 使用 cltNPC_Object
        cltNPC_Object* pObj = new cltNPC_Object();
        std::uint16_t* chatTextIDs = (std::uint16_t*)((char*)pInfo + 40);
        pObj->Initialize(posX, posY, nameTextCode, titleTextCode,
                         resourceID, maxFrames, flipFlag, toggleFlag,
                         chatTextIDs);
        pNPC = pObj;
    }

    g_ObjectManager.AddObject(pNPC);
}

// -------------------------------------------------------------------------
// AddQuestMark — 為 NPC 加入任務標記
// Ground truth: 建立 cltQuestMark 物件，取得 NPC 座標，加入 ObjectManager
// -------------------------------------------------------------------------
void CNPCObject::AddQuestMark(std::uint16_t npcID)
{
    // 檢查是否已存在相同 NPC 的任務標記 (type 101)
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 101)
            return;  // 已存在，不重複新增
    }

    // 建立 cltQuestMark 物件
    cltQuestMark* pMark = new cltQuestMark();
    float posY = (float)g_clNPCManager.GetPosYByID(npcID);
    float posX = (float)g_clNPCManager.GetPosXByID(npcID);
    pMark->Initialize(npcID, posX, posY, 0x65u);
    g_ObjectManager.AddObject(pMark);
}

// -------------------------------------------------------------------------
// AddRewardMark — 為 NPC 加入獎勵標記
// Ground truth: 建立 cltRewardMark 物件，取得 NPC 座標，加入 ObjectManager
// 注意：ground truth 的重複檢查也是用 type 101 (非 102)
// -------------------------------------------------------------------------
void CNPCObject::AddRewardMark(std::uint16_t npcID)
{
    // Ground truth: 重複檢查使用 type 101 (與 AddQuestMark 相同)
    int count = g_ObjectManager.GetObjectCount();
    for (int i = 0; i < count; ++i)
    {
        CBaseObject* pObj = g_ObjectManager.GetObjectA((unsigned short)i);
        if (pObj && pObj->m_siField4 == npcID && pObj->m_siField5 == 101)
            return;  // 已存在
    }

    // 建立 cltRewardMark 物件
    cltRewardMark* pMark = new cltRewardMark();
    float posY = (float)g_clNPCManager.GetPosYByID(npcID);
    float posX = (float)g_clNPCManager.GetPosXByID(npcID);
    pMark->Initialize(npcID, posX, posY, 0x66u);
    g_ObjectManager.AddObject(pMark);
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
            pObj->m_dwField3 = 0;
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
            pObj->m_dwField3 = 0;
            g_ObjectManager.DelObject(i);
            return;
        }
    }
}
