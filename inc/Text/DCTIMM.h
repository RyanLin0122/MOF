#pragma once
#include <windows.h>
#include <imm.h>
#include <cstring>
#include <string>
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "User32.lib")
/**
 * 反編譯語意對齊版 DCTIMM
 *
 * 重要欄位對應（以 DWORD 索引說明）：
 * +0  : char*  主要文字緩衝區（m_pTextBuffer）
 * +4~ : this+4 開始的 0x0A bytes 與其後 0xC8 bytes 的區塊，用於組字暫存/候選字串串接（見 union）
 * +54 : 候選清單起始索引（m_nCandListStartIdx）
 * +55 : 候選數量（m_nCandListCount）
 * +57 : 最大長度（m_nMaxLength）
 * +58 : 編輯游標（m_nEditPosition）
 * +69 : 多行模式的行資訊暫存（m_nUnk_69；保留）
 * +70 : 選取起點（m_nBlockStart）
 * +71 : 選取終點（m_nBlockEnd）
 * +72 : 是否正在 Shift 選取（m_bIsBlockSelecting）
 * +73 : 是否由右側開始（m_bBlockSelectFromRight）
 * +74 : 是否「正在組字」（m_bIsComposing）
 * +76 : 行距（m_nLineSpace）
 * +77 : 送出後是否要清空（m_bShouldClearBuffer）
 * +78 : 模式：1=純數字，其餘=一般文字（m_nMode）【請不要再用字高當模式】
 * +80 : IME 活動旗標（m_nUnk_80）
 * +81 : 是否有底線範圍（m_nUnk_81）
 * +82 : 底線起點（m_nCompAttrStart）
 * +83 : 底線終點（m_nCompAttrLen）
 * +85 : IME 組字游標位置（m_nUnk_85）
 * +86 : HWND（m_hWnd）
 * +87 : 送出後是否已清空（m_bBufferCleared）
 * +88~+91 : RECT（m_rcTextBox）
 * +92 : 對齊（m_nAlign）
 * +94 : Ctrl 是否按下（m_nCtrlPressed）
 * +95 : 是否 Active（m_bIsActive）
 * +96 : SkipComposition 旗標（m_nUnk_96）
 * +98 : 密碼模式（m_bIsPassword）
 * +168(word) : 目前碼頁（m_usCodePage）
 * +169(word) : 目前語系（m_usLangId）
 */

class DCTIMM {
public:
    DCTIMM();
    ~DCTIMM();

    // === 初始/一般控制 ===
    void Init(unsigned int /*unused*/, unsigned short max_length, int mode,
        RECT* rect, char multi_line, int line_space, int align);
    void SetTextSize(int new_max_len, int new_mode);
    int  Poll(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam);
    void SetActive(int active, HWND hwnd);
    inline int  IsActive() const { return m_bIsActive; }
    void ChangeLanguage(int to_native, HWND hwnd);

    // === 文字 I/O ===
    void GetIMMText(char* buffer, int buffer_size);
    void SetIMMText(char* text);

    // === 游標/選取 ===
    int  SetEditPosition(int pos);
    inline int  GetEditPosition() const { return m_nEditPosition; }
    unsigned int GetSelectedBlockLen();
    int  GetBlockStartPos();
    void SetBlockPos(int pos);
    void ClearBlockPos();
    inline int  IsComposition() const { return m_bIsComposing; }
    void SkipComposition();

    // === 幾何 ===
    void GetRect(RECT* out) const { if (out) *out = m_rcTextBox; }
    void SetRect(RECT rc) { m_rcTextBox = rc; }

    // === 你要我補齊的 5 個方法 ===
    void OnInputLangChange(unsigned short lang_id);
    void GetCandi(HWND hwnd, unsigned int msg_type, int a4, int a5);
    void OnIMEKeyComposition(HWND hwnd, unsigned int wParam, int lParam);
    void OnIMEKeyDown(HWND hwnd, unsigned int wParam, int lParam);
    void OnIMEKeyChar(HWND hwnd, unsigned int wParam, int lParam);
    HWND GetNative(HWND h);
    void GetUnderLine(int* start, int* end);

private:
    // === 內部工具 ===
    int  GetNumeric(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam);
    int  GetText(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam);

    void InsertString(char* dest, const char* src, int pos);
    void InsertChar(char* dest, char c, int pos);
    void DelStrByBlockSelect();
    bool RightAsCstr();
    unsigned char JudgeMultiStr(const char* str, int pos);

    int  GetCharsetFromLang(unsigned short lang_id);
    int  GetCodePageFromCharset(int charset);
    unsigned short GetCodePageFromLang(unsigned short lang_id);

private:
    // +0
    char* m_pTextBuffer;

    // +4  起：this+4 到 this+4+0x0A 是未使用/保留；之後 0xC8 做候選串接
    union {
        char  m_szCompositionStr[212]; // 組字暫存（ASCII，用來放 RESULT/COMPSTR）
        struct {
            char _padding_4_to_14[10];
            char m_szCandidateList[200]; // this+14 開始的 0xC8 字元區，用於候選串接
        };
    };

    // +216 ~
    int  m_nCandListStartIdx;   // +54
    int  m_nCandListCount;      // +55
    int  m_nUnk_56;             // +56（保留）
    int  m_nMaxLength;          // +57
    int  m_nEditPosition;       // +58

    // +236 組字屬性（底線區）
    char m_CompAttr[40];        // 對齊反編譯碼（0x28）：這裡當作暫存使用

    // +276~
    int  m_nUnk_69;             // +69
    int  m_nBlockStart;         // +70
    int  m_nBlockEnd;           // +71
    int  m_bIsBlockSelecting;   // +72
    int  m_bBlockSelectFromRight; // +73
    int  m_bIsComposing;        // +74
    int  m_nUnk_75;             // +75（保留）
    int  m_nLineSpace;          // +76
    int  m_bShouldClearBuffer;  // +77
    int  m_nMode;               // +78（1=數字，其餘=文字）

    // +80~
    int  m_nUnk_80;             // +80（IME流程旗標）
    int  m_nUnk_81;             // +81（是否有底線範圍）
    int  m_nCompAttrStart;      // +82
    int  m_nCompAttrLen;        // +83
    unsigned short m_usCodePage;// +168 (word)
    unsigned short m_usLangId;  // +169 (word)
    int  m_nUnk_85;             // +85（組字游標位置）
    HWND m_hWnd;                // +86
    int  m_bBufferCleared;      // +87

    // +88 ~ +91
    RECT m_rcTextBox;

    int  m_nAlign;              // +92
    int  m_nUnk_93;             // +93（保留）
    int  m_nCtrlPressed;        // +94（Ctrl 是否按下）
    int  m_bIsActive;           // +95
    int  m_nUnk_96;             // +96（SkipComposition 旗標）
    int  m_nUnk_97;             // +97（保留）
    int  m_bIsPassword;         // +98（1=密碼）

    BYTE m_bIsMultiLine = 0;
};
