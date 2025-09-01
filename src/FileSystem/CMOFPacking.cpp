#include "FileSystem/CMOFPacking.h"
#include <string> 
#include <new>    // For std::nothrow
#include <algorithm> // For std::transform if needed for C++ style string conversion
#include <shlwapi.h> // 引入 Shlwapi.h 以使用路徑處理函式
#pragma comment(lib, "shlwapi.lib")

// 從CMOFPacking.c中觀察到的全域字串
const char STR_DOT[] = ".";
const char STR_DOTDOT[] = "..";

// 初始化靜態成員指標
CMofPacking* CMofPacking::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
CMofPacking* CMofPacking::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CMofPacking();
    }
    return s_pInstance;
}

// 靜態 DestroyInstance 方法的實現
void CMofPacking::DestroyInstance() {
    delete s_pInstance;
    s_pInstance = nullptr;
}

// 建構函式
CMofPacking::CMofPacking() {
    m_pNfsHandle = nullptr;
    m_pReadBuffer = nullptr;
    m_pBuffer1 = nullptr; // Note: This buffer is never used in the provided logic.
    m_nReadBytes = 0;

    m_globResults.gl_pathc = 0;
    m_globResults.gl_pathv = nullptr;
    m_globResults.gl_offs = 0;
    m_globResults.internal_callback_error_flag = 0;

    m_isLoadingFlag = false;

    memset(m_backgroundLoadBufferField, 0, sizeof(m_backgroundLoadBufferField));
    memset(m_tempPathBuffer, 0, sizeof(m_tempPathBuffer));
}

// 解構函式
CMofPacking::~CMofPacking() {
    DeleteBuffer();
    DeleteBuffer1();
    PackFileClose();
}

bool CMofPacking::Init() {
    return true;
}

// 開啟NFS封裝檔案
bool CMofPacking::PackFileOpen(const char* packFileName) {
    if (m_pNfsHandle) {
        PackFileClose(); // 如果已開啟，先關閉舊的
    }
    // 模式 3 在原始碼中被使用，通常意味著讀寫權限
    m_pNfsHandle = nfs_start(packFileName, 3);
    return (m_pNfsHandle != nullptr);
}

// 關閉NFS封裝檔案
bool CMofPacking::PackFileClose() {
    if (m_pNfsHandle) {
        nfs_end(m_pNfsHandle, 0); // 參數 0 表示不銷毀實體檔案
        m_pNfsHandle = nullptr;
    }
    return true;
}

// [已重構] 新增檔案至封裝
bool CMofPacking::AddFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;

    // 如果檔案已存在，則移除以達到覆寫效果
    if (nfs_file_exists(m_pNfsHandle, filePathInPack)) {
        if (!RemoveFile(filePathInPack)) {
            // 如果移除失敗，可以選擇中止或繼續嘗試建立
            // 此處選擇中止以保持一致性
            return false;
        }
    }

    int fd = nfs_file_create(m_pNfsHandle, filePathInPack);
    if (fd < 0) {
        return false; // 建立檔案失敗
    }

    // [修正] 移除原先不必要的 2048 位元組寫入操作。
    // nfs_file_create 已經在 VFS 中建立了大小為 0 的檔案條目，這就足夠了。

    nfs_file_close(m_pNfsHandle, fd);
    return true;
}

// [已重構] 從封裝中移除檔案
bool CMofPacking::RemoveFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;

    // [修正] 檢查 nfs_file_unlink 的回傳值。成功時返回 0。
    if (nfs_file_unlink(m_pNfsHandle, filePathInPack) == 0) {
        return true;
    }

    return false;
}

// [已重構] 將指定目錄下的所有檔案打包進NFS
int CMofPacking::DataPacking(const char* directoryPath) {
    if (!m_pNfsHandle || !directoryPath) return 0;

    char rootPath[MAX_PATH];
    strcpy_s(rootPath, sizeof(rootPath), directoryPath);

    // [新增] 使用 PathAddBackslashA 確保路徑以 '\' 結尾，這對後續計算相對路徑很重要
    PathAddBackslashA(rootPath);

    // 呼叫私有的遞迴函式開始打包
    return DataPacking_Recursive(rootPath, rootPath);
}


