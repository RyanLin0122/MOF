#pragma once

/**
 * @struct stFontInfo
 * @brief �x�s�q�r���]�w�ɤ�Ū������@�r���]�w�C
 *
 * �o�ӵ��c���w�q�O�ھ� MoFFont::InitFontInfo (0x0051B660) ���ѪR�޿���_�X�Ӫ��C
 * �����j�p�O 264 �줸�աA�P��l�X�����t���B��(stride)�۲šC
 */
struct stFontInfo {
    char szKeyName[128];      // �Ω�d�䪺��W�A�Ҧp "Normal"
    char szFaceName[128];     // �r���W�١A�Ҧp "Tahoma"
    int  nHeight;             // �r������
    int  nWeight;             // �r���ʲ� (e.g., 400 for FW_NORMAL)
};