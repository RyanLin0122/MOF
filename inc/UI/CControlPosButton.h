#pragma once
#include <cstdint>
#include <cstring>
#include "UI/CControlImage.h"


// 來自反編譯的定位按鈕：按下時將本體座標位移 (+2,+2)，放開還原 (-2,-2)
// 事件碼：0=Click、3=MouseDown、4=恢復常態（滑出/彈起）
class CControlPosButton : public CControlImage
{
public:
	CControlPosButton();
	virtual ~CControlPosButton();


	// 生命週期
	virtual void Create(CControlBase* pParent) override; // 對齊 0041C100
	virtual void Create(int x, int y, unsigned int giid, unsigned short block,
		CControlBase* pParent); // 對齊 0041C110


	// 事件冒泡：照反編譯流程處理位移與音效，最後上拋給 CControlBase
	virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override; // 0041C140


	// 供 vtbl 呼叫（對齊 88 / 92 偏移出現的兩個成員函式）
	virtual void ButtonPosDown(); // 0041C1C0
	virtual void ButtonPosUp(); // 0041C1E0


private:
	// 反編譯：*((BYTE*)this + 192)
	uint8_t m_bShifted = 0; // 1 表示目前已下壓位移(+2,+2)
	// 反編譯：strcpy(this+193, "J0003")
	char m_szSoundKey[16]; // 用於 GameSound::PlaySoundA 的鍵，預設 "J0003"
};