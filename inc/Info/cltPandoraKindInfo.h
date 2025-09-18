#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>

// �P�ϽsĶ�۲Ū������G�u���� TranslateKindCode
class cltPandoraKindInfo
{
public:
    // �N 5 �X�N���]���r�� + 4��ơ^�ন 16-bit kind code�C
    // �W�h�]�P�ϽsĶ�@�P�^�G
    // 1) ���ץ��� 5�F�_�h�^�� 0
    // 2) hi = (toupper(s[0]) + 31) << 11
    // 3) lo = atoi(s + 1)�F���� < 0x800
    // 4) �^�� (hi | lo)�A�_�h 0
    static std::uint16_t TranslateKindCode(char* s);
};
