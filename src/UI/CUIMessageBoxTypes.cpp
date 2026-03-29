#include "UI/CUIMessageBoxTypes.h"

// ================================================================
//  CUIMessageBoxNotice
// ================================================================
CUIMessageBoxNotice::CUIMessageBoxNotice() {}
CUIMessageBoxNotice::~CUIMessageBoxNotice() {}
void CUIMessageBoxNotice::Set(int /*a2*/) {}

// ================================================================
//  CUIMessageBoxOK
// ================================================================
CUIMessageBoxOK::CUIMessageBoxOK() {}
CUIMessageBoxOK::~CUIMessageBoxOK() {}

void CUIMessageBoxOK::Set(char* /*text*/, char /*mode*/, int /*a3*/, int /*a4*/,
                           int /*a5*/, unsigned int /*a6*/, int /*a7*/) {}

void CUIMessageBoxOK::SetWedding(char* /*text*/, CControlBase* /*a3*/,
                                  int /*a4*/, int /*a5*/, int /*a6*/,
                                  unsigned int /*a7*/) {}

void CUIMessageBoxOK::SetTimer(int /*a2*/, int /*a3*/, int /*a4*/, int /*a5*/) {}

// ================================================================
//  CUIMessageBoxCalc
// ================================================================
CUIMessageBoxCalc::CUIMessageBoxCalc() {}
CUIMessageBoxCalc::~CUIMessageBoxCalc() {}
void CUIMessageBoxCalc::Set(int /*a2*/, int /*a3*/, int /*a4*/, int /*a5*/, int /*a6*/) {}

// ================================================================
//  CUIMessageBoxEdit
// ================================================================
CUIMessageBoxEdit::CUIMessageBoxEdit() {}
CUIMessageBoxEdit::~CUIMessageBoxEdit() {}

// ================================================================
//  CUIMessageBoxList
// ================================================================
CUIMessageBoxList::CUIMessageBoxList() {}
CUIMessageBoxList::~CUIMessageBoxList() {}

// ================================================================
//  CUIMessageBoxRadioList
// ================================================================
CUIMessageBoxRadioList::CUIMessageBoxRadioList() {}
CUIMessageBoxRadioList::~CUIMessageBoxRadioList() {}
void CUIMessageBoxRadioList::Set(int /*a2*/) {}

// ================================================================
//  CUIMessageBoxSortList
// ================================================================
CUIMessageBoxSortList::CUIMessageBoxSortList() {}
CUIMessageBoxSortList::~CUIMessageBoxSortList() {}
void CUIMessageBoxSortList::Set(int /*a2*/) {}

// ================================================================
//  CUIMessageBoxMultLineOK
// ================================================================
CUIMessageBoxMultLineOK::CUIMessageBoxMultLineOK() {}
CUIMessageBoxMultLineOK::~CUIMessageBoxMultLineOK() {}
void CUIMessageBoxMultLineOK::Set(char* /*text*/, char /*mode*/,
                                   int /*a3*/, int /*a4*/, int /*a5*/,
                                   unsigned int /*a6*/) {}

// ================================================================
//  CUIMessageBoxMultLineOKLarge
// ================================================================
CUIMessageBoxMultLineOKLarge::CUIMessageBoxMultLineOKLarge() {}
CUIMessageBoxMultLineOKLarge::~CUIMessageBoxMultLineOKLarge() {}
void CUIMessageBoxMultLineOKLarge::Set(char* /*text*/, char /*mode*/,
                                        int /*a3*/, int /*a4*/, int /*a5*/,
                                        unsigned int /*a6*/) {}
