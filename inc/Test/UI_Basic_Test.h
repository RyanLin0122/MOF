#pragma once
#include <d3d9.h>
#include "UI/CControlAlphaBox.h"
#include "UI/CControlText.h"

/**
 * @class UIBasicTest
 * @brief 用於測試基礎 UI 控制項 (CControlBase 及其衍生類) 的功能。
 *
 * 這個類別遵循現有的測試框架 (Initialize, Update, Render)，
 * 專注於驗證 UI 控制項的階層、座標、可見性等核心機制。
 */
class UIBasicTest
{
public:
    UIBasicTest();
    ~UIBasicTest();

    // 初始化測試場景與資源
    HRESULT Initialize();

    // 每幀更新，用於執行動態測試 (例如移動、顯隱)
    void Update(float fElapsedTime);

    // 繪製所有測試中的 UI 元件
    void Render();

private:
    // 清理所有動態配置的資源
    void Cleanup();

private:
    // 根控制項，作為所有其他測試控制項的容器
    CControlAlphaBox* m_pRootControl;

    // 測試控制項 1：用於測試相對/絕對座標
    CControlAlphaBox* m_pTestControl1;

    // 測試控制項 2：作為 TestControl1 的子物件，測試階層關係
    CControlAlphaBox* m_pTestControl2;

    // 用於在畫面上顯示狀態資訊的文字控制項
    CControlText* m_pStatusText;

    // 用於 Update 內的動畫計時
    float m_fTotalTime;
};