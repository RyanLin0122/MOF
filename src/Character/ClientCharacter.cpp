#include "Character/ClientCharacter.h"

#include "Info/cltClassKindInfo.h"
#include "global.h"

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

void ClientCharacter::SetClassCode(unsigned short classCode, int updateNameTag) {
	auto* info = g_clClassKindInfo.GetClassKindInfo(classCode);
	if (!info) {
		return;
	}

	reinterpret_cast<unsigned short*>(this)[4856] = classCode;
	const auto classMark = *(reinterpret_cast<unsigned char*>(info) + 18);
	reinterpret_cast<unsigned char*>(this)[9728] = classMark;
	SetNameTagInfo(classMark, updateNameTag);
}

void ClientCharacter::SetNameTagInfo(unsigned char /*classMark*/, int /*updateNameTag*/) {
	// Ground truth 會進一步更新 NameTag 顏色與 UI。此專案目前缺少完整相依函式，
	// 先保留呼叫路徑以對齊 SetClassCode 的副作用流程。
}

unsigned int ClientCharacter::GetSearchMonster() {
    // Stub: real implementation returns the account ID of the nearest targeted monster.
    return 0;
}

void ClientCharacter::SetOrderAttack(stCharOrder* /*pOrder*/, unsigned int /*targetAccount*/,
                                     int /*a3*/, int /*damage*/, int /*remainHP*/,
                                     int /*a6*/, int /*a7*/) {
    // Stub: real implementation fills pOrder with an attack order payload.
}

void ClientCharacter::SetCurPosition(ClientCharacter* pChar, int x, int y) {
    if (!pChar) return;
    pChar->m_iPosX = x;
    pChar->m_iPosY = y;
}

int ClientCharacter::GetHP() const {
    // Stub: real implementation returns the character's current HP.
    return 0;
}

void ClientCharacter::SetHP() {
    // Stub: real implementation updates the HP bar from the raw HP field.
}

unsigned int ClientCharacter::GetLastOrder() {
    // Stub: real implementation returns the type of the most recently issued order.
    return 0;
}

void ClientCharacter::PushOrder(stCharOrder* /*pOrder*/) {
    // Stub: real implementation enqueues the order onto the character's order queue.
}
