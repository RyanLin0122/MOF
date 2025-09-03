#include "UI/CControlPosButton.h"
#include "global.h"

// 反編譯：建構 → CControlImage::CControlImage; vftable 指向本類；strcpy(this+193, "J0003"); *((BYTE*)this+192)=0
CControlPosButton::CControlPosButton()
{
	std::strcpy(m_szSoundKey, "J0003");
	m_bShifted = 0;
}


CControlPosButton::~CControlPosButton()
{
	// 交由基底正常解構即可
}


// 0041C100：Create(this, parent) → 呼叫 CControlBase::Create
void CControlPosButton::Create(CControlBase* pParent)
{
	CControlBase::Create(pParent);
}


// 0041C110：Create(this, x,y, giid, block, parent) → 呼叫 CControlImage::Create 多載
void CControlPosButton::Create(int x, int y, unsigned int giid, unsigned short block, CControlBase* pParent)
{
	CControlImage::Create(x, y, giid, block, pParent);
}


// 0041C140：事件流程
int* CControlPosButton::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
	if (msg)
	{
		if (msg == 3)
		{
			// vtbl+92
			ButtonPosUp();
			g_LButtonUp = 1;
		}
		else if (msg == 4)
		{
			// vtbl+92
			ButtonPosUp();
		}
	}
	else
	{
		// vtbl+88
		ButtonPosDown();
		//GameSound::PlaySoundA(&g_GameSoundManager, m_szSoundKey, 0, 0);
		g_LButtonUp = 0;
	}


	return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}


// 0041C1C0
void CControlPosButton::ButtonPosDown()
{
	if (!m_bShifted)
	{
		CControlBase::MoveWindow(2, 2);
		m_bShifted = 1;
	}
}


// 0041C1E0
void CControlPosButton::ButtonPosUp()
{
	if (m_bShifted)
	{
		CControlBase::MoveWindow(-2, -2);
		m_bShifted = 0;
	}
}