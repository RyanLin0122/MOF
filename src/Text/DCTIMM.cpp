#include "Text/DCTIMM.h"
#include "Font/MoFFont.h" // 假設 MoFFont.h 提供了 g_MoFFont 和 IsDBCSLeadByte 的宣告
#include <cstring>
#include <cstdlib>
#include "global.h"

DCTIMM::DCTIMM() {
    // 只清空主文字指標，其他欄位維持未定（交由 Init() 設定）
    m_pTextBuffer = nullptr;
}

DCTIMM::~DCTIMM() {
    if (m_pTextBuffer) {
        ::operator delete(m_pTextBuffer); // 對齊反編譯碼
        m_pTextBuffer = nullptr;
    }
}

void DCTIMM::Init(unsigned int /*a2*/, unsigned short max_length, int mode,
    RECT* rect, char multi_line, int line_space, int align) {
    if (m_pTextBuffer) {
        ::operator delete(m_pTextBuffer);
        m_pTextBuffer = nullptr;
    }
    // 對齊：operator new(a3 + 2) 並清 0
    m_pTextBuffer = static_cast<char*>(::operator new(max_length + 2));
    std::memset(m_pTextBuffer, 0, max_length + 2);

    // 對齊：this+4(=DWORD), this+8(=DWORD), this+12(=WORD) 清 0，再把 this+14 長度 0xC8 清 0
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 8) = 0;
    *reinterpret_cast<WORD*>(reinterpret_cast<char*>(this) + 12) = 0;
    std::memset(reinterpret_cast<char*>(this) + 14, 0, 0xC8);

    // 其餘欄位按 IDA 初值
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 56 * 4) = 0; // +56
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 57 * 4) = max_length; // +57
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 54 * 4) = 0; // +54
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 55 * 4) = 0; // +55
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 77 * 4) = 0; // +77
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 58 * 4) = 0; // edit pos

    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 74 * 4) = 0; // composing
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 75 * 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 76 * 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 87 * 4) = 0;

    // 模式（關鍵）：+78 = a4
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 78 * 4) = mode;

    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 94 * 4) = 0; // Ctrl flag

    if (rect) {
        *reinterpret_cast<LONG*>(reinterpret_cast<char*>(this) + 88 * 4) = rect->left;
        *reinterpret_cast<LONG*>(reinterpret_cast<char*>(this) + 89 * 4) = rect->top;
        *reinterpret_cast<LONG*>(reinterpret_cast<char*>(this) + 90 * 4) = rect->right;
        *reinterpret_cast<LONG*>(reinterpret_cast<char*>(this) + 91 * 4) = rect->bottom;
    }
    else {
        RECT z{};
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 88 * 4) = z.left;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 89 * 4) = z.top;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 90 * 4) = z.right;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 91 * 4) = z.bottom;
    }
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 96 * 4) = 0; // Skip flag

    // 參數綁定：+76 = a7（行距），+92 = a8（對齊），+316 = a6（是否多行）
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 76 * 4) = line_space;
    *reinterpret_cast<BYTE*>(reinterpret_cast<char*>(this) + 316) = static_cast<BYTE>(multi_line);
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 92 * 4) = align;

    // 區塊選取
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 70 * 4) = -1; // start
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 71 * 4) = -1; // end
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 72 * 4) = 0;  // selecting
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 73 * 4) = 0;  // fromRight
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 69 * 4) = 0;  // line count?（保留）
    std::memset(reinterpret_cast<char*>(this) + 236, 0, 0x28); // +236 長度 0x28 清 0

    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 95 * 4) = 0; // active
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 80 * 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 81 * 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 85 * 4) = 0;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 98 * 4) = 0; // password flag

    m_bIsMultiLine = static_cast<BYTE>(multi_line);
}

bool DCTIMM::RightAsCstr() {
    // +72: m_bIsBlockSelecting
    if (!m_bIsBlockSelecting) return false;
    // +73: m_bBlockSelectFromRight
    if (m_bBlockSelectFromRight == 1) return false;
    // +70: m_nBlockStart ； +58: m_nEditPosition
    m_nBlockStart = m_nEditPosition;
    return true;
}

unsigned int DCTIMM::GetSelectedBlockLen() {
    int e = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 71 * 4);
    int s = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 70 * 4);
    if (e == -1 || s == -1) return static_cast<unsigned int>(-1);
    return static_cast<unsigned int>(std::abs(e - s));
}


int DCTIMM::GetBlockStartPos()
{
    if (m_nBlockStart > m_nBlockEnd)
        return m_nBlockEnd;
    return m_nBlockStart;
}

void DCTIMM::SetBlockPos(int pos)
{
    if (m_bIsBlockSelecting)
    {
        m_nBlockEnd = pos;
    }
    else
    {
        m_nBlockStart = -1;
        m_nBlockEnd = -1;
    }
}

