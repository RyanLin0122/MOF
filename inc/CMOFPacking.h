#ifndef CMOFPACKING_H
#define CMOFPACKING_H

#include "nfs.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <new> // ���F std::nothrow

class CMofPacking {
public:
    // ���}�R�A��k�A�Ω�����ߤ@����ҫ���
    static CMofPacking* GetInstance();

    // ���}�R�A��k�A�Ω�P���ߤ@�����
    static void DestroyInstance();

    // �R���ƻs�غc�禡�M�ƻs��ȹB��l
    CMofPacking(const CMofPacking&) = delete;
    CMofPacking& operator=(const CMofPacking&) = delete;
    CMofPacking(CMofPacking&&) = delete;
    CMofPacking& operator=(CMofPacking&&) = delete;

    // ���}�Ѻc�禡�A�H�K�i�H delete s_pInstance
    ~CMofPacking();

    // --- �즳�����}�����禡 ---
    bool Init();
    bool PackFileOpen(const char* packFileName);
    bool PackFileClose();
    bool AddFile(const char* filePathInPack);
    bool RemoveFile(const char* filePathInPack);
    int DataPacking(const char* directoryPath);
    void FileReadBackGroundLoading(const char* filePathInPack);
    char* FileRead(const char* filePathInPack);
    void DeleteBuffer();
    void DeleteBuffer1();
    int GetBufferSize() const;
    NfsGlobResults* SearchString(const char* pattern);
    void DeleteSearchData();
    char* ChangeString(char* str);

    // �Ҽ{�N�������ʸ˰_�ӡA���� getter/setter ��k
    char m_backgroundLoadBufferField[0x7A1200];

private:
    // �p���غc�禡
    CMofPacking();

    // ���V�ߤ@��Ҫ��R�A����
    static CMofPacking* s_pInstance;

    // --- �즳���p�������ܼ� ---
    NfsHandle* m_pNfsHandle;
    void* m_pReadBuffer;
    void* m_pBuffer1;
    char m_tempPathBuffer[256];
    int m_nReadBytes;
    NfsGlobResults m_globResults;
    bool m_isLoadingFlag;
};

#endif // CMOFPACKING_H