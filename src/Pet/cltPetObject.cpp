#include "Pet/cltPetObject.h"


// 建構函式
cltPetObject::cltPetObject(float posX, float posY) : m_PosX(posX), m_PosY(posY) {}
// 虛擬解構函式
cltPetObject::~cltPetObject() {}

// 取得寵物的 X 座標
float cltPetObject::GetPosX(){
	return m_PosX;
}
// 取得寵物的 Y 座標
float cltPetObject::GetPosY(){
	return m_PosY;
}
// 設定寵物的 X 座標
void cltPetObject::SetPosX(float posX) {
	m_PosX = posX;
}
// 設定寵物的 Y 座標
void cltPetObject::SetPosY(float posY) {
	m_PosY = posY;
}
