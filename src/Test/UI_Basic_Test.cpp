#include "Test/UI_Basic_Test.h"
#include <cmath> // for sin
#include <cstdio> // for sprintf_s

UIBasicTest::UIBasicTest() :
    m_pRootControl(nullptr),
    m_pTestControl1(nullptr),
    m_pTestControl2(nullptr),
    m_pStatusText(nullptr),
    m_fTotalTime(0.0f)
{
}

UIBasicTest::~UIBasicTest()
{
    Cleanup();
}

void UIBasicTest::Cleanup()
{
    // �̧Ǧw���a�R���Ҧ� UI ����
    // �ѩ���l���Y�ȬO�޿��쵲�A�O����ݭn��ʺ޲z
    if (m_pStatusText)  delete m_pStatusText;
    if (m_pTestControl2) delete m_pTestControl2;
    if (m_pTestControl1) delete m_pTestControl1;
    if (m_pRootControl)  delete m_pRootControl;

    m_pRootControl = nullptr;
    m_pTestControl1 = nullptr;
    m_pTestControl2 = nullptr;
    m_pStatusText = nullptr;
}

HRESULT UIBasicTest::Initialize()
{
    // --- ���եؼ� ---
    // 1. CControlBase::Create(): �إ߱���ó]�w���l���Y�C
    // 2. CControlBase::SetPos() / SetAbsPos(): ���Ҭ۹�P����y�СC
    // 3. CControlBase::GetAbsX/Y(): ���ҵ���y�Ъ��p��C
    // 4. ���h���c�G�����󲾰ʮɡA�l��������۲��ʡC

    // 1. �إߤ@�ӥb�z�����ڮe�� (�Ǧ�)�A��K�[���d��
    m_pRootControl = new CControlAlphaBox();
    m_pRootControl->Create(300, 100, 400, 300, 0.5f, 0.5f, 0.5f, 0.5f, nullptr);

    // 2. �إߴ��ձ��1 (����)�A���b Root���U�C�y�� (50, 50) �O�۹�� Root ���C
    m_pTestControl1 = new CControlAlphaBox();
    m_pTestControl1->Create(50, 50, 200, 80, 1.0f, 0.0f, 0.0f, 1.0f, m_pRootControl);

    // 3. �إߴ��ձ��2 (�Ŧ�)�A���b TestControl1 ���U�C�y�� (20, 20) �O�۹�� TestControl1 ���C
    m_pTestControl2 = new CControlAlphaBox();
    m_pTestControl2->Create(20, 20, 100, 40, 0.0f, 0.0f, 1.0f, 1.0f, m_pTestControl1);

    // 4. �إߤ@�Ӥ�r����A�Ω���ܪ��A��T
    m_pStatusText = new CControlText();
    m_pStatusText->Create(10, 10, nullptr); // ���b�ک��U (nullptr)
    m_pStatusText->SetTextColor(0xFFFFFFFF); // �զ�
    m_pStatusText->SetFontHeight(16);

    printf("[DBG] root first child = %p\n", m_pRootControl->GetFirstChild());
    printf("[DBG] ctrl1 parent == root ? %d\n", m_pTestControl1->GetParent() == m_pRootControl);
    printf("[DBG] ctrl1 first child = %p\n", m_pTestControl1->GetFirstChild());
    printf("[DBG] ctrl2 parent == ctrl1 ? %d\n", m_pTestControl2->GetParent() == m_pTestControl1);
    return S_OK;
}

void UIBasicTest::Update(float fElapsedTime)
{
    m_fTotalTime += fElapsedTime;

    // --- ���եؼ� ---
    // 1. �ʺA�ק�y�СG���� SetX/SetY �M SetAbsX/SetAbsY�C
    // 2. �ʺA�����G���� Show() / Hide()�C
    // 3. ���A�^Ū�G�ϥ� GetAbsX/Y Ū���y�Ш���ܡA�T�{�䥿�T�ʡC

    // �������� (m_pTestControl1) �����Ӧ^���ʡA���� SetX()
    // �ѩ��Ŧ����O�����l����A���ӷ|��ۤ@�_��
    float newRelativeX = 50.0f + sin(m_fTotalTime) * 40.0f;
    m_pTestControl1->SetX(static_cast<int>(newRelativeX));

    // ���Ŧ��� (m_pTestControl2) �C 2 ��{�{�@���A���� SetVisible()
    bool isVisible = static_cast<int>(m_fTotalTime) % 2 == 0;
    m_pTestControl2->SetVisible(isVisible);

    m_pStatusText->SetText(4751);
    // ��s���A��r�A��ܦU���������y��
    /*
    m_pStatusText->SetTextFmtW(
        L"CControlBase ����:\n"
        L" - ������ (Control1) X �y�Х��b�H sin() �ܤơC\n"
        L" - �Ŧ��� (Control2) �O���������l���A�����H���ʡA�B�C��{�{�C\n\n"
        L"�Y�ɮy��:\n"
        L" - Root Abs: (%d, %d)\n"
        L" - Control1 Abs: (%d, %d)\n"
        L" - Control2 Abs: (%d, %d)",
        m_pRootControl->GetAbsX(), m_pRootControl->GetAbsY(),
        m_pTestControl1->GetAbsX(), m_pTestControl1->GetAbsY(),
        m_pTestControl2->GetAbsX(), m_pTestControl2->GetAbsY()
    );*/
    // �ѩ��r�O�h��A�ݭn���w�@��ø�s�e��
    m_pStatusText->SetSize(600, 200);

    // ����Ҧ������ø�s�ǳ�
    // �q�ڸ`�I�I�s�A�|���j��s�Ҧ��l����
    m_pRootControl->PrepareDrawing();
    m_pStatusText->PrepareDrawing();
}

void UIBasicTest::Render()
{
    if (!m_pRootControl) return;

    // ø�s UI ��
    m_pRootControl->Draw();

    // ø�s���A��r
    if (m_pStatusText) {
        m_pStatusText->Draw();
    }
}