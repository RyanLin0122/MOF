#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CControlNumberBox.h"

/**
 * Quick Slot (QSL) 控制盒
 * 可顯示道具(1)、技能(2)、紋章(3)、時裝道具(4) 四種類型
 *
 * 成員位移（對照反編譯）：
 *   +312 m_dataType        (BYTE)
 *   +316 m_cachedImageID   (DWORD)
 *   +320 m_cachedFrame     (WORD)
 *   +322 m_cachedIndex     (WORD)
 *   +324 m_cachedQuantity  (WORD)
 *   +328 m_hotKeyText      (CControlText)
 *   +760 m_skillLevelIcon  (CControlImage)
 *   +952 m_selectBox       (CControlAlphaBox)
 *  +1160 m_sealOverlay     (CControlAlphaBox)
 *  +1368 m_countBox        (CControlNumberBox)
 *  +2128 m_sealImg6        (CControlImage)
 *  +2320 m_sealImg7        (CControlImage)
 *  +2512 m_sealStatus      (int)
 */
class CControlBoxQSL : public CControlBoxBase
{
public:
    CControlBoxQSL();
    virtual ~CControlBoxQSL();

    // 初始化
    void Init();

    // 建立子控件
    void CreateChildren();

    // 數量顯示 (flagShow=1 或 count<=0 則隱藏)
    void SetBoxItemCountSetNumber(char flagShow, int count);

    // 陣列索引
    void SetArrayIndex(int idx);

    // 快捷鍵文字
    void SetHotKey(char* text);

    // 繪製
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

    // 封印狀態 (0=無, 1=黃色, 2=綠色)
    void SetSealStatus(int status);

    // 設定道具資料
    void SetQSLBoxDataForItem(unsigned int imageId, unsigned short frame,
                              unsigned short index, unsigned short quantity,
                              unsigned short fashionParam);

    // 設定技能資料
    void SetQSLBoxDataForSkill(unsigned int imageId, unsigned short frame,
                               unsigned short index, unsigned char skillLevel);

    // 設定紋章資料
    void SetQSLboxDataForEmblem(unsigned int imageId, unsigned short frame,
                                unsigned short index);

    // 顯示/隱藏子控件
    void ShowChildren();
    void HideChildren();

private:
    // 資料快取
    char           m_dataType{ 0 };       // 0=空, 1=道具, 2=技能, 3=紋章, 4=時裝
    char           _pad313[3]{ 0 };
    unsigned int   m_cachedImageID{ 0 };
    unsigned short m_cachedFrame{ 0 };
    unsigned short m_cachedIndex{ 0 };
    unsigned short m_cachedQuantity{ 0 };
    unsigned short _pad326{ 0 };

    // 子控件
    CControlText     m_hotKeyText;      // +328
    CControlImage    m_skillLevelIcon;  // +760
    CControlAlphaBox m_selectBox;       // +952
    CControlAlphaBox m_sealOverlay;     // +1160
    CControlNumberBox m_countBox;       // +1368
    CControlImage    m_sealImg6;        // +2128
    CControlImage    m_sealImg7;        // +2320

    // 封印狀態
    int m_sealStatus{ 0 };              // +2512
};
