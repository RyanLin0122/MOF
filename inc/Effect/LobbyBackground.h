#pragma once
//
// LobbyBackground 入口函式 — 還原 mofclient.c GsInit*/GsSelectChannel_*
// 系列（位址 0x5BFA60 / 0x5C24B0 / 0x5C2500 / 0x5C2620）的 MainTitleBG 部分。
//
// mofclient.c 的 Lobby state machine 是由 g_dwSubGameState 切換之 Gs*() 函式
// 群構成；本還原把 MainTitleBG 相關片段獨立成可被 main loop 呼叫的入口，讓
// 即使整個 GameState 系統還沒被還原，背景動畫仍可被啟動 / 推進 / 繪製 / 銷毀。
//
// 對應原始碼：
//   GsInitMakeChar       (0x5BFA60)
//   GsInitSelectChar     (0x5C2620)
//   GsInitSelectChannel  (...)
//     → 三個入口若 g_pLobbyBackground 為 null，
//       new MainTitleBG → CreateMainTitleBG(0,0) → 寫回 g_pLobbyBackground
//
//   GsSelectChannel_PrepareDrawing (0x5C24B0)：if(g_pLobbyBackground) Process()
//   GsSelectChannel_Draw           (0x5C2500)：if(g_pLobbyBackground) Draw()
//   退出大廳時：delete g_pLobbyBackground
//
namespace LobbyBackground {

    // 等同 GsInit*Lobby 中的 MainTitleBG 區塊
    // 已存在則 noop；尚未建立則 new + CreateMainTitleBG。
    void Ensure();

    // 等同 GsSelectChannel_PrepareDrawing 中的 if(g_pLobbyBackground) 分支
    // 對 CCAEffect 推進一幀（FrameProcess + Process）。
    void Process();

    // 等同 GsSelectChannel_Draw 中的 if(g_pLobbyBackground) 分支
    void Draw();

    // 等同 mofclient.c 釋放點：delete g_pLobbyBackground; g_pLobbyBackground = nullptr.
    void Free();

} // namespace LobbyBackground
