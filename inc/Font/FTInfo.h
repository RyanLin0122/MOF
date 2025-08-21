#pragma once
#include <d3d9.h>
#include <windef.h> // 包含 WORD, DWORD, BOOL 等 Windows 資料類型

/**
 * @struct FTInfo
 * @brief Font Texture Info，代表字型紋理池中的一個紋理單元。
 *
 * 這個結構從 MOFFont.cpp (constructor at 0x0051E6C0, destructor at 0x0051E6E0) 的
 * 反編譯程式碼中還原。它封裝了 Direct3D 紋理和表面物件，並透過引用計數來管理其生命週期。
 * 結構大小為 20 位元組 (0x14)，與反編譯碼中的 'eh vector constructor iterator' 相符。
 */
struct FTInfo {
public:
    // --- 成員變數 (順序與記憶體佈局對應原始碼) ---

    // 指向D3D9紋理物件的指標。 (位移: +0)
    IDirect3DTexture9* m_pTexture;

    // 指向紋理表面(Surface)的指標，GDI操作會在這個表面上進行。 (位移: +4)
    IDirect3DSurface9* m_pSurface;

    // 引用計數，記錄有多少個文字行正在使用此紋理。 (位移: +8)
    // 當計數歸零時，此紋理可被回收至池中。
    WORD m_wRefCnt;

    // (位移: +10) - 2位元組的記憶體對齊填充 (padding)

    // 此紋理在 CMoFFontTextureManager 池中的索引。 (位移: +12)
    DWORD m_dwIndex;

    // 標記此紋理當前是否被從池中取出使用。 (位移: +16)
    BOOL m_bUsed;

    // (位移: +17) - 3位元組的記憶體對齊填充 (padding)

public:
    /**
     * @brief 建構函式，初始化所有成員為零或null。
     */
    FTInfo();

    /**
     * @brief 解構函式，安全地釋放D3D物件。
     */
    ~FTInfo();

    /**
     * @brief 釋放內部持有的D3D資源。
     */
    void Release();
};
