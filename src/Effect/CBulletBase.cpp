#include "Effect/CBulletBase.h"

// 對應反組譯碼: 0x0052D700
CBulletBase::CBulletBase()
    : m_pVftable(nullptr),
    m_dwOwnerID(0),
    m_fSpeed(0.0f)
{
    // m_vecPos 和 m_vecDir 會被預設初始化
}

// 對應反組譯碼: 0x0052D740
CBulletBase::~CBulletBase()
{
    // 基底類別的解構函式
}

// 對應反組譯碼: 0x0052D750
bool CBulletBase::Process(float fElapsedTime)
{
    // 基底類別的預設行為是“什麼都不做”並且“永不結束”。
    // 衍生類別必須覆寫此函式來實現自己的移動和生命週期邏輯。
    return false;
}