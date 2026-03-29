#include "UI/CControlBoxInvenItem.h"
#include "global.h"

// ------------- ctor / dtor -------------

CControlBoxInvenItem::CControlBoxInvenItem()
    : CControlBoxBase()
    , m_countBox()
    , m_sealBox()
    , m_itemImage()
    , m_frameYellow()
    , m_frameGreen()
    , m_sealStatus(0)
{
    // vftable
    // 依照反編譯序列建立子控制
    CreateChildren();
    // BoxBase 預設初始化（會設置 m_bShow 等）
    CControlBoxBase::Init();

    // 反編譯：*((_DWORD *)this + 13) = 0 → Init() 設了 m_bPassKeyInputToParent=true，這裡清回 false
    SetPassKeyInputToParent(false);
    // 反編譯：*((_DWORD *)this + 44) = 0 → m_Background 的 m_bEnabled
    GetBackground()->SetEnabled(false);
    // 反編譯：*((_DWORD *)this + 382) = 0 → m_frameYellow.m_bEnabled
    m_frameYellow.SetEnabled(false);
    // 反編譯：*((_DWORD *)this + 430) = 0 → m_frameGreen.m_bEnabled
    m_frameGreen.SetEnabled(false);
    // 反編譯：*((_DWORD *)this + 282) = 0 → m_sealBox.m_bEnabled
    m_sealBox.SetEnabled(false);
    // 反編譯：*((_DWORD *)this + 92) = 0 → m_countBox.m_bEnabled
    m_countBox.SetEnabled(false);

    m_sealStatus = 0;
}

CControlBoxInvenItem::~CControlBoxInvenItem()
{
    // 反編譯碼是逐一拆子物件；C++ 以成員自動解構即可。
    // 順序符合成員宣告的逆序：m_frameGreen, m_frameYellow, m_itemImage,
    // m_sealBox, m_countBox(其內含成員), 然後 CControlBoxBase(~CControlImage + Base)
}

// ------------- lifecycle -------------

void CControlBoxInvenItem::CreateChildren()
{
    // 先讓 BoxBase 把底圖(m_bg)掛上自己
    CControlBoxBase::CreateChildren();

    // 底圖向右推 1 像素（對應：CControlBase::SetX((this+120), 1)）
    GetBackground()->SetX(1);

    // 數字盒掛在底圖上（對應：(*(vft+78)+12)((this+312), (this+120))）
    m_countBox.Create(GetBackground());
    // 先不顯示

    // 封印半透明色塊掛自己（對應：(*(this+268)+12)((this+1072), this)）
    m_sealBox.Create(this);
    // 尺寸 32x32（反編譯直接寫入 WORD 欄位）
    m_sealBox.SetSize(32, 32);

    // 主物件圖片（反編譯使用 5 參數 Create：pos=(-1,-1), imageId=536870933, block=2）
    m_itemImage.Create(-1, -1, 536870933u, 2, this);

    // 外框（反編譯使用 5 參數 Create：pos=(-3,-3), imageId=570425419, block=6/7）
    m_frameYellow.Create(-3, -3, 570425419u, 6, this);
    m_frameGreen.Create(-3, -3, 570425419u, 7, this);

    // 依反編譯碼末行：CControlBase::SetSize(this, *((_DWORD*)this + 38))
    SetSize(GetBackground()->GetWidth(), GetBackground()->GetHeight());
}

void CControlBoxInvenItem::ShowChildren(int showItemImage)
{
    CControlBoxBase::ShowChildren();

    // 反編譯：*((_DWORD *)this + 13) = 0 → m_bPassKeyInputToParent 清回 false
    SetPassKeyInputToParent(false);

    // 反編譯：先把 sealBox/數字盒/兩個外框都 Hide，並設置 sealBox 顏色為紅(初值)
    m_sealBox.Hide();
    m_sealBox.SetColor(1.0f, 0.0f, 0.0f, 0.3921569f); // 初值同原碼
    m_countBox.Hide();
    m_frameYellow.Hide();
    m_frameGreen.Hide();

    m_sealStatus = 0; // *((_DWORD*)this + 464) = 0

    // 主圖依參數決定顯示/隱藏（原碼 a2!=0 -> Show，否則 Hide）
    if (showItemImage)
        m_itemImage.Show();
    else
        m_itemImage.Hide();
}

void CControlBoxInvenItem::HideChildren()
{
    CControlBoxBase::HideChildren();

    m_sealBox.Hide();
    m_sealBox.SetColor(1.0f, 0.0f, 0.0f, 0.3921569f);
    m_countBox.Hide();
    m_frameYellow.Hide();
    m_frameGreen.Hide();
    m_itemImage.Hide();

    m_sealStatus = 0; // *((_DWORD*)this + 464) = 0
}

// ------------- drawing -------------

void CControlBoxInvenItem::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    // v = *((_DWORD*)this + 464); 1..2 時準備底圖與外框兩張
    if (m_sealStatus > 0 && m_sealStatus <= 2)
    {
        GetBackground()->PrepareDrawing();          // *(this+120) +24
        m_frameYellow.PrepareDrawing(); // *(this+1472)+24
        m_frameGreen.PrepareDrawing();  // *(this+1664)+24
    }
}

void CControlBoxInvenItem::Draw()
{
    CControlBoxBase::Draw();

    if (m_sealStatus > 0 && m_sealStatus <= 2)
    {
        GetBackground()->Draw();          // *(this+120) +28
        m_frameYellow.Draw(); // *(this+368)+28
        m_frameGreen.Draw();  // *(this+416)+28
    }
}

// ------------- behaviors -------------

int CControlBoxInvenItem::SetSealStatus(int status)
{
    m_sealStatus = status; // *(this+1856) = a2

    if (status == static_cast<int>(SealStatus::Yellow))
    {
        // 黃框：顯示色塊(黃) + 顯示黃框 + 隱藏綠框
        m_sealBox.SetColor(1.0f, 1.0f, 0.0f, 0.3921569f);
        m_sealBox.Show();
        m_frameYellow.Show();
        m_frameGreen.Hide();
    }
    else if (status == static_cast<int>(SealStatus::Green))
    {
        // 綠框：顯示色塊(綠) + 隱藏黃框 + 顯示綠框
        m_sealBox.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_sealBox.Show();
        m_frameYellow.Hide();
        m_frameGreen.Show();
    }
    else
    {
        // 其他：全部隱藏
        m_sealBox.Hide();
        m_frameYellow.Hide();
        m_frameGreen.Hide();
    }
    // ground truth 回傳最後一次 Hide/Show 的結果（虛函式回傳值），語意上不重要
    return 0;
}

void CControlBoxInvenItem::SetAlpha(int a)
{
    // 主圖也跟著變更透明度
    m_itemImage.SetAlpha(a);
    CControlBoxBase::SetAlpha(a);
}

void CControlBoxInvenItem::SetBoxItemCountSetNumber(char showCount, int count)
{
    if (showCount == 1 || count <= 0)
    {
        m_countBox.Hide();
    }
    else
    {
        m_countBox.SetNumber(count);
        m_countBox.Show();
    }
}
