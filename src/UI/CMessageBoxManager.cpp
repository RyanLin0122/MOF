#include "UI/CMessageBoxManager.h"
#include "UI/CUIMessageBoxTypes.h"
#include "UI/CUIManager.h"
#include "UI/CUITutorial.h"
#include "global.h"

// ---- 外部函式宣告 ----
extern void ExPrintSystemMessage(char* text, int a2, int a3);

// ================================================================
//  strMsgBoxAttr
// ================================================================
strMsgBoxAttr::strMsgBoxAttr()
    : m_nField0(1)
    , m_nField1(1)
    , m_nField2(0)
    , m_nField3(0)
    , m_nField4(0)
    , m_nField5(0)
    , m_nField6(0)
    , m_nField7(-1)
    , m_nField8(0)
    // m_nField9 未初始化 — 對齊 ground truth
{
}

// ================================================================
//  CUIMessageBoxBase
// ================================================================
CUIMessageBoxBase::CUIMessageBoxBase()
    : m_Attr()
{
    m_nType = 42;   // 對齊反編譯 *((_DWORD *)this + 3) = 42
}

CUIMessageBoxBase::~CUIMessageBoxBase()
{
}

void CUIMessageBoxBase::SetType(CUIBase* a2, int a3, int a4, int a5, int a6,
                                 int a7, int a8, int a9)
{
    // 所有欄位都在 strMsgBoxAttr 內，對齊反編譯 this[826]~this[834]
    m_Attr.m_nField3 = (int)(intptr_t)a2;   // this[829] — parent CUIBase*
    m_Attr.m_nField4 = a3;                   // this[830]
    m_Attr.m_nField5 = a4;                   // this[831]
    m_Attr.m_nField6 = a5;                   // this[832]
    m_Attr.m_nField7 = a6;                   // this[833]
    m_Attr.m_nField8 = a9;                   // this[834] — callback
    m_Attr.m_nField0 = a7;                   // this[826]
    m_Attr.m_nField1 = a8;                   // this[827]

    if (a2)
        a2->SetChildMessageBox(this);
}

void CUIMessageBoxBase::SetType(strMsgBoxAttr* pAttr)
{
    if (pAttr)
    {
        memcpy(&m_Attr, pAttr, 0x28);   // 40 bytes

        CUIBase* parent = (CUIBase*)(intptr_t)m_Attr.m_nField3;
        if (parent)
            parent->SetChildMessageBox(this);
    }
}

strMsgBoxAttr* CUIMessageBoxBase::GetAttr()
{
    return &m_Attr;
}

int CUIMessageBoxBase::KeyboardInputProcess(int a2, int a3, int a4, int a5, int a6, int a7)
{
    if (a2 != 10)
        return 0;
    if (!a6)
        return OnOK(a2, a3, a4, a5, a6, a7);
    if (a6 != 27)
        return 0;
    return OnCancel(a2, a3, a4, a5, a6, a7);
}

// ================================================================
//  stMessageBoxList
// ================================================================
stMessageBoxList::stMessageBoxList()
    : m_pBox(nullptr)
    , m_pPrev(nullptr)
    , m_pNext(nullptr)
{
}

stMessageBoxList::~stMessageBoxList()
{
    if (m_pBox)
    {
        delete m_pBox;
        m_pBox = nullptr;
    }
}

// ================================================================
//  CMsgBoxOptionOK
// ================================================================
CMsgBoxOptionOK::CMsgBoxOptionOK()
    : m_bFlag(1)
    , m_pad1{}
    , m_nField1(0)
    , m_nField2(0)
    , m_nField3(0)
    , m_nField4(0)
    , m_nField5(0)
{
}

