#include "UI/CUIMenu.h"

#include <cstring>

int* CUIMenu::GetMenuKeyIndex() {
    return m_nMenuKeyIndex;
}

void CUIMenu::SetMenuShortKeyName(char names[6][256]) {
    std::memcpy(m_strMenuName, names, sizeof(m_strMenuName));
}
