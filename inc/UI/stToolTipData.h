#pragma once
#include <string>
#include <cstring>

// 工具提示數據結構
class stToolTipData {
public:
    // 建構函數
    stToolTipData();

    // 解構函數
    ~stToolTipData();

    // 初始化
    void Init();

    // 設定字串類型的工具提示
    void SetStringType(const char* text, int color);

    // 設定物品類型的工具提示
    void SetKindType(int type, short id, int count, int color, char grade, short durability, int extra);

private:
    // 偏移 0: 類型 (-1=未設定, 1=字串類型, 其他=物品類型)
    int m_type;

    // 偏移 4: 顏色值
    int m_color;

    // 偏移 8: 物品ID (僅物品類型使用)
    short m_itemId;

    // 偏移 10: 保留空間
    short m_reserved1;

    // 偏移 12: 數量 (僅物品類型使用)
    int m_count;

    // 偏移 16: 字串數據 (std::string 結構)
    std::string m_text;

    // 偏移 32: 品級 (僅物品類型使用)
    char m_grade;

    // 偏移 33: 保留空間
    char m_reserved2;

    // 偏移 34: 耐久度 (僅物品類型使用)
    short m_durability;

    // 偏移 36: 額外數據 (僅物品類型使用)
    int m_extra;
};