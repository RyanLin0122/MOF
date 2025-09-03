#pragma once
#include <array>
#include <algorithm>
#include <cstring>
#include "UI/CControlBoxBase.h"
#include "UI/CControlButton.h"
#include "UI/CControlText.h"

class CControlBoxQuestAlarm : public CControlBoxBase
{
public:
    CControlBoxQuestAlarm();
    virtual ~CControlBoxQuestAlarm();

    // lifecycle
    void Init();                // 0041B0D0
    void CreateChildren();      // 0041B220
    void Hide();                // 0041B0F0
    void Show();                // 0041B150

    // helpers / accessors
    CControlButton* GetButtonRootTree();          // 0041B320
    CControlText* GetTextRoot();                // 0041B330
    CControlButton* GetButtonChild(int idx);      // 0041B340

    // mutations
    void SetRootName(char* name);                 // 0041B360
    void SetChildText(int idx, char* text);       // 0041B3B0
    int  SetChildTextColor(int idx, float r, float g, float b, float a); // 0041B3E0
    void ClearTextData(int idx);                  // 0041B520
    void ClearTextData();                         // 0041B550
    int  GetMaxLength();                          // 0041B5B0
    int  GetMaxHeight();                          // 0041B660
    void OnClickedButtonRoot();                   // 0041B6D0
    void HideChild();                             // 0041B1E0

private:
    // 成員順序需符合位移：+312, +1036, +1468(×5), +5088(×5)
    CControlButton                 m_rootBtn;       // +312
    CControlText                   m_rootText;      // +1036
    std::array<CControlButton, 5>  m_childBtns;     // +1468
    std::array<CControlText, 5>    m_childTexts;    // +5088

    // 反編譯顯示的尾端旗標（+1812、+1813）
    int m_expandedFlag = 0; // 是否展開（1 展開 / 0 收起）
    int m_reserved = 0; // 未使用，保留一致性
};