void DCTIMM::ClearBlockPos()
{
    m_nBlockStart = -1;
    m_nBlockEnd = -1;
    m_bIsBlockSelecting = 0;
    m_bBlockSelectFromRight = 0;
}

int DCTIMM::SetEditPosition(int pos)
{
    if (JudgeMultiStr(m_pTextBuffer, pos) == 2)
    {
        pos += 1;
    }
    m_nEditPosition = pos;
    return pos;
}

void DCTIMM::SetTextSize(int new_max_len, int new_mode) {
    if (m_pTextBuffer) {
        ::operator delete(m_pTextBuffer);
        m_pTextBuffer = nullptr;
    }
    m_pTextBuffer = static_cast<char*>(::operator new(new_max_len + 2));
    std::memset(m_pTextBuffer, 0, new_max_len + 2);

    // +57 = 長度，+78 = 模式（不是字型）
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 57 * 4) = new_max_len;
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 78 * 4) = new_mode;

    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 58 * 4) = 0; // edit pos
    ClearBlockPos();
}

int DCTIMM::Poll(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam) {
    if (*reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 78 * 4) == 1)
        return GetNumeric(hwnd, uMsg, wParam, lParam);
    return GetText(hwnd, uMsg, wParam, lParam);
}

int DCTIMM::GetNumeric(HWND, unsigned int uMsg, unsigned int wParam, int) {
    if (!IsActive()) return 1;

    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_LEFT) {
            int pos = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4);
            if (pos > 0) *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = pos - 1;
        }
        else if (wParam == VK_RIGHT) {
            int pos = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4);
            int len = std::strlen(m_pTextBuffer);
            int maxL = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 57 * 4);
            if (pos < len && pos < maxL) *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = pos + 1;
        }
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 74 * 4) = 0; // 清 composing
        return 1;
    }

    if (uMsg != WM_CHAR) return 1;

    if (wParam == VK_BACK) {
        int pos = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4);
        if (pos > 0) {
            unsigned char kind = JudgeMultiStr(m_pTextBuffer, pos - 1);
            int del = (kind == 2) ? 2 : 1;
            std::memmove(&m_pTextBuffer[pos - del], &m_pTextBuffer[pos],
                std::strlen(m_pTextBuffer) - pos + 1);
            *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = pos - del;

            // 等價於 _wsprintfA((LPSTR)this + 4, &`string')
            *reinterpret_cast<char*>(reinterpret_cast<char*>(this) + 4) = '\0';
        }
        return 1;
    }

    if (wParam >= '0' && wParam <= '9') {
        int len = static_cast<int>(std::strlen(m_pTextBuffer));
        int maxL = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 57 * 4);
        if (len < maxL) {
            int pos = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4);
            InsertChar(m_pTextBuffer, static_cast<char>(wParam), pos);
            *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = pos + 1;
        }
    }
    return 1;
}

int DCTIMM::GetText(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam) {
    if (!hwnd) hwnd = *reinterpret_cast<HWND*>(reinterpret_cast<char*>(this) + 86 * 4);
    if (!IsActive()) return 1;

    // 若 +77 && +87，清空（等價於原碼）
    if (*reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 77 * 4) &&
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 87 * 4)) {
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = 0;
        *reinterpret_cast<char*>(reinterpret_cast<char*>(this) + 4) = '\0'; // 清 this+4
        if (m_pTextBuffer) m_pTextBuffer[0] = '\0';
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 87 * 4) = 0;
        ClearBlockPos();
    }

    if (uMsg > 0x10D) {               // > WM_IME_STARTCOMPOSITION
        if (uMsg >= 0x281) return 1;   // WM_IME_SETCONTEXT 以上 => 1
        if (uMsg == 0x10E) return 0;   // WM_IME_ENDCOMPOSITION => 0
        if (uMsg != 0x10F) return 1;   // 非 WM_IME_COMPOSITION => 1

        // WM_IME_COMPOSITION：受國別碼/密碼旗標限制
        // if (MoFFont::GetNationCode(&g_MoFFont) != 4 && *((DWORD*)this+98) != 1) ...
        OnIMEKeyComposition(hwnd, wParam, lParam);
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 80 * 4) = 1;
        return 0;
    }
    else {
        if (uMsg != 269 /*WM_IME_STARTCOMPOSITION*/) {
            switch (uMsg) {
            case 0x51 /*WM_INPUTLANGCHANGE*/:
                *reinterpret_cast<WORD*>(reinterpret_cast<char*>(this) + 168 * 2)
                    = GetCodePageFromLang(static_cast<unsigned short>(lParam));
                *reinterpret_cast<WORD*>(reinterpret_cast<char*>(this) + 169 * 2)
                    = static_cast<unsigned short>(lParam);
                return 0;
            case WM_KEYDOWN:
                OnIMEKeyDown(hwnd, wParam, lParam);
                return 0;
            case WM_KEYUP:
                if (wParam == VK_SHIFT)
                    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 72 * 4) = 0; // 停止選取
                if (wParam == VK_CONTROL)
                    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 94 * 4) = 0; // Ctrl off
                return 1;
            case WM_CHAR:
                OnIMEKeyChar(hwnd, wParam, lParam);
                return 0;
            default:
                return 1;
            }
        }
        // WM_IME_STARTCOMPOSITION
        *reinterpret_cast<BYTE*>(reinterpret_cast<char*>(this) + 4) = 0;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 80 * 4) = 1;
        return 0;
    }
}

