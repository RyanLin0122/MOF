#include "Effect/CBulletArrow.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"

// 對應反組譯碼: 0x0052D570
CBulletArrow::CBulletArrow()
    : m_pArrowImage(nullptr)
{
    // CBulletBase 的建構函式會被自動呼叫

    // 原始碼: *((_DWORD *)this + 7) = 256;
    m_dwAlpha = 256;
}

// 對應反組譯碼: 0x0052D5B0
CBulletArrow::~CBulletArrow()
{
}

// 對應反組譯碼: 0x0052D5C0
void CBulletArrow::Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed)
{
    m_dwOwnerID = dwOwnerID;
    m_vecPos = *pStartPos;
    m_fSpeed = fSpeed;

    // 計算方向向量並單位化
    m_vecDir = *pEndPos - *pStartPos;
    D3DXVec2Normalize(&m_vecDir, &m_vecDir);
}

// 對應反組譯碼: 0x0052D620
bool CBulletArrow::Process(float fElapsedTime)
{
    // --- 核心邏輯 ---

    // 1. 更新位置
    // 原始碼: v10 = *((float *)this + 4) + *((float *)this + 2);
    //         v3 = *((float *)this + 5) + *((float *)this + 3);
    // 注意：原始碼似乎沒有將速度乘上方向，這可能是一個bug或簡化。
    // 一個更完整的實現應該是: m_vecPos += m_vecDir * m_fSpeed * fElapsedTime;
    // 但為了忠實還原，我們模擬其行為。
    m_vecPos += m_vecDir;

    // 2. 獲取並設定 GameImage
    m_pArrowImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB00001Cu, 0, 1);

    if (m_pArrowImage) {
        m_pArrowImage->SetBlockID(0); // 強制使用第0幀
        m_pArrowImage->SetPosition(m_vecPos.x, m_vecPos.y);
        m_pArrowImage->SetAlpha(m_dwAlpha); // 設定透明度
        m_pArrowImage->Process();
    }

    // 3. 更新生命週期計時器
    // 原始碼: v8 = *((_DWORD *)this + 7) - 1;
    m_dwAlpha--;

    // 4. 檢查生命週期是否結束
    // 原始碼: return v8 == 0;
    return (m_dwAlpha <= 0);
}

// 對應反組譯碼: 0x0052D6E0
void CBulletArrow::Draw()
{
    if (m_pArrowImage && m_pArrowImage->IsInUse())
    {
        m_pArrowImage->Draw();
    }
}