#pragma once

#include <cstdint>

// 反編譯對應：cltMoF_CharacterActionStateSystem
// 嵌入 ClientCharacter 偏移量 +11544
// 狀態位元欄位：低 4 位 = 動作狀態，高 4 位 = 修飾旗標
class cltMoF_CharacterActionStateSystem {
public:
    cltMoF_CharacterActionStateSystem();
    virtual ~cltMoF_CharacterActionStateSystem();

    // 設定角色動作狀態
    // 若 a2 的高 4 位不為零，先清除目前的低 4 位
    // 然後將 a2 的低 4 位 OR 到目前狀態的高 4 位上
    void SetCharActionState(std::uint8_t a2);

    // 重設為 0
    void ResetState();

    // 低 4 位 == 0 即為正常
    int IsActionStateNormal();

    // bit 0: 對話狀態
    int IsActionStateConversation();

    // bit 1: 離席狀態
    int IsActionStateAbsence();

    // bit 2: 交易狀態
    int IsActionStateTrade();

private:
    std::uint8_t m_state = 0;
};
