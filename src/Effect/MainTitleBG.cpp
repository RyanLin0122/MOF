// MainTitleBG — 1:1 還原 mofclient.c 0x544230~0x5442C0
#include "Effect/MainTitleBG.h"

#include "Effect/CEAManager.h"

//----- (00544230) --------------------------------------------------------
MainTitleBG::MainTitleBG()
    : CCAEffect()
{
}

//----- (00544250) --------------------------------------------------------
void MainTitleBG::CreateMainTitleBG(float x, float y)
{
    CEAManager* pEA = CEAManager::GetInstance();
    pEA->GetEAData(122, "MoFData/Effect/Ui_CharBG.ea", this);
    SetFrameTime();
    Play(0, false);
    SetPosition(x, y);
}

//----- (00544290) --------------------------------------------------------
void MainTitleBG::FrameProcess()
{
    CCAEffect::FrameProcess(flt_21CB358);
}
