#include "Effect/CEffectManager.h"
#include "Info/cltMoFC_EffectKindInfo.h" // 假設的特效資訊管理器
#include "Character/ClientCharacter.h"       // 角色類別
#include "Image/CDeviceManager.h"        // 為了重設渲染狀態
#include <new>

// --- 各種具體特效類別的標頭檔 ---
#include "Effect/CEffect_Skill_Type_Once.h"
#include "Effect/CEffect_Skill_Type_Directed.h"
#include "Effect/CEffect_Skill_Type_Sustain.h"
#include "Effect/CEffect_Skill_Type_ShootUnit.h"
#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffect_Item_Type_Once.h"
#include "Character/ClientCharacterManager.h"


// 靜態實例初始化
CEffectManager* CEffectManager::s_pInstance = nullptr;

CEffectManager* CEffectManager::GetInstance() {
    if (!s_pInstance) {
        s_pInstance = new (std::nothrow) CEffectManager();
    }
    return s_pInstance;
}

// 對應反組譯碼: 0x0053AEE0
CEffectManager::CEffectManager() : m_pHead(nullptr), m_pTail(nullptr), m_uEffectCount(0)
{
    // m_OverMindScreenEffect & cltMoFC_EffectKindInfo 和 m_SkillTypeOnceEffect 的建構函式會在此處自動呼叫
	//g_clEffectKindInfo->Initialize("Effect/EffectKindInfo.txt");
}

// 對應反組譯碼: 0x0053AEE0
CEffectManager::~CEffectManager()
{
    BulletListAllDel();
    // m_OverMindScreenEffect 和 m_SkillTypeOnceEffect 的解構函式會在此處自動呼叫
}

// 對應反組譯碼: 0x0053AF40
void CEffectManager::BulletAdd(CEffectBase* pEffect)
{
    if (!pEffect) return;

    EffectInfo* pNewNode = new (std::nothrow) EffectInfo();
    if (!pNewNode) return;

    pNewNode->pEffect = pEffect;
    pNewNode->pNext = nullptr;

    if (m_pHead == nullptr) { // 如果是第一個節點
        m_pHead = pNewNode;
        m_pTail = pNewNode;
        pNewNode->pPrev = nullptr;
    }
    else { // 加入到鏈結串列尾端
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }
    m_uEffectCount++;
}

// 對應反組譯碼: 0x0053AFA0
CEffectBase* CEffectManager::AddEffect(unsigned short effectKindID, ClientCharacter* pCaster, ClientCharacter* pTarget, int a5, unsigned short a6, unsigned short a7, unsigned char a8)
{
    stEffectKindInfo* pKindInfo = this->g_clEffectKindInfo.GetEffectKindInfo(effectKindID);
    if (!pKindInfo) return nullptr;

    // 如果特效檔案名是 "0"，且不是特殊類型5，則不創建
    if (strcmp(pKindInfo->eaFile, "0") == 0 && pKindInfo->skillType != 5) {
        return nullptr;
    }

    CEffectBase* pNewEffect = nullptr;

    // 根據特效種類(Type)，創建對應的特效物件
    switch (pKindInfo->skillType)
    {
    case 1: // 一次性施法者特效
        pNewEffect = new CEffect_Skill_Type_Once();
        static_cast<CEffect_Skill_Type_Once*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->eaFile);
        break;
    case 2: // 指向性特效
        pNewEffect = new CEffect_Skill_Type_Directed();
        static_cast<CEffect_Skill_Type_Directed*>(pNewEffect)->SetEffect(pCaster, pTarget, effectKindID, pKindInfo->eaFile, a8);
        break;
    case 3: // 持續性特效
        pNewEffect = new CEffect_Skill_Type_Sustain();
        static_cast<CEffect_Skill_Type_Sustain*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->eaFile, a7);
        break;
    case 4: // 投射物特效
        pNewEffect = new CEffect_Skill_Type_ShootUnit();
        static_cast<CEffect_Skill_Type_ShootUnit*>(pNewEffect)->SetEffect(pCaster, pTarget, effectKindID, pKindInfo->eaFile, a5);
        break;
    case 5: // 特殊：槍手射擊
        pNewEffect = new CEffect_Battle_GunShoot();
        if (pTarget) {
            int hitedNum = pTarget->GetHitedInfoNum(pCaster->GetAccountID());
            static_cast<CEffect_Battle_GunShoot*>(pNewEffect)->SetEffect(pCaster, pTarget, a6, hitedNum);
        }
        break;
    case 6: // 物品使用特效
        pNewEffect = new CEffect_Item_Type_Once();
        static_cast<CEffect_Item_Type_Once*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->eaFile);
        break;
    default:
        // 其他類型不處理
        break;
    }

    if (pNewEffect) {
        BulletAdd(pNewEffect);
    }

    return pNewEffect;
}

