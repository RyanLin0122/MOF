#pragma once
#include "global.h"
#include "Effect/CCAEffect.h" // 包含 CCAEffect 的定義

class CEAManager
{
public:
    void Reset();
    // IDA: void __thiscall GetEAData(this, int effectId, char* fileName, CCAEffect* outEffect)
    void GetEAData(int effectId, const char* fileName, CCAEffect* outEffect);

    static CEAManager* GetInstance();
private:
    // Loaders
    void LoadEAInPack(int effectID, char* szFileName);
    void LoadEA(int effectId, const char* fileName);
    CEAManager();
    ~CEAManager();
private:
    // 65535 entries
    EADATALISTINFO** m_pEAData;

    // trailing DWORD at offset 0x3FFFC (IDA sets to 0 in ctor/reset)
    uint32_t m_dwTailFlag;

    static CEAManager* s_pInstance;
};