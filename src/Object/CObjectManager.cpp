#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Object/CStaticObject.h"
#include "Object/CEffectObject.h"
#include "Object/CObjectResourceManager.h"
#include "Info/cltItemKindInfo.h"
#include "Text/cltTextFileManager.h"
#include "Character/ClientCharacter.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <windows.h>


CObjectManager::CObjectManager()
    : m_pHead(nullptr)
    , m_pTail(nullptr)
    , m_wObjectCount(0)
    , _pad1(0)
    , m_pCharacter(nullptr)
    , m_pEffectObjectInfos(nullptr)
    , m_wEffectObjectCount(0)
{
}

CObjectManager::~CObjectManager()
{
    DelAllObject();
}

void CObjectManager::AddObject(CBaseObject* pObject)
{
    ObjectNode* pNode = new ObjectNode();
    pNode->pPrev = nullptr;
    pNode->pNext = nullptr;
    pNode->pObject = pObject;

    if (m_pHead || m_pTail)
    {
        // Append to the end of the list
        m_pTail->pNext = pNode;
        pNode->pPrev = m_pTail;
        ++m_wObjectCount;
        m_pTail = pNode;
    }
    else
    {
        // First node
        ++m_wObjectCount;
        m_pTail = pNode;
        m_pHead = pNode;
    }
}

CBaseObject* CObjectManager::AddObject(unsigned short wObjectID)
{
    ObjectNode* pNode = new ObjectNode();
    pNode->pPrev = nullptr;
    pNode->pNext = nullptr;
    pNode->pObject = nullptr;

    stObjectInfo* pInfo = g_ObjectResourceManager.GetObjectInfoByID(wObjectID);
    unsigned short wKind = pInfo->m_wKind;

    if (wKind == 0)
    {
        // Static object (kind 0)
        pNode->pObject = new CStaticObject(wObjectID);
    }
    else if (wKind == 1 || wKind == 2)
    {
        // Kind 1 or 2: node is added but pObject remains null
        // (original binary behavior - these kinds are placeholders)
    }
    else
    {
        // Invalid object kind (>= 3)
        char szBuffer[100];
        sprintf(szBuffer, "%d Object kind is incorrect.", (unsigned short)pInfo->m_wKind);
        MessageBoxA(0, szBuffer, "Object Kind Error", 0);
        delete pNode;
        return nullptr;
    }

    // Link into the list
    if (m_pHead || m_pTail)
    {
        m_pTail->pNext = pNode;
        pNode->pPrev = m_pTail;
        m_pTail = pNode;
    }
    else
    {
        m_pTail = pNode;
        m_pHead = pNode;
    }
    ++m_wObjectCount;

    return pNode->pObject;
}

void CObjectManager::AddEffectObject(unsigned short wObjectKind, ClientCharacter* pCharacter)
{
    int offset = 0;

    for (int i = 0; i < m_wEffectObjectCount; ++i)
    {
        EffectObjectInfo* pInfo = (EffectObjectInfo*)((char*)m_pEffectObjectInfos + offset);

        if (pInfo->m_wObjectKind == wObjectKind)
        {
            CEffectObject* pEffect = new CEffectObject();
            CBaseObject* pObj = pEffect;

            if (pEffect->InitEffectObject(
                    pInfo->m_wEffectKind,
                    pInfo->m_wPlacementType,
                    wObjectKind,
                    pCharacter,
                    pInfo->m_nOffsetX,
                    pInfo->m_nOffsetY))
            {
                AddObject(pObj);
            }
        }

        offset += 16;
    }
}

void CObjectManager::DelObject(int nIndex)
{
    ObjectNode* pHead = m_pHead;
    ObjectNode* pNode = pHead;

    // Walk to the node at nIndex
    for (int i = 0; i < nIndex; ++i)
    {
        pNode = pNode->pNext;
    }

    if (pNode == pHead && pNode == m_pTail)
    {
        // Only node in the list
        if (pNode)
        {
            if (pNode->pObject)
            {
                delete pNode->pObject;
                pNode->pObject = nullptr;
            }
            delete pNode;
        }
        --m_wObjectCount;
        m_pTail = nullptr;
        m_pHead = nullptr;
        return;
    }

    if (pNode == pHead)
    {
        // Removing the head node
        pNode->pNext->pPrev = nullptr;
        m_pHead = pNode->pNext;
    }
    else if (pNode == m_pTail)
    {
        // Removing the tail node
        pNode->pPrev->pNext = nullptr;
        m_pTail = pNode->pPrev;
    }
    else
    {
        // Removing a middle node
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }

    if (pNode)
    {
        if (pNode->pObject)
        {
            delete pNode->pObject;
            pNode->pObject = nullptr;
        }
        delete pNode;
    }
    --m_wObjectCount;
}

void CObjectManager::DelAllObject()
{
    ObjectNode* pCurrent = m_pHead;
    while (pCurrent)
    {
        ObjectNode* pNext = pCurrent->pNext;
        if (pCurrent->pObject)
        {
            delete pCurrent->pObject;
            pCurrent->pObject = nullptr;
        }
        delete pCurrent;
        pCurrent = pNext;
    }
    m_pTail = nullptr;
    m_pHead = nullptr;
    m_wObjectCount = 0;
}

void CObjectManager::SwapObject(int nIndex1, int nIndex2)
{
    ObjectNode* pNode1 = m_pHead;
    for (int i = 0; i < nIndex1; ++i)
        pNode1 = pNode1->pNext;

    ObjectNode* pNode2 = m_pHead;
    for (int i = 0; i < nIndex2; ++i)
        pNode2 = pNode2->pNext;

    // Swap the object pointers
    CBaseObject* pTemp = pNode1->pObject;
    pNode1->pObject = pNode2->pObject;
    pNode2->pObject = pTemp;
}

