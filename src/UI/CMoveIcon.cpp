#include "UI/CMoveIcon.h"
#include "UI/CUIBase.h"
#include "UI/CUIManager.h"
#include "global.h"
#include "Character/ClientCharacterManager.h"

// ------------------------------------------------------------
// 建構 / 解構
// ------------------------------------------------------------
CMoveIcon::CMoveIcon()
{
	// 清 64 組槽位
	memset(m_slots, 0, sizeof(m_slots));

	// ground truth 順序：先設定初始狀態，再設定文字控制
	m_pDownUI = nullptr;
	m_pPutUI = nullptr;
	m_bStarted = 0;
	InitData();

	// 內嵌文字控制：掛到自己底下（ground truth: InitData 之後）
	m_Text.Create(this);
	m_Text.SetControlSetFont("MoveIconText");
	m_Text.SetMultiLineSpace(0);
	m_Text.SetMultiLineSize(36, 36);
	m_Text.SetCenterOrigin(true);
	// 對齊反編譯 *((DWORD*)this + 218) = -5418489 → 0xFFACACC7
	m_Text.SetTextColor(0xFFACACC7u);
}

CMoveIcon::~CMoveIcon()
{
	// C++ 自動依成員/繼承順序解構
}

// ------------------------------------------------------------
// 內部初始化
// ------------------------------------------------------------
void CMoveIcon::InitData()
{
	m_bAutoTerminate = 1;
	m_setCount = 0;
	m_pPutUI = nullptr;

	memset(m_slots, 0, sizeof(m_slots));

	// 文字控制清資料（vtbl+36）
	m_Text.ClearData();
}

// ------------------------------------------------------------
// Start：3 參數版本
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI, int autoTerminateFlag)
{
	if (m_bStarted)
		return 0;
	if (IsVisible())
		return 0;

	m_bStarted = 1;
	m_pDownUI = pDownUI;
	InitData();

	// ground truth: *((_DWORD *)this + 28) = 0; → 清除 CControlBase 的 m_bCenterOrigin
	m_bCenterOrigin = false;
	if (autoTerminateFlag == 0)
		SetNotAutoTerminate();

	return 1;
}

// ------------------------------------------------------------
// Start：以群組/GI/block + 置中座標啟動
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI,
	unsigned int giGroup, unsigned int giid, unsigned short block,
	int centerAbsX, int centerAbsY)
{
	int result = Start(pDownUI, 1);
	if (!result)
		return 0;

	SetImageID(giGroup, giid, block);
	SetCenterPos(centerAbsX, centerAbsY);
	Show();
	return 1;
}

// ------------------------------------------------------------
// Start：從來源 CControlImage 複製 GI 與 shade
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI,
	CControlImage* pSrcCtrl,
	int centerAbsX, int centerAbsY)
{
	int result = Start(pDownUI, 1);
	if (!result)
		return 0;

	SetImageID((unsigned int)pSrcCtrl->m_nGIGroup, (unsigned int)pSrcCtrl->m_nGIID, pSrcCtrl->m_usBlockID);
	// ground truth: *((_DWORD *)this + 28) = *(_DWORD *)(a3 + 112);
	// 從來源控制元件複製 m_bCenterOrigin（CControlBase DWORD 28）
	m_bCenterOrigin = pSrcCtrl->GetCenterOrigin();
	SetCenterPos(centerAbsX, centerAbsY);
	Show();
	return 1;
}

// ------------------------------------------------------------
// Data：增/覆/取/查
// ------------------------------------------------------------
void CMoveIcon::AddData(uint8_t idx, unsigned int value)
{
	if (!m_bStarted) return;

	Slot& s = m_slots[idx];
	if (s.isSet == 0) {
		s.isSet = 1;
		s.value = value;
		++m_setCount;
	}
}

void CMoveIcon::OverlapData(uint8_t idx, unsigned int value)
{
	if (!m_bStarted) return;

	Slot& s = m_slots[idx];
	if (s.isSet) {
		s.isSet = 1;
		s.value = value;
	}
}

unsigned int CMoveIcon::GetData(uint8_t idx)
{
	if (!m_bStarted) return 0;
	if (!m_slots[idx].isSet) return 0;

	g_ClientCharMgr.ResetMoveTarget();
	return m_slots[idx].value;
}

int CMoveIcon::IsSetData(uint8_t idx)
{
	return m_slots[idx].isSet;
}

// ------------------------------------------------------------
// IsStart（ground truth: 獨立函式，回傳 *((_DWORD *)this + 51)）
// ------------------------------------------------------------
int CMoveIcon::IsStart()
{
	return m_bStarted;
}

// ------------------------------------------------------------
// GetTextControl（ground truth: 回傳 (char*)this + 724）
// ------------------------------------------------------------
CControlText* CMoveIcon::GetTextControl()
{
	return &m_Text;
}

// ------------------------------------------------------------
// 移動
// ------------------------------------------------------------
void CMoveIcon::Move(int absX, int absY)
{
	if (IsVisible())
		SetCenterPos(absX, absY);
}

// ------------------------------------------------------------
// 放置
// ------------------------------------------------------------
int CMoveIcon::Put(CUIBase* pTargetUI)
{
	if (!m_bStarted) return 0;
	if (!m_bAutoTerminate) return 0;

	m_usBlockID = 0xFFFF;
	m_pPutUI = pTargetUI;
	m_usWidth = 0;
	m_usHeight = 0;
	m_fadeCurA = 0;
	SetVisible(false);

	return 1;
}

int CMoveIcon::Put(int uiId)
{
	CUIBase* p = g_UIMgr->GetUIWindow(uiId);
	return Put(p);
}

// ------------------------------------------------------------
// 判斷拖曳起點 UI 類型
// ------------------------------------------------------------
int CMoveIcon::IsDownUIType(int typeId)
{
	if (!m_bStarted) return 0;
	if (!m_pDownUI) return 0;
	return (m_pDownUI->GetType() == typeId) ? 1 : 0;
}

int CMoveIcon::IsDownUIType(CUIBase* pUI)
{
	return IsDownUIType(pUI->GetType());
}

// ------------------------------------------------------------
// 自動終結 / 強制終結
// ------------------------------------------------------------
void CMoveIcon::SetNotAutoTerminate()
{
	m_bAutoTerminate = 0;
}

void CMoveIcon::Terminate()
{
	if (!m_bStarted) return;

	m_bStarted = 0;
	Hide();

	if (m_pDownUI)
		m_pDownUI->OnDragEnd();
	if (m_pPutUI && m_pPutUI != m_pDownUI)
		m_pPutUI->OnDragEnd();

	InitData();
}

int CMoveIcon::AutoTerminate()
{
	if (!m_bStarted) return 0;
	if (!m_bAutoTerminate) return 0;
	Terminate();
	return 1;
}
