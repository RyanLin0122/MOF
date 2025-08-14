#ifndef STTOOLTIPDATA_H
#define STTOOLTIPDATA_H

#include <string>

/**
 * @struct stToolTipData
 * @brief �x�s�u�㴣�ܡ]ToolTip�^�һݪ���ơC
 */
struct stToolTipData
{
    int m_nToolTipType;     // ��������
    int m_nData;            // ������� (�Ҧp�G����ɶ��BID��)
    short m_usData;         // �����u�㫬���
    int m_nUnk;             // �����γ~�����
    std::string m_strText;  // ������ܪ���r
    char m_cUIType;         // UI����
    short m_usSlotIndex;    // ���~�Χޯ�b�e����������
    void* m_pExtraData;     // ���V�B�~��ƪ�����

    // �غc�禡
    stToolTipData();
    // �Ѻc�禡
    ~stToolTipData();

    // ��l�ƨ禡
    void Init();
    // �]�w�r�����������ܸ��
    void SetStringType(char* text, int data);
    // �]�w����(Kind)���������ܸ��
    void SetKindType(int type, short usData, int unk, int data, char uiType, short slotIndex, void* extraData);
};

#endif // STTOOLTIPDATA_H