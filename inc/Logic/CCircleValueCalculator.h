#pragma once
//
// CCircleValueCalculator — mofclient.c 還原（位址 0x558D50~0x558DB0）
//
// 兩個純計算函式：把「公會迷你遊戲種類 / 訓練成果差距」轉成系統內部的
// 公會貢獻分數。物件本身為 stateless，建構子是 no-op。
//
class CCircleValueCalculator {
public:
    CCircleValueCalculator() = default;

    // mofclient.c 0x558D70：mode==1 時，type 0/1/2 對應 4/5/6，否則 0
    unsigned short GetCircleMiniGameValue(unsigned short type, int mode);

    // mofclient.c 0x558DB0：依 (a3 - a2) 區分 0 / 1 / 2 三檔
    //    diff <= -3 → 0；-2 <= diff <= 2 → 1；diff >= 3 → 2
    unsigned short GetCircleTrainingValue(unsigned short a2, unsigned short a3);
};