// [已重構] 背景載入式讀取檔案
void CMofPacking::FileReadBackGroundLoading(const char* filePathInPack) {
    if (!m_pNfsHandle) return;

    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // 模式2, 假設為讀取
    if (fd >= 0) {
        m_isLoadingFlag = true;
        int totalBytesReadThisCall = 0;
        int bytesReadThisChunk;
        do {
            bytesReadThisChunk = nfs_file_read(m_pNfsHandle, fd, m_backgroundLoadBufferField, sizeof(m_backgroundLoadBufferField));
            if (bytesReadThisChunk > 0) {
                totalBytesReadThisCall += bytesReadThisChunk;
                // 在此處可以對讀入 m_backgroundLoadBufferField 的數據塊進行即時處理
            }
        } while (bytesReadThisChunk > 0); // 只要有讀到資料就繼續

        // [修正] 將讀取的總位元組數賦值給 m_nReadBytes
        m_nReadBytes = totalBytesReadThisCall;

        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
    }
    else {
        m_nReadBytes = 0;
    }
}

// 輔助函式：透過存取NFS內部結構來取得檔案大小
// 理想情況下，此功能應由 nfs.h 提供一個公開的API
static int get_nfs_file_size(NfsHandle* handle, int fd) {
    // 警告：以下程式碼直接存取了 NfsHandle 的內部成員 (open_files_array, nt_handle)，
    // 這嚴重違反了 API 的封裝原則。
    // 這樣做的唯一原因是 nfs.h API 目前缺少一個公開的、透過檔案描述符(fd)
    // 查詢檔案大小的函式 (如 nfs_file_get_size(fd))。
    // 這是為了解決 API 不足而採取的權宜之計，若 nfs 內部結構變更，此處可能失效。
    if (!handle || fd < 0 || fd >= handle->open_files_array_capacity || !handle->open_files_array[fd]) {
        return -1;
    }
    NfsOpenFileHandle* fh = handle->open_files_array[fd];
    return nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx);
}

// [已重構] 讀取檔案至動態分配的緩衝區
char* CMofPacking::FileRead(const char* filePathInPack) {
    if (!m_pNfsHandle) return nullptr;

    m_nReadBytes = 0;
    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // 模式2
    if (fd >= 0) {
        m_isLoadingFlag = true;

        // [修正] 使用輔助函式來隱藏直接存取，並清楚標示其問題
        int fileSize = get_nfs_file_size(m_pNfsHandle, fd);

        if (fileSize >= 0) {
            DeleteBuffer(); // 釋放舊的 m_pReadBuffer
            m_pReadBuffer = new (std::nothrow) char[fileSize + 1]; // +1 for null terminator
            if (m_pReadBuffer) {
                m_nReadBytes = nfs_file_read(m_pNfsHandle, fd, m_pReadBuffer, fileSize);
                if (m_nReadBytes != fileSize) {
                    DeleteBuffer();
                }
                else {
                    // 確保檔案內容即使是二進位，在作為字串使用時也是安全的
                    ((char*)m_pReadBuffer)[m_nReadBytes] = '\0';
                }
            }
            else {
                m_nReadBytes = 0; // 記憶體分配失敗
            }
        }
        else {
            m_nReadBytes = 0; // 獲取檔案大小失敗
        }
        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
    }
    return (char*)m_pReadBuffer;
}

// 釋放 m_pReadBuffer
void CMofPacking::DeleteBuffer() {
    if (m_pReadBuffer) {
        delete[] static_cast<char*>(m_pReadBuffer);
        m_pReadBuffer = nullptr;
    }
}

// 釋放 m_pBuffer1
void CMofPacking::DeleteBuffer1() {
    if (m_pBuffer1) {
        delete[] static_cast<char*>(m_pBuffer1);
        m_pBuffer1 = nullptr;
    }
}

// 取得上次讀取操作的位元組數
int CMofPacking::GetBufferSize() const {
    return m_nReadBytes;
}

// 在NFS中搜尋符合模式的檔案名稱
// 在 CMOFPacking.cpp 中

NfsGlobResults* CMofPacking::SearchString(const char* pattern) {
    if (!m_pNfsHandle) return nullptr;

    // 保留大小寫轉換，確保 API 一致性
    char lowerCasePattern[MAX_PATH];
    strcpy_s(lowerCasePattern, sizeof(lowerCasePattern), pattern);
    _strlwr_s(lowerCasePattern, sizeof(lowerCasePattern));

    // 直接呼叫已修復的 nfs_glob 即可，不再需要 if/else 判斷
    int result = nfs_glob(m_pNfsHandle, lowerCasePattern, 4, nullptr, &m_globResults);

    if (result == 0 && m_globResults.gl_pathc > 0) {
        return &m_globResults;
    }

    return nullptr;
}

