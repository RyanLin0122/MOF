#include "Character/ClientCharacter.h"

/// @brief 建構函式
ClientCharacter::ClientCharacter() {

}

/// @brief 虛擬解構函式
ClientCharacter::~ClientCharacter() {

}

/// @brief 取得角色的 X 座標
int ClientCharacter::GetPosX() {
	return 600;
}
/// @brief 取得角色的 Y 座標
int ClientCharacter::GetPosY() {
	return 300;
}
/// @brief 取得角色的高度
int ClientCharacter::GetCharHeight() {
	return 100;
}

void ClientCharacter::SetHited(int a, int b)
{
	return;
}

bool ClientCharacter::IsHide() {
	return false;
}

bool ClientCharacter::IsTransparent() {
	return false;
}

int ClientCharacter::GetActionSide() {
	return 1;
}

int ClientCharacter::GetHitedInfoNum(int a) {
	return 1;
}

int ClientCharacter::GetAccountID() {
	return 1;
}

bool ClientCharacter::GetSustainSkillState(unsigned short a) {
	return true;
}

int ClientCharacter::GetPetPosX() {
	return 0;
}

int ClientCharacter::GetPetPosY() {
	return 0;
}