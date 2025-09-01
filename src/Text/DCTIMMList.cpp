#include "Text/DCTIMMList.h"
#include <cassert>
#include <cstdint>
#include <algorithm>
#include "global.h"

DCTIMMList::DCTIMMList()
    : m_count(0),
    m_currentIndex(0),
    m_capacity(0),
    m_savedImc(nullptr),
    m_savedConv(0),
    m_savedSentence(8),
    m_firstReleaseFlag(1) {
    std::fill(std::begin(m_items), std::end(m_items), nullptr);
    std::memset(m_used, 0, sizeof(m_used));
    hWnd = g_hWnd;
}

DCTIMMList::~DCTIMMList() {
    // 反編譯：從 12 到 168(含) 每 4bytes 檢查並釋放，共 40 個
    for (int i = 0; i < 40; ++i) {
        if (m_items[i]) {
            delete m_items[i];     // 等價於呼叫 DCTIMM::~DCTIMM + operator delete
            m_items[i] = nullptr;
            if (m_count > 0) --m_count;
        }
        m_used[i] = 0;
    }
}

void DCTIMMList::Create(int capacity) {
    m_capacity = capacity;
    HIMC imc = ImmGetContext(hWnd);
    m_savedImc = imc;
    ImmReleaseContext(hWnd, imc);
}

void DCTIMMList::SetAlphaDigit(int index, int flag) {
    if (index < m_capacity && m_items[index]) {
        // 反編譯：*(DWORD*)(dctimm + 392) = a3;  (392/4 = +98)
        *reinterpret_cast<int*>(reinterpret_cast<char*>(m_items[index]) + 392) = flag;
    }
}

void DCTIMMList::SetIMMInfo(int index, unsigned int a3, unsigned short a4,
    int a5, RECT* a6, char a7, int a8, int a9) {
    if (index < m_capacity) {
        auto* p = new DCTIMM();                  // 等價 operator new(0x18C)+ctor
        m_items[index] = p;
        ++m_count;
        m_used[index] = 1;                       // +a2+43 = 1
        p->Init(0 /*unused*/, a4 /*maxlen*/, a5 /*mode*/,
            a6, a7, a8, a9);                 // 注意：此簽章依你現有 DCTIMM 定義
        // 反編譯原型：Init(this,a3,a4,a5,a6,a7,a8,a9)
        // 若你 DCTIMM::Init 簽章與這裡不同，請依你的版本對齊參數順序。
        // （我沿用你目前 DCTIMM 的還原說明）
        (void)a3; // 保留未用，僅為對齊反編譯介面
    }
}

void DCTIMMList::DeleteIMMInfo(int index) {
    if (index < m_capacity) {
        if (m_items[index]) {
            delete m_items[index];
            m_items[index] = nullptr;
            if (m_count > 0) --m_count;
        }
        m_used[index] = 0;
    }
}

int DCTIMMList::GetText(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam) {
    if (m_capacity > 0 && m_items[m_currentIndex])
        return m_items[m_currentIndex]->Poll(hwnd, uMsg, wParam, lParam);
    return 1;
}

void DCTIMMList::GetIMMText(int index, LPSTR out, int out_size) {
    if (index < m_capacity && m_items[index])
        m_items[index]->GetIMMText(out, out_size);
}

void DCTIMMList::SetIMMText(int index, char* text) {
    if (index < m_capacity && m_items[index])
        m_items[index]->SetIMMText(text);
}

void DCTIMMList::SetAutoDelete(int index, int flag) {
    if (index < m_capacity && m_items[index]) {
        // *(DWORD*)(dctimm + 308) = flag; (308/4 = +77) → m_bShouldClearBuffer
        *reinterpret_cast<int*>(reinterpret_cast<char*>(m_items[index]) + 308) = flag;
    }
}

