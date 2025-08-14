#ifndef CMOVEICON_H
#define CMOVEICON_H

#include "UI/CControlImage.h"
#include "UI/CControlText.h"

// 前向宣告
class CUIBase;

/**
 * @struct stMoveIconData
 * @brief 用於儲存拖動項目附加資料的結構。
 */
struct stMoveIconData
{
    unsigned int m_dwData; // 儲存的資料 (如：物品ID, 技能ID)
    bool m_bIsSet;         // 標記此欄位是否已設定
};

/**
 * @class CMoveIcon
 * @brief 負責處理拖放操作的移動圖示控制項。
 *
 * 這是一個單例或全域物件，用於在UI上顯示一個跟隨滑鼠的圖示，
 * 並攜帶從來源UI到目標UI的資料。
 */
class CMoveIcon : public CControlImage
{
public:
    // 建構函式與解構函式
    CMoveIcon();
    virtual ~CMoveIcon();

    // --- 拖放生命週期函式 ---

    /**
     * @brief 開始一個拖放操作。
     * @param pOwner 拖放操作的來源UI元件。
     * @param bAutoTerminate 釋放滑鼠時是否自動終止。
     * @return 成功開始返回 true，如果已在拖放中則返回 false。
     */
    bool Start(CUIBase* pOwner, bool bAutoTerminate = true);
    bool Start(CUIBase* pOwner, unsigned int uiGIID, unsigned short usBlockID, int nPosX, int nPosY);
    // 另一種 Start 的多載，可能用於從一個已存在的控制項複製外觀
    bool Start(CUIBase* pOwner, CControlImage* pSrcImage, int nPosX, int nPosY);

    /**
     * @brief 結束並放下圖示。
     * @param pTarget 拖放操作的目標UI元件。
     * @return 如果操作成功結束返回 true。
     */
    bool Put(CUIBase* pTarget);
    bool Put(int nUITypeID); // 透過UI類型ID放下

    /**
     * @brief 強制終止拖放操作。
     */
    void Terminate();
    
    // --- 拖放過程中的函式 ---

    // 移動圖示到指定位置（通常是滑鼠座標）
    void Move(int nPosX, int nPosY);
    
    // 為移動的圖示附加資料
    void AddData(unsigned char ucIndex, unsigned int dwData);
    // 覆蓋移動圖示的資料
    void OverlapData(unsigned char ucIndex, unsigned int dwData);
    
    // --- 狀態查詢 ---
    bool IsStart() const;
    unsigned int GetData(unsigned char ucIndex);
    bool IsSetData(unsigned char ucIndex) const;
    bool IsDownUIType(int nUITypeID);
    bool IsDownUIType(CUIBase* pUI);
    
    // --- 其他 ---
    void InitData();
    void SetNotAutoTerminate(); // 設定為手動終止模式
    bool AutoTerminate();
    CControlText* GetTextControl();

protected:
    CUIBase* m_pOwner;       // 拖動操作的來源UI
    CUIBase* m_pTarget;      // 拖動操作的目標UI
    
    bool m_bAutoTerminate; // 放下時是否自動終止
    bool m_bIsStarted;     // 目前是否正在拖動中

    stMoveIconData m_Data[64]; // 用於攜帶資料的陣列
    unsigned char m_cDataCount;  // 已附加資料的數量

    CControlText m_Text;     // 用於在圖示上顯示文字（如數量）
};

#endif // CMOVEICON_H
