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
    // 依序安全地刪除所有 UI 物件
    // 由於父子關係僅是邏輯鏈結，記憶體需要手動管理
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
    // --- 測試目標 ---
    // 1. CControlBase::Create(): 建立控制項並設定父子關係。
    // 2. CControlBase::SetPos() / SetAbsPos(): 驗證相對與絕對座標。
    // 3. CControlBase::GetAbsX/Y(): 驗證絕對座標的計算。
    // 4. 階層結構：父物件移動時，子物件應跟著移動。

    // 1. 建立一個半透明的根容器 (灰色)，方便觀察其範圍
    m_pRootControl = new CControlAlphaBox();
    m_pRootControl->Create(300, 100, 400, 300, 0.5f, 0.5f, 0.5f, 0.5f, nullptr);

    // 2. 建立測試控制項1 (紅色)，掛在 Root底下。座標 (50, 50) 是相對於 Root 的。
    m_pTestControl1 = new CControlAlphaBox();
    m_pTestControl1->Create(50, 50, 200, 80, 1.0f, 0.0f, 0.0f, 1.0f, m_pRootControl);

    // 3. 建立測試控制項2 (藍色)，掛在 TestControl1 底下。座標 (20, 20) 是相對於 TestControl1 的。
    m_pTestControl2 = new CControlAlphaBox();
    m_pTestControl2->Create(20, 20, 100, 40, 0.0f, 0.0f, 1.0f, 1.0f, m_pTestControl1);

    // 4. 建立一個文字控制項，用於顯示狀態資訊
    m_pStatusText = new CControlText();
    m_pStatusText->Create(10, 10, nullptr); // 掛在根底下 (nullptr)
    m_pStatusText->SetTextColor(0xFFFFFFFF); // 白色
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

    // --- 測試目標 ---
    // 1. 動態修改座標：驗證 SetX/SetY 和 SetAbsX/SetAbsY。
    // 2. 動態顯隱：驗證 Show() / Hide()。
    // 3. 狀態回讀：使用 GetAbsX/Y 讀取座標並顯示，確認其正確性。

    // 讓紅色方塊 (m_pTestControl1) 水平來回移動，測試 SetX()
    // 由於藍色方塊是它的子物件，應該會跟著一起動
    float newRelativeX = 50.0f + sin(m_fTotalTime) * 40.0f;
    m_pTestControl1->SetX(static_cast<int>(newRelativeX));

    // 讓藍色方塊 (m_pTestControl2) 每 2 秒閃爍一次，測試 SetVisible()
    bool isVisible = static_cast<int>(m_fTotalTime) % 2 == 0;
    m_pTestControl2->SetVisible(isVisible);

    //m_pStatusText->SetText(4751);
    // 更新狀態文字，顯示各控制項的絕對座標
    //m_pStatusText->SetTextW(L"紅色方塊(Control1) X 座標正在以 sin() 變化");
    
    m_pStatusText->SetTextFmtW(
        L"CControlBase 測試:\n"
        L" - 紅色方塊 (Control1) X 座標正在以 sin() 變化。\n"
        L" - 藍色方塊 (Control2) 是紅色方塊的子項，應跟隨移動，且每秒閃爍。\n\n"
        L"即時座標:\n"
        L" - Root Abs: (%d, %d)\n"
        L" - Control1 Abs: (%d, %d)\n"
        L" - Control2 Abs: (%d, %d)",
        m_pRootControl->GetAbsX(), m_pRootControl->GetAbsY(),
        m_pTestControl1->GetAbsX(), m_pTestControl1->GetAbsY(),
        m_pTestControl2->GetAbsX(), m_pTestControl2->GetAbsY()
    );
    // 由於文字是多行，需要給定一個繪製寬度
    m_pStatusText->SetSize(600, 200);

    // 執行所有控制項的繪製準備
    // 從根節點呼叫，會遞迴更新所有子物件
    m_pRootControl->PrepareDrawing();
    m_pStatusText->PrepareDrawing();
}

void UIBasicTest::Render()
{
    if (!m_pRootControl) return;

    // 繪製 UI 樹
    m_pRootControl->Draw();

    // 繪製狀態文字
    if (m_pStatusText) {
        m_pStatusText->Draw();
    }
}