#pragma once
#include "Font/MoFFont.h"
#include <d3d9.h>
#include <cstdio> // for printf

// 前向宣告，避免在 main.cpp 中包含過多標頭
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

/**
 * @class FontSystemTest
 * @brief 負責展示與測試 MoFFont 系統所有功能的類別
 */
class FontSystemTest {
public:
    FontSystemTest();
    ~FontSystemTest();

    // 初始化所有資源
    HRESULT Initialize();

    // 每幀更新 (用於資源管理)
    void Update(float fElapsedTime);

    // 執行所有渲染指令
    void Render();

private:
    // 準備測試用的字型設定檔
    bool PrepareFontConfigFile();

private:
    MoFFont* m_pFont;
};