// ================================================================
//  CMsgBoxOptionEdit — 預設建構子
// ================================================================
CMsgBoxOptionEdit::CMsgBoxOptionEdit()
    : m_nField0(0)
    , m_nField1(0)
    , m_nField2(0)
    , m_usWidth(126)
    , m_usHeight(0)
    , m_nField4(43)
    , m_nField5(55)
    , m_bField6(1)
    , m_pad1{}
    , m_nField7(0)
    , m_nField8(3386)
    , m_nField9(0)
    , m_nField10(0)
    , m_bField11(0)
    , m_pad2{}
    , m_nField12(0)
    , m_nField13(0)
    , m_nField14(2)
{
}

// ================================================================
//  CMsgBoxOptionEdit — 帶參數建構子
// ================================================================
CMsgBoxOptionEdit::CMsgBoxOptionEdit(int a2)
{
    // 對齊 ground truth：不預先清零，只設定各分支明確寫入的欄位
    // m_nField8 在三個分支中均未被寫入（保持未初始化）
    if (a2 == 0)
    {
        m_nField0  = 12;
        m_nField1  = 0;
        m_nField2  = 0;
        m_usWidth  = 126;
        m_usHeight = 0;
        m_nField4  = 43;
        m_nField5  = 55;
        m_bField6  = 1;
        m_nField7  = 0;
        m_nField13 = 0;
        m_nField14 = 2;
        m_nField9  = 1;
        m_nField10 = 0;
        m_bField11 = 0;
        m_nField12 = 0;
    }
    else if (a2 == 1)
    {
        m_nField0  = 40;
        m_nField1  = 0;
        m_nField2  = 0;
        m_usWidth  = 190;
        m_usHeight = 76;
        m_nField4  = 28;
        m_nField5  = 55;
        m_bField6  = 5;
        m_nField7  = 0;
        m_nField13 = 0;
        m_nField14 = 2;
        m_nField9  = 0;
        m_nField10 = 0;
        m_bField11 = 2;
        m_nField12 = 1;
    }
    else if (a2 == 2)
    {
        m_nField0  = 12;
        m_nField1  = 0;
        m_nField2  = 0;
        m_usWidth  = 157;
        m_usHeight = 0;
        m_nField4  = 28;
        m_nField5  = 55;
        m_bField6  = 1;
        m_nField7  = 0;
        m_nField13 = 1;
        m_nField14 = 2;
        // m_nField9 在 a2==2 分支未被寫入（保持未初始化）
        m_nField10 = 1;
        m_bField11 = 0;
        m_nField12 = 0;
    }
}

// ================================================================
//  CMsgBoxOptionTutorial
// ================================================================
CMsgBoxOptionTutorial::CMsgBoxOptionTutorial()
    : m_nValue(0)
{
}

// ================================================================
//  CMessageBoxManager
// ================================================================
CMessageBoxManager::CMessageBoxManager()
    : m_pHead(nullptr)
{
}

CMessageBoxManager::~CMessageBoxManager()
{
}

// ------------------------------------------------------------
//  InitLogin — 走訪所有訊息框，呼叫 OnInitLogin
// ------------------------------------------------------------
void CMessageBoxManager::InitLogin()
{
    stMessageBoxList* pNode = m_pHead;
    while (pNode)
    {
        if (pNode->m_pBox)
            pNode->m_pBox->OnInitLogin();
        pNode = pNode->m_pNext;
    }
}

// ------------------------------------------------------------
//  AddList — 將 CUIMessageBoxBase 加入雙向鏈結串列尾端
// ------------------------------------------------------------
stMessageBoxList* CMessageBoxManager::AddList(CUIMessageBoxBase* pBox)
{
    stMessageBoxList* pNew = nullptr;

    if (m_pHead)
    {
        // 走到尾端
        stMessageBoxList* pTail = m_pHead;
        while (pTail->m_pNext)
            pTail = pTail->m_pNext;

        pNew = new stMessageBoxList();
        pTail->m_pNext = pNew;
        pNew->m_pPrev = pTail;
    }
    else
    {
        pNew = new stMessageBoxList();
        m_pHead = pNew;
    }

    // 對齊 ground truth：不做 null 保護，直接寫入並 AddOrder
    pNew->m_pBox = pBox;
    g_UIMgr->AddOrder(pBox);
    return pNew;
}

