#pragma once

#include <cstdint>

class CMofMsg;

struct strMyMsg {
    std::int64_t i64MsgID = 0;
    int iType = 0;
    char szMsg[260]{};
};

class cltMyMsgSystem {
public:
    using ExternInitFn = void(__cdecl*)(void*, cltMyMsgSystem*);

    static void InitializeStaticVariable(ExternInitFn fn);

    cltMyMsgSystem();

    void Initialize(void* owner, CMofMsg* msg);
    void Free();

    int GetMsgNum();
    strMyMsg* GetMsg(int index);
    void DelMsg(int index);
    void FilloutInfo(CMofMsg* msg);

private:
    static ExternInitFn m_pExternInitializeFuncPtr;

    void* m_pOwner = nullptr;
    strMyMsg m_msgs[100]{};
    int m_iMsgNum = 0;
};
