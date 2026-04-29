// LobbyBackground — MainTitleBG 的 main-loop 接點
//
// 對齊 mofclient.c 0x5BFA60 / 0x5C24B0 / 0x5C2500 等 Gs* 函式裡的 MainTitleBG 區塊。
//
#include "Effect/LobbyBackground.h"
#include "Effect/MainTitleBG.h"
#include "global.h"

namespace LobbyBackground {

void Ensure()
{
    // 對齊 mofclient.c GsInitMakeChar / GsInitSelectChar：
    //   if (!g_pLobbyBackground) {
    //       v0 = (MainTitleBG*)operator new(0x60u);
    //       v1 = MainTitleBG::MainTitleBG(v0);
    //       g_pLobbyBackground = v1;
    //       MainTitleBG::CreateMainTitleBG(v1, 0.0, 0.0);
    //   }
    if (g_pLobbyBackground) return;
    g_pLobbyBackground = new MainTitleBG();
    g_pLobbyBackground->CreateMainTitleBG(0.0f, 0.0f);
}

void Process()
{
    // 對齊 GsSelectChannel_PrepareDrawing (0x5C24B0)：
    //   if (g_pLobbyBackground) MainTitleBG::Process(g_pLobbyBackground);
    if (!g_pLobbyBackground) return;
    g_pLobbyBackground->FrameProcess();   // 用 flt_21CB358 推一幀
    g_pLobbyBackground->CCAEffect::Process();
}

void Draw()
{
    // 對齊 GsSelectChannel_Draw (0x5C2500)：
    //   if (g_pLobbyBackground) MainTitleBG::Draw(g_pLobbyBackground);
    if (!g_pLobbyBackground) return;
    g_pLobbyBackground->CCAEffect::Draw();
}

void Free()
{
    if (g_pLobbyBackground) {
        delete g_pLobbyBackground;
        g_pLobbyBackground = nullptr;
    }
}

} // namespace LobbyBackground
