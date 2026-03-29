#include "UI/CControlBoxQSL.h"
#include "global.h"
#include "Info/cltItemKindInfo.h"

//--------------------------------------------------
// ctor / dtor
//--------------------------------------------------
CControlBoxQSL::CControlBoxQSL()
    : CControlBoxBase()
    , m_hotKeyText()
    , m_skillLevelIcon()
    , m_selectBox()
    , m_sealOverlay()
    , m_countBox()
    , m_sealImg6()
    , m_sealImg7()
{
    CreateChildren();
    Init();
}

CControlBoxQSL::~CControlBoxQSL()
{
    // 成員自動解構（順序由編譯器反向處理）
}

//--------------------------------------------------
// Init (00418F70)
//--------------------------------------------------
void CControlBoxQSL::Init()
{
    CControlBoxBase::Init();
    m_dataType       = 0;
    m_cachedImageID  = 0;
    m_cachedFrame    = 0;
    m_cachedIndex    = 0;
    m_cachedQuantity = 0;
}

//--------------------------------------------------
// SetBoxItemCountSetNumber (00418FB0)
//--------------------------------------------------
void CControlBoxQSL::SetBoxItemCountSetNumber(char flagShow, int count)
{
    if (flagShow == 1 || count <= 0)
    {
        m_countBox.Hide();
    }
    else
    {
        m_countBox.SetNumber(count);
        m_countBox.Show();
    }
}

//--------------------------------------------------
// CreateChildren (00418FF0)
//--------------------------------------------------
void CControlBoxQSL::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // 技能等級圖示 (+760)
    m_skillLevelIcon.Create(this);

    // 數量盒 (+1368)
    m_countBox.Create(this);

    // 快捷鍵文字 (+328)
    m_hotKeyText.Create(this);
    m_hotKeyText.SetControlSetFont("HotKeyText");
    m_hotKeyText.SetPos(0, 23);
    m_hotKeyText.SetTextColor(0xFFFFFFFF);    // -1
    m_hotKeyText.SetShadowColor(0xFF000000);  // -16777216

    // 選取框 (+952) —— 32×32
    m_selectBox.Create(this);
    m_selectBox.SetPos(0, 0);
    m_selectBox.SetSize(32, 32);

    // 封印覆蓋 (+1160) —— 32×32
    m_sealOverlay.Create(this);
    m_sealOverlay.SetSize(32, 32);

    // 封印邊框圖 (+2128, +2320)
    m_sealImg6.Create(-3, -3, 570425419u, 6, this);
    m_sealImg7.Create(-3, -3, 570425419u, 7, this);
}

//--------------------------------------------------
// SetArrayIndex (004190E0)
//--------------------------------------------------
void CControlBoxQSL::SetArrayIndex(int idx)
{
    CControlBase::SetArrayIndex(idx);
}

//--------------------------------------------------
// SetHotKey (004190F0)
//--------------------------------------------------
void CControlBoxQSL::SetHotKey(char* text)
{
    m_hotKeyText.SetText(text);
}

//--------------------------------------------------
// PrepareDrawing (00419110)
//--------------------------------------------------
void CControlBoxQSL::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    if (m_sealStatus > 0 && m_sealStatus <= 2)
    {
        GetBackground()->PrepareDrawing();
        m_sealImg6.PrepareDrawing();
        m_sealImg7.PrepareDrawing();
        m_hotKeyText.PrepareDrawing();
    }
}

//--------------------------------------------------
// Draw (00419160)
//--------------------------------------------------
void CControlBoxQSL::Draw()
{
    CControlBoxBase::Draw();

    if (m_sealStatus > 0 && m_sealStatus <= 2)
    {
        GetBackground()->Draw();
        m_sealImg6.Draw();
        m_sealImg7.Draw();
        m_hotKeyText.Draw();
    }
}