void DCTIMM::GetIMMText(char* out, int out_size) {
    std::memset(out, 0, out_size);
    if (m_pTextBuffer) std::strcpy(out, m_pTextBuffer);

    // 若 this+4（組字暫存）有東西，插入到編輯位置
    if (std::strlen(reinterpret_cast<char*>(this) + 4) > 0) {
        int pos = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4);
        InsertString(out, reinterpret_cast<char*>(this) + 4, pos);
    }

    // 若 +77 為真：清空兩個緩衝、editpos=0，+87=1，清選取
    if (*reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 77 * 4)) {
        if (m_pTextBuffer) m_pTextBuffer[0] = '\0';
        *reinterpret_cast<BYTE*>(reinterpret_cast<char*>(this) + 4) = 0;
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58 * 4) = 0;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 87 * 4) = 1;
        ClearBlockPos();
    }
}

void DCTIMM::SetIMMText(char* Source) {
    if (*reinterpret_cast<void**>(this)) {
        int maxL = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 57*4);
        std::memset(m_pTextBuffer, 0, maxL);
        std::strncpy(m_pTextBuffer, Source, maxL);
        *reinterpret_cast<BYTE*>(reinterpret_cast<char*>(this) + 4) = 0; // 清 this+4
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 58*4) = std::strlen(m_pTextBuffer);
    }
}

void DCTIMM::SetActive(int a2, HWND) {
    *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 95 * 4) = a2;
    if (a2 == 1) {
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 94 * 4) = 0;
    }
    else {
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 81 * 4) = 0;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 80 * 4) = 0;
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(this) + 85 * 4) = 0;
    }
}

// =======================
// 1) OnInputLangChange
// =======================
void DCTIMM::OnInputLangChange(unsigned short lang_id)
{
    // 對齊 0051F960：先求碼頁，再設語系
    m_usCodePage = GetCodePageFromLang(lang_id);
    m_usLangId = lang_id;
}

// =======================
// 2) GetCandi
// =======================
// msg_type: 3=IMN_CHANGECANDIDATE(增量串接，從 m_nCandListStartIdx 開始取 9 個)
//           4=清空候選串接區第一個字元
//           5=依 a5（起始索引）往後取 9 個，並更新 m_nCandListCount 與 m_nCandListStartIdx=0
void DCTIMM::GetCandi(HWND hwnd, unsigned int msg_type, int a4, int a5)
{
    HIMC hImc = ImmGetContext(hwnd);
    if (!hImc) return;

    switch (msg_type)
    {
    case 3: // 追加顯示：從 m_nCandListStartIdx 開始往後取 9 個
    {
        std::memset(m_szCandidateList, 0, sizeof(m_szCandidateList));

        DWORD need = ImmGetCandidateListA(hImc, 0, nullptr, 0);
        if (need)
        {
            auto pList = (CANDIDATELIST*)::operator new(need);
            ImmGetCandidateListA(hImc, 0, pList, need);

            int start = m_nCandListStartIdx;
            size_t used = 0;
            for (int i = start; i < (int)pList->dwCount && i < start + 9; ++i)
            {
                const char* s = (const char*)((BYTE*)pList + pList->dwOffset[i]);
                size_t sl = std::strlen(s);
                if (used + sl + 1 >= sizeof(m_szCandidateList)) break;
                std::strcpy(&m_szCandidateList[used], s);
                used += sl;
            }
            ::operator delete(pList);
        }
        break;
    }
    case 4: // 清空（其實是把第一個 byte 設 0）
        m_szCandidateList[0] = '\0';
        break;

    case 5: // 以 a5 為起點，取 9 個；順便更新總數 m_nCandListCount 與 m_nCandListStartIdx=0
    {
        std::memset(m_szCandidateList, 0, sizeof(m_szCandidateList));
        DWORD need = ImmGetCandidateListA(hImc, 0, nullptr, 0);
        if (need)
        {
            auto pList = (CANDIDATELIST*)::operator new(need);
            ImmGetCandidateListA(hImc, 0, pList, need);

            m_nCandListCount = (int)pList->dwCount;

            int start = a5;
            size_t used = 0;
            for (int i = start; i < (int)pList->dwCount && i < start + 9; ++i)
            {
                const char* s = (const char*)((BYTE*)pList + pList->dwOffset[i]);
                size_t sl = std::strlen(s);
                if (used + sl + 1 >= sizeof(m_szCandidateList)) break;
                std::strcpy(&m_szCandidateList[used], s);
                used += sl;
            }

            ::operator delete(pList);
            m_nCandListStartIdx = 0;
        }
        break;
    }
    default:
        break;
    }

    ImmReleaseContext(hwnd, hImc);
}

