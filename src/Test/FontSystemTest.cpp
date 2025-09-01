#include "Test/FontSystemTest.h"
#include "Font/CMoFFontTextureManager.h"
#include "Font/FileCrypt.h"
#include <windows.h> // for RECT
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include "global.h"

FontSystemTest::FontSystemTest() : m_pFont(nullptr) {
}

FontSystemTest::~FontSystemTest() {
    // 確保資源被釋放
    if (m_pFont) {
        delete m_pFont;
        m_pFont = nullptr;
    }
    // 重設紋理管理器
    CMoFFontTextureManager::GetInstance()->Reset();
}

HRESULT FontSystemTest::Initialize() {
    printf("--- [Font Test] 開始初始化 ---\n");

    // 3. 建立 MoFFont 實體並初始化
    m_pFont = new MoFFont();
    if (!m_pFont) return E_OUTOFMEMORY;

    if (!m_pFont->InitFontInfo("mofdata/fontinfo.dat")) {
        MessageBoxA(NULL, "MoFFont::InitFontInfo 失敗!", "錯誤", MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    printf("  字型資訊 FontInfo.dat 載入成功。\n");

    // 4. 建立預設要使用的字型
    if (!m_pFont->CreateMoFFont(g_pd3dDevice, "MiniGameMyRank")) {
        MessageBoxA(NULL, "MoFFont::CreateMoFFont 失敗!", "錯誤", MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    printf("  預設字型 建立成功。\n");

    // 5. 測試 GetTextLength
    printf("--- [Font Test] 測試文字測量 ---\n");
    int width = 0, height = 0;
    m_pFont->GetTextLength(&width, &height, "BossMonsterName", "Measure This Text!");
    printf("  'Measure This Text!' (Title 字型) -> 寬度: %d, 高度: %d\n", width, height);

    printf("--- [Font Test] 初始化完成 ---\n");
    return S_OK;
}

static std::wstring KR_to_wide(const char* bytes) {
    if (!bytes) return L"";
    const UINT cps[] = { 51949, 949 };
    for (UINT cp : cps) {
        int n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, bytes, -1, nullptr, 0);
        if (n > 0) {
            std::wstring w(n, L'\0');
            MultiByteToWideChar(cp, 0, bytes, -1, &w[0], n);
            if (!w.empty() && w.back() == L'\0') w.pop_back();
            return w;
        }
    }
    return std::wstring(bytes, bytes + std::strlen(bytes)); // ASCII 後援
}

static std::string wide_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

void FontSystemTest::Update(float fElapsedTime) {
    if (m_pFont) {
        // 每幀呼叫資源管理器，模擬遊戲循環中的快取清理
        m_pFont->MoFFontResourceManager();
    }
}

void FontSystemTest::Render() {
    if (!m_pFont) return;

    // --- 測試案例 ---
    m_pFont->SetBlendType(1);
    // 1. 基本單行文字 (使用預設字型 CharacterName)
    m_pFont->SetTextLine(50, 50, 0xFFFFFFFF, "Hello, MoFFont! This is the 'CharacterName' font (Tahoma 16).");

    // 2. 文字對齊
    m_pFont->SetTextLine(640, 90, 0xFF00FF00, "Centered Text", 1); // alignment=1 (中)
    m_pFont->SetTextLine(1230, 90, 0xFFFF0000, "Right Aligned", 2); // alignment=2 (右)
    auto str = g_DCTTextManager.GetText(4751);

    m_pFont->SetTextLine(1230, 90, 0xFFFF0000, wide_to_utf8(KR_to_wide(str)).c_str(), 2); // alignment=2 (右)

    // 3. 更換字型並繪製帶陰影的標題
    m_pFont->SetTextLineShadow(50, 150, 0xFF303030, "This is a Title with Shadow", 0);
    m_pFont->SetTextLineA(50, 150, 0xFFFFFF00, "This is a Title with Shadow", 0);

    // 4. TextBox 自動換行測試
    m_pFont->SetFont("Notice");
    RECT textBox = { 50, 220, 550, 400 }; // left, top, right, bottom

    // 修正：使用字串拼接，確保所有字串在同一行或使用括號
    const char* longText = "This is a long string of text inside a RECT to demonstrate the automatic "
        "word-wrapping capabilities of the SetTextBox function. It should break into "
        "multiple lines correctly.";

    m_pFont->SetTextBox(&textBox, 0xFF00FFFF, longText, 5, 0); // 5px 行距

    // 5. 測試快取: 多次繪製相同文字
    //m_pFont->SetFont("Small");
    m_pFont->SetTextLine(50, 420, 0xFFDDDDDD, "This line is drawn twice to test the cache system. (1st time)");
    m_pFont->SetTextLine(50, 420, 0xFFDDDDDD, "This line is drawn twice to test the cache system. (2nd time)");
    m_pFont->SetTextLine(50, 440, 0xFFAAAAAA, "This is another string to ensure a new cache entry is created.");

    // 6. 測試多國語言文字 (DBCS)
    //m_pFont->SetFont("Normal");
    const char* chineseWelcome = "中文測試: 歡迎來到奇幻世界";
    const char* japaneseWelcome = "日本語テスト：ファンタジーの世界へようこそ";
    m_pFont->SetTextLine(50, 500, 0xFFFFFFAA, chineseWelcome);
    m_pFont->SetTextLineA(50, 530, 0xFFAAFFAA, japaneseWelcome);
}