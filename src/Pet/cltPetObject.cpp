#include "Pet/cltPetObject.h"


// �غc�禡
cltPetObject::cltPetObject(float posX, float posY) : m_PosX(posX), m_PosY(posY) {}
// �����Ѻc�禡
cltPetObject::~cltPetObject() {}

// ���o�d���� X �y��
float cltPetObject::GetPosX(){
	return m_PosX;
}
// ���o�d���� Y �y��
float cltPetObject::GetPosY(){
	return m_PosY;
}
// �]�w�d���� X �y��
void cltPetObject::SetPosX(float posX) {
	m_PosX = posX;
}
// �]�w�d���� Y �y��
void cltPetObject::SetPosY(float posY) {
	m_PosY = posY;
}
