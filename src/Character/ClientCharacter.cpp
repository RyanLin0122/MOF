#include "Character/ClientCharacter.h"

/// @brief �غc�禡
ClientCharacter::ClientCharacter() {

}

/// @brief �����Ѻc�禡
ClientCharacter::~ClientCharacter() {

}

/// @brief ���o���⪺ X �y��
int ClientCharacter::GetPosX() {
	return 600;
}
/// @brief ���o���⪺ Y �y��
int ClientCharacter::GetPosY() {
	return 300;
}
/// @brief ���o���⪺����
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