// ------------------------------------------------------------
//  CloseMessagBox
// ------------------------------------------------------------
void CMessageBoxManager::CloseMessagBox(CUIMessageBoxBase* pBox)
{
    if (FindList(pBox))
        pBox->Close();
}

// ------------------------------------------------------------
//  FindList — 搜尋鏈結串列中是否有該 CUIMessageBoxBase
// ------------------------------------------------------------
stMessageBoxList* CMessageBoxManager::FindList(CUIMessageBoxBase* pBox)
{
    stMessageBoxList* pNode = m_pHead;
    while (pNode)
    {
        if (pNode->m_pBox == pBox)
            return pNode;
        pNode = pNode->m_pNext;
    }
    return nullptr;
}

// ------------------------------------------------------------
//  DelList — 從雙向鏈結串列移除節點並釋放
// ------------------------------------------------------------
void CMessageBoxManager::DelList(stMessageBoxList* pNode)
{
    if (pNode)
    {
        g_UIMgr->DelOrder((CUIBase*)pNode->m_pBox);
        g_UIMgr->DeleteFocusWindow((CUIBase*)pNode->m_pBox);

        if (pNode->m_pPrev)
        {
            pNode->m_pPrev->m_pNext = pNode->m_pNext;
            if (pNode->m_pNext)
                pNode->m_pNext->m_pPrev = pNode->m_pPrev;
        }
        else
        {
            // pNode 是 head
            if (pNode->m_pNext)
            {
                m_pHead = pNode->m_pNext;
                m_pHead->m_pPrev = nullptr;
            }
            else
            {
                m_pHead = nullptr;
            }
        }

        pNode->~stMessageBoxList();
        operator delete(pNode);
    }
    else
    {
        g_UIMgr->DeleteFocusWindow(nullptr);
    }
}

// ------------------------------------------------------------
//  Del — 便捷包裝：找到後移除
// ------------------------------------------------------------
void CMessageBoxManager::Del(CUIMessageBoxBase* pBox)
{
    stMessageBoxList* pNode = FindList(pBox);
    DelList(pNode);
}

// ------------------------------------------------------------
//  Add — 工廠方法：依 type 建立對應的訊息框子類別
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::Add(int type, int option, strMsgBoxAttr* pAttr)
{
    if (!this)
        return nullptr;

    CUIMessageBoxBase* pBox = nullptr;

    switch (type)
    {
    case 0:
        pBox = new CUIMessageBoxNotice();
        break;
    case 1:
        pBox = new CUIMessageBoxOK();
        break;
    case 2:
        pBox = new CUIMessageBoxCalc();
        break;
    case 3:
        pBox = new CUIMessageBoxEdit();
        break;
    case 4:
        pBox = new CUIMessageBoxList();
        break;
    case 5:
        pBox = (CUIMessageBoxBase*)(new CUITutorial());
        break;
    case 6:
        pBox = new CUIMessageBoxRadioList();
        break;
    case 7:
        pBox = new CUIMessageBoxSortList();
        break;
    case 8:
        pBox = new CUIMessageBoxMultLineOK();
        break;
    default:
        return nullptr;
    }

    // 對齊 ground truth：即便 pBox 為 null 也繼續呼叫（不提前 return）
    AddList(pBox);
    pBox->SetType(pAttr);
    pBox->Set(option);
    return pBox;
}

// ------------------------------------------------------------
//  AddTutorial
// ------------------------------------------------------------
CUITutorial* CMessageBoxManager::AddTutorial(int a2)
{
    strMsgBoxAttr attr;
    attr.m_nField1 = 0;
    attr.m_nField2 = 0;

    if (a2)
    {
        attr.m_nField3 = 0;
        attr.m_nField0 = 0;
    }
    else
    {
        attr.m_nField3 = (int)(intptr_t)g_UIMgr->GetUIWindow(23);
        attr.m_nField0 = 1;
    }

    attr.m_nField5 = 0;
    attr.m_nField4 = 0;
    attr.m_nField6 = 0;
    attr.m_nField7 = -1;
    attr.m_nField8 = 0;

    CMsgBoxOptionTutorial opt;
    opt.m_nValue = a2;

    return (CUITutorial*)g_pMsgBoxMgr->Add(5, (int)(intptr_t)&opt, &attr);
}

