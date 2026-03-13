#include "System/cltMyMsgSystem.h"


cltMyMsgSystem::ExternInitFn cltMyMsgSystem::m_pExternInitializeFuncPtr = nullptr;

void cltMyMsgSystem::InitializeStaticVariable(ExternInitFn fn) {
    m_pExternInitializeFuncPtr = fn;
}

cltMyMsgSystem::cltMyMsgSystem() = default;

void cltMyMsgSystem::Initialize(void* owner, CMofMsg* msg) {
    m_pOwner = owner;

    if (!msg) {
        m_iMsgNum = 0;
        return;
    }

    msg->Get_LONG(&m_iMsgNum);

    int i = 0;
    while (i < m_iMsgNum && i < 100) {
        msg->Get_INT64(&m_msgs[i].i64MsgID);
        msg->Get_LONG(&m_msgs[i].iType);
        msg->Get_Z1(m_msgs[i].szMsg, 0, 0, nullptr);
        ++i;
    }

    if (m_pExternInitializeFuncPtr) {
        m_pExternInitializeFuncPtr(m_pOwner, this);
    }
}

void cltMyMsgSystem::Free() {
    std::memset(m_msgs, 0, sizeof(m_msgs));
    m_iMsgNum = 0;
}

int cltMyMsgSystem::GetMsgNum() { return m_iMsgNum; }

strMyMsg* cltMyMsgSystem::GetMsg(int index) {
    if (index < 0 || index >= m_iMsgNum) {
        return nullptr;
    }
    return &m_msgs[index];
}

void cltMyMsgSystem::DelMsg(int index) {
    if (index < 0 || index >= m_iMsgNum) {
        return;
    }

    std::memmove(&m_msgs[index], &m_msgs[index + 1], sizeof(strMyMsg) * (m_iMsgNum - (index + 1)));
    --m_iMsgNum;
}

void cltMyMsgSystem::FilloutInfo(CMofMsg* msg) {
    if (!msg) {
        return;
    }

    msg->Put_LONG(m_iMsgNum);
    for (int i = 0; i < m_iMsgNum; ++i) {
        msg->Put(reinterpret_cast<char*>(&m_msgs[i].i64MsgID), 8);
        msg->Put_LONG(m_msgs[i].iType);
        msg->Put_Z1(m_msgs[i].szMsg);
    }
}
