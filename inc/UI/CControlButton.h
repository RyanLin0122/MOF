#pragma once
#include <cstdint>
#include "UI/CControlButtonBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

/**
 * @brief 具四態(常態/滑入/按下/停用)外觀的按鈕控制
 * - 狀態圖以 (groupId, imageId, block) 三元組描述
 * - 文字色對應四態顏色
 * - 事件碼依反編譯：0=點擊、3=按下、4=回復常態、7=滑入
 */
class CControlButton : public CControlButtonBase
{
public:
    CControlButton();
    virtual ~CControlButton();

    // 建立：僅掛到父控制
    void Create(CControlBase* pParent);
    // 建立：指定座標 + 父控制
    void Create(int x, int y, CControlBase* pParent);

    // 顯示（反編譯：套用 normal 圖與 normal 文字色）
    void Show();

    // 狀態切換（反編譯：停用/啟用時切換外觀與文字色）
    void NoneActive();
    void Active();

    // ---- 設定圖片（對齊反編譯多載） ----
    // 使用同一 imageId，不同 block 分別對應四態（groupId 使用預設或先前設定值）
    void SetImage(unsigned int imageIdAll,
        uint16_t blockNormal,
        uint16_t blockHover,
        uint16_t blockPressed,
        uint16_t blockDisabled);

    // 僅設定「常態」的 imageId + block（groupId 使用預設或先前設定值）
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal);

    // 為常態/滑入/按下各自指定 imageId + block
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
        unsigned int imageIdHover, uint16_t blockHover,
        unsigned int imageIdPressed, uint16_t blockPressed);

    // 再加上停用（disabled）
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
        unsigned int imageIdHover, uint16_t blockHover,
        unsigned int imageIdPressed, uint16_t blockPressed,
        unsigned int imageIdDisabled, uint16_t blockDisabled);

    // 設定四態的文字顏色（對應 this[177..180]）
    void SetButtonTextColor(uint32_t colNormal,
        uint32_t colHover,
        uint32_t colPressed,
        uint32_t colDisabled);

    // 事件處理（對齊反編譯的 switch-case 與回傳型別）
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

private:
    struct StateImg {
        uint32_t group{ 5 };     // 反編譯預設 5
        uint32_t id{ 0 };
        uint16_t block{ 0xFFFF }; // 0xFFFF 表示無效/不切換
    };

    // 四態資料
    StateImg m_imgNormal{};
    StateImg m_imgHover{};
    StateImg m_imgPressed{};
    StateImg m_imgDisabled{};

    // 四態文字色
    uint32_t m_colNormal{ 0xFFFFFFFF };   // 反編譯：this[177] = -1
    uint32_t m_colHover{ 0xFFEA5400 };   // 反編譯：this[178] = -1376512（依專案色盤可再調整）
    uint32_t m_colPressed{ 0xFFEA5400 };   // 反編譯：this[179] = -1376512
    uint32_t m_colDisabled{ 0xFFC98E33 };  // 反編譯：this[180] = -3566989

    // 目前文字色（反編譯：this[93]）
    uint32_t m_curTextColor{ 0xFFFFFFFF };

private:
    // 套用某狀態的圖塊（僅當 block != 0xFFFF 才切換）
    void ApplyStateImage(const StateImg& st);
    // 套用某狀態的文字色到內建文字
    void ApplyTextColor(uint32_t c);
};
