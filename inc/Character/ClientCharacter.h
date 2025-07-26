#pragma once

class ClientCharacter {
public:
    /// @brief 建構函式
    ClientCharacter();

    /// @brief 虛擬解構函式
    ~ClientCharacter();

    /// @brief 取得角色的 X 座標
    int GetPosX();
    /// @brief 取得角色的 Y 座標
    int GetPosY();
    /// @brief 取得角色的高度
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