#pragma once
//
// MainTitleBG — mofclient.c 還原（位址 0x544230~0x5442C0）
//
// 主畫面 / Lobby 大廳 角色背景動畫物件。直接繼承 CCAEffect，把
// Ui_CharBG.ea 用 CEAManager::GetEAData 載入後播放第 0 段動畫。
//
// 全域指標 g_pLobbyBackground 由初始化流程 `new MainTitleBG` 配置；
// Lobby 結束時 ~MainTitleBG (透過 operator delete) 釋放。
//
#include "Effect/CCAEffect.h"

class MainTitleBG : public CCAEffect {
public:
    MainTitleBG();
    ~MainTitleBG() override = default;   // 對齊 mofclient.c：行為等同 ~CCAEffect

    void CreateMainTitleBG(float x, float y);
    void FrameProcess();   // 用單一 deltaTime 全域 (flt_21CB358)
    // Process / Draw 由 CCAEffect 繼承，無覆寫。
};

// mofclient.c 0x6E1668：Lobby 場景常駐之背景動畫指標。
extern MainTitleBG* g_pLobbyBackground;

// mofclient.c flt_21CB358：FrameProcess 使用之全域 deltaTime（每 tick 由
// 主迴圈寫入「上一幀經過的秒數」）。本還原預設 1/60 秒。
extern float flt_21CB358;