CBaseObject* CObjectManager::GetObjectA(unsigned short wIndex)
{
    ObjectNode* pNode = m_pHead;
    for (int i = 0; i < wIndex; ++i)
        pNode = pNode->pNext;
    return pNode->pObject;
}

unsigned short CObjectManager::GetObjectCount()
{
    return m_wObjectCount;
}

unsigned short CObjectManager::GetType(unsigned short wIndex)
{
    CBaseObject* pObj = GetObjectA(wIndex);
    return pObj->m_siType;
}

int CObjectManager::ColloisionObject(CBaseObject* pObject, POINT pt)
{
    if (pObject)
    {
        if (pObject->m_dwValid)
        {
            int posX = pObject->m_nPosX;
            int width = pObject->m_nWidth;
            pObject->m_byAlpha = 0xFF; // reset alpha

            RECT rc;
            rc.left = posX - width / 2 + 20;
            rc.right = posX + width / 2 - 20;
            rc.bottom = pObject->m_nPosY;
            rc.top = pObject->m_nPosY - pObject->m_nHeight;

            if (PtInRect(&rc, pt))
                pObject->m_byAlpha = 0xAA; // highlighted
        }
    }
    return -1;
}

CBaseObject* CObjectManager::ColloisionObjectKind(unsigned char byKind)
{
    return nullptr;
}

CBaseObject* CObjectManager::ColloisionObject(unsigned short wObjectID)
{
    return nullptr;
}

void CObjectManager::Process()
{
    for (ObjectNode* pNode = m_pHead; pNode; pNode = pNode->pNext)
    {
        CBaseObject* pObj = pNode->pObject;
        if (pObj)
            pObj->Process();
    }
}

void CObjectManager::FrameProcess(float fElapsedTime)
{
    for (ObjectNode* pNode = m_pHead; pNode; pNode = pNode->pNext)
    {
        CBaseObject* pObj = pNode->pObject;
        if (pObj)
        {
            unsigned short type = pObj->m_siType;
            if (type != 0 && type != 3)
                pObj->FrameProcess(fElapsedTime);
        }
    }
}

void CObjectManager::Draw()
{
    int count = 0;
    ObjectNode* pNode = m_pHead;
    while (count < m_wObjectCount)
    {
        if (pNode)
        {
            CBaseObject* pObj = pNode->pObject;
            if (pObj)
            {
                pObj->Draw();
                pNode = pNode->pNext;
            }
        }
        ++count;
    }
}

void CObjectManager::SetCharacter(ClientCharacter* pCharacter)
{
    m_pCharacter = pCharacter;
}

void CObjectManager::Poll()
{
    int count = 0;
    ObjectNode* pNode = m_pHead;

    // Get character position for collision
    // In decompiled code: v5 = *(_DWORD *)(v3 + 556); v6 = *(_DWORD *)(v3 + 560);
    // These are the character's pixel X and Y coordinates
    unsigned int charPosX = *(unsigned int*)((char*)m_pCharacter + 556);
    unsigned int charPosY = *(unsigned int*)((char*)m_pCharacter + 560);

    POINT pt;
    pt.x = (LONG)charPosX;
    pt.y = (LONG)charPosY;

    while (count < m_wObjectCount)
    {
        CBaseObject* pObj = pNode->pObject;

        // Call virtual Poll on the object (vtable[4])
        pObj->Poll();

        ColloisionObject(pObj, pt);

        pNode = pNode->pNext;
        ++count;
    }
}

int CObjectManager::InitMapEffectObject(char* szFileName)
{
    char szBuffer[1024];
    memset(szBuffer, 0, sizeof(szBuffer));
    char szDelimiter[] = "\t\n";
    int result = 0;

    FILE* fp = g_clTextFileManager.fopen(szFileName);
    if (!fp)
    {
        MessageBoxA(0, szFileName, "Map Effect Object List Error", 0);
        return 0;
    }

    // Skip 3 header lines
    fgets(szBuffer, 1023, fp);
    fgets(szBuffer, 1023, fp);
    fgets(szBuffer, 1023, fp);

    // Count data lines
    fpos_t position;
    fgetpos(fp, &position);
    while (fgets(szBuffer, 1023, fp))
        ++m_wEffectObjectCount;

    // Allocate array
    int count = m_wEffectObjectCount;
    m_pEffectObjectInfos = (EffectObjectInfo*)new char[16 * count];
    memset(m_pEffectObjectInfos, 0, 16 * m_wEffectObjectCount);

    // Seek back and parse
    fsetpos(fp, &position);

    char* pRecord = (char*)m_pEffectObjectInfos;

    if (!fgets(szBuffer, 1023, fp))
    {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    while (true)
    {
        // Parse object kind code
        char* pToken = strtok(szBuffer, szDelimiter);
        if (!pToken) break;
        *(unsigned short*)pRecord = cltItemKindInfo::TranslateKindCode(pToken);

        // Parse effect kind code
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        *(unsigned short*)(pRecord + 2) = cltItemKindInfo::TranslateKindCode(pToken);

        // Parse placement type (NORMAL or TOP)
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        if (strcmp(pToken, "NORMAL") == 0)
            *(unsigned short*)(pRecord + 4) = 2;
        else if (strcmp(pToken, "TOP") == 0)
            *(unsigned short*)(pRecord + 4) = 3;
        else
            break;

        // Parse X offset
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        *(int*)(pRecord + 8) = atoi(pToken);

        // Parse Y offset
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        *(int*)(pRecord + 12) = atoi(pToken);

        pRecord += 16;

        if (!fgets(szBuffer, 1023, fp))
        {
            g_clTextFileManager.fclose(fp);
            return 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return result;
}
