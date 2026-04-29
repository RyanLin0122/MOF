// CCircleValueCalculator — 1:1 還原 mofclient.c 0x558D50~0x558DB0
#include "Logic/CCircleValueCalculator.h"

//----- (00558D70) --------------------------------------------------------
unsigned short CCircleValueCalculator::GetCircleMiniGameValue(unsigned short type, int mode)
{
    if (mode != 1) return 0;
    switch (type) {
        case 0: return 4;
        case 1: return 5;
        case 2: return 6;
        default: return 0;
    }
}

//----- (00558DB0) --------------------------------------------------------
unsigned short CCircleValueCalculator::GetCircleTrainingValue(unsigned short a2, unsigned short a3)
{
    int diff = (int)a3 - (int)a2;
    if (diff <= -3) return 0;
    if (diff > 2)  return 2;   // mofclient.c 註解確認 v3<3 always false 在此分支
    return 1;
}
