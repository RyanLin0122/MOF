#pragma once
//
// CCmdLine — mofclient.c 還原（位址 0x4FA080~0x4FA0F0）
//
// 把 WinMain 的 lpCmdLine 存起來供之後 ShellExecute 啟動 Recharge.exe 使用。
// 整個 class 就是一個 char buffer：
//   - SetAuthParameter(s) ⇒ strcpy(this, s)
//   - GetAuthParameter()  ⇒ return (CCmdLine*)this  （後續被 cast 為 const char*）
//
// 對應 binary 結構：1024 bytes 之 char[]，dword 開頭即字串第一個字元。
//
class CCmdLine {
public:
    CCmdLine();
    ~CCmdLine() = default;

    int  SetCmdLine(const char* lpCmdLine);
    void SetAuthParameter(const char* str);
    const char* GetAuthParameter() const;

private:
    char m_szAuth[1024];
};