// ------------------------------------------------------------
//  AddNotice
// ------------------------------------------------------------
CUIMessageBoxNotice* CMessageBoxManager::AddNotice(CUIBase* a2, int a3, int a4, int a5)
{
    if (!this)
        return nullptr;

    CUIMessageBoxNotice* pNotice = new CUIMessageBoxNotice();

    AddList(pNotice);
    pNotice->SetType(a2, 0, 0, 0, -1, a4, a5, 0);
    pNotice->Set(a3);
    return pNotice;
}

// ------------------------------------------------------------
//  AddOKCancel (textId 版本)
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                                    int a7, char a8, int a9, int a10, int a11,
                                                    int a12, int a13, unsigned int a14)
{
    if (!this)
        return nullptr;

    char* text = g_DCTTextManager.GetParsedText(a7, 0, 0);
    return AddOKCancel(a2, a3, a4, a5, a6, text, a8, a9, a10, a11, a12, a13, a14);
}

// ------------------------------------------------------------
//  AddOKCancel (char* 版本)
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                                    char* a7, char a8, int a9, int a10, int a11,
                                                    int a12, int a13, unsigned int a14)
{
    if (!this)
        return nullptr;

    if (a14 || g_UIMgr->GetGameState() != 2)
    {
        CUIMessageBoxOK* pOK = new CUIMessageBoxOK();

        AddList(pOK);
        pOK->SetType(a2, a3, a4, a5, a6, a12, a13, 0);
        pOK->Set(a7, a8, a9, a10, a11, a14, 0);
        return pOK;
    }
    else
    {
        ExPrintSystemMessage(a7, 0, 0);
        return nullptr;
    }
}

// ------------------------------------------------------------
//  AddOKCancelA (textId 版本)
// ------------------------------------------------------------
CUIMessageBoxOK* CMessageBoxManager::AddOKCancelA(MsgBoxCallback a2, int a3, char a4,
                                                    int a5, int a6, int a7, int a8, int a9,
                                                    unsigned int a10, int a11)
{
    if (!this)
        return nullptr;

    char* text = g_DCTTextManager.GetParsedText(a3, 0, 0);
    return AddOKCancelA(a2, text, a4, a5, a6, a7, a8, a9, a10, a11);
}

// ------------------------------------------------------------
//  AddOKCancelA (char* 版本)
// ------------------------------------------------------------
CUIMessageBoxOK* CMessageBoxManager::AddOKCancelA(MsgBoxCallback a2, char* a3, char a4,
                                                    int a5, int a6, int a7, int a8, int a9,
                                                    unsigned int a10, int a11)
{
    if (!this)
        return nullptr;

    if (a10 || g_UIMgr->GetGameState() != 2)
    {
        CUIMessageBoxOK* pOK = new CUIMessageBoxOK();

        AddList(pOK);
        pOK->SetType(nullptr, 0, 0, 0, -1, a8, a9, (int)(intptr_t)a2);
        pOK->Set(a3, a4, a5, a6, a7, a10, a11);
        return pOK;
    }
    else
    {
        ExPrintSystemMessage(a3, 0, 0);
        return nullptr;
    }
}

// ------------------------------------------------------------
//  AddOK (4 overloads)
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddOK(CUIBase* a2, int a3, int a4, int a5, int a6,
                                              int a7, int a8, int a9, unsigned int a10)
{
    if (!this)
        return nullptr;
    return AddOKCancel(a2, a4, a5, a6, a7, a3, 2, 0, 0, 3003, a8, a9, a10);
}