// =======================
// 3) OnIMEKeyComposition
// =======================
// lParam flags: GCS_RESULTSTR(0x800), GCS_COMPSTR(0x8), GCS_COMPATTR(0x10), GCS_CURSORPOS(0x80)
void DCTIMM::OnIMEKeyComposition(HWND hwnd, unsigned int /*wParam*/, int lParam)
{
    // 首先把選取區塊刪掉（與 0051FE10 開頭一致）
    DelStrByBlockSelect();

    // 預留 2 個字節空間（反編譯碼 <= m_nMaxLength-2）
    if ((int)std::strlen(m_pTextBuffer) > m_nMaxLength - 2)
        return;

    HIMC hImc = ImmGetContext(hwnd);
    if (!hImc) return;

    // 若設定過「跳過這次組字」，則把內部狀態清掉並結束（對齊 +96）
    if (m_nUnk_96) {
        m_nUnk_96 = 0;
        if (m_pTextBuffer) m_pTextBuffer[0] = '\0';
        // this+4 的組字暫存與旗標
        m_szCompositionStr[0] = '\0';
        m_nEditPosition = 0;
        m_bIsComposing = 0;
        ImmReleaseContext(hwnd, hImc);
        return;
    }

    // 1) 有 RESULTSTR 時，先把結果字串插入主緩衝
    if (lParam & GCS_RESULTSTR)
    {
        LONG need = ImmGetCompositionStringA(hImc, GCS_RESULTSTR, nullptr, 0);
        if (need > 0)
        {
            // 避免超長
            int can_add = m_nMaxLength - (int)std::strlen(m_pTextBuffer);
            if (need > can_add) need = can_add;

            // 把 RESULTSTR 讀到 this+4（即 m_szCompositionStr），注意 NUL
            ImmGetCompositionStringA(hImc, GCS_RESULTSTR, m_szCompositionStr, need);
            m_szCompositionStr[need] = '\0';

            // 插入到當前游標
            InsertString(m_pTextBuffer, m_szCompositionStr, m_nEditPosition);
            m_nEditPosition += (int)std::strlen(m_szCompositionStr);

            // 清除屬性/狀態（對齊原碼）
            m_nUnk_81 = 0;          // 無底線範圍
            m_bIsComposing = 0;     // 不在組字
            m_szCompositionStr[0] = '\0';
        }
    }

    // 2) 有 COMPSTR 時，把組字字串放到 this+4，並且設定「正在組字」旗標
    if (lParam & GCS_COMPSTR)
    {
        LONG need = ImmGetCompositionStringA(hImc, GCS_COMPSTR, nullptr, 0);
        if (need + (LONG)std::strlen(m_pTextBuffer) > m_nMaxLength) {
            // 超過最大長度就忽略本次（對齊原碼 return）
            ImmReleaseContext(hwnd, hImc);
            return;
        }

        ImmGetCompositionStringA(hImc, GCS_COMPSTR, m_szCompositionStr, need);
        m_szCompositionStr[need] = '\0';

        m_bIsComposing = (m_szCompositionStr[0] != '\0') ? 1 : 0;
    }
    else {
        // 沒有 COMPSTR，清掉暫存並關閉組字狀態
        m_szCompositionStr[0] = '\0';
        m_bIsComposing = 0;
    }

    // 3) 有 COMPATTR 時，取出底線起止範圍（找第一段 attr==1 或 3）
    if (lParam & GCS_COMPATTR)
    {
        LONG need = ImmGetCompositionStringW(hImc, GCS_COMPATTR, nullptr, 0);
        if (need > 0)
        {
            std::string attr;
            attr.resize((size_t)need);
            ImmGetCompositionStringW(hImc, GCS_COMPATTR, attr.data(), need);

            int i = 0;
            for (; i < need; ++i) {
                unsigned char a = (unsigned char)attr[(size_t)i];
                if (a == 1 || a == 3) break;
            }
            int j = i;
            for (; j < need; ++j) {
                if ((unsigned char)attr[(size_t)j] != (unsigned char)attr[(size_t)i]) break;
            }
            m_nCompAttrStart = i;
            m_nCompAttrLen = j;
            m_nUnk_81 = 1; // 有底線
        }
    }

    // 4) 取組字游標位置（若 lParam 最高位有設，對齊原碼的 (<char>lParam<0)）
    if (lParam & GCS_CURSORPOS)
    {
        LONG pos = ImmGetCompositionStringW(hImc, GCS_CURSORPOS, nullptr, 0);
        m_nUnk_85 = (int)pos;
    }

    ImmReleaseContext(hwnd, hImc);
}

