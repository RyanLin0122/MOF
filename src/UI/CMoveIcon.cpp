#include "UI/CMoveIcon.h"
#include "Image/cltImageManager.h" // 只為了與 CControlImage 一致，無直接使用

// ------------------------------------------------------------
// 建構 / 解構（依照反編譯初始化 + 文字控制建立）
// ------------------------------------------------------------
CMoveIcon::CMoveIcon()
{
    // 基底先建構
    // 清 64 組槽位
    std::memset(m_slots, 0, sizeof(m_slots));

    // 內嵌文字控制建構 + 掛到自己底下
    m_Text.Create(this);
    m_Text.SetControlSetFont("MoveIconText");
    m_Text.SetMultiLineSpace(0);
    m_Text.SetMultiLineSize(36, 36);
    // 等價 *((DWORD*)this + 217) = 1; 讓文字以中心為原點
    m_Text.SetCenterOrigin(true);
    // 等價 *((DWORD*)this + 218) = -5418489; 文字主色
    m_Text.SetMainColor(0xFFACACC7u); // -5418489 (若使用帶號整數，可直接寫 -5418489)

    // 其它欄位在成員宣告處已有預設值：m_bAutoTerminate=1, m_bStarted=0, m_setCount=0
    InitData();
}

CMoveIcon::~CMoveIcon()
{
    // 反編譯順序：先 ~CControlText，再 ~CControlImage
    // C++ 會自動依成員/繼承順序解構，無需手動
}

// ------------------------------------------------------------
// 內部初始化（0x426920）
// ------------------------------------------------------------
void CMoveIcon::InitData()
{
    m_bAutoTerminate = 1; // +50
    m_setCount = 0;       // +720
    m_pPutUI = nullptr;   // +49

    // 清 64 組槽位（等價從 +208 起每兩個 DWORD 清 0）
    std::memset(m_slots, 0, sizeof(m_slots));

    // 文字控制清資料（等價呼叫 m_Text.vtbl+36）
    m_Text.ClearData();
}

// ------------------------------------------------------------
// Start：只開標誌與狀態，必要時關閉自動終結（0x426960）
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI, int autoTerminateFlag)
{
    if (m_bStarted)          return 0;      // 已在進行
    if (IsVisible())         return 0;      // 反編譯以 +12 判斷；這裡用 IsVisible()

    m_bStarted = 1;                          // +51
    m_pDownUI = pDownUI;                    // +48
    InitData();                              // 重置槽位/文字

    m_nShadeAlpha = 0;                       // +28
    if (autoTerminateFlag == 0)
        SetNotAutoTerminate();

    return 1;
}

// ------------------------------------------------------------
// Start(群組/資源/區塊 + 置中 + 顯示)（0x4269C0）
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI,
    unsigned int giGroup, unsigned int giid, unsigned short block,
    int centerAbsX, int centerAbsY)
{
    if (!Start(pDownUI, /*autoTerminateFlag*/1))
        return 0;

    SetImageID(giGroup, giid, block);
    SetCenterPos(centerAbsX, centerAbsY);
    Show();         // vtbl+40
    return 1;
}

// ------------------------------------------------------------
// Start(由來源控制複製 GI 與 shade + 置中 + 顯示)（0x426A20）
// （反編譯以 a3+120/124/128/112 取值；這裡改由參數直接傳入）
// ------------------------------------------------------------
int CMoveIcon::Start(CUIBase* pDownUI,
    unsigned int srcGIGroup, unsigned int srcGIID, unsigned short srcBlock,
    int srcShadeAlpha,
    int centerAbsX, int centerAbsY)
{
    if (!Start(pDownUI, /*autoTerminateFlag*/1))
        return 0;

    SetImageID(srcGIGroup, srcGIID, srcBlock);
    m_nShadeAlpha = srcShadeAlpha;     // +28
    SetCenterPos(centerAbsX, centerAbsY);
    Show();                            // vtbl+40
    return 1;
}

