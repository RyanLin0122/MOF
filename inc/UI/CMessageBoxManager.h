#pragma once

#include <cstdint>
#include <cstring>
#include <windows.h>
#include "UI/CUIBase.h"

// ---- 前置宣告 ----
class CControlBase;
class CUIManager;
class CUITutorial;
class CUIMessageBoxNotice;
class CUIMessageBoxOK;
class CUIMessageBoxCalc;
class CUIMessageBoxEdit;
class CUIMessageBoxList;
class CUIMessageBoxRadioList;
class CUIMessageBoxSortList;
class CUIMessageBoxMultLineOK;
class CUIMessageBoxMultLineOKLarge;

struct stMSGBoxData;

// ============================================================
//  strMsgBoxAttr — 對齊反編譯 0x28 = 40 bytes
// ============================================================
struct strMsgBoxAttr
{
    strMsgBoxAttr();

    int m_nField0;          // this[0]  = 1
    int m_nField1;          // this[1]  = 1
    int m_nField2;          // this[2]  = 0
    int m_nField3;          // this[3]  = 0
    int m_nField4;          // this[4]  = 0
    int m_nField5;          // this[5]  = 0
    int m_nField6;          // this[6]  = 0
    int m_nField7;          // this[7]  = -1
    int m_nField8;          // this[8]  = 0
    int m_nField9;          // this[9]  — 建構子未初始化
};

// ============================================================
//  CUIMessageBoxBase — 最小定義，供 CMessageBoxManager 使用
//  完整子類別（CUIMessageBoxOK 等）在各自翻譯單元中
// ============================================================
class CUIMessageBoxBase : public CUIBase
{
public:
    CUIMessageBoxBase();
    virtual ~CUIMessageBoxBase();

    // ---- vtable 虛擬方法（CMessageBoxManager 透過 vtable 呼叫） ----
    virtual void OnInitLogin() {}               // InitLogin 走訪時呼叫
    virtual void Close() {}                     // CloseMessagBox 呼叫
    virtual int  OnCancel(int a2, int a3, int a4, int a5, int a6, int a7) { return 0; }
    virtual int  OnOK(int a2, int a3, int a4, int a5, int a6, int a7) { return 0; }
    virtual void Set(int option) {}             // Add 中建立後呼叫

    // ---- 非虛函式 ----
    void SetType(CUIBase* a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9);
    void SetType(strMsgBoxAttr* pAttr);
    strMsgBoxAttr* GetAttr();
    BOOL Open();
    void Terminate(int a2, int a3, int a4);
    int  IsOpenParent();
    CUIBase* GetMsgBoxParent();
    int  KeyboardInputProcess(int a2, int a3, int a4, int a5, int a6, int a7);

protected:
    strMsgBoxAttr m_Attr;           // 對齊反編譯 offset 3304 (= this[826])
    // strMsgBoxAttr 欄位對照：
    //   m_nField0 = this[826] ← SetType 的 a7
    //   m_nField1 = this[827] ← SetType 的 a8
    //   m_nField2 = this[828]
    //   m_nField3 = this[829] ← SetType 的 a2 (parent CUIBase*, 存為 int)
    //   m_nField4 = this[830] ← SetType 的 a3
    //   m_nField5 = this[831] ← SetType 的 a4
    //   m_nField6 = this[832] ← SetType 的 a5
    //   m_nField7 = this[833] ← SetType 的 a6
    //   m_nField8 = this[834] ← SetType 的 a9 (callback)
};

// ============================================================
//  stMessageBoxList — 雙向鏈結串列節點（12 bytes）
// ============================================================
struct stMessageBoxList
{
    stMessageBoxList();
    ~stMessageBoxList();

    CUIMessageBoxBase* m_pBox;    // offset 0 : 指向 CUIMessageBoxBase
    stMessageBoxList*  m_pPrev;   // offset 4 : 前一個節點
    stMessageBoxList*  m_pNext;   // offset 8 : 下一個節點
};

// ============================================================
//  CMsgBoxOptionOK — 24 bytes
// ============================================================
struct CMsgBoxOptionOK
{
    CMsgBoxOptionOK();

    uint8_t  m_bFlag;          // offset 0  : 1
    uint8_t  m_pad1[3];        // padding
    uint32_t m_nField1;        // offset 4  : 0
    uint32_t m_nField2;        // offset 8  : 0
    uint32_t m_nField3;        // offset 12 : 0
    uint32_t m_nField4;        // offset 16 : 0
    uint32_t m_nField5;        // offset 20 : 0
};

// ============================================================
//  CMsgBoxOptionEdit — 60 bytes
// ============================================================
struct CMsgBoxOptionEdit
{
    CMsgBoxOptionEdit();
    CMsgBoxOptionEdit(int a2);