// =======================
// 4) OnIMEKeyDown
// =======================
void DCTIMM::OnIMEKeyDown(HWND hwnd, unsigned int wParam, int lParam)
{
    const int len = (int)std::strlen(m_pTextBuffer);

    // 若未在 Shift 選取，任何操作前清空選取端點（對齊：+72==0 時 +70/+71=-1）
    if (!m_bIsBlockSelecting) {
        m_nBlockStart = -1;
        m_nBlockEnd = -1;
    }

    switch (wParam)
    {
    case VK_BACK:
    {
        // 先處理「刪除選取」
        unsigned int blk = GetSelectedBlockLen();
        if (blk != (unsigned)-1 && (int)blk > 0)
        {
            int spos = GetBlockStartPos();
            std::memmove(&m_pTextBuffer[spos],
                &m_pTextBuffer[spos + blk],
                len - spos - blk + 1);
            m_nEditPosition = spos;
            m_bIsComposing = 0;
            ClearBlockPos();
            break;
        }

        // 沒有選取：刪除一個（或一個 DBCS）
        if (m_nEditPosition > 0)
        {
            unsigned char kind = JudgeMultiStr(m_pTextBuffer, m_nEditPosition - 1); // 2=DBCS trail
            int del = (kind == 2) ? 2 : 1;
            if (m_nEditPosition - del >= 0)
            {
                std::memmove(&m_pTextBuffer[m_nEditPosition - del],
                    &m_pTextBuffer[m_nEditPosition],
                    len - m_nEditPosition + 1);
                m_nEditPosition -= del;

                // 清掉 this+4（組字暫存），對齊原碼的 _wsprintfA(...,&`string')
                m_szCompositionStr[0] = '\0';
            }
        }
        break;
    }

    case VK_SHIFT:
        if ((unsigned char)m_bIsMultiLine <= 1) {
            if (m_nBlockStart == -1 && m_nBlockEnd == -1) {
                m_bIsBlockSelecting = 1;
                m_nBlockStart = m_nEditPosition;
                m_nBlockEnd = m_nEditPosition;
            }
            else {
                m_bIsBlockSelecting = 1;
                m_nBlockEnd = m_nEditPosition;
            }
        }
        else {
            // 多行時也開啟選取
            m_bIsBlockSelecting = 1;
            m_nBlockEnd = m_nEditPosition;
        }
        break;

    case VK_CONTROL:
        m_nCtrlPressed = 1;
        break;

    case VK_END:
        if ((unsigned char)m_bIsMultiLine <= 1) {
            m_nEditPosition = len;
        }
        else {
            // 多行：移到本行尾（下一個 \n 或字尾）
            int i = m_nEditPosition;
            while (i < len && m_pTextBuffer[i] != '\n') ++i;
            m_nEditPosition = i;
        }
        if (m_bIsBlockSelecting) {
            if (m_nBlockEnd <= m_nBlockStart) m_nBlockStart = m_nBlockEnd;
            m_nBlockEnd = m_nEditPosition;
            m_bBlockSelectFromRight = 1;
        }
        else {
            ClearBlockPos();
        }
        break;

    case VK_HOME:
        if ((unsigned char)m_bIsMultiLine <= 1) {
            RightAsCstr(); // 對齊原碼
            m_nEditPosition = 0;
        }
        else {
            // 多行：移到本行頭（上一個 \n 之後）
            int i = m_nEditPosition - 1;
            while (i >= 0 && m_pTextBuffer[i] != '\n') --i;
            m_nEditPosition = (i < 0) ? 0 : i + 1;
        }
        if (m_bIsBlockSelecting) {
            if (m_nBlockEnd >= m_nBlockStart) m_nBlockStart = m_nBlockEnd;
            m_nBlockEnd = m_nEditPosition;
            m_bBlockSelectFromRight = 1;
        }
        else {
            ClearBlockPos();
        }
        break;

    case VK_LEFT:
    {
        if (!m_bIsBlockSelecting) {
            ClearBlockPos();
        }
        unsigned char t = (m_nEditPosition > 0) ? JudgeMultiStr(m_pTextBuffer, m_nEditPosition - 1) : 0;
        int step = (t == 2) ? 2 : 1;
        if (m_bIsBlockSelecting) {
            RightAsCstr();
            if (m_nEditPosition > step) m_nEditPosition -= step;
            else m_nEditPosition = 0;
            m_nBlockEnd = m_nEditPosition;
            m_bBlockSelectFromRight = 1;
        }
        else {
            if (m_nEditPosition > step) m_nEditPosition -= step;
            else m_nEditPosition = 0;
        }
        break;
    }

    case VK_RIGHT:
    {
        if (!m_bIsBlockSelecting) {
            ClearBlockPos();
        }
        if (m_nEditPosition < len && m_nEditPosition < m_nMaxLength) {
            unsigned char tnow = JudgeMultiStr(m_pTextBuffer, m_nEditPosition);
            int step = (tnow == 0) ? 1 : (tnow == 1 ? 2 : 1); // 0:ASCII 1:DBCS lead 2:trail（此處以 2->1 處理）
            m_nEditPosition += step;
            if (m_bIsBlockSelecting) {
                m_nBlockEnd = m_nEditPosition;
                m_bBlockSelectFromRight = 1;
            }
        }
        break;
    }

    case VK_DELETE:
    {
        unsigned int blk = GetSelectedBlockLen();
        if ((int)blk > 0) {
            int spos = GetBlockStartPos();
            std::memmove(&m_pTextBuffer[spos],
                &m_pTextBuffer[spos + blk],
                len - spos - blk + 1);
            m_bIsComposing = 0;
        }
        ClearBlockPos();
        break;
    }

    case VK_UP:
    case VK_DOWN:
        // 多行游標上下移動：以「本行列號」為準，對齊行首/行尾後再位移
        if ((unsigned char)m_bIsMultiLine > 1) {
            // 先算目前在本行的 column
            int col = 0, i = m_nEditPosition - 1;
            while (i >= 0 && m_pTextBuffer[i] != '\n') { ++col; --i; }

            if (wParam == VK_UP) {
                // 找上一行行首
                int p = (i <= 0) ? -1 : i - 1; // i 停在上一個 \n 位置，往前找上一行 \n
                while (p >= 0 && m_pTextBuffer[p] != '\n') --p;
                int prev_beg = (p < 0) ? 0 : p + 1;
                int prev_len = 0; while (m_pTextBuffer[prev_beg + prev_len] && m_pTextBuffer[prev_beg + prev_len] != '\n') ++prev_len;
                m_nEditPosition = prev_beg + ((col <= prev_len) ? col : prev_len);
            }
            else {
                // 找下一行行首
                int q = m_nEditPosition; while (q < len && m_pTextBuffer[q] != '\n') ++q;
                if (q < len && m_pTextBuffer[q] == '\n') ++q; // 下一行首
                int next_beg = q;
                int next_len = 0; while (m_pTextBuffer[next_beg + next_len] && m_pTextBuffer[next_beg + next_len] != '\n') ++next_len;
                m_nEditPosition = next_beg + ((col <= next_len) ? col : next_len);
            }
            ClearBlockPos();
            m_bIsComposing = 0;
        }
        break;

    default:
        break;
    }

    // 對齊原碼結尾：若兩端都是 -1，則清掉整個 Block 狀態
    if (m_nBlockStart == -1 && m_nBlockEnd == -1)
        ClearBlockPos();

    // （候選清單翻頁邏輯在原碼末端藉由 IME 通知處理；這裡維持由 GetText 的 WM_IME_NOTIFY 路由實作）
    (void)hwnd; (void)lParam;
}

