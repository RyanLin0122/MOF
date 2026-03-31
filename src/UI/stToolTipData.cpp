#include "UI/stToolTipData.h"

// 建構函數
// ground truth: 先將 string 內部三欄位清 0，再呼叫 Init
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

// 複製建構函數
// ground truth (line 158968): 逐欄位複製，含 string 深複製
stToolTipData::stToolTipData(const stToolTipData& other)
    : m_type(other.m_type)
    , m_color(other.m_color)
    , m_itemId(other.m_itemId)
    , m_reserved1(other.m_reserved1)
    , m_count(other.m_count)
    , m_text(other.m_text)
    , m_grade(other.m_grade)
    , m_reserved2(other.m_reserved2)
    , m_durability(other.m_durability)
    , m_extra(other.m_extra)
{
}

// 解構函數
// ground truth: 只處理 string 釋放（std::string 自動處理）並清零 string 三欄位
// 不清 m_type/m_color/m_count/m_extra
stToolTipData::~stToolTipData()
{
    // std::string 解構子自動處理記憶體釋放
    // ground truth 只清除 string 的 ptr/size/capacity 三個 DWORD（由 std::string 解構完成）
}

// 初始化函數
// ground truth: 設 m_type, m_color, m_grade, m_durability，清字串，
// 並重置 m_itemId, m_count, m_extra
void stToolTipData::Init()
{
    m_type = -1;
    m_color = 1;
    m_grade = -1;
    m_durability = -1;

    // 清空字串（對齊 ground truth 的 string 操作）
    m_text.clear();

    // ground truth: 結尾重置這三個欄位
    m_itemId = 0;
    m_count = 0;
    m_extra = 0;
}

// 設定字串類型的工具提示
// ground truth: 不丟例外，直接賦值
void stToolTipData::SetStringType(char* text, int color)
{
    m_type = 1;  // 字串類型
    m_color = color;
    m_text = text;
}

// 設定物品類型的工具提示
// ground truth: 回傳 int（回傳 extra/a8）
int stToolTipData::SetKindType(int type, short id, int count, int color, char grade, short durability, int extra)
{
    m_type = type;        // 物品類型
    m_color = color;      // 顏色
    m_itemId = id;        // 物品ID
    m_count = count;      // 數量
    m_grade = grade;      // 品級
    m_durability = durability;  // 耐久度
    m_extra = extra;      // 額外數據
    return extra;
}
