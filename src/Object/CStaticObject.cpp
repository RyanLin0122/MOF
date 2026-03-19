#include "Object/CStaticObject.h"
#include "Object/CObjectResourceManager.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/ImageResourceListDataMgr.h"
#include <io.h>
#include <cstdio>
#include <cstring>
#include <windows.h>

extern int dword_A73088;
extern int dword_A7308C;

CStaticObject::CStaticObject()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_pObjectInfo(nullptr)
{
    m_bInitialized = 0;
}

CStaticObject::CStaticObject(unsigned short wObjectID)
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_pObjectInfo(nullptr)
{
    m_bInitialized = 0;
    m_pObjectInfo = g_ObjectResourceManager.GetObjectInfoByID(wObjectID);
    if (m_pObjectInfo)
        m_bInitialized = 1;
}

CStaticObject::~CStaticObject()
{
}

void CStaticObject::Process()
{
    if (!m_bInitialized)
        return;

    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(
        8u, m_pObjectInfo->m_dwResourceID, 0, 1);

    if (!m_pGameImage)
        return;

    // Set flip flags from object info
    m_pGameImage->m_bFlipX = m_pObjectInfo->m_bFlipX ? true : false;
    m_pGameImage->m_bFlipY = m_pObjectInfo->m_bFlipY ? true : false;

    // Set block ID
    m_pGameImage->SetBlockID(m_pObjectInfo->m_wBlockID);

    // Set flags
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;

    // Set position (world to screen)
    float screenX = (float)(m_nPosX - dword_A73088);
    float screenY = (float)(m_nPosY - dword_A7308C);
    m_pGameImage->SetPosition(screenX, screenY);

    // Get block dimensions from image resource data
    unsigned short blockID = m_pObjectInfo->m_wBlockID;
    ImageResourceListData* pGIData = m_pGameImage->m_pGIData;

    int blockHeight = 0;
    int blockWidth = 0;

    if (pGIData)
    {
        ImageResource& res = pGIData->m_Resource;
        if (res.m_animationFrameCount > blockID && res.m_pAnimationFrames)
        {
            blockHeight = res.m_pAnimationFrames[blockID].height;
        }
        else
        {
            // Block error check (only if Local.dat exists)
            if (_access("MofData/Local.dat", 0) != -1)
            {
                CHAR szText[256];
                wsprintfA(szText, "%s:%i", (const char*)((char*)&res + 52), blockID);
                MessageBoxA(0, szText, "Block Error", 0);
            }
        }
    }
    m_nHeight = blockHeight;

    // Get width similarly
    if (pGIData)
    {
        ImageResource& res = pGIData->m_Resource;
        if (res.m_animationFrameCount > blockID && res.m_pAnimationFrames)
        {
            blockWidth = res.m_pAnimationFrames[blockID].width;
        }
        else
        {
            if (_access("MofData/Local.dat", 0) != -1)
            {
                CHAR szText[256];
                wsprintfA(szText, "%s:%i", (const char*)((char*)&res + 52), blockID);
                MessageBoxA(0, szText, "Block Error", 0);
            }
        }
    }
    m_nWidth = blockWidth;

    // Set alpha from object's alpha value
    m_pGameImage->m_bFlag_450 = true;
    m_pGameImage->SetAlpha((unsigned int)m_byAlpha);
    m_pGameImage->m_bVertexAnimation = false;

    // Set scale from object info
    m_pGameImage->m_bFlag_449 = true;
    m_pGameImage->SetScale((int)m_pObjectInfo->m_wScale);
    m_pGameImage->m_bVertexAnimation = false;

    m_pGameImage->Process();
}

void CStaticObject::Draw()
{
    if (m_bInitialized)
    {
        if (m_pGameImage)
            m_pGameImage->Draw();
    }
}

int CStaticObject::GetSXtoD(const char* pStr)
{
    if (!pStr)
        return 0;

    int result = 0;
    char szBuffer[9] = {};
    memcpy(szBuffer, pStr, 8);
    sscanf_s(szBuffer, "%X", &result);
    return result;
}
