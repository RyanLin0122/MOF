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
#include <new> // 為了 std::nothrow

class CMofPacking {
public:
    // 公開靜態方法，用於獲取唯一的實例指標
    static CMofPacking* GetInstance();

    // 公開靜態方法，用於銷毀唯一的實例
    static void DestroyInstance();

    // 刪除複製建構函式和複製賦值運算子
    CMofPacking(const CMofPacking&) = delete;
    CMofPacking& operator=(const CMofPacking&) = delete;
    CMofPacking(CMofPacking&&) = delete;
    CMofPacking& operator=(CMofPacking&&) = delete;

    // 公開解構函式，以便可以 delete s_pInstance
    ~CMofPacking();

    // --- 原有的公開成員函式 ---
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

    // 考慮將此成員封裝起來，提供 getter/setter 方法
    char m_backgroundLoadBufferField[0x7A1200];

private:
    // 私有建構函式
    CMofPacking();

    // 指向唯一實例的靜態指標
    static CMofPacking* s_pInstance;

    // --- 原有的私有成員變數 ---
    NfsHandle* m_pNfsHandle;
    void* m_pReadBuffer;
    void* m_pBuffer1;
    char m_tempPathBuffer[256];
    int m_nReadBytes;
    NfsGlobResults m_globResults;
    bool m_isLoadingFlag;
};

#endif // CMOFPACKING_H