// =======================
// 5) OnIMEKeyChar
// =======================
void DCTIMM::OnIMEKeyChar(HWND hwnd, unsigned int wParam, int /*lParam*/)
{
    const int len = (int)std::strlen(m_pTextBuffer);

    auto do_copy_selected_to_clip = [&](bool cut)
        {
            unsigned int blk = GetSelectedBlockLen();
            if (blk == (unsigned)-1 || (int)blk <= 0) return;
            int spos = GetBlockStartPos();

            if (OpenClipboard(hwnd))
            {
                EmptyClipboard();
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, blk + 1);
                if (hMem) {
                    char* dst = (char*)GlobalLock(hMem);
                    if (dst) {
                        std::memcpy(dst, &m_pTextBuffer[spos], blk);
                        dst[blk] = '\0';
                        GlobalUnlock(hMem);
                        SetClipboardData(CF_TEXT, hMem);
                    }
                }
                CloseClipboard();
            }

            if (cut) {
                std::memmove(&m_pTextBuffer[spos],
                    &m_pTextBuffer[spos + blk],
                    len - spos - blk + 1);
                m_nEditPosition = spos;
                ClearBlockPos();
            }
        };

    // Ctrl 系列
    if (m_nCtrlPressed == 1)
    {
        switch (wParam)
        {
        case 1:   // Ctrl+A
            m_nBlockStart = 0; m_nBlockEnd = len; m_nEditPosition = len;
            break;
        case 3:   // Ctrl+C
            do_copy_selected_to_clip(false);
            break;
        case 24:  // Ctrl+X
            do_copy_selected_to_clip(true);
            break;
        case 22:  // Ctrl+V
            if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(hwnd))
            {
                HGLOBAL h = GetClipboardData(CF_TEXT);
                if (h) {
                    const char* src = (const char*)GlobalLock(h);
                    if (src) {
                        // 若有選取，先刪掉
                        DelStrByBlockSelect();
                        size_t src_len = std::strlen(src);
                        if ((int)src_len + (int)std::strlen(m_pTextBuffer) <= m_nMaxLength) {
                            InsertString(m_pTextBuffer, src, m_nEditPosition);
                            m_bIsComposing = 0;
                            m_nEditPosition += (int)src_len;
                        }
                        GlobalUnlock(h);
                    }
                }
                CloseClipboard();
            }
            break;
        default:
            break;
        }
        return;
    }

    // 一般字元
    if (wParam == VK_BACK || (wParam <= 0x1F && wParam != VK_RETURN) || wParam == 127) {
        // 交給 KeyDown 的處理路徑（原碼在 OnIMEKeyChar 尾端會遞減一個計數器後釋放；此處不需要）
        return;
    }

    if (wParam == VK_RETURN)
    {
        // 多行才插入 \n
        if ((unsigned char)m_bIsMultiLine > 1) {
            // 預留空間
            if ((int)std::strlen(m_pTextBuffer) < m_nMaxLength) {
                // 把游標之後的字串暫存起來接回去
                std::string tail = &m_pTextBuffer[m_nEditPosition];
                m_pTextBuffer[m_nEditPosition] = '\n';
                ++m_nEditPosition;
                std::strcpy(&m_pTextBuffer[m_nEditPosition], tail.c_str());
                // 行資訊（+69 與 +58+...）在原碼會累加，這裡略過保留
            }
        }
        return;
    }

    // 一般可見字元
    if (((wParam >= '0' && wParam <= '9') ||
        (wParam >= 'A' && wParam <= 'Z') ||
        (wParam >= 'a' && wParam <= 'z') ||
        std::isspace((int)wParam) ||
        m_bIsPassword != 1) &&
        (int)std::strlen(m_pTextBuffer) < m_nMaxLength)
    {
        DelStrByBlockSelect();                      // 先把選取刪掉
        OnIMEKeyDown(hwnd, VK_DELETE, 0);           // 對齊原碼會先走一次 DEL 路（把組字殘留清乾淨）
        m_nUnk_96 = 0;                               // 清 Skip 標記
        InsertChar(m_pTextBuffer, (char)wParam, m_nEditPosition);
        m_bIsComposing = 0;
        ++m_nEditPosition;
    }
}


