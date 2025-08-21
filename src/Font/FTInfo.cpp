#include "Font/FTInfo.h"

// 對應反編譯碼: 0x0051E6C0
FTInfo::FTInfo() {
    m_pTexture = nullptr;
    m_pSurface = nullptr;
    m_wRefCnt  = 0;
    m_dwIndex  = 0;
    m_bUsed    = FALSE;
}

// 對應反編譯碼: 0x0051E6E0
FTInfo::~FTInfo() {
    Release();
}

/**
 * @brief 釋放D3D紋理和表面。
 *
 * 這是從解構函式和 CMoFFontTextureManager::Reset 的邏輯中提取的共用函式，
 * 用於確保D3D COM物件被正確釋放。
 */
void FTInfo::Release() {
    // 安全地釋放 Texture 物件
    if (m_pTexture) {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }

    // 安全地釋放 Surface 物件
    // 注意：Surface 是從 Texture 取得的，理論上應在 Texture 之前釋放，
    // 但我們遵循反編譯碼的順序。在D3D9中，只要確保最終都釋放即可。
    if (m_pSurface) {
        m_pSurface->Release();
        m_pSurface = nullptr;
    }
}