CUIMessageBoxBase* CMessageBoxManager::AddOK(CUIBase* a2, char* a3, int a4, int a5, int a6,
                                              int a7, int a8, int a9, unsigned int a10)
{
    if (!this)
        return nullptr;
    return AddOKCancel(a2, a4, a5, a6, a7, a3, 2, 0, 0, 3003, a8, a9, a10);
}

CUIMessageBoxBase* CMessageBoxManager::AddOK(char* a2, int a3, int a4, int a5, int a6)
{
    if (!this)
        return nullptr;
    return AddOKCancel(nullptr, a3, a4, a5, a6, a2, 2, 0, 0, 3003, 0, 0, 0);
}

CUIMessageBoxBase* CMessageBoxManager::AddOK(int a2, int a3, int a4, int a5, int a6)
{
    if (!this)
        return nullptr;
    return AddOKCancel(nullptr, a3, a4, a5, a6, a2, 2, 0, 0, 3003, 0, 0, 0);
}

// ------------------------------------------------------------
//  AddOKCancelWedding
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddOKCancelWedding(CUIBase* a2, int a3, int a4, int a5,
                                                           int a6, char* a7, CControlBase* a8,
                                                           int a9, int a10, int a11,
                                                           int a12, int a13, unsigned int a14)
{
    if (!this)
        return nullptr;

    if (a14 || g_UIMgr->GetGameState() != 2)
    {
        CUIMessageBoxOK* pOK = new CUIMessageBoxOK();

        AddList(pOK);
        pOK->SetType(a2, a3, a4, a5, a6, a12, a13, 0);
        pOK->SetWedding(a7, a8, a9, a10, a11, a14);
        return pOK;
    }
    else
    {
        ExPrintSystemMessage(a7, 0, 0);
        return nullptr;
    }
}

// ------------------------------------------------------------
//  AddOKWedding
// ------------------------------------------------------------
CUIMessageBoxOK* CMessageBoxManager::AddOKWedding(char* a2)
{
    if (!this)
        return nullptr;
    return (CUIMessageBoxOK*)AddOKCancelWedding(nullptr, 0, 0, 0, -1, a2,
                                                 (CControlBase*)2, 0, 0, 3003, 0, 0, 0);
}

CUIMessageBoxOK* CMessageBoxManager::AddOKWedding(int a2)
{
    if (!this)
        return nullptr;

    char* text = g_DCTTextManager.GetParsedText(a2, 0, 0);
    return (CUIMessageBoxOK*)AddOKCancelWedding(nullptr, 0, 0, 0, -1, text,
                                                 (CControlBase*)2, 0, 0, 3003, 0, 0, 0);
}

// ------------------------------------------------------------
//  AddCalc
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddCalc(CUIBase* a2, int a3, int a4, int a5, int a6,
                                                int a7, int a8, int a9, int a10,
                                                int a11, int a12, int a13)
{
    if (!this)
        return nullptr;

    CUIMessageBoxCalc* pCalc = new CUIMessageBoxCalc();

    AddList(pCalc);
    pCalc->SetType(a2, a3, a4, a5, a6, a11, a12, 0);
    pCalc->Set(a7, a8, a9, a10, a13);
    return pCalc;
}

// ------------------------------------------------------------
//  AddListA
// ------------------------------------------------------------
CUIMessageBoxList* CMessageBoxManager::AddListA(MsgBoxCallback a2)
{
    if (!this)
        return nullptr;

    CUIMessageBoxList* pList = new CUIMessageBoxList();

    AddList(pList);
    pList->SetType(nullptr, 0, 0, 0, -1, 0, 0, (int)(intptr_t)a2);
    return pList;
}