void DCTIMM::InsertString(char* dest, const char* src, int pos)
{
    if (!dest || !src) return;

    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    if (pos > (int)dest_len) pos = dest_len;

    // 建立暫存區來儲存插入點之後的字串
    char* temp = new (std::nothrow) char[dest_len - pos + 1];
    if (temp) {
        strcpy_s(temp, dest_len - pos + 1, &dest[pos]);
        // 複製要插入的字串
        strcpy_s(&dest[pos], src_len + 1, src);
        // 附加回暫存的字串
        strcat_s(dest, dest_len + src_len + 1, temp);
        delete[] temp;
    }
}

void DCTIMM::InsertChar(char* dest, char c, int pos)
{
    char temp_char[2] = { c, '\0' };
    InsertString(dest, temp_char, pos);
}

void DCTIMM::DelStrByBlockSelect()
{
    unsigned int block_len = GetSelectedBlockLen();
    size_t total_len = strlen(m_pTextBuffer);

    if (block_len > 0)
    {
        int start_pos = GetBlockStartPos();
        m_nEditPosition = start_pos;

        if (static_cast<int>(total_len - start_pos - block_len) >= 0)
        {
            // 將選取區塊後的內容往前移動，覆蓋選取區塊
            memmove(
                &m_pTextBuffer[start_pos],
                &m_pTextBuffer[start_pos + block_len],
                total_len - start_pos - block_len
            );

            // 清理字串尾部多餘的資料並設定新的結束符
            size_t new_len = total_len - block_len;
            memset(&m_pTextBuffer[new_len], 0, block_len);
            m_pTextBuffer[new_len] = '\0';
        }

        ClearBlockPos();
    }
    else // block_len is 0
    {
        int current_pos = m_nEditPosition;
        m_bBlockSelectFromRight = 0;
        m_nBlockEnd = current_pos;
        m_nBlockStart = current_pos;
    }

    // 根據反編譯碼，若非處於選取狀態，則再次清除選取區塊
    if (!m_bIsBlockSelecting)
    {
        m_nBlockStart = -1;
        m_nBlockEnd = -1;
    }
}

unsigned char DCTIMM::JudgeMultiStr(const char* str, int pos)
{
    unsigned char result = 0;
    for (int i = 0; i <= pos; ++i)
    {
        if (result == 1)
            result = 2;
        else
            result = IsDBCSLeadByte(str[i]);
    }
    return result;
}

