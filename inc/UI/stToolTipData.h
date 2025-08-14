#pragma once
#include <string>
#include <cstring>

// �u�㴣�ܼƾڵ��c
class stToolTipData {
public:
    // �غc���
    stToolTipData();

    // �Ѻc���
    ~stToolTipData();

    // ��l��
    void Init();

    // �]�w�r���������u�㴣��
    void SetStringType(const char* text, int color);

    // �]�w���~�������u�㴣��
    void SetKindType(int type, short id, int count, int color, char grade, short durability, int extra);

private:
    // ���� 0: ���� (-1=���]�w, 1=�r������, ��L=���~����)
    int m_type;

    // ���� 4: �C���
    int m_color;

    // ���� 8: ���~ID (�Ȫ��~�����ϥ�)
    short m_itemId;

    // ���� 10: �O�d�Ŷ�
    short m_reserved1;

    // ���� 12: �ƶq (�Ȫ��~�����ϥ�)
    int m_count;

    // ���� 16: �r��ƾ� (std::string ���c)
    std::string m_text;

    // ���� 32: �~�� (�Ȫ��~�����ϥ�)
    char m_grade;

    // ���� 33: �O�d�Ŷ�
    char m_reserved2;

    // ���� 34: �@�[�� (�Ȫ��~�����ϥ�)
    short m_durability;

    // ���� 36: �B�~�ƾ� (�Ȫ��~�����ϥ�)
    int m_extra;
};