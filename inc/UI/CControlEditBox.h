#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <windows.h>

#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CEditBoxBackgroundImage.h"
#include "Text/DCTTextManager.h"
#include "Text/DCTIMMList.h"
#include "Font/MoFFont.h"
#include "global.h"

// 外部：IME 管理（等同反編譯中的 g_IMMList）
extern DCTIMMList g_IMMList;

// EditBox 內部使用的簡單階梯選取描述（多行用；單行通常不會用到）
struct stBlockStair {
    // 每一行的「該行起點的位元組數」與「該行選取長度（位元組）」。
    // 反編譯碼以 pair 陣列使用，這裡保留 10 行的空間（與 GetCharByteByLine 的 10 相呼應）。
    size_t len[10 * 2]{}; // [pos0,len0, pos1,len1, ...]
};

/**
 * @class CControlEditBox
 * @brief 對齊反編譯行為的可輸入文字控制項（支援 IME、單/多行、密碼遮罩、反白區塊）。
 */
class CControlEditBox : public CControlBase
{
public:
    CControlEditBox();
    virtual ~CControlEditBox();

    // --- 建立 ---
    void Create(int x, int y, int eboxWidth, uint16_t maxLen, int mode,
        CControlBase* pParent, int align /*0:L,1:C,2:R*/, int multiLine /*0/1*/, int reserved /*未用*/, uint8_t visibleLines);
    void Create(int x, int y, int eboxWidth, CControlBase* pParent, int align /*0:L,1:C,2:R*/);
    void Create(CControlBase* pParent);

    // --- 外觀/尺寸 ---
    void SetEBoxSize(int width, int height /*0=沿用背景*/, int align /*0:L,1:C,2:R*/);

    // --- 可輸入/IME 設定 ---
    void SetWritable(int writable, uint16_t maxLen, int mode, uint8_t visibleLines /*行數上限（單行=1）*/);
    void SetupIME(int enable, uint16_t maxLen, int mode, uint8_t multiLineFlag);
    void SetAlphaDigit(int flag); // 對齊反編譯：其實是設定密碼/數字模式（DCTIMM +98） 

    // 顯示/焦點
    virtual void Show() override;
    void SetFocus(int on);

    // 文字 API
    void SetText(int stringId);
    void SetText(const char* s);
    void SetTextItoa(int v);
    void TextClear();
    char* GetText();

    // 量測
    uint16_t GetMaxTextSize() const;
    unsigned int GetCurTextSize() const;
    BOOL IsMultiLine() const;

    // 事件
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 點擊轉游標位置（滑鼠點選文字內哪個字元之後）
    // 傳入：ptAbs = {x,y} 絕對座標；a3 = IME index（通常用 m_imeIndex）
    // 回傳：字元索引（0..len），找不到時 -1
    int RenewMousePos(int* ptAbs /*[x,y]*/, int a3);

    // 密碼遮罩（對齊反編譯中的 this+[41] 旗標）
    void SetPasswordMode(bool on) { m_password = on ? 1 : 0; }

protected:
    virtual void PrepareDrawing() override;

private:
    // 內部工具（都對齊反編譯呼叫樣式）
    void CreateChildren();
    void DeleteBlockBox();
    void SetBlockShow(int on, int idx);
    void SetBlockBox(CControlBase* /*unused*/, int x, int y, uint16_t w, uint16_t h, int which);
    void DiscriminStairBlock(stBlockStair* stair); // 保留；單行不常用

    // caret 位置量測：outXY[0]=x, outXY[1]=y；a3=原字串（非遮罩）、Source=實際顯示（可能為遮罩）
    void GetCaretPos(int outXY[2], const char* a3, const char* Source, int imeIndex, size_t Count /*0=用IME游標*/);

    // 滑鼠點擊落在哪個區段，用以決定 caret 要前/後（對齊反編譯）
    BOOL SearchTextPos(uint32_t* pThisAlias, size_t* curCount, uint32_t* ptAbs /*[x,y]*/,
        int segL, int segT, int segR, int segB, const char* fullText);

private:
    // ---- 子控制 ----
    CControlAlphaBox        m_Block[5];    // 選取矩形（可疊 1~數個段）
    CEditBoxBackgroundImage m_Back;        // 背景（九宮格）
    CControlText            m_Text;        // 主文字（實際 IME 文字）
    CControlText            m_Caret;       // 游標（字元 'I'，粗體）
    CControlText            m_Mask;        // 密碼遮罩文字（用 '*'）

    // ---- 狀態 ----
    int       m_writable{ 1 };           // +33
    uint16_t  m_maxLen{ 0 };             // +64
    uint16_t  m_imeIndex{ 0xFFFF };      // +68
    uint8_t   m_visibleLines{ 1 };       // +138（單行=1）
    int       m_focus{ 0 };              // +31
    int       m_caretIndex{ 0 };         // +35
    int       m_caretX{ 0 };             // +39
    int       m_caretY{ 0 };             // +40
    uint16_t  m_prefHeight{ 0 };         // +17（當多行時指定高度）
    int       m_align{ 0 };              // +800（0:L 1:C 2:R）
    int       m_password{ 0 };           // +41（本類別自管的密碼遮罩旗標）

    // 行分割快取（最多 10 行）
    unsigned char m_lineBreakBytes[16]{};

    // 游標閃爍
    static inline DWORD sBlinkTick{ 0 };
    static inline bool  sBlinkInit{ false };
};
