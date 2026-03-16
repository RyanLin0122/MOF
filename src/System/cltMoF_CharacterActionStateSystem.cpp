#include "System/cltMoF_CharacterActionStateSystem.h"

// 反編譯對應：cltMoF_CharacterActionStateSystem::cltMoF_CharacterActionStateSystem
// 原始碼 mofclient.c:292045
cltMoF_CharacterActionStateSystem::cltMoF_CharacterActionStateSystem()
    : m_state(0)
{
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::~cltMoF_CharacterActionStateSystem
// 原始碼 mofclient.c:292066
cltMoF_CharacterActionStateSystem::~cltMoF_CharacterActionStateSystem() = default;

// 反編譯對應：cltMoF_CharacterActionStateSystem::SetCharActionState
// 原始碼 mofclient.c:292073
//
// 原始 IDA 反編譯：
//   v2 = *((_BYTE *)this + 4);
//   if ( v2 != a2 ) {
//     if ( (a2 & 0xF0) != 0 )
//       *((_BYTE *)this + 4) = v2 & 0xF;
//     *((_BYTE *)this + 4) = a2 | *((_BYTE *)this + 4) & 0xF0;
//   }
void cltMoF_CharacterActionStateSystem::SetCharActionState(std::uint8_t a2) {
    if (m_state == a2)
        return;

    if ((a2 & 0xF0) != 0)
        m_state = m_state & 0x0F;

    m_state = a2 | (m_state & 0xF0);
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::ResetState
// 原始碼 mofclient.c:292087
void cltMoF_CharacterActionStateSystem::ResetState() {
    m_state = 0;
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::IsActionStateNormal
// 原始碼 mofclient.c:292093
int cltMoF_CharacterActionStateSystem::IsActionStateNormal() {
    return (m_state & 0x0F) == 0;
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::IsActionStateConversation
// 原始碼 mofclient.c:292099
int cltMoF_CharacterActionStateSystem::IsActionStateConversation() {
    return m_state & 1;
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::IsActionStateAbsence
// 原始碼 mofclient.c:292105
int cltMoF_CharacterActionStateSystem::IsActionStateAbsence() {
    return (m_state >> 1) & 1;
}

// 反編譯對應：cltMoF_CharacterActionStateSystem::IsActionStateTrade
// 原始碼 mofclient.c:292112
int cltMoF_CharacterActionStateSystem::IsActionStateTrade() {
    return (m_state >> 2) & 1;
}
