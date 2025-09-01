#pragma once
#include <d3d9.h>
#include "UI/CControlAlphaBox.h"
#include "UI/CControlText.h"

/**
 * @class UIBasicTest
 * @brief �Ω���հ�¦ UI ��� (CControlBase �Ψ�l����) ���\��C
 *
 * �o�����O��`�{�������ծج[ (Initialize, Update, Render)�A
 * �M�`������ UI ��������h�B�y�СB�i���ʵ��֤߾���C
 */
class UIBasicTest
{
public:
    UIBasicTest();
    ~UIBasicTest();

    // ��l�ƴ��ճ����P�귽
    HRESULT Initialize();

    // �C�V��s�A�Ω����ʺA���� (�Ҧp���ʡB����)
    void Update(float fElapsedTime);

    // ø�s�Ҧ����դ��� UI ����
    void Render();

private:
    // �M�z�Ҧ��ʺA�t�m���귽
    void Cleanup();

private:
    // �ڱ���A�@���Ҧ���L���ձ�����e��
    CControlAlphaBox* m_pRootControl;

    // ���ձ�� 1�G�Ω���լ۹�/����y��
    CControlAlphaBox* m_pTestControl1;

    // ���ձ�� 2�G�@�� TestControl1 ���l����A���ն��h���Y
    CControlAlphaBox* m_pTestControl2;

    // �Ω�b�e���W��ܪ��A��T����r���
    CControlText* m_pStatusText;

    // �Ω� Update �����ʵe�p��
    float m_fTotalTime;
};