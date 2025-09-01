#pragma once
#include <windows.h>
#include <imm.h>
#include <cstring>
#include <string>
#include "DCTIMM.h"

#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "User32.lib")


/**
 * DCTIMMList�G�޲z�h�� DCTIMM ���e��
 * - �̦h 40 �Ӷ��ء]�����ϽsĶ�̪� 12 ~ 168 �P 172 ~ 172+0xA0�^
 * - �����O�s IME Context �P�ഫ���A�A�æb�ҥ�/���ήɤ���
 */
class DCTIMMList {
public:
    DCTIMMList();
    ~DCTIMMList();

    // �إ߮e���e�q�B��l�� IME �������A
    void Create(int capacity);

    // �̤ϽsĶ�欰�G�]�w DCTIMM �� +98�]�b�ڭ̪� DCTIMM ���� m_bIsPassword�^
    // �W�٪u�ΤϽsĶ�M�� SetAlphaDigit()
    void SetAlphaDigit(int index, int flag);

    // �إߤ@�� DCTIMM �ê�l��
    void SetIMMInfo(int index, unsigned int a3, unsigned short a4,
        int a5, RECT* a6, char a7, int a8, int a9);

    // �R���@�� DCTIMM
    void DeleteIMMInfo(int index);

    // �ƥ�뻼�G�Y DCTIMM::Poll �i�ΡA�欰������ϽsĶ���� DCTIMM::Poll
    int  GetText(HWND hwnd, unsigned int uMsg, unsigned int wParam, int lParam);

    // ��r I/O
    void GetIMMText(int index, LPSTR out, int out_size);
    void SetIMMText(int index, char* text);

    // �ϽsĶ���� SetAutoDelete�G��ڼg�J DCTIMM +77�]��� m_bShouldClearBuffer�^
    void SetAutoDelete(int index, int flag);

    // ��/���� + ���� IME ���A
    void SetActive(int index, int active, HWND hwnd_for_dctimm);

    // �ݬO�_�ҥ�
    int  IsActive(int index) const;
    int  IsActive() const; // �w��ثe index

    // ���o Native
    HWND GetNative(int index, HWND h) const;
    HWND GetNative(HWND h) const;

    // ��m�B����B���u���u
    RECT GetRect(int index) const;
    unsigned int GetSelectedBlockLen(int index) const;
    void SetRect(int index, RECT rc);
    int  IsComposition(int index) const;
    int  IsComposition() const;

    void ResetBegineComposition(); // ����G�� DCTIMM +80 �M 0
    int  GetBegineComposition() const; // Ū DCTIMM +80
    int  GetCandidate() const;         // Ū DCTIMM +81

    int  GetEditPosition(int index) const;
    int  SetEditPosition(int index, int pos);

    void SetTextSize(int index, int new_max_len, int new_mode);

    // �ϽsĶ�� GetText(v4, 0, a3, TestChar, 0)�G
    // �o�̥� Poll(0, a3, TestChar, 0) �������N�]�]�� GetText �D���}�^
    void SetEditKey(int index, unsigned int uMsg, unsigned int wParam);

    // ���o�Ĥ@�ӥi�Ϊ� IME Index�]��� memset �Ϭq�^
    int  GetUsableIMEIndex() const;

    void SkipComposition(int index);
    void ChangeLanguage(int index, int to_native, HWND hwnd);
    void ChangeLanguage(int to_native, HWND hwnd);

    int  GetBlockStartPos(int index) const;
    void SetHWND(int index, HWND hwnd);
    void SetBlockPos(int index, int pos);
    void ClearBlockPos(int index);
    void GetUnderLine(int index, int* start, int* end);

    // �^�ǲզr�Ȧs�]this+4�^
    char* GetCompositionString(int index) const;

    // ���r�����]�u�ΤϽsĶ�I�s�A���������̿� DCTIMM �p����k�A��ε����޿�^
    int  GetCharSet(int index) const;

    // ���զr��Ц�m�]DCTIMM +85�F�ϽsĶŪ +340�^
    int  GetComCursorPos(int index) const;

    // �ثe�ϥΤ��� index
    inline int CurrentIndex() const { return m_currentIndex; }

private:
    // === �u��G���� DCTIMM::GetCharsetFromLang �P DCTIMM::GetCodePageFromCharset ===
    static int  _GetCharsetFromLang(unsigned short lang_id);
    static int  _GetCodePageFromCharset(int cs);

private:
    int      m_count;                 // +0
    int      m_currentIndex;          // +1
    int      m_capacity;              // +2
    DCTIMM* m_items[40];             // +3 .. +42
    int      m_used[40];              // +43 .. +82�]0/1�^
    HIMC     m_savedImc;              // +83
    DWORD    m_savedConv;             // +84
    DWORD    m_savedSentence;         // +85
    int      m_firstReleaseFlag;      // +86�]�غc��=1�A�Ĥ@������ɧ� sentence �] 8�^
    HWND hWnd;
};