int DCTIMM::GetCharsetFromLang(unsigned short lang_id)
{
    int result;
    switch (lang_id & 0x3FF)
    {
    case LANG_ARABIC:
        result = ARABIC_CHARSET;
        break;
    case LANG_BULGARIAN:
    case LANG_RUSSIAN:
    case LANG_UKRAINIAN:
    case LANG_BELARUSIAN:
        result = RUSSIAN_CHARSET;
        break;
    case LANG_CHINESE:
        if (lang_id >> 10 == SUBLANG_CHINESE_TRADITIONAL) {
            result = CHINESEBIG5_CHARSET;
        }
        else {
            if (lang_id >> 10 == SUBLANG_CHINESE_SIMPLIFIED){
                result = GB2312_CHARSET;
            }
            else {
                result = ANSI_CHARSET;
            }
        }
        break;
    case LANG_CZECH:
    case LANG_HUNGARIAN:
    case LANG_POLISH:
    case LANG_ROMANIAN:
    case LANG_SERBIAN:
    case LANG_SLOVAK:
    case LANG_SLOVENIAN:
    case LANG_MACEDONIAN:
        result = EASTEUROPE_CHARSET;
        break;
    case LANG_GREEK:
        result = GREEK_CHARSET;
        break;
    case LANG_HEBREW:
        result = HEBREW_CHARSET;
        break;
    case LANG_JAPANESE:
        result = SHIFTJIS_CHARSET;
        break;
    case LANG_KOREAN:
        result = HANGEUL_CHARSET;
        break;
    case LANG_THAI:
        result = THAI_CHARSET;
        break;
    case LANG_TURKISH:
        result = TURKISH_CHARSET;
        break;
    case LANG_ESTONIAN:
    case LANG_LATVIAN:
    case LANG_LITHUANIAN:
        result = BALTIC_CHARSET;
        break;
    default: 
        result = ANSI_CHARSET;
        break;
    }
    
    return result;
}

int DCTIMM::GetCodePageFromCharset(int charset)
{
    switch (charset)
    {
    case SHIFTJIS_CHARSET: return 932;
    case HANGEUL_CHARSET: return 949;
    case GB2312_CHARSET: return 936;
    case CHINESEBIG5_CHARSET: return 950;
    case GREEK_CHARSET: return 1253;
    case TURKISH_CHARSET: return 1254;
    case HEBREW_CHARSET: return 1255;
    case ARABIC_CHARSET: return 1256;
    case BALTIC_CHARSET: return 1257;
    case THAI_CHARSET: return 874;
    case EASTEUROPE_CHARSET: return 1250;
    default: return 1252; // Latin 1 (ANSI)
    }
}

unsigned short DCTIMM::GetCodePageFromLang(unsigned short lang_id)
{
    return GetCodePageFromCharset(GetCharsetFromLang(lang_id));
}

void DCTIMM::GetUnderLine(int* a2, int* a3)
{
    // 反編譯把 (char*)this + 4 當成寬字元起點
    const WCHAR* wptr = reinterpret_cast<const WCHAR*>(
        reinterpret_cast<const char*>(this) + 4);

    // +168(word) 是目前碼頁（先前 OnInputLangChange 會更新）
    const UINT cp = static_cast<UINT>(m_usCodePage);

    // a2 = 把前 +82（m_nCompAttrStart）個寬字元轉成多位元組後的長度
    *a2 = ::WideCharToMultiByte(
        cp, 0, wptr,
        m_nCompAttrStart,      // 反編譯：*((DWORD*)this + 82)
        nullptr, 0, nullptr, nullptr);

    // a3 = 把前 +83（m_nCompAttrLen）個寬字元轉成多位元組後的長度
    *a3 = ::WideCharToMultiByte(
        cp, 0, wptr,
        m_nCompAttrLen,        // 反編譯：*((DWORD*)this + 83)
        nullptr, 0, nullptr, nullptr);
}

HWND DCTIMM::GetNative(HWND h)
{
    // 反編譯用法：fdwSentence 先塞入 this，再呼叫 ImmGetConversionStatus
    DWORD conv = 0;
    DWORD sentence = reinterpret_cast<DWORD>(this);

    HIMC hImc = ::ImmGetContext(h);
    ::ImmGetConversionStatus(hImc, &conv, &sentence);

    // 反編譯版本沒有 ImmReleaseContext；為了行為一致，這裡也不釋放。
    // 若你想避免資源外洩，可自行改成釋放，但將與「完全一致」略有出入。
    // ::ImmReleaseContext(h, hImc);

    // 反編譯把 conv 放到 a2，最後 return a2；我們直接回傳 conv 當作 HWND。
    return reinterpret_cast<HWND>(conv);
}
// 反編譯 (0051FDB0) SkipComposition：把 +96 設為 1
void DCTIMM::SkipComposition() {
    m_nUnk_96 = 1;
}

// 反編譯 (0051FDC0) ChangeLanguage
// 讀取現有 Conversion 狀態，清掉低 2 bits，視 to_native 決定是否把 bit0 設 1；最後寫回。
void DCTIMM::ChangeLanguage(int to_native, HWND hwnd) {
    if (!hwnd) return;
    HIMC imc = ::ImmGetContext(hwnd);
    if (!imc) return;

    DWORD conv = 0, sentence = 0;
    ::ImmGetConversionStatus(imc, &conv, &sentence);

    // 對齊反編譯：LOBYTE(conv) &= 0xFC; if (to_native) LOBYTE(conv) |= 1;
    conv = (conv & ~0x3u);
    if (to_native) conv |= 0x1u;

    ::ImmSetConversionStatus(imc, conv, sentence);
    ::ImmReleaseContext(hwnd, imc);
}