#pragma once
#include <windows.h>
#include <imm.h>
#include <cstring>
#include <string>
#include "DCTIMM.h"

#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "User32.lib")


/**
 * DCTIMMList：管理多個 DCTIMM 的容器
 * - 最多 40 個項目（對應反編譯裡的 12 ~ 168 與 172 ~ 172+0xA0）
 * - 內部保存 IME Context 與轉換狀態，並在啟用/停用時切換
 */
class DCTIMMList {
public:
    DCTIMMList();
    ~DCTIMMList();

    // 建立容器容量、初始化 IME 相關狀態
    void Create(int capacity);

    // 依反編譯行為：設定 DCTIMM 的 +98（在我們的 DCTIMM 對應 m_bIsPassword）
    // 名稱沿用反編譯清單 SetAlphaDigit()
    void SetAlphaDigit(int index, int flag);

    // 建立一個 DCTIMM 並初始化
    void SetIMMInfo(int index, unsigned int a3, unsigned short a4,
        int a5, RECT* a6, char a7, int a8, int a9);

    // 刪除一個 DCTIMM
    void DeleteIMMInfo(int index);

    // 事件投遞：若 DCTIMM::Poll 可用，行為等價於反編譯中的 DCTIMM::Poll
    int  GetText(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam);

    // 文字 I/O
    void GetIMMText(int index, LPSTR out, int out_size);
    void SetIMMText(int index, char* text);

    // 反編譯中的 SetAutoDelete：實際寫入 DCTIMM +77（對齊 m_bShouldClearBuffer）
    void SetAutoDelete(int index, int flag);

    // 啟/停用 + 切換 IME 狀態
    void SetActive(int index, int active, HWND hwnd_for_dctimm);

    // 問是否啟用
    int  IsActive(int index) const;
    int  IsActive() const; // 針對目前 index

    // 取得 Native
    HWND GetNative(int index, HWND h) const;
    HWND GetNative(HWND h) const;

    // 位置、選取、直線底線
    RECT GetRect(int index) const;
    unsigned int GetSelectedBlockLen(int index) const;
    void SetRect(int index, RECT rc);
    int  IsComposition(int index) const;
    int  IsComposition() const;

    void ResetBegineComposition(); // 對齊：把 DCTIMM +80 清 0
    int  GetBegineComposition() const; // 讀 DCTIMM +80
    int  GetCandidate() const;         // 讀 DCTIMM +81

    int  GetEditPosition(int index) const;
    int  SetEditPosition(int index, int pos);

    void SetTextSize(int index, int new_max_len, int new_mode);

    // 反編譯用 GetText(v4, 0, a3, TestChar, 0)：
    // 這裡用 Poll(0, a3, TestChar, 0) 等價替代（因為 GetText 非公開）
    void SetEditKey(int index, unsigned int uMsg, unsigned int wParam);

    // 取得第一個可用的 IME Index（對照 memset 區段）
    int  GetUsableIMEIndex() const;

    void SkipComposition(int index);
    void ChangeLanguage(int index, int to_native, HWND hwnd);
    void ChangeLanguage(int to_native, HWND hwnd);

    int  GetBlockStartPos(int index) const;
    void SetHWND(int index, HWND hwnd);
    void SetBlockPos(int index, int pos);
    void ClearBlockPos(int index);
    void GetUnderLine(int index, int* start, int* end);

    // 回傳組字暫存（this+4）
    char* GetCompositionString(int index) const;

    // 取字元集（沿用反編譯呼叫，但不直接依賴 DCTIMM 私有方法，改用等價邏輯）
    int  GetCharSet(int index) const;

    // 取組字游標位置（DCTIMM +85；反編譯讀 +340）
    int  GetComCursorPos(int index) const;

    // 目前使用中的 index
    inline int CurrentIndex() const { return m_currentIndex; }

private:
    // === 工具：等價 DCTIMM::GetCharsetFromLang 與 DCTIMM::GetCodePageFromCharset ===
    static int  _GetCharsetFromLang(unsigned short lang_id);
    static int  _GetCodePageFromCharset(int cs);

private:
    int      m_count;                 // +0
    int      m_currentIndex;          // +1
    int      m_capacity;              // +2
    DCTIMM* m_items[40];             // +3 .. +42
    int      m_used[40];              // +43 .. +82（0/1）
    HIMC     m_savedImc;              // +83
    DWORD    m_savedConv;             // +84
    DWORD    m_savedSentence;         // +85
    int      m_firstReleaseFlag;      // +86（建構時=1，第一次釋放時把 sentence 設 8）
    HWND hWnd;
};
