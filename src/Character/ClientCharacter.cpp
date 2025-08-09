#include "Character/ClientCharacter.h"

/// @brief 建構函式
ClientCharacter::ClientCharacter() {
	m_wMapID = 0; // 假設初始地圖 ID 為 0
	m_iPosX = 600;
	m_iPosY = 300;
}

/// @brief 虛擬解構函式
ClientCharacter::~ClientCharacter() {

}

/// @brief 取得角色的 X 座標
int ClientCharacter::GetPosX() {
	return m_iPosX;
}
/// @brief 取得角色的 Y 座標
int ClientCharacter::GetPosY() {
	return m_iPosY;
}
void ClientCharacter::SetPosX(float x) {
	m_iPosX = x;
}
/// @brief 取得角色的 Y 座標
void ClientCharacter::SetPosY(float y) {
	m_iPosY = y;
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