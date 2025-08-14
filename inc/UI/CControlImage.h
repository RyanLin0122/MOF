#ifndef CCONTROLIMAGE_H
#define CCONTROLIMAGE_H

#include "UI/CControlBase.h"
#include "Image/GameImage.h"

// 前向宣告，代表外部的圖片資源管理器和圖片物件
// Forward declarations for external image resource manager and image object.
struct GameImage;

/**
 * @class CControlImage
 * @brief 用於顯示圖片的UI控制項。
 *
 * 繼承自 CControlBase，增加了處理圖片資源、渲染屬性（如透明度、角度、顏色遮罩）
 * 以及淡入淡出效果的功能。
 */
class CControlImage : public CControlBase
{
public:
    // 建構函式與解構函式
    CControlImage();
    virtual ~CControlImage();

    // --- 覆寫的虛擬函式 ---
    virtual void ClearData() override;
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;
    virtual void Create(CControlBase* pParent);
    virtual void Create(int nPosX, int nPosY, CControlBase* pParent);
    virtual void Create(int nPosX, int nPosY, unsigned int uiGIID, unsigned short usBlockID, CControlBase* pParent);

    virtual void Hide() override;
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

    // --- 圖片設定 ---
    // 設定圖片資源（GIID: 圖片組ID, BlockID: 圖片在組中的索引）
    void SetImage(unsigned int uiGIID, unsigned short usBlockID);
    void SetImageID(unsigned int uiImageType, unsigned int uiGIID, unsigned short usBlockID);
    void SetImageID(unsigned int uiGIID, unsigned short usBlockID);
    void SetImageID(int, int, short); // 為了相容性保留
    void SetGIID(unsigned int uiGIID);
    void SetBlockID(unsigned short usBlockID);
    
    // --- 渲染屬性設定 ---
    // 設定透明度 (0-255)
    void SetAlpha(int nAlpha);
    // 設定旋轉角度
    void SetAngle(int nAngle);
    // 設定顏色遮罩/濾鏡 (值為 0.0f - 1.0f)
    void SetColorTint(float r, float g, float b, float a);
    // 設定渲染模式（例如：淡入效果）
    void SetShadeMode(int nMode);
    // 設定圖片從頂部開始繪製的百分比
    void SetOutputImagePercentFromTop(unsigned char ucPercent);

protected:
    // 根據圖片的 BlockID 自動設定控制項的大小
    void SetImageSize();

protected:
    // --- 成員變數 ---
    int   m_nImageType;       // 圖片類型 (例如：ICON, UI)
    unsigned int m_nImageGIID;         // 圖片組資源 ID (Group Image ID)
    unsigned short m_usImageBlockID;   // 圖片在組中的區塊 ID (Block ID)

    GameImage* m_pGameImage;  // 指向實際載入的圖片資源物件
    int   m_nAlpha;           // 主要透明度 (0-255)
    int   m_nAngle;           // 旋轉角度

    char  m_nRenderType;      // 渲染類型
    bool  m_bIsImageLoaded;   // 圖片是否已成功載入並準備好繪製
    unsigned char m_cOutputPercentTop; // 從頂部開始繪製的百分比 (0-100)

    // 用於淡入淡出效果的變數
    int   m_nShadeAlpha;      // 動態變化的透明度
    unsigned long m_dwShadeStartTime; // 效果開始的時間戳
    int   m_nShadeMode;       // 0: 無效果, 1: 淡入
    int   m_nShadeDirection;  // 效果方向 (未使用，但存在)
    
    // 顏色遮罩/濾鏡
    float m_fRed;             // 紅色通道
    float m_fGreen;           // 綠色通道
    float m_fBlue;            // 藍色通道
    float m_fAlphaTint;       // 透明度通道
    
    bool  m_bIsMouseOver;     // 滑鼠是否懸停在上方
};

#endif // CCONTROLIMAGE_H