void DCTIMMList::SetActive(int index, int active, HWND hwnd_for_dctimm) {
    if (index >= m_capacity) return;
    m_currentIndex = index;

    if (!m_items[index]) return;

    if (active) {
        // 啟用：掛回原本的 IME context，並恢復之前儲存的 Conversion/Sentence
        ImmAssociateContext(hWnd, m_savedImc);
        HIMC h = ImmGetContext(hWnd);
        ImmSetConversionStatus(h, m_savedConv, m_savedSentence);
        ImmReleaseContext(hWnd, h);
    }
    else {
        // 停用：讀取目前 Conversion/Sentence，第一次釋放把 sentence 設為 8，解除 IME
        HIMC h = ImmGetContext(hWnd);
        ImmGetConversionStatus(h, &m_savedConv, &m_savedSentence);
        if (m_firstReleaseFlag) {
            m_savedSentence = 8;
            m_firstReleaseFlag = 0;
        }
        ImmReleaseContext(hWnd, h);
        ImmAssociateContext(hWnd, nullptr);
        OutputDebugStringA("#### IME Release All ####\n");
    }
    m_items[index]->SetActive(active, hwnd_for_dctimm);
}

int DCTIMMList::IsActive(int index) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->IsActive();
    return 0;
}

int DCTIMMList::IsActive() const {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i])
        return m_items[i]->IsActive();
    return 0;
}

HWND DCTIMMList::GetNative(int index, HWND h) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->GetNative(h);
    return nullptr;
}

HWND DCTIMMList::GetNative(HWND h) const {
    int i = m_currentIndex;
    if (i >= m_capacity) return nullptr;
    if (m_items[i]) return m_items[i]->GetNative(h);
    return nullptr;
}

RECT DCTIMMList::GetRect(int index) const {
    RECT rc{};
    if (index < m_capacity && m_items[index]) {
        m_items[index]->GetRect(&rc);
    }
    else {
        rc = RECT{ 0,0,0,0 };
    }
    return rc;
}

unsigned int DCTIMMList::GetSelectedBlockLen(int index) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->GetSelectedBlockLen();
    return static_cast<unsigned int>(-1);
}

void DCTIMMList::SetRect(int index, RECT rc) {
    if (index < m_capacity && m_items[index])
        m_items[index]->SetRect(rc);
}

int DCTIMMList::IsComposition(int index) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->IsComposition();
    return 0;
}

int DCTIMMList::IsComposition() const {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i])
        return m_items[i]->IsComposition();
    return 0;
}

void DCTIMMList::ResetBegineComposition() {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i]) {
        // *(DWORD*)(dctimm + 320) = 0;  (320/4 = +80)
        *reinterpret_cast<int*>(reinterpret_cast<char*>(m_items[i]) + 320) = 0;
    }
}

int DCTIMMList::GetBegineComposition() const {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i]) {
        return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(m_items[i]) + 320);
    }
    return 0;
}

int DCTIMMList::GetCandidate() const {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i]) {
        return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(m_items[i]) + 324); // +81
    }
    return 0;
}

int DCTIMMList::GetEditPosition(int index) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->GetEditPosition(); // 等價 *(_DWORD*)(dctimm + 232)
    return -1;
}

int DCTIMMList::SetEditPosition(int index, int pos) {
    if (index < m_capacity && m_items[index])
        return m_items[index]->SetEditPosition(pos);
    return -1;
}

void DCTIMMList::SetTextSize(int index, int new_max_len, int new_mode) {
    if (index < m_capacity && m_items[index])
        m_items[index]->SetTextSize(new_max_len, new_mode);
}

void DCTIMMList::SetEditKey(int index, unsigned int uMsg, unsigned int wParam) {
    if (index < m_capacity && m_items[index]) {
        // 反編譯呼叫 GetText(v4, 0, a3, TestChar, 0)
        // 由於 GetText 非公開，改用 Poll(0, a3, TestChar, 0) 等價
        (void)m_items[index]->Poll(nullptr, uMsg, wParam, 0);
    }
}

int DCTIMMList::GetUsableIMEIndex() const {
    // 從 m_used[0] 起掃描第一個為 0 的位置；滿 40 回傳 -1
    for (int i = 0; i < 40; ++i) {
        if (m_used[i] == 0) return i;
    }
    return -1;
}

