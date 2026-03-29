#pragma once

#include <cstdint>
#include <algorithm>
#include "UI/CControlBase.h"
#include "Image/cltImageManager.h"   // 包含 GameImage.h
// timeGetTime：避免拉進 windows.h
extern "C" unsigned long __stdcall timeGetTime(void);

// 這個常數在基準中用來做淡入速度換算
extern int SETTING_FRAME; // 例如 60

class CControlImage : public CControlBase
{
public:
    CControlImage();
    virtual ~CControlImage();

    // ---- 生命週期 ----
    virtual void Create(CControlBase* pParent) override;
    virtual void Create(int x, int y, CControlBase* pParent) override;
    virtual void Create(int x, int y, unsigned int giid, unsigned short block, CControlBase* pParent);

    // ---- 清理 ----
    void ClearData();

    // ---- 指定圖像 ----
    void SetImage(unsigned int giid, unsigned short block);
    void SetImageID(unsigned int giGroup, unsigned int giid, unsigned short block);
    void SetImageID(unsigned int giid, unsigned short block);
    void SetImageID(int giGroup, int giid, short block);  // 對齊反編譯 00420A90：無條件版本
    void SetGIID(unsigned int giid);
    void SetBlockID(unsigned short blk);

    // 依資源尺寸回填控制寬高（對齊反編譯 00420F90）
    void SetImageSize();
    void SetImageSizeFromResource() { SetImageSize(); }

    // ---- 視覺屬性 ----
    void SetAlpha(int a);   // 0~255（基準 [34]）
    void SetAngle(int deg); // （基準 [35]）

    // 以 0~1 的顏色係數（基準 [43]~[46] 分別 R,G,B,A）
    void SetColorFactor(float r, float g, float b, float a)
    {
        m_fColorR = r; m_fColorG = g; m_fColorB = b; m_fColorA = a;
    }

    // 從頂端輸出比例（基準 [152]）
    void SetOutputImagePercentFromTop(uint8_t v);

    // 0 = 關閉陰影/淡入（直接使用 m_nAlpha）
    // 1 = 開啟陰影/淡入（用 m_fadeCurA 從 0 開始）
    void SetShadeMode(int enable);

    // ---- 繪製流程 ----
    virtual void PrepareDrawing() override;
    virtual void Draw() override;
    virtual void Hide() override; // 基準：若淡入啟用，Hide 時把 alpha 歸零

    // ---- 事件傳遞（可選）----
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 對齊反編譯：檢查 m_pGameImage 是否已填入（由 PrepareDrawing 填）
    GameImage* GetGameImage() const { return m_pGameImage; }

    unsigned short m_usBlockID = 0xFFFF;   // [64]，-1 表示未指定
    int           m_nGIID = 0;        // [31]
    int           m_nGIGroup = 5;        // [30]，預設群組 5
    int           m_fadeCurA = 0;        // [39] 目前淡入 alpha（0~255）
private:
    static inline uint8_t ToByte01(float v)
    {
        if (v <= 0.0f) return 0;
        if (v >= 1.0f) return 255;
        // 對齊反編譯：ground truth 使用 double 精度（255.0 / 0.5）並以 __int64 截斷
        return static_cast<uint8_t>(static_cast<int64_t>(v * 255.0 + 0.5));
    }

private:
    // ---- 對齊基準的內部狀態 ----
    GameImage* m_pGameImage = nullptr;  // [33]（由 PrepareDrawing 填）
    int           m_bPrepared = 0;        // [37] 是否可繪

    int           m_nAlpha = 255;      // [34]
    int           m_nAngle = 0;        // [35]
    uint8_t       m_drawFlag = 3;        // [144]（基準設 3）

    // 淡入（基準 [39]、[40]、[41]、[42]）
    
    unsigned long m_lastTick = 0;        // [40] timeGetTime()
    int           m_bFadeIn = 1;        // [41] 1=啟用淡入，用 m_fadeCurA；0=直接用 m_nAlpha
    int           m_fadeStep = 1;        // [42] 基準以它參與換算

    // 顏色係數（float 0~1），順序 R,G,B,A（基準 [43]~[46]）
    float         m_fColorR = 1.0f;
    float         m_fColorG = 1.0f;
    float         m_fColorB = 1.0f;
    float         m_fColorA = 1.0f;

    // 其它
    uint8_t       m_outputPercentFromTop = 0; // [152]
    int           m_bHover = 0;               // [47] 命中快取（事件流程用）
};