#include "Object/CBaseObject.h"
#include "global.h"

extern int dword_A73088;
extern int dword_A7308C;

CBaseObject::CBaseObject()
    : m_siID(0)
    , m_siKind(0)
    , m_dwField1(0)
    , m_siField2(0)
    , _pad1(0)
    , m_dwValid(0)
    , m_siType(0)
    , _pad2(0)
    , m_dwField3(0)
    , m_siField4(0)
    , _pad3(0)
    , m_nPosX(0)
    , m_nPosY(0)
    , m_siField5(0)
    , _pad4(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_byAlpha(0xFF)
    , m_byField6(0)
    , m_byField7(0)
    , _pad5(0)
    , m_bInitialized(0)
{
    // Ground truth constructor does NOT initialize m_nNameWidth / m_nTitleWidth
    // Those are zeroed in InitNameBackBox instead
}

CBaseObject::~CBaseObject()
{
}

void CBaseObject::InitNameBackBox(char* szName, char* szTitle)
{
    m_nNameWidth = 0;
    m_nTitleWidth = 0;

    // Create name box with semi-transparent black background
    D3DXCOLOR nameColor(0x80000000);
    m_NameBox.Create(0, 0, 0, 15, 0.0f, 0.0f, 0.0f, 0.5f, nullptr);

    // If title string exists, create title box
    if (strlen(szTitle))
    {
        D3DXCOLOR titleColor(0x60000000);
        m_TitleBox.Create(0, 0, 0, 15, 0.0f, 0.0f, 0.0f, 0.375f, nullptr);
    }

    // Create chat balloon
    m_ChatBallon.Create(nullptr, 0, 4, -463873);

    // Calculate text widths
    g_MoFFont.SetFont("ObjectName");

    int nWidth = 0, nHeight = 0;
    g_MoFFont.GetTextLength(&nWidth, &nHeight, "ObjectName", szName);
    m_nNameWidth = nWidth + 6;
    if (szTitle)
    {
        g_MoFFont.GetTextLength(&nWidth, &nHeight, "ObjectName", szTitle);
        m_nTitleWidth = nWidth + 6;
    }

    // Set name box size
    m_NameBox.SetSize((unsigned short)m_nNameWidth, 15);

    // Set title box size if applicable
    if (m_nTitleWidth)
    {
        m_TitleBox.SetSize((unsigned short)m_nTitleWidth, 15);
    }
}

void CBaseObject::SetPosNameBox(float fWorldX, float fWorldY)
{
    // Position name box centered above the object
    m_NameBox.SetPos(
        (int)(fWorldX - (double)dword_A73088) - (m_nNameWidth >> 1),
        (int)(fWorldY - (double)dword_A7308C) - 92);

    // Position title box if it exists
    if (m_nTitleWidth)
    {
        m_TitleBox.SetPos(
            (int)(fWorldX - (double)dword_A73088) - (m_nTitleWidth >> 1),
            (int)(fWorldY - (double)dword_A7308C) - 107);
    }

    // Position chat balloon
    m_ChatBallon.SetPos(
        (int)(fWorldX - (double)dword_A73088),
        (int)(fWorldY - (double)dword_A7308C) - 112);
}

void CBaseObject::PrepareDrawingNameBox()
{
    // Show name box controls so they get rendered in the UI pass
    m_NameBox.Show();
    if (m_nTitleWidth)
        m_TitleBox.Show();
    if (m_ChatBallon.IsVisible())
        m_ChatBallon.Show();
}

void CBaseObject::DrawNameBox()
{
    // Hide name box controls after rendering pass
    m_NameBox.Hide();
    if (m_nTitleWidth)
        m_TitleBox.Hide();
    if (m_ChatBallon.IsVisible())
        m_ChatBallon.Hide();
}
