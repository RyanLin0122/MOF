#pragma once
#include "FTInfo.h" // 包含我們上一步還原的 FTInfo 結構
#include <d3d9.h>

/**
 * @class CMoFFontTextureManager
 * @brief 管理字型紋理的物件池。
 *
 * 這個類別從 MOFFont.cpp (constructor at 0x0051E670) 的反編譯程式碼中還原。
 * 它的核心職責是預先創建一組固定的 D3D 紋理 (FTInfo)，並在需要時
 * 將其分配給 MoFFont 使用，在用畢後再回收，以避免頻繁創建和銷毀資源的效能開銷。
 */
class CMoFFontTextureManager {
public:
    // --- 靜態常數 ---
    static const int FONT_TEXTURE_POOL_SIZE = 500;  // 紋理池的大小
    static const int FONT_TEXTURE_WIDTH     = 128;  // 紋理寬度
    static const int FONT_TEXTURE_HEIGHT    = 128;  // 紋理高度

public:
    static CMoFFontTextureManager* GetInstance();
    ~CMoFFontTextureManager();

    /**
     * @brief 初始化管理器，創建所有紋理資源。
     * @param pDevice 指向 Direct3D 裝置的指標。
     */
    void InitCMoFFontTextureManager(IDirect3DDevice9* pDevice);

    /**
     * @brief 重設管理器狀態，釋放所有紋理。
     */
    void Reset();

    /**
     * @brief 從池中獲取一個可用的字型紋理。
     * @return 如果池中還有可用的紋理，返回指向 FTInfo 的指標；否則返回 nullptr。
     */
    FTInfo* GetFontTexture();

    /**
     * @brief 將一個字型紋理歸還至池中。
     * @param pFTInfo 要歸還的 FTInfo 物件指標。
     *
     * 函式會減少其引用計數，當計數為 0 時，該紋理會被標記為可用並放回池中。
     */
    void DeleteFontTexture(FTInfo* pFTInfo);

private:
    // --- 成員變數 (順序與記憶體佈局對應原始碼) ---

    // 指向 Direct3D 裝置的指標。 (位移: +0)
    IDirect3DDevice9* m_pDevice;

    // 一個指標陣列，作為管理可用紋理的堆疊(Stack)。 (位移: +4)
    // 儲存指向 m_FTInfoPool 中可用元素的指標。
    FTInfo* m_pAvailableTextureStack[FONT_TEXTURE_POOL_SIZE];

    // 實際儲存 FTInfo 物件的陣列 (物件池本體)。 (位移: +2004)
    FTInfo m_FTInfoPool[FONT_TEXTURE_POOL_SIZE];

    // 指向 m_pAvailableTextureStack 中下一個可用位置的索引。 (位移: +12004)
    int m_nNextAvailableIndex;

private:
    CMoFFontTextureManager();
    static CMoFFontTextureManager* s_pInstance;
};
