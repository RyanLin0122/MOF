#ifndef STTOOLTIPDATA_H
#define STTOOLTIPDATA_H

#include <string>

/**
 * @struct stToolTipData
 * @brief 儲存工具提示（ToolTip）所需的資料。
 */
struct stToolTipData
{
    int m_nToolTipType;     // 提示類型
    int m_nData;            // 相關資料 (例如：持續時間、ID等)
    short m_usData;         // 相關短整型資料
    int m_nUnk;             // 未知用途的資料
    std::string m_strText;  // 提示顯示的文字
    char m_cUIType;         // UI類型
    short m_usSlotIndex;    // 物品或技能在容器中的索引
    void* m_pExtraData;     // 指向額外資料的指標

    // 建構函式
    stToolTipData();
    // 解構函式
    ~stToolTipData();

    // 初始化函式
    void Init();
    // 設定字串類型的提示資料
    void SetStringType(char* text, int data);
    // 設定種類(Kind)類型的提示資料
    void SetKindType(int type, short usData, int unk, int data, char uiType, short slotIndex, void* extraData);
};

#endif // STTOOLTIPDATA_H