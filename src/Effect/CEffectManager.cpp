#include "Effect/CEffectManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h" // ���]���S�ĸ�T�޲z��
#include "Character/ClientCharacter.h"       // �������O
#include "Image/CDeviceManager.h"        // ���F���]��V���A
#include <new>

// --- �U�ب���S�����O�����Y�� ---
#include "Effect/CEffect_Skill_Type_Once.h"
#include "Effect/CEffect_Skill_Type_Directed.h"
#include "Effect/CEffect_Skill_Type_Sustain.h"
#include "Effect/CEffect_Skill_Type_ShootUnit.h"
#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffect_Item_Type_Once.h"

// ���]��������
extern cltMoFC_EffectKindInfo g_clEffectKindInfo;
extern ClientCharacterManager g_ClientCharMgr;

// �R�A��Ҫ�l��
CEffectManager* CEffectManager::s_pInstance = nullptr;

CEffectManager* CEffectManager::GetInstance() {
    if (!s_pInstance) {
        s_pInstance = new (std::nothrow) CEffectManager();
    }
    return s_pInstance;
}

// �����ϲ�Ķ�X: 0x0053AEE0
CEffectManager::CEffectManager() : m_pHead(nullptr), m_pTail(nullptr), m_uEffectCount(0)
{
    // m_OverMindScreenEffect �M m_SkillTypeOnceEffect ���غc�禡�|�b���B�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x0053AEE0
CEffectManager::~CEffectManager()
{
    BulletListAllDel();
    // m_OverMindScreenEffect �M m_SkillTypeOnceEffect ���Ѻc�禡�|�b���B�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x0053AF40
void CEffectManager::BulletAdd(CEffectBase* pEffect)
{
    if (!pEffect) return;

    EffectInfo* pNewNode = new (std::nothrow) EffectInfo();
    if (!pNewNode) return;

    pNewNode->pEffect = pEffect;
    pNewNode->pNext = nullptr;

    if (m_pHead == nullptr) { // �p�G�O�Ĥ@�Ӹ`�I
        m_pHead = pNewNode;
        m_pTail = pNewNode;
        pNewNode->pPrev = nullptr;
    }
    else { // �[�J���쵲��C����
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }
    m_uEffectCount++;
}

// �����ϲ�Ķ�X: 0x0053AFA0
CEffectBase* CEffectManager::AddEffect(unsigned short effectKindID, ClientCharacter* pCaster, ClientCharacter* pTarget, int a5, unsigned short a6, unsigned short a7, unsigned char a8)
{
    stEffectKindInfo* pKindInfo = g_clEffectKindInfo.GetEffectKindInfo(effectKindID);
    if (!pKindInfo) return nullptr;

    // �p�G�S���ɮצW�O "0"�A�B���O�S������5�A�h���Ы�
    if (strcmp(pKindInfo->szFileName, "0") == 0 && pKindInfo->ucType != 5) {
        return nullptr;
    }

    CEffectBase* pNewEffect = nullptr;

    // �ھگS�ĺ���(Type)�A�Ыع������S�Ī���
    switch (pKindInfo->ucType)
    {
    case 1: // �@���ʬI�k�̯S��
        pNewEffect = new CEffect_Skill_Type_Once();
        static_cast<CEffect_Skill_Type_Once*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->szFileName);
        break;
    case 2: // ���V�ʯS��
        pNewEffect = new CEffect_Skill_Type_Directed();
        static_cast<CEffect_Skill_Type_Directed*>(pNewEffect)->SetEffect(pCaster, pTarget, effectKindID, pKindInfo->szFileName, a8);
        break;
    case 3: // ����ʯS��
        pNewEffect = new CEffect_Skill_Type_Sustain();
        static_cast<CEffect_Skill_Type_Sustain*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->szFileName, a7);
        break;
    case 4: // ��g���S��
        pNewEffect = new CEffect_Skill_Type_ShootUnit();
        static_cast<CEffect_Skill_Type_ShootUnit*>(pNewEffect)->SetEffect(pCaster, pTarget, effectKindID, pKindInfo->szFileName, a5);
        break;
    case 5: // �S��G�j��g��
        pNewEffect = new CEffect_Battle_GunShoot();
        if (pTarget) {
            int hitedNum = pTarget->GetHitedInfoNum(pCaster->GetAccountID());
            static_cast<CEffect_Battle_GunShoot*>(pNewEffect)->SetEffect(pCaster, pTarget, a6, hitedNum);
        }
        break;
    case 6: // ���~�ϥίS��
        pNewEffect = new CEffect_Item_Type_Once();
        static_cast<CEffect_Item_Type_Once*>(pNewEffect)->SetEffect(pCaster, effectKindID, pKindInfo->szFileName);
        break;
    default:
        // ��L�������B�z
        break;
    }

    if (pNewEffect) {
        BulletAdd(pNewEffect);
    }

    return pNewEffect;
}

// �����ϲ�Ķ�X: 0x0053B280
CEffectBase* CEffectManager::AddEffect(char* szEffectName, ClientCharacter* pCaster)
{
    unsigned short kindID = g_clEffectKindInfo.TranslateKindCode(szEffectName);
    return AddEffect(kindID, pCaster, nullptr, 0, 0, 0, 2);
}

// �����ϲ�Ķ�X: 0x0053B5A0
void CEffectManager::FrameProcess(float fElapsedTime, bool bForceDeleteAll)
{
    EffectInfo* pCurrent = m_pHead;
    while (pCurrent)
    {
        EffectInfo* pNext = pCurrent->pNext; // ���O�s�U�@�Ӹ`�I
        bool isFinished = pCurrent->pEffect->FrameProcess(fElapsedTime);

        if (isFinished || bForceDeleteAll)
        {
            // --- �q�쵲��C�������`�I ---
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

            // --- �R���S�Ī���M�`�I���� ---
            if (pCurrent->pEffect) {
                delete pCurrent->pEffect;
            }
            delete pCurrent;
            m_uEffectCount--;
        }
        pCurrent = pNext; // ���ʨ�U�@�Ӹ`�I
    }

    // m_OverMindScreenEffect.Poll(); // ��l�X�����I�s�A�������O�L���禡
}

// �����ϲ�Ķ�X: 0x0053B6B0
void CEffectManager::Process()
{
    for (EffectInfo* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (pNode->pEffect) {
            pNode->pEffect->Process();
        }
    }
    m_OverMindScreenEffect.PrepareDrawing();
}

// �����ϲ�Ķ�X: 0x0053B6E0
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

// �����ϲ�Ķ�X: 0x0053B730
bool CEffectManager::DeleteEffect(CEffectBase* pEffect)
{
    if (!pEffect) return false;

    for (EffectInfo* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (pNode->pEffect == pEffect) {
            // --- �޿�P FrameProcess �����R�����������ۦP ---
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

// �����ϲ�Ķ�X: 0x0053B820
void CEffectManager::AddEtcEffect(unsigned short type, unsigned int accountID)
{
    ClientCharacter* pChar = g_ClientCharMgr.GetCharByAccount(accountID);
    if (pChar) {
        if (type == 0) { // ���] 0 �N�� OverMind �ĪG
            m_OverMindScreenEffect.SetActive(pChar);
        }
    }
}

// �����ϲ�Ķ�X: 0x0053B550
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