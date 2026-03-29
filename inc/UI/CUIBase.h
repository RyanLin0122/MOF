#pragma once

struct CUIMessageBoxBase;

class CUIBase {
public:
    virtual ~CUIBase() = default;
    virtual int  GetType() const { return m_nType; }   // 對應 *(_DWORD*)(p+12)
    virtual void OnDragEnd() {}                         // 對應 vtbl+20 呼叫

    void SetChildMessageBox(CUIMessageBoxBase* pMsgBox);

protected:
    int m_nType{ 0 };        // 對應 *((_DWORD*)this + 3)，CUIMessageBoxBase 設為 42
};
