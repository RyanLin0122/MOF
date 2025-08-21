#pragma once

// 前向宣告，避免與 FTInfo.h 循環引用
struct FTInfo;

/**
 * @struct TLILInfo
 * @brief Text Line Info List Info，代表一個可渲染的文字片段。
 *
 * 這個結構是 MoFFont 快取系統中最基礎的資料單元。它的定義是根據
 * TextLineInfoListMgr::Add (0x0051E8F0) 中分配的記憶體大小 (0x28 位元組，即 40 位元組)
 * 以及 MoFFont::SetTextLine 中對其成員的賦值操作還原而來。
 *
 * 每個 TLILInfo 物件都包含了在螢幕上繪製一個矩形(Quad)所需的所有資訊。
 */
struct TLILInfo {
    // --- 鏈結串列指標 ---
    TLILInfo* m_pPrev;        // 指向前一個節點 (位移: +0)
    TLILInfo* m_pNext;        // 指向後一個節點 (位移: +4)

    // --- 渲染資源 ---
    FTInfo* m_pFTInfo;      // 指向其字元所在的紋理 (位移: +8)

    // --- 紋理座標 (UVs) ---
    float     m_fTexU1;       // 左上角 U (位移: +12)
    float     m_fTexV1;       // 左上角 V (位移: +16)
    float     m_fTexU2;       // 右下角 U (位移: +20)
    float     m_fTexV2;       // 右下角 V (位移: +24)

    // --- 螢幕位置與尺寸 ---
    float     m_fPosX;        // 片段在螢幕上的起始 X 座標 (位移: +28)
    float     m_fPosY;        // 片段在螢幕上的起始 Y 座標 (位移: +32)
    float     m_fWidth;       // 片段在螢幕上的寬度 (位移: +36)

    // 預設建構函式
    TLILInfo() {
        m_pPrev = nullptr;
        m_pNext = nullptr;
        m_pFTInfo = nullptr;
        m_fTexU1 = m_fTexV1 = m_fTexU2 = m_fTexV2 = 0.0f;
        m_fPosX = m_fPosY = m_fWidth = 0.0f;
    }
};
