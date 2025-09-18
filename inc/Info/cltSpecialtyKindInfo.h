#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>

class cltSpecialtyKindInfo
{
public:
    // �N 5 �X�N���]���r�� + 4 ��ơ^�ର 16-bit kind code
    // �ϽsĶ�޿�G
    // - ���׫D 5 �� �^�� 0
    // - hi = (toupper(s[0]) + 31) << 11
    // - lo = atoi(s+1)�F���� < 0x800
    // - �^�� (hi | lo)�A�_�h 0
    static std::uint16_t TranslateKindCode(char* s);
};
