#pragma once

#include "UI/CMessageBoxManager.h"

class CControlBase;

// ============================================================
//  CUIMessageBox 子類別最小宣告
//  各自的建構子與方法實作待其他翻譯單元還原
//  此處僅宣告 CMessageBoxManager 直接呼叫的方法
// ============================================================

class CUIMessageBoxNotice : public CUIMessageBoxBase
{
public:
    CUIMessageBoxNotice();
    virtual ~CUIMessageBoxNotice();
    void Set(int a2);
};

class CUIMessageBoxOK : public CUIMessageBoxBase
{
public:
    CUIMessageBoxOK();
    virtual ~CUIMessageBoxOK();
    void Set(char* text, char mode, int a3, int a4, int a5, unsigned int a6, int a7);
    void SetWedding(char* text, CControlBase* a3, int a4, int a5, int a6, unsigned int a7);
    void SetTimer(int a2, int a3, int a4, int a5);
};

class CUIMessageBoxCalc : public CUIMessageBoxBase
{
public:
    CUIMessageBoxCalc();
    virtual ~CUIMessageBoxCalc();
    void Set(int a2, int a3, int a4, int a5, int a6);
};

class CUIMessageBoxEdit : public CUIMessageBoxBase
{
public:
    CUIMessageBoxEdit();
    virtual ~CUIMessageBoxEdit();
};

class CUIMessageBoxList : public CUIMessageBoxBase
{
public:
    CUIMessageBoxList();
    virtual ~CUIMessageBoxList();
};

class CUIMessageBoxRadioList : public CUIMessageBoxBase
{
public:
    CUIMessageBoxRadioList();
    virtual ~CUIMessageBoxRadioList();
    void Set(int a2);
};

class CUIMessageBoxSortList : public CUIMessageBoxBase
{
public:
    CUIMessageBoxSortList();
    virtual ~CUIMessageBoxSortList();
    void Set(int a2);
};

class CUIMessageBoxMultLineOK : public CUIMessageBoxBase
{
public:
    CUIMessageBoxMultLineOK();
    virtual ~CUIMessageBoxMultLineOK();
    void Set(char* text, char mode, int a3, int a4, int a5, unsigned int a6);
};

class CUIMessageBoxMultLineOKLarge : public CUIMessageBoxBase
{
public:
    CUIMessageBoxMultLineOKLarge();
    virtual ~CUIMessageBoxMultLineOKLarge();
    void Set(char* text, char mode, int a3, int a4, int a5, unsigned int a6);
};
