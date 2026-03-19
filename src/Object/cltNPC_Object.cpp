#include "Object/cltNPC_Object.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "global.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

extern int dword_A73088;
extern int dword_A7308C;
extern int SETTING_FRAME;
extern DCTTextManager g_DCTTextManager;
extern MoFFont g_MoFFont;

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
cltNPC_Object::cltNPC_Object()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_wCurrentFrame(0)
    , m_fAniFrame(0.0f)
    , m_dwResourceID(0)
    , m_wMaxFrames(0)
    , m_nFlipFlag(0)
    , m_nToggleFlag(0)
    , m_dwLastChatTime(0)
    , m_byChatCount(0)
    , m_byChatIndex(0)
{
    memset(m_szName, 0, sizeof(m_szName));
    memset(m_szTitle, 0, sizeof(m_szTitle));
    memset(m_wChatTextIDs, 0, sizeof(m_wChatTextIDs));
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
cltNPC_Object::~cltNPC_Object()
{
}

// -------------------------------------------------------------------------
// Process — 每幀更新 NPC 動畫、聊天氣泡和圖像
// -------------------------------------------------------------------------
void cltNPC_Object::Process()
{
    // 推進動畫
    m_fAniFrame += 0.2f / (float)SETTING_FRAME;
    m_wCurrentFrame = (std::uint16_t)(std::int64_t)m_fAniFrame;

    if (m_wCurrentFrame > m_wMaxFrames)
    {
        m_wCurrentFrame = 0;
        m_fAniFrame = 0.0f;
        // 如果啟用了方向切換，翻轉面向
        if (m_nToggleFlag)
            m_nFlipFlag = (m_nFlipFlag == 0) ? 1 : 0;
    }

    // 聊天氣泡邏輯
    if (m_byChatCount && (timeGetTime() - m_dwLastChatTime > 10000))
    {
        // 從 DCTTextManager 取得聊天文字
        char* text = DCTTextManager::GetText(
            (DCTTextManager*)&g_DCTTextManager,
            m_wChatTextIDs[m_byChatIndex]);
        m_ChatBallon.SetString(text, 0, 0, 0, 0, (Direction)(DirLeft | DirRight));

        // 輪播到下一句
        ++m_byChatIndex;
        if (m_byChatIndex >= m_byChatCount)
            m_byChatIndex = 0;

        // 設定下次聊天時間（隨機延遲）
        int delay = rand() % 5000;
        m_dwLastChatTime = timeGetTime() - delay;
    }

    // 取得圖像
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(8u, m_dwResourceID, 0, 1);

    // 設定圖像屬性
    m_pGameImage->SetBlockID(m_wCurrentFrame);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;

    float screenX = (float)(m_nPosX - dword_A73088);
    float screenY = (float)(m_nPosY - dword_A7308C);
    m_pGameImage->SetPosition(screenX, screenY);

    // 方向旗標
    m_pGameImage->m_bFlipX = (m_nFlipFlag != 0);

    // 設定頂點變換參數 (ground truth 的固定值)
    m_pGameImage->m_baseVertices[0].x = -10.0f;
    m_pGameImage->m_baseVertices[0].y = 0.0f;
    m_pGameImage->m_baseVertices[3].x = 60.0f;
    m_pGameImage->m_baseVertices[3].y = -10.0f;
    m_pGameImage->m_baseVertices[2].x = 63.0f;
    m_pGameImage->m_baseVertices[2].y = 0.0f;
    m_pGameImage->m_bDrawPart2 = true;

    m_pGameImage->SetAlpha(255);
    m_pGameImage->m_bFlag_450 = true;
    m_pGameImage->m_bVertexAnimation = false;

    m_pGameImage->Process();

    // 更新名稱方塊位置
    float worldX = (float)m_nPosX;
    float worldY = (float)m_nPosY;
    CBaseObject::SetPosNameBox(worldX, worldY);
    CBaseObject::PrepareDrawingNameBox();
}

// -------------------------------------------------------------------------
// Draw — 繪製 NPC 圖像和名稱
// -------------------------------------------------------------------------
void cltNPC_Object::Draw()
{
    if (m_pGameImage)
        m_pGameImage->Draw();

    CBaseObject::DrawNameBox();

    // 繪製 NPC 名稱 (綠色)
    g_MoFFont.SetFont("NPCName");
    g_MoFFont.SetTextLineA(
        m_nPosX - dword_A73088,
        m_nPosY - dword_A7308C - 90,
        0xFF00FF00,
        m_szName,
        1, -1, -1);

    // 繪製 NPC 頭銜 (白色)
    g_MoFFont.SetTextLineA(
        m_nPosX - dword_A73088,
        m_nPosY - dword_A7308C - 105,
        0xFFFFFFFF,
        m_szTitle,
        1, -1, -1);
}

// -------------------------------------------------------------------------
// Initialize — 初始化 NPC 物件
// -------------------------------------------------------------------------
void cltNPC_Object::Initialize(float posX, float posY,
                                std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                                unsigned int resourceID, std::uint16_t maxFrames,
                                int flipFlag, int toggleFlag,
                                std::uint16_t* chatTextIDs)
{
    m_nPosX = (int)(std::int64_t)posX;
    m_wMaxFrames = maxFrames;
    m_nPosY = (int)(std::int64_t)posY;
    m_dwResourceID = resourceID;
    m_siField5 = 100;  // WORD offset 20 = 100
    m_nFlipFlag = flipFlag;
    m_nToggleFlag = toggleFlag;
    m_szTitle[0] = '\0';
    m_fAniFrame = 0.0f;

    // 複製 NPC 名稱
    strcpy(m_szName, DCTTextManager::GetText((DCTTextManager*)&g_DCTTextManager, nameTextCode));

    // 如果有頭銜，加上括弧
    if (titleTextCode)
    {
        const char* title = DCTTextManager::GetText((DCTTextManager*)&g_DCTTextManager, titleTextCode);
        sprintf(m_szTitle, "[%s]", title);
    }

    // 複製聊天文字 ID
    memset(m_wChatTextIDs, 0, sizeof(m_wChatTextIDs));
    memcpy(m_wChatTextIDs, chatTextIDs, 6);  // 3 x uint16_t = 6 bytes

    // 隨機初始聊天時間
    int delay = rand() % 10000;
    m_dwLastChatTime = timeGetTime() - delay;

    // 計算有效聊天文字數量
    m_byChatCount = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (m_wChatTextIDs[i])
            ++m_byChatCount;
    }
    m_byChatIndex = 0;

    // 初始化名稱方塊
    CBaseObject::InitNameBackBox(m_szName, m_szTitle);
}
