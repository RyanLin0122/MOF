#include "Object/cltNPC_Taxi.h"
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
extern DCTTextManager g_DCTTextManager;
extern MoFFont g_MoFFont;

// -------------------------------------------------------------------------
// Constructor — ground truth 對應 005A2AA0
// -------------------------------------------------------------------------
cltNPC_Taxi::cltNPC_Taxi()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_wCurrentFrame(0)
    , m_fAniFrame(0.0f)
    , m_nFlipFlag(0)
    , m_byState(0)
    , m_byMaxFrame(3)
    , m_dwLastChatTime(0)
    , m_byChatCount(0)
    , m_byChatIndex(0)
{
    // Ground truth: 設定 4 組資源 ID (0x0C000374 ~ 0x0C000377)
    m_dwResourceIDs[0] = 201327220;  // 0x0C000374
    m_dwResourceIDs[1] = 201327221;  // 0x0C000375
    m_dwResourceIDs[2] = 201327222;  // 0x0C000376
    m_dwResourceIDs[3] = 201327223;  // 0x0C000377

    memset(m_szName, 0, sizeof(m_szName));
    memset(m_szTitle, 0, sizeof(m_szTitle));
    memset(m_wChatTextIDs, 0, sizeof(m_wChatTextIDs));
}

// -------------------------------------------------------------------------
// Destructor — ground truth 對應 005A2B20
// -------------------------------------------------------------------------
cltNPC_Taxi::~cltNPC_Taxi()
{
    // CBaseObject::~CBaseObject() 由 C++ 自動呼叫
}

// -------------------------------------------------------------------------
// Process — ground truth 對應 005A2B30
// Taxi 動畫使用多狀態循環：state 0,1,2 各有 m_byMaxFrame 幀
// -------------------------------------------------------------------------
void cltNPC_Taxi::Process()
{
    // 檢查 frame 是否超過每狀態最大幀
    if ((unsigned int)m_wCurrentFrame > m_byMaxFrame)
    {
        m_wCurrentFrame = 0;
        m_fAniFrame = 0.0f;
        ++m_byState;

        if (m_byState == 3)
        {
            m_byMaxFrame = 2;
        }
        else if (m_byState > 3)
        {
            m_byState = 0;
            m_byMaxFrame = 3;
        }
    }

    // 聊天氣泡
    if (m_byChatCount && (timeGetTime() - m_dwLastChatTime > 10000))
    {
        char* text = g_DCTTextManager.GetText(m_wChatTextIDs[m_byChatIndex]);
        m_ChatBallon.SetString(text, 0, 0, 0, 0, (Direction)(DirLeft | DirRight));

        ++m_byChatIndex;
        if (m_byChatIndex >= m_byChatCount)
            m_byChatIndex = 0;

        int delay = rand() % 5000;
        m_dwLastChatTime = timeGetTime() - delay;
    }

    // 取得圖像 — 使用目前狀態對應的資源 ID
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(
        8u, m_dwResourceIDs[m_byState], 0, 1);

    // 設定圖像屬性
    float screenY = (float)(m_nPosY - dword_A7308C);
    float screenX = (float)(m_nPosX - dword_A73088);
    m_pGameImage->SetBlockID(m_wCurrentFrame);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->SetPosition(screenX, screenY);

    // 方向旗標
    m_pGameImage->m_bFlipX = (m_nFlipFlag != 0);

    // 固定頂點參數
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

    // 推進動畫 — ground truth: +=0.15000001
    m_fAniFrame += 0.15000001f;
    m_wCurrentFrame = (std::uint16_t)(std::int64_t)m_fAniFrame;

    // 名稱方塊 — ground truth: 位置偏移 (+50, -90)
    float nameX = (float)(m_nPosX + 50);
    float nameY = (float)(m_nPosY - 90);
    CBaseObject::SetPosNameBox(nameX, nameY);
    CBaseObject::PrepareDrawingNameBox();
}

// -------------------------------------------------------------------------
// Draw — ground truth 對應 005A2D80
// -------------------------------------------------------------------------
void cltNPC_Taxi::Draw()
{
    if (m_pGameImage)
        m_pGameImage->Draw();

    CBaseObject::DrawNameBox();

    // 繪製名稱 — ground truth 偏移: +50 X, -180 Y (綠色)
    g_MoFFont.SetFont("NPCName");
    g_MoFFont.SetTextLineA(
        m_nPosX - dword_A73088 + 50,
        m_nPosY - dword_A7308C - 180,
        0xFF00FF00,
        m_szName,
        1, -1, -1);

    // 繪製頭銜 — ground truth 偏移: +50 X, -195 Y (白色)
    g_MoFFont.SetTextLineA(
        m_nPosX - dword_A73088 + 50,
        m_nPosY - dword_A7308C - 195,
        0xFFFFFFFF,
        m_szTitle,
        1, -1, -1);
}

// -------------------------------------------------------------------------
// Initialize — ground truth 對應 005A2E20
// 注意：Taxi 的 Initialize 參數組合與 cltNPC_Object 不同
// -------------------------------------------------------------------------
void cltNPC_Taxi::Initialize(float posX, float posY,
                              std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                              unsigned int resourceID, int flipFlag,
                              std::uint16_t* chatTextIDs)
{
    m_nPosX = (int)(std::int64_t)posX;
    m_nPosY = (int)(std::int64_t)posY;
    m_siField5 = 100;
    m_byState = 0;
    m_nFlipFlag = flipFlag;
    m_byChatIndex = 0;
    m_szTitle[0] = '\0';

    strcpy(m_szName, g_DCTTextManager.GetText(nameTextCode));

    if (titleTextCode)
    {
        const char* title = g_DCTTextManager.GetText(titleTextCode);
        sprintf(m_szTitle, "[%s]", title);
    }

    CBaseObject::InitNameBackBox(m_szName, m_szTitle);

    // 複製聊天文字 ID
    memset(m_wChatTextIDs, 0, sizeof(m_wChatTextIDs));
    memcpy(m_wChatTextIDs, chatTextIDs, 6);

    int delay = rand() % 10000;
    m_dwLastChatTime = timeGetTime() - delay;

    m_byChatCount = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (m_wChatTextIDs[i])
            ++m_byChatCount;
    }
}
