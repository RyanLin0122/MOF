#include "UI/stToolTipData.h"
#include <iostream>
#include <stdexcept>

// 建構函數
stToolTipData::stToolTipData()
    : m_type(-1)
    , m_color(1)
    , m_itemId(0)
    , m_reserved1(0)
    , m_count(0)
    , m_grade(-1)
    , m_reserved2(0)
    , m_durability(-1)
    , m_extra(0)
{
    Init();
}

// 解構函數
stToolTipData::~stToolTipData()
{
    // std::string 會自動清理，不需要手動處理
    m_type = -1;
    m_color = 0;
    m_count = 0;
    m_extra = 0;
}

// 初始化函數
void stToolTipData::Init()
{
    m_type = -1;
    m_color = 1;
    m_grade = -1;
    m_durability = -1;

    // 清空字串
    m_text.clear();

    // 重置其他數值
    m_itemId = 0;
    m_count = 0;
    m_extra = 0;
}

// 設定字串類型的工具提示
void stToolTipData::SetStringType(const char* text, int color)
{
    if (!text) {
        throw std::invalid_argument("Text cannot be null");
    }

    m_type = 1;  // 字串類型
    m_color = color;

    try {
        // 設定文字內容
        m_text = text;
    }
    catch (const std::length_error&) {
        throw std::length_error("String too long");
    }
    catch (const std::bad_alloc&) {
        throw std::bad_alloc();
    }
}

// 設定物品類型的工具提示
void stToolTipData::SetKindType(int type, short id, int count, int color, char grade, short durability, int extra)
{
    m_type = type;        // 物品類型
    m_color = color;      // 顏色
    m_itemId = id;        // 物品ID
    m_count = count;      // 數量
    m_grade = grade;      // 品級
    m_durability = durability;  // 耐久度
    m_extra = extra;      // 額外數據
}