void DCTIMMList::SkipComposition(int index) {
    if (index < m_capacity && m_items[index])
        m_items[index]->SkipComposition();
}

void DCTIMMList::ChangeLanguage(int index, int to_native, HWND hwnd) {
    if (index < m_capacity && m_items[index])
        m_items[index]->ChangeLanguage(to_native, hwnd);
}

void DCTIMMList::ChangeLanguage(int to_native, HWND hwnd) {
    int i = m_currentIndex;
    if (i < m_capacity && m_items[i])
        m_items[i]->ChangeLanguage(to_native, hwnd);
}

int DCTIMMList::GetBlockStartPos(int index) const {
    if (index < m_capacity && m_items[index])
        return m_items[index]->GetBlockStartPos();
    return -1;
}

void DCTIMMList::SetHWND(int index, HWND hwnd) {
    if (index < m_capacity && m_items[index]) {
        // *(DWORD*)(dctimm + 344) = hwnd; (344/4 = +86)
        *reinterpret_cast<HWND*>(reinterpret_cast<char*>(m_items[index]) + 344) = hwnd;
    }
}

void DCTIMMList::SetBlockPos(int index, int pos) {
    if (index < m_capacity && m_items[index])
        m_items[index]->SetBlockPos(pos);
}

void DCTIMMList::ClearBlockPos(int index) {
    if (index < m_capacity && m_items[index])
        m_items[index]->ClearBlockPos();
}

void DCTIMMList::GetUnderLine(int index, int* start, int* end) {
    if (index < m_capacity && m_items[index])
        m_items[index]->GetUnderLine(start, end);
}

char* DCTIMMList::GetCompositionString(int index) const {
    if (index < m_capacity && m_items[index]) {
        // return dctimm + 4;
        return reinterpret_cast<char*>(m_items[index]) + 4;
    }
    return nullptr;
}

int DCTIMMList::GetCharSet(int index) const {
    if (index < m_capacity && m_items[index]) {
        // 反編譯：DCTIMM::GetCharsetFromLang(dctimm, *(WORD*)(dctimm + 169*2))
        // 這裡不直接呼叫 DCTIMM 私有方法，改用等價的靜態方法。
        unsigned short lang = *reinterpret_cast<unsigned short*>(
            reinterpret_cast<char*>(m_items[index]) + (169 * 2));
        return _GetCharsetFromLang(lang);
    }
    return 0;
}

int DCTIMMList::GetComCursorPos(int index) const {
    if (index < m_capacity && m_items[index]) {
        // *(DWORD*)(dctimm + 340)  (340/4 = +85)
        return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(m_items[index]) + 340);
    }
    return 0;
}

// === 等價邏輯：取自你現有 DCTIMM 的對應實作 ===
int DCTIMMList::_GetCharsetFromLang(unsigned short a2) {
    switch (a2 & 0x03FF) {
    case 1:   return 178;
    case 2: case 0x19: case 0x22: case 0x23: return 204;
    case 4:
        if (a2 >> 10 == 1) return 136; // 繁中
        else if (a2 >> 10 == 2) return 134; // 簡中
        else return 0;
    case 5: case 0xE: case 0x15: case 0x18: case 0x1A: case 0x1B: case 0x24: case 0x2F:
        return 238;
    case 8:   return 161;
    case 0xD: return 177;
    case 0x11: return 128; // Shift-JIS
    case 0x12: return 129; // 韓文
    case 0x1E: return 222;
    case 0x1F: return 162;
    case 0x25: case 0x26: case 0x27: return 186;
    default:  return 0;
    }
}

int DCTIMMList::_GetCodePageFromCharset(int cs) {
    switch (cs) {
    case 128: return 932;
    case 129: return 949;
    case 134: return 936;
    case 136: return 950;
    case 161: return 1253;
    case 162: return 1254;
    case 177: return 1255;
    case 178: return 1256;
    case 186: return 1257;
    case 222: return 874;
    case 238: return 1250;
    default:  return 1252;
    }
}