    uint32_t m_nField0;        // offset 0
    uint32_t m_nField1;        // offset 4
    uint32_t m_nField2;        // offset 8
    uint16_t m_usWidth;        // offset 12
    uint16_t m_usHeight;       // offset 14
    uint32_t m_nField4;        // offset 16
    uint32_t m_nField5;        // offset 20
    uint8_t  m_bField6;        // offset 24
    uint8_t  m_pad1[3];        // padding to align offset 28
    uint32_t m_nField7;        // offset 28
    uint32_t m_nField8;        // offset 32
    uint32_t m_nField9;        // offset 36
    uint32_t m_nField10;       // offset 40
    uint8_t  m_bField11;       // offset 44
    uint8_t  m_pad2[3];        // padding to align offset 48
    uint32_t m_nField12;       // offset 48
    uint32_t m_nField13;       // offset 52
    uint32_t m_nField14;       // offset 56
};

// ============================================================
//  CMsgBoxOptionTutorial — 4 bytes
// ============================================================
struct CMsgBoxOptionTutorial
{
    CMsgBoxOptionTutorial();

    uint32_t m_nValue;   // offset 0 : 0
};

// ============================================================
//  CMessageBoxManager
// ============================================================
typedef void (__cdecl *MsgBoxCallback)(stMSGBoxData);

class CMessageBoxManager
{
public:
    CMessageBoxManager();
    ~CMessageBoxManager();

    // ---- 基礎管理 ----
    void              InitLogin();
    stMessageBoxList* AddList(CUIMessageBoxBase* pBox);
    void              CloseMessagBox(CUIMessageBoxBase* pBox);
    stMessageBoxList* FindList(CUIMessageBoxBase* pBox);
    void              DelList(stMessageBoxList* pNode);
    void              Del(CUIMessageBoxBase* pBox);

    // ---- 建立工廠 ----
    CUIMessageBoxBase* Add(int type, int option, strMsgBoxAttr* pAttr);

    CUITutorial*       AddTutorial(int a2);

    CUIMessageBoxNotice* AddNotice(CUIBase* a2, int a3, int a4, int a5);

    CUIMessageBoxBase* AddOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                   int a7, char a8, int a9, int a10, int a11,
                                   int a12, int a13, unsigned int a14);
    CUIMessageBoxBase* AddOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                   char* a7, char a8, int a9, int a10, int a11,
                                   int a12, int a13, unsigned int a14);

    CUIMessageBoxOK*   AddOKCancelA(MsgBoxCallback a2, int a3, char a4,
                                    int a5, int a6, int a7, int a8, int a9,
                                    unsigned int a10, int a11);
    CUIMessageBoxOK*   AddOKCancelA(MsgBoxCallback a2, char* a3, char a4,
                                    int a5, int a6, int a7, int a8, int a9,
                                    unsigned int a10, int a11);

    CUIMessageBoxBase* AddOK(CUIBase* a2, int a3, int a4, int a5, int a6,
                             int a7, int a8, int a9, unsigned int a10);
    CUIMessageBoxBase* AddOK(CUIBase* a2, char* a3, int a4, int a5, int a6,
                             int a7, int a8, int a9, unsigned int a10);
    CUIMessageBoxBase* AddOK(char* a2, int a3, int a4, int a5, int a6);
    CUIMessageBoxBase* AddOK(int a2, int a3, int a4, int a5, int a6);

    CUIMessageBoxBase* AddOKCancelWedding(CUIBase* a2, int a3, int a4, int a5,
                                          int a6, char* a7, CControlBase* a8,
                                          int a9, int a10, int a11,
                                          int a12, int a13, unsigned int a14);

    CUIMessageBoxOK*   AddOKWedding(char* a2);
    CUIMessageBoxOK*   AddOKWedding(int a2);

    CUIMessageBoxBase* AddCalc(CUIBase* a2, int a3, int a4, int a5, int a6,
                               int a7, int a8, int a9, int a10,
                               int a11, int a12, int a13);

    CUIMessageBoxList* AddListA(MsgBoxCallback a2);

    CUIMessageBoxRadioList* AddRadio(MsgBoxCallback a2, int a3, int a4, int a5, int a6);

    CUIMessageBoxSortList*  AddSort(CUIBase* a2, int a3, int a4, int a5, int a6);

    CUIMessageBoxMultLineOK* AddMultLineOK(CUIBase* a2, char* a3, unsigned char a4,
                                           int a5, int a6, int a7, int a8, int a9, int a10);

    CUIMessageBoxBase* AddMultLineOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                           char* a7, char a8, int a9, int a10, int a11,
                                           int a12, int a13, unsigned int a14);

    CUIMessageBoxMultLineOKLarge* AddMultLineOKCancelALarge(MsgBoxCallback a2, int a3,
                                                            char a4, int a5, int a6,
                                                            int a7, int a8, int a9,
                                                            unsigned int a10);
    CUIMessageBoxMultLineOKLarge* AddMultLineOKCancelALarge(MsgBoxCallback a2, char* a3,
                                                            char a4, int a5, int a6,
                                                            int a7, int a8, int a9,
                                                            unsigned int a10);

private:
    stMessageBoxList* m_pHead;    // offset 0 : 鏈結串列頭
};

// ---- 全域指標 ----
extern CMessageBoxManager* g_pMsgBoxMgr;
