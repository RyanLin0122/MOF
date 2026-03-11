#pragma once


class cltEmblemSystem {
public:
    static void UpdateValidity(cltEmblemSystem* self);
    void OnEvent_ChangeClass();
    int GetAPowerAdvantage(int);
    int GetDPowerAdvantage();
    int GetShopItemPriceAdvantage();
    int GetSellingAgencyTaxAdvantage();
    int GetDeadPenaltyExpAdvantage();
    int GetExpAdvantage();
    int GetFaintingInfo(int*, int*);
    int GetConfusionInfo(int*, int*);
    int GetFreezingInfo(int*, int*);
};