// 對應反組譯碼: 0x0053B280
CEffectBase* CEffectManager::AddEffect(char* szEffectName, ClientCharacter* pCaster)
{
    unsigned short kindID = this->g_clEffectKindInfo.TranslateKindCode(szEffectName);
    return AddEffect(kindID, pCaster, nullptr, 0, 0, 0, 2);
}

// 對應反組譯碼: 0x0053B5A0
void CEffectManager::FrameProcess(float fElapsedTime, bool bForceDeleteAll)
{
    EffectInfo* pCurrent = m_pHead;
    while (pCurrent)
    {
        EffectInfo* pNext = pCurrent->pNext; // 先保存下一個節點
        bool isFinished = pCurrent->pEffect->FrameProcess(fElapsedTime);

        if (isFinished || bForceDeleteAll)
        {
            // --- 從鏈結串列中移除節點 ---
            if (pCurrent->pPrev) {
                pCurrent->pPrev->pNext = pCurrent->pNext;
            }
            else {
                m_pHead = pCurrent->pNext;
            }

            if (pCurrent->pNext) {
                pCurrent->pNext->pPrev = pCurrent->pPrev;
            }
            else {
                m_pTail = pCurrent->pPrev;
            }

            // --- 刪除特效物件和節點本身 ---
            if (pCurrent->pEffect) {
                delete pCurrent->pEffect;
            }
            delete pCurrent;
            m_uEffectCount--;
        }
        pCurrent = pNext; // 移動到下一個節點
    }

    // m_OverMindScreenEffect.Poll(); // 原始碼有此呼叫，但該類別無此函式
}

// 對應反組譯碼: 0x0053B6B0
void CEffectManager::Process()
{
    for (EffectInfo* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (pNode->pEffect) {
            pNode->pEffect->Process();
        }
    }
    m_OverMindScreenEffect.PrepareDrawing();
}

// 對應反組譯碼: 0x0053B6E0
void CEffectManager::Draw()
{
    if (m_uEffectCount > 0) {
        for (EffectInfo* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
            if (pNode->pEffect) {
                pNode->pEffect->Draw();
            }
        }
        CDeviceManager::GetInstance()->ResetRenderState();
    }
    m_OverMindScreenEffect.Draw();
}

// 對應反組譯碼: 0x0053B730
bool CEffectManager::DeleteEffect(CEffectBase* pEffect)
{
    if (!pEffect) return false;

    for (EffectInfo* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (pNode->pEffect == pEffect) {
            // --- 邏輯與 FrameProcess 中的刪除部分完全相同 ---
            if (pNode->pPrev) pNode->pPrev->pNext = pNode->pNext;
            else m_pHead = pNode->pNext;

            if (pNode->pNext) pNode->pNext->pPrev = pNode->pPrev;
            else m_pTail = pNode->pPrev;

            delete pNode->pEffect;
            delete pNode;
            m_uEffectCount--;
            return true;
        }
    }
    return false;
}

// 對應反組譯碼: 0x0053B820
void CEffectManager::AddEtcEffect(unsigned short type, unsigned int accountID)
{
    ClientCharacter* pChar = ClientCharacterManager::GetInstance()->GetCharByAccount(accountID);
    if (pChar) {
        if (type == 0) { // 假設 0 代表 OverMind 效果
            m_OverMindScreenEffect.SetActive(pChar);
        }
    }
}

// 對應反組譯碼: 0x0053B550
void CEffectManager::BulletListAllDel()
{
    EffectInfo* pCurrent = m_pHead;
    while (pCurrent) {
        EffectInfo* pToDelete = pCurrent;
        pCurrent = pCurrent->pNext;

        if (pToDelete->pEffect) {
            delete pToDelete->pEffect;
        }
        delete pToDelete;
    }
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_uEffectCount = 0;
}