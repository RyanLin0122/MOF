#pragma once

class ClientCharacter {
public:
    /// @brief �غc�禡
    ClientCharacter();

    /// @brief �����Ѻc�禡
    ~ClientCharacter();

    /// @brief ���o���⪺ X �y��
    int GetPosX();
    /// @brief ���o���⪺ Y �y��
    int GetPosY();
    /// @brief ���o���⪺����
    int GetCharHeight();

    void SetHited(int a, int b);

    bool IsHide();

    bool IsTransparent();

    int GetActionSide();

    int GetHitedInfoNum(int a);

    int GetAccountID();

    bool GetSustainSkillState(unsigned short a);

    int GetPetPosX();

	int GetPetPosY();
};