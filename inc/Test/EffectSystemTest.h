#pragma once
#include <windows.h> // for HRESULT

// �e�V�ŧi (Forward Declarations)
class ClientCharacter;

/**
 * @class EffectSystemTest
 * @brief �t�d��l�ƻP�X�ʾ�ӯS�Ĩt�Ϊ��������O�C
 *
 * �o�����O�|���N�z main.cpp ���� ImageDrawTest�A
 * �Ω�����C���D�`���A�ë��򲣥͡B�޲z�M��V�S�ġC
 */
class EffectSystemTest {
public:
    EffectSystemTest();
    ~EffectSystemTest();

    /// @brief ��l�ƴ������ҡA�إߺ޲z���P��������C
    HRESULT Initialize();

    /// @brief �M�z�Ҧ��귽�C
    void Cleanup();

    /// @brief �C�V���޿��s�C
    /// @param fElapsedTime �ۤW�@�V�H�Ӹg�L���ɶ�(��)�C
    void Update(float fElapsedTime);

    /// @brief ��V�Ҧ��i�����S�ġC
    void Render();

private:
    /// @brief �@�ӻ��U�禡�A�Ω�b�����m���ͷs���U�A�ٯS�ġC
    void SpawnDownCutEffect();

    void SpawnHitNormalEffect();

    void SpawnUpperCutEffect();

    // --- �p������ ---

    /// @brief �������I�k�̨���C
    ClientCharacter* m_pCaster;

    /// @brief �Ω�p�ɡA�H�T�w���j���ͷs�S�ġC
    float m_fTimeSinceLastEffect;
};