//--------------------------------------------------
// SetSealStatus (004191B0)
//--------------------------------------------------
void CControlBoxQSL::SetSealStatus(int status)
{
    m_sealStatus = status;

    if (status == 1)
    {
        // 黃色高亮：顯示 overlay + sealImg6
        m_sealOverlay.SetColor(1.0f, 1.0f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_sealImg6.Show();
    }
    else if (status == 2)
    {
        // 綠色高亮：顯示 overlay + sealImg7，隱藏 sealImg6
        m_sealOverlay.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_sealImg6.Hide();
        m_sealImg7.Show();
        m_selectBox.Hide();
        return;
    }
    else
    {
        // 全部隱藏
        m_sealOverlay.Hide();
        m_sealImg6.Hide();
    }

    m_sealImg7.Hide();
    m_selectBox.Hide();
}

//--------------------------------------------------
// SetQSLBoxDataForItem (004192C0)
//--------------------------------------------------
void CControlBoxQSL::SetQSLBoxDataForItem(unsigned int imageId, unsigned short frame,
                                           unsigned short index, unsigned short quantity,
                                           unsigned short fashionParam)
{
    if (m_cachedImageID != imageId
        || m_cachedFrame != frame
        || m_cachedIndex != index
        || m_cachedQuantity != quantity)
    {
        m_cachedFrame    = frame;
        m_cachedImageID  = imageId;
        m_cachedIndex    = index;
        m_cachedQuantity = quantity;

        GetBackground()->SetImageID(3u, imageId, frame);
        m_countBox.SetNumber(m_cachedQuantity);

        if (!g_clItemKindInfo.IsFashionItem(index) || fashionParam == (unsigned short)-1)
        {
            m_dataType = 1;
            SetToolTipData(m_cachedIndex, 3, 0, 1, (char)255, 0xFFFF, 0);
        }
        else
        {
            m_dataType = 4;
            SetToolTipData(m_cachedIndex, 5, 0, 1, 4, fashionParam, 0);
        }
        ShowChildren();
    }
}

//--------------------------------------------------
// SetQSLBoxDataForSkill (004193C0)
//--------------------------------------------------
void CControlBoxQSL::SetQSLBoxDataForSkill(unsigned int imageId, unsigned short frame,
                                            unsigned short index, unsigned char skillLevel)
{
    if (m_cachedImageID != imageId || m_cachedFrame != frame || m_cachedIndex != index)
    {
        m_cachedFrame   = frame;
        m_cachedImageID = imageId;
        m_cachedIndex   = index;
        m_dataType      = 2;

        GetBackground()->SetImageID(3u, imageId, frame);
        SetToolTipData(m_cachedIndex, 6, 0, 1, (char)255, 0xFFFF, 0);
        ShowChildren();
    }

    if (skillLevel == 0xFF)
    {
        m_skillLevelIcon.Hide();
    }
    else
    {
        m_skillLevelIcon.SetImage(184550801u, skillLevel);
        ShowChildren();
    }
}

//--------------------------------------------------
// SetQSLboxDataForEmblem (00419470)
//--------------------------------------------------
void CControlBoxQSL::SetQSLboxDataForEmblem(unsigned int imageId, unsigned short frame,
                                             unsigned short index)
{
    m_selectBox.Hide();

    if (m_cachedImageID != imageId || m_cachedFrame != frame || m_cachedIndex != index)
    {
        m_cachedFrame   = frame;
        m_cachedImageID = imageId;
        m_cachedIndex   = index;
        m_dataType      = 3;

        GetBackground()->SetImageID(3u, imageId, frame);
        SetToolTipData(m_cachedIndex, 10, 0, 1, (char)255, 0xFFFF, 0);
        ShowChildren();
    }
}

//--------------------------------------------------
// ShowChildren (00419500)
//--------------------------------------------------
void CControlBoxQSL::ShowChildren()
{
    CControlBoxBase::ShowChildren();

    m_hotKeyText.Show();
    m_sealOverlay.Hide();
    m_sealImg6.Hide();
    m_sealImg7.Hide();
    m_selectBox.Hide();

    m_sealStatus = 0;

    if (m_dataType)
    {
        GetBackground()->Show();

        switch (m_dataType)
        {
        case 1: // 道具
            m_countBox.Show();
            m_skillLevelIcon.Hide();
            break;
        case 2: // 技能
            m_countBox.Hide();
            m_skillLevelIcon.Show();
            break;
        case 3: // 紋章
        case 4: // 時裝
            m_countBox.Hide();
            m_skillLevelIcon.Hide();
            break;
        }
    }
}

//--------------------------------------------------
// HideChildren (00419600)
//--------------------------------------------------
void CControlBoxQSL::HideChildren()
{
    CControlBoxBase::HideChildren();

    m_sealOverlay.Hide();
    m_sealImg6.Hide();
    m_sealImg7.Hide();
    m_selectBox.Hide();

    m_sealStatus = 0;
    m_skillLevelIcon.m_usBlockID = 0xFFFF;
    m_skillLevelIcon.SetSize(0, 0);
    m_skillLevelIcon.m_nGIID = 0;

    m_hotKeyText.Show();
}
