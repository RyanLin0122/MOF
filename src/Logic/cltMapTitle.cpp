#include "Logic/cltMapTitle.h"
#include "Logic/Map.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "global.h"

// ---------------------------------------------------------------------------
// cltMapTitle
// ---------------------------------------------------------------------------

cltMapTitle::cltMapTitle()
    : m_bShowTitle(0)
    , m_bShowBoss(0)
    , m_nAlpha(0)
    , m_mapID(0)
    , m_titleResID(0)
    , m_titleBlockID(0)
    , m_bossResID(0)
    , m_bossBlockID(0)
    , m_nUnknown(0)
    , m_pTitleImage(nullptr)
    , m_pBossImage(nullptr)
    , m_chatBallon()
{
}

cltMapTitle::~cltMapTitle()
{
}

// ---------------------------------------------------------------------------
// Poll — 每 frame 遞減 alpha，alpha 歸零就關閉顯示
// ---------------------------------------------------------------------------
void cltMapTitle::Poll()
{
    if (m_bShowTitle || m_bShowBoss)
    {
        m_nAlpha -= 5;
        if (m_nAlpha < 1)
        {
            m_bShowBoss = 0;
            m_bShowTitle = 0;
        }
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing
// ---------------------------------------------------------------------------
void cltMapTitle::PrepareDrawing()
{
    // --- 地圖標題圖片 ---
    if (m_bShowTitle)
    {
        GameImage* img = cltImageManager::GetInstance()->GetGameImage(
            6u, m_titleResID, 0, 1);
        m_pTitleImage = img;

        int yOffset = (g_Game_System_Info.ScreenHeight - 600) / 2 + 90;
        int xBase   = g_Game_System_Info.ScreenWidth - 800;

        img->SetBlockID(m_titleBlockID);
        img->m_bFlag_447       = true;
        img->m_bFlag_446       = true;
        img->m_bVertexAnimation = false;
        img->SetPosition((float)(xBase / 2 + 258), (float)yOffset);

        int alpha = m_nAlpha;
        if (alpha > 255) alpha = 255;
        img->SetAlpha((unsigned int)alpha);
        img->m_bFlag_450        = true;
        img->m_bVertexAnimation = false;
    }

    // --- Boss 圖片 ---
    if (m_bShowBoss)
    {
        GameImage* img = cltImageManager::GetInstance()->GetGameImage(
            1u, m_bossResID, 0, 1);
        m_pBossImage = img;

        if (img)
        {
            int yOffset = (g_Game_System_Info.ScreenHeight - 600) / 2 + 350;
            int xBase   = g_Game_System_Info.ScreenWidth - 800;

            img->SetBlockID(m_bossBlockID);
            img->m_bFlag_447       = true;
            img->m_bFlag_446       = true;
            img->m_bVertexAnimation = false;
            img->SetPosition((float)(xBase / 2 + 400), (float)yOffset);

            int alpha = m_nAlpha;
            if (alpha > 255) alpha = 255;
            img->SetAlpha((unsigned int)alpha);
            img->m_bFlag_450        = true;
            img->m_bVertexAnimation = false;

            img->SetScale(300);
            img->m_bFlag_449 = true;
            img->m_bVertexAnimation = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void cltMapTitle::Draw()
{
    if (m_bShowTitle)
    {
        if (m_pTitleImage)
            m_pTitleImage->Draw();
    }
    if (m_bShowBoss)
    {
        if (m_pBossImage)
            m_pBossImage->Draw();
    }
}

// ---------------------------------------------------------------------------
// SetMapTitle — 設定地圖標題（透過 mapID 查詢資源）
// ---------------------------------------------------------------------------
void cltMapTitle::SetMapTitle(uint16_t mapID)
{
    if (mapID)
    {
        m_pTitleImage = nullptr;
        m_mapID       = mapID;
        m_bShowTitle  = 0;
        m_nAlpha      = 450;
        m_titleResID  = g_Map.GetMapAreaTitleResourceID(mapID);
        m_titleBlockID = g_Map.GetMapAreaTitleBlockID(m_mapID);

        if (m_titleResID)
            m_bShowTitle = 1;

        m_nUnknown = 0;
    }
}

// ---------------------------------------------------------------------------
// SetBossMopID — 設定 Boss 顯示（由封包傳入 mopID）
// ---------------------------------------------------------------------------
void cltMapTitle::SetBossMopID(uint16_t mopID)
{
    m_pBossImage  = nullptr;
    m_bossResID   = mopID;
    m_bossBlockID = 0;
    m_bShowBoss   = 1;
}