// 釋放 SearchString 的結果
void CMofPacking::DeleteSearchData() {
    if (!m_pNfsHandle) return;
    // nfs_glob_free 的簽章是 void nfs_glob_free(NfsGlobResults*), 只需傳入結果結構即可。
    nfs_glob_free(&m_globResults);
}

// 將字串轉為小寫
char* CMofPacking::ChangeString(char* str) {
    if (str) {
        // _strlwr 已被棄用，使用 _strlwr_s 更安全
        _strlwr_s(str, strlen(str) + 1);
        return str;
    }
    return nullptr;
}

int CMofPacking::DataPacking_Recursive(const char* currentDirectory, const char* rootPath) {
    // 這裡的邏輯基本上就是舊的 DataPacking 函式，但有關鍵修改
    if (!m_pNfsHandle || !currentDirectory || !rootPath) return 0;

    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH];
    char fullLocalPath[MAX_PATH]; // [修改] 變數名稱更清晰

    // 構建搜尋路徑
    sprintf_s(searchPath, sizeof(searchPath), "%s*.*", currentDirectory);

    HANDLE hFindFile = FindFirstFileA(searchPath, &findFileData);
    if (hFindFile == INVALID_HANDLE_VALUE) {
        return 2; // 目錄為空或無效，正常結束
    }

    size_t rootPathLen = strlen(rootPath);

    do {
        if (strcmp(findFileData.cFileName, STR_DOT) == 0 || strcmp(findFileData.cFileName, STR_DOTDOT) == 0) {
            continue;
        }

        // 構建當前檔案/目錄的完整本地路徑
        sprintf_s(fullLocalPath, sizeof(fullLocalPath), "%s%s", currentDirectory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char subDirectoryPath[MAX_PATH];
            sprintf_s(subDirectoryPath, sizeof(subDirectoryPath), "%s\\", fullLocalPath);
            // [修改] 遞迴呼叫新的工作函式
            int packing_result = DataPacking_Recursive(subDirectoryPath, rootPath);
            if (packing_result == 1) {
                FindClose(hFindFile);
                return 1;
            }
        }
        else {
            if (_stricmp(findFileData.cFileName, "mof.ini") == 0) {
                FindClose(hFindFile);
                return 1;
            }

            FILE* pFile = nullptr;
            if (fopen_s(&pFile, fullLocalPath, "rb") == 0 && pFile) {
                fseek(pFile, 0, SEEK_END);
                long fileSize = ftell(pFile);
                fseek(pFile, 0, SEEK_SET);

                if (fileSize > 0) {
                    char* buffer = new (std::nothrow) char[fileSize];
                    if (buffer) {
                        fread(buffer, 1, fileSize, pFile);

                        // --- [核心修改] ---
                        // 計算相對於 rootPath 的路徑作為 VFS 中的鍵
                        char relativePathForVFS[MAX_PATH];
                        // fullLocalPath 的開頭就是 rootPath，我們取 rootPath 之後的部分
                        strcpy_s(relativePathForVFS, sizeof(relativePathForVFS), fullLocalPath + rootPathLen);

                        // [建議] 將路徑分隔符 '\' 轉換為 '/'，這在 VFS 中是更好的習慣
                        for (char* p = relativePathForVFS; *p; ++p) {
                            if (*p == '\\') *p = '/';
                        }

                        // 轉換為小寫
                        _strlwr_s(relativePathForVFS, sizeof(relativePathForVFS));

                        // 使用計算出的相對路徑來建立檔案
                        int fd = nfs_file_create(m_pNfsHandle, relativePathForVFS);
                        if (fd >= 0) {
                            printf("Packing: %s\n", relativePathForVFS); // [可選] 輸出打包的檔案路徑
                            nfs_file_write(m_pNfsHandle, fd, buffer, fileSize);
                            nfs_file_close(m_pNfsHandle, fd);
                        }
                        else {
                            printf("Failed to create file in VFS: %s\n", relativePathForVFS);
                        }
                        delete[] buffer;
                    }
                }
                fclose(pFile);
            }
        }
    } while (FindNextFileA(hFindFile, &findFileData));

    FindClose(hFindFile);
    return 2; // 正常完成
}