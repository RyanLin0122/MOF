#include "Character/ClientCharacter.h"

/// @brief �غc�禡
ClientCharacter::ClientCharacter() {
	m_wMapID = 0; // ���]��l�a�� ID �� 0
	m_iPosX = 600;
	m_iPosY = 300;
}

/// @brief �����Ѻc�禡
ClientCharacter::~ClientCharacter() {

}

/// @brief ���o���⪺ X �y��
int ClientCharacter::GetPosX() {
	return m_iPosX;
}
/// @brief ���o���⪺ Y �y��
int ClientCharacter::GetPosY() {
	return m_iPosY;
}
void ClientCharacter::SetPosX(float x) {
	m_iPosX = x;
}
/// @brief ���o���⪺ Y �y��
void ClientCharacter::SetPosY(float y) {
	m_iPosY = y;
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