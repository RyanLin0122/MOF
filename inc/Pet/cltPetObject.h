#pragma once

class cltPetObject {
public:
	// 建構函式
	cltPetObject(float posX, float posY);
	~cltPetObject();

	// 取得寵物的 X 座標
	float GetPosX();
	// 取得寵物的 Y 座標
	float GetPosY();
	// 設定寵物的 X 座標
	void SetPosX(float posX);
	// 設定寵物的 Y 座標
	void SetPosY(float posY);

private:
	float m_PosX; //寵物的 X 座標
	float m_PosY; //寵物的 Y 座標
};