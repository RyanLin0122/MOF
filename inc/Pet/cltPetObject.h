#pragma once

class cltPetObject {
public:
	// �غc�禡
	cltPetObject(float posX, float posY);
	~cltPetObject();

	// ���o�d���� X �y��
	float GetPosX();
	// ���o�d���� Y �y��
	float GetPosY();
	// �]�w�d���� X �y��
	void SetPosX(float posX);
	// �]�w�d���� Y �y��
	void SetPosY(float posY);

private:
	float m_PosX; //�d���� X �y��
	float m_PosY; //�d���� Y �y��
};