// ------------------------------------------------------------
// Data：增/覆/取/查（0x426AA0/…）
// ------------------------------------------------------------
void CMoveIcon::AddData(uint8_t idx, unsigned int value)
{
    if (!m_bStarted) return;
    if (idx >= 64)   return;

    Slot& s = m_slots[idx];
    if (s.isSet == 0) {
        s.isSet = 1;
        s.value = value;
        ++m_setCount; // +720
    }
}

void CMoveIcon::OverlapData(uint8_t idx, unsigned int value)
{
    if (!m_bStarted) return;
    if (idx >= 64)   return;

    Slot& s = m_slots[idx];
    if (s.isSet) {
        s.isSet = 1;
        s.value = value;
    }
}

unsigned int CMoveIcon::GetData(uint8_t idx)
{
    if (!m_bStarted) return 0;
    if (idx >= 64)   return 0;
    if (!m_slots[idx].isSet) return 0;

    ClientCharacterManager::GetInstance()->ResetMoveTarget(); // 對齊反編譯副作用
    return m_slots[idx].value;
}

int CMoveIcon::IsSetData(uint8_t idx) const
{
    if (idx >= 64) return 0;
    return m_slots[idx].isSet ? 1 : 0;
}

// ------------------------------------------------------------
// 移動（0x426BA0）：僅在可見時才 SetCenterPos
// ------------------------------------------------------------
void CMoveIcon::Move(int absX, int absY)
{
    if (IsVisible())
        SetCenterPos(absX, absY);
}

// ------------------------------------------------------------
// 放置（0x426BC0 / 0x426C10）
// ------------------------------------------------------------
int CMoveIcon::Put(CUIBase* pTargetUI)
{
    if (!m_bStarted)        return 0;
    if (!m_bAutoTerminate)  return 0;

    // 清目前顯示的圖片
    m_usBlockID = 0xFFFF;      // 匹配 *((WORD*)this+64) = -1
    m_pPutUI = pTargetUI;   // +49
    m_usWidth = 0;           // +16
    m_usHeight = 0;           // +17
    m_fadeCurA = 0;           // +39（來自 CControlImage）
    SetVisible(false);         // +12 = 0（反編譯直接寫 0）

    return 1;
}

int CMoveIcon::Put(int uiId)
{
    //if (!g_UIMgr) return 0;
    //CUIBase* p = g_UIMgr->GetUIWindow(uiId);
    return 0;// Put(p);
}

// ------------------------------------------------------------
// 判斷拖曳起點 UI 類型（0x426C30 / 0x426C60）
// ------------------------------------------------------------
int CMoveIcon::IsDownUIType(int typeId) const
{
    if (!m_bStarted)  return 0;
    if (!m_pDownUI)   return 0;
    return (m_pDownUI->GetType() == typeId) ? 1 : 0;
}

int CMoveIcon::IsDownUIType(CUIBase* pUI) const
{
    return (pUI ? IsDownUIType(pUI->GetType()) : 0);
}

// ------------------------------------------------------------
// 自動終結 / 強制終結（0x426C70 / 0x426C80 / 0x426CD0）
// ------------------------------------------------------------
void CMoveIcon::SetNotAutoTerminate()
{
    m_bAutoTerminate = 0;  // +50 = 0
}

void CMoveIcon::Terminate()
{
    if (!m_bStarted) return;

    m_bStarted = 0;        // +51 = 0

    Hide();                // 反編譯 vtbl+44 的無 this 呼叫，這裡用 Hide()

    // 通知 UI
    if (m_pDownUI)
        m_pDownUI->OnDragEnd();
    if (m_pPutUI && m_pPutUI != m_pDownUI)
        m_pPutUI->OnDragEnd();

    InitData();
}

int CMoveIcon::AutoTerminate()
{
    if (!m_bStarted)       return 0;
    if (!m_bAutoTerminate) return 0;
    Terminate();
    return 1;
}