#pragma once
#include <cstdint>
#include <cstddef>

class cltWorldMapColorInfo {
public:
    cltWorldMapColorInfo();
    ~cltWorldMapColorInfo();

    // �P�ϽsĶ�@�P�G���\�^�� 1�A���Ѧ^�� 0
    int InitializeInPack(char* pathInPack);
    int Initialize(const char* fileName);
    void Free();

    // ���o�C��G���\�^�ǹ���(0�V255)�A�V�ɩΥ���l�Ʀ^�� 0
    int GetColor(int x, int y) const;

private:
    // �������� operator new / operator delete �t��]�D new[]/delete[]�^
    uint8_t* m_pData;

    static constexpr int    kWidth = 409;     // �P���ަ� 409*a3 + a2 �@�P
    static constexpr int    kHeight = 331;
    static constexpr size_t kSize = 0x210D3; // �ϽsĶ�����t�m�j�p
};