// ------------------------------------------------------------
//  AddRadio
// ------------------------------------------------------------
CUIMessageBoxRadioList* CMessageBoxManager::AddRadio(MsgBoxCallback a2, int a3, int a4, int a5, int a6)
{
    if (!this)
        return nullptr;

    CUIMessageBoxRadioList* pRadio = new CUIMessageBoxRadioList();

    AddList(pRadio);
    pRadio->SetType(nullptr, 0, 0, 0, -1, a4, a5, (int)(intptr_t)a2);
    pRadio->Set(a3);
    return pRadio;
}

// ------------------------------------------------------------
//  AddSort
// ------------------------------------------------------------
CUIMessageBoxSortList* CMessageBoxManager::AddSort(CUIBase* a2, int a3, int a4, int a5, int a6)
{
    if (!this)
        return nullptr;

    CUIMessageBoxSortList* pSort = new CUIMessageBoxSortList();

    AddList(pSort);
    pSort->SetType(a2, 0, 0, 0, -1, a4, a5, 0);
    pSort->Set(a3);
    return pSort;
}

// ------------------------------------------------------------
//  AddMultLineOK
// ------------------------------------------------------------
CUIMessageBoxMultLineOK* CMessageBoxManager::AddMultLineOK(CUIBase* a2, char* a3, unsigned char a4,
                                                            int a5, int a6, int a7, int a8, int a9, int a10)
{
    if (!this)
        return nullptr;
    return (CUIMessageBoxMultLineOK*)AddMultLineOKCancel(a2, 0, 0, 0, -1, a3, 1, a5, a6, a7, a8, a9, a10);
}

// ------------------------------------------------------------
//  AddMultLineOKCancel
// ------------------------------------------------------------
CUIMessageBoxBase* CMessageBoxManager::AddMultLineOKCancel(CUIBase* a2, int a3, int a4, int a5, int a6,
                                                            char* a7, char a8, int a9, int a10, int a11,
                                                            int a12, int a13, unsigned int a14)
{
    if (!this)
        return nullptr;

    if (a14 || g_UIMgr->GetGameState() != 2)
    {
        CUIMessageBoxMultLineOK* pMLOK = new CUIMessageBoxMultLineOK();

        AddList(pMLOK);
        pMLOK->SetType(a2, a3, a4, a5, a6, a12, a13, 0);
        pMLOK->Set(a7, a8, a9, a10, a11, a14);
        return pMLOK;
    }
    else
    {
        ExPrintSystemMessage(a7, 0, 0);
        return nullptr;
    }
}

// ------------------------------------------------------------
//  AddMultLineOKCancelALarge (textId 版本)
// ------------------------------------------------------------
CUIMessageBoxMultLineOKLarge* CMessageBoxManager::AddMultLineOKCancelALarge(MsgBoxCallback a2, int a3,
                                                                             char a4, int a5, int a6,
                                                                             int a7, int a8, int a9,
                                                                             unsigned int a10)
{
    if (!this)
        return nullptr;

    char* text = g_DCTTextManager.GetParsedText(a3, 0, 0);
    return AddMultLineOKCancelALarge(a2, text, a4, a5, a6, a7, a8, a9, a10);
}

// ------------------------------------------------------------
//  AddMultLineOKCancelALarge (char* 版本)
// ------------------------------------------------------------
CUIMessageBoxMultLineOKLarge* CMessageBoxManager::AddMultLineOKCancelALarge(MsgBoxCallback a2, char* a3,
                                                                             char a4, int a5, int a6,
                                                                             int a7, int a8, int a9,
                                                                             unsigned int a10)
{
    if (!this)
        return nullptr;

    if (a10 || g_UIMgr->GetGameState() != 2)
    {
        CUIMessageBoxMultLineOKLarge* pLarge = new CUIMessageBoxMultLineOKLarge();

        AddList(pLarge);
        pLarge->SetType(nullptr, 0, 0, 0, -1, a8, a9, (int)(intptr_t)a2);
        pLarge->Set(a3, a4, a5, a6, a7, a10);
        return pLarge;
    }
    else
    {
        ExPrintSystemMessage(a3, 0, 0);
        return nullptr;
    }
}
