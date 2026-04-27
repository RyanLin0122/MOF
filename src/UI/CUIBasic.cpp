#include "UI/CUIBasic.h"

#include <cstring>

CUIBasic::CUIBasic() = default;
CUIBasic::~CUIBasic() = default;

void CUIBasic::SetChat(char* /*buffer*/) {}
void CUIBasic::SetInputChat(int /*a2*/, int /*a3*/, char* /*source*/, int /*cursorBlink*/, char* /*composed*/) {}
void CUIBasic::ReceivedWhisper(char* /*name*/) {}

// mofclient.c：婚禮地圖切換重置 / 復活 UI 關閉 / debug 浮動文字。
// 真實實作牽涉到尚未還原的子 widget；這裡僅維持呼叫面。
void CUIBasic::InitWeddingMap() {}
void CUIBasic::ResurrectDiedChar() {}
void CUIBasic::SetChatMoveTextBonusEvent(const char* /*text*/) {}

void CUIBasic::SetQSLShortKeyName(char names[12][256]) {
    std::memcpy(m_strQSLName, names, sizeof(m_strQSLName));
}

void CUIBasic::SetQSLPageShortKeyName(char names[2][256]) {
    std::memcpy(m_strQSLPageName, names, sizeof(m_strQSLPageName));
}

void CUIBasic::SetMenuShortKeyName(char names[10][256]) {
    std::memcpy(m_strMenuName, names, sizeof(m_strMenuName));
}

int* CUIBasic::GetMenuKeyIndex() {
    return m_nMenuKeyIndex;
}
