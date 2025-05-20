#include "CMOFPacking.h"
#include <string> // 可用於更安全的字串操作，但此處盡量維持C風格以符合原始碼

// 從CMOFPacking.c中觀察到的全域字串 (原始碼中使用的是字串字面量)
const char STR_DOT[] = ".";
const char STR_DOTDOT[] = "..";
// const char STR_MOF_INI[] = "mof.ini"; // 在DataPacking中直接使用 "mof.ini"

// 建構函式
CMofPacking::CMofPacking() {
    // *(_DWORD *)this = &CMofPacking::`vftable'; // vfptr由編譯器自動設定
    m_pNfsHandle = nullptr;                     // *((_DWORD *)this + 1) = 0;
    m_pReadBuffer = nullptr;                    // *((_DWORD *)this + 2) = 0;
    m_pBuffer1 = nullptr;                       // *((_DWORD *)this + 3) = 0;
    m_nReadBytes = 0;                           // *((_DWORD *)this + 68) = 0;

    // 初始化 m_globResults 的成員
    // *((_DWORD *)this + 72) = 0; // -> m_globResults.internal_callback_error_flag
    m_globResults.gl_pathc = 0;
    m_globResults.gl_pathv = nullptr;
    m_globResults.gl_offs = 0;
    m_globResults.internal_callback_error_flag = 0;

    m_isLoadingFlag = false;                    // byte_7A1324[(_DWORD)this] = 0;

    // memset((char *)this + 292, 0, 0x7A1200u);
    memset(m_backgroundLoadBufferField, 0, sizeof(m_backgroundLoadBufferField));

    // memset((char *)this + 16, 0, 0x100u);
    memset(m_tempPathBuffer, 0, sizeof(m_tempPathBuffer));
}

// 解構函式
CMofPacking::~CMofPacking() {
    // *(_DWORD *)this = &CMofPacking::`vftable'; // vfptr由編譯器自動設定
    DeleteBuffer();  // 清理 m_pReadBuffer
    // DeleteBuffer1(); // 也應該清理 m_pBuffer1，如果它被使用了
    PackFileClose(); // 關閉NFS檔案
}

// 初始化 (似乎沒做特別的事)
bool CMofPacking::Init() {
    return true; // 原始碼返回1
}

// 開啟NFS封裝檔案
bool CMofPacking::PackFileOpen(const char* packFileName) {
    m_pNfsHandle = nfs_start(packFileName, 3); // 模式3在原始碼中被使用
    return (m_pNfsHandle != nullptr);
}

// 關閉NFS封裝檔案
bool CMofPacking::PackFileClose() {
    if (m_pNfsHandle) {
        nfs_end(m_pNfsHandle, 0); // 參數0表示不銷毀實體檔案
        m_pNfsHandle = nullptr;
    }
    return true; // 原始碼返回1
}

// 新增檔案至封裝
bool CMofPacking::AddFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;

    if (nfs_file_exists(m_pNfsHandle, filePathInPack)) {
        RemoveFile(filePathInPack); // 如果檔案已存在，先移除
    }

    int fd = nfs_file_create(m_pNfsHandle, filePathInPack);
    if (fd < 0) {
        return false; // 建立檔案失敗
    }

    // 原始碼中寫入了2048位元組的未初始化堆疊數據 v5。
    // 這通常是個bug或者有特殊目的（例如預分配空間，但寫入未初始化數據很危險）。
    // 為了行為一致，我們模擬寫入一些數據，但使用0填充的數據以避免安全風險。
    // 如果嚴格要求寫入"垃圾"數據，則需要char uninitialized_buffer[2048];
    char zero_buffer[2048];
    memset(zero_buffer, 0, sizeof(zero_buffer)); // 使用0填充的緩衝區

    if (nfs_file_write(m_pNfsHandle, fd, zero_buffer, sizeof(zero_buffer)) != sizeof(zero_buffer)) {
        nfs_file_close(m_pNfsHandle, fd);
        return false; // 寫入失敗
    }

    nfs_file_close(m_pNfsHandle, fd);
    return true;
}

// 從封裝中移除檔案
bool CMofPacking::RemoveFile(const char* filePathInPack) {
    if (!m_pNfsHandle) return false;
    nfs_file_unlink(m_pNfsHandle, filePathInPack);
    return true; // 原始碼返回1，即使unlink失敗也如此 (nfs_file_unlink本身不返回值)
}

// 將指定目錄下的所有檔案打包進NFS
int CMofPacking::DataPacking(const char* directoryPath) {
    if (!m_pNfsHandle || !directoryPath) return 0;

    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH];
    char fullFilePath[MAX_PATH];
    char subDirectoryPath[MAX_PATH];

    // 構建搜尋路徑，例如 "C:\\Data\\*.*"
    sprintf_s(searchPath, sizeof(searchPath), "%s*.*", directoryPath);

    HANDLE hFindFile = FindFirstFileA(searchPath, &findFileData);
    if (hFindFile == INVALID_HANDLE_VALUE) {
        return 0; // FindFirstFileA 失敗
    }

    do {
        if (lstrcmpA(findFileData.cFileName, STR_DOT) == 0 || lstrcmpA(findFileData.cFileName, STR_DOTDOT) == 0) {
            continue; // 跳過 "." 和 ".."
        }

        // 原始碼中有對 "mof.ini" 的特殊處理，但字串不匹配 ('&`string`')，可能是反編譯錯誤
        // `if ( !lstrcmpA(FindFileData.cFileName, &`string`) ) return 1;` -> `&`string`` 不太對
        // 假設是檢查特定檔案名，例如 "mof.ini" (基於常見模式)
        if (lstrcmpA(findFileData.cFileName, "mof.ini") == 0) { // 假設檢查 "mof.ini"
            FindClose(hFindFile);
            return 1; // 找到 "mof.ini"，停止處理
        }

        sprintf_s(fullFilePath, sizeof(fullFilePath), "%s%s", directoryPath, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 如果是子目錄，遞歸處理
            sprintf_s(subDirectoryPath, sizeof(subDirectoryPath), "%s\\", fullFilePath);
            int packing_result = DataPacking(subDirectoryPath);
            if (packing_result == 1) { // 如果在子目錄中找到 "mof.ini"
                FindClose(hFindFile);
                return 1;
            }
            // 如果 packing_result == 0 (子目錄處理失敗)，原始碼似乎會繼續處理當前目錄的其他檔案
            // 而不是直接返回0。原始碼邏輯：`v4 = CMofPacking::DataPacking(this, v13), v4 != 1) && v4`
            // 如果 v4 (packing_result) 是0，`(v4 != 1 && v4)` 為假，就會執行後面的檔案處理邏輯，這不對。
            // 應該是: if (packing_result == 0 && packing_result != 1) { /* handle error or continue */ }
            // 或者: if (packing_result != 1 && packing_result != 2) { /* error in subdir */ }
            // 原始碼的 `v4 != 1 && v4` 意思是 `packing_result != 1 && packing_result != 0` -> `packing_result == 2`
            // 所以只有子目錄成功返回2時，才跳過後續的檔案處理。
            // 簡化：如果遞歸呼叫不是1（mof.ini），也不是0（錯誤），那麼就繼續。
            // 如果遞歸呼叫是0，那麼原始碼的 `v4` (packing_result) 為0，`v4 != 1 && v4` 為假，
            // 會導致執行後面的檔案拷貝邏輯，這是不對的，因為子目錄是個目錄。
            // 正確的邏輯應為：如果 (FindFileData.dwFileAttributes != 16) then process file
            // else (it's a directory) recurse; if recurse_result == 1 return 1; (if recurse_result == 0, maybe error)
            // 反編譯碼 `if ( FindFileData.dwFileAttributes != 16 || (..., v4 = DataPacking(...), v4 != 1) && v4 )`
            // 等價於 `if (IS_FILE || (IS_DIR && DataPacking_Returns_NOT_MOF_INI && DataPacking_Returns_NOT_ERROR_OR_EMPTY))`
            // 實際上，如果 DataPacking 返回 0 (錯誤) 或 1 (找到mof.ini)，則 `(v4 != 1 && v4)` 為假。
            // 此時 if 條件取決於 `FindFileData.dwFileAttributes != 16`。
            // 這表示如果是一個檔案，或者是一個目錄且遞歸打包失敗或找到mof.ini，則執行後面的檔案複製。這顯然是錯的。
            // 正確邏輯應為：若是檔案，則打包。若是目錄，則遞歸。
            // 我將修正此邏輯為更合理的形式。
        }
        else {
            // 如果是檔案，讀取並寫入NFS
            FILE* pFile = nullptr;
            errno_t err = fopen_s(&pFile, fullFilePath, "rb");
            if (err == 0 && pFile) {
                fseek(pFile, 0, SEEK_END);
                long fileSize = ftell(pFile);
                fseek(pFile, 0, SEEK_SET);

                if (fileSize > 0) {
                    char* buffer = new (std::nothrow) char[fileSize];
                    if (buffer) {
                        fread(buffer, 1, fileSize, pFile);

                        // 將檔案名轉為小寫（NFS內部通常是大小寫不敏感或統一小寫）
                        char lowerCaseFileName[MAX_PATH];
                        strcpy_s(lowerCaseFileName, sizeof(lowerCaseFileName), fullFilePath);
                        _strlwr_s(lowerCaseFileName, sizeof(lowerCaseFileName)); // 使用安全版本

                        int fd = nfs_file_create(m_pNfsHandle, lowerCaseFileName);
                        if (fd >= 0) {
                            nfs_file_write(m_pNfsHandle, fd, buffer, fileSize);
                            nfs_file_close(m_pNfsHandle, fd);
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


// 背景載入式讀取檔案
void CMofPacking::FileReadBackGroundLoading(const char* filePathInPack) {
    if (!m_pNfsHandle) return;

    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // 模式2，可能是唯讀或讀寫
    if (fd >= 0) {
        m_isLoadingFlag = true;
        int totalBytesReadThisCall = 0; // 用於累計本次背景載入的總位元組數
        int bytesReadThisChunk;
        do {
            bytesReadThisChunk = nfs_file_read(m_pNfsHandle, fd, m_backgroundLoadBufferField, sizeof(m_backgroundLoadBufferField));
            if (bytesReadThisChunk > 0) {
                totalBytesReadThisCall += bytesReadThisChunk;
                // 這裡可以處理讀取到的 m_backgroundLoadBufferField 中的數據
                // 原始碼將單個 chunk 的讀取量存儲在 m_nReadBytes (`*((_DWORD *)this + 68) = v7;`)
                // 並在一個局部變數 v6 中累加。此處我們僅更新 m_nReadBytes 為最後一塊的大小。
                m_nReadBytes = bytesReadThisChunk;
            }
            else {
                m_nReadBytes = 0; // 如果讀取失敗或到檔案尾端
            }
        } while (bytesReadThisChunk >= static_cast<int>(sizeof(m_backgroundLoadBufferField))); // 如果讀滿了緩衝區，可能還有更多

        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
        // 原始碼中 v6 累加了總量，但最終 m_nReadBytes 只保留最後一塊的大小。
        // 如果需要總量，應該有一個另外的成員變數來儲存 totalBytesReadThisCall。
        // 根據 GetBufferSize() 的行為，它返回 m_nReadBytes，所以我們保持 m_nReadBytes 為最後一塊的大小。
    }
    else {
        m_nReadBytes = 0;
    }
}

// 讀取檔案至動態分配的緩衝區
char* CMofPacking::FileRead(const char* filePathInPack) {
    if (!m_pNfsHandle) return nullptr;

    m_nReadBytes = 0;
    int fd = nfs_file_open(m_pNfsHandle, filePathInPack, 2); // 模式2
    if (fd >= 0) {
        m_isLoadingFlag = true;

        // 從NfsHandle獲取OpenFileHandle，再獲取nt_node_idx
        // 這是對nfs內部結構的假設，直接使用API更佳
        // int fileSize = nfs_nt_node_get_size(m_pNfsHandle->nt_handle, m_pNfsHandle->open_files_array[fd]->nt_node_idx);
        // 更安全的做法是先lseek到末尾獲取大小，或者nfs API提供直接獲取大小的方法。
        // nfs.cpp 中的 nfs_file_lseek 會更新或返回大小，但沒有直接的 nfs_file_size(fd)。
        // nfs_nt_node_get_size 需要 nt_handle 和 nt_node_idx。
        // open_files_array[fd]->nt_node_idx 是正確的。
        NfsOpenFileHandle* fh = m_pNfsHandle->open_files_array[fd]; // 需要確保 fd 有效且 fh 已被填充
        int fileSize = -1;
        if (fh) { // 確保 fh 有效
            fileSize = nfs_nt_node_get_size(m_pNfsHandle->nt_handle, fh->nt_node_idx);
        }

        if (fileSize >= 0) {
            DeleteBuffer(); // 釋放舊的 m_pReadBuffer
            m_pReadBuffer = new (std::nothrow) char[fileSize + 1]; // +1 for null terminator if needed, though binary read
            if (m_pReadBuffer) {
                m_nReadBytes = nfs_file_read(m_pNfsHandle, fd, m_pReadBuffer, fileSize);
                if (m_nReadBytes != fileSize) { // 讀取不完整
                    DeleteBuffer(); // 清理並返回nullptr
                }
                else {
                    // ((char*)m_pReadBuffer)[m_nReadBytes] = '\0'; // 如果是文本，確保空結尾 (可選)
                }
            }
            else {
                m_nReadBytes = 0; // 分配失敗
            }
        }
        else {
            m_nReadBytes = 0; // 獲取大小失敗
        }
        nfs_file_close(m_pNfsHandle, fd);
        m_isLoadingFlag = false;
    }
    return (char*)m_pReadBuffer; // 如果失敗，m_pReadBuffer 會是 nullptr
}

// 釋放 m_pReadBuffer
void CMofPacking::DeleteBuffer() {
    if (m_pReadBuffer) {
        delete[](char*)m_pReadBuffer;
        m_pReadBuffer = nullptr;
    }
}

// 釋放 m_pBuffer1
void CMofPacking::DeleteBuffer1() {
    if (m_pBuffer1) {
        delete[](char*)m_pBuffer1;
        m_pBuffer1 = nullptr;
    }
}

// 取得上次讀取操作的位元組數
int CMofPacking::GetBufferSize() const {
    return m_nReadBytes;
}

// 在NFS中搜尋符合模式的檔案名稱
NfsGlobResults* CMofPacking::SearchString(const char* pattern) {
    if (!m_pNfsHandle) return nullptr;

    // nfs_glob 的 flags 參數在C碼中是4。
    // 假設 4 對應 NFS_FNM_NOSORT (不排序結果)
    // 錯誤回呼函式為 nullptr (0)
    int result = nfs_glob(m_pNfsHandle, pattern, 4 /*NFS_FNM_NOSORT?*/, nullptr, &m_globResults);

    if (result == 0 && m_globResults.gl_pathc > 0) { // 成功且有匹配
        return &m_globResults;
    }
    // 如果 result != 0 (例如 GLOB_NOMATCH) 或 gl_pathc == 0，則返回 nullptr
    // 原始碼邏輯 `*v2 <= 0 ? 0 : (unsigned int)v2`，v2 指向 gl_pathc 的地址，但返回的是 m_globResults 的地址。
    // 所以是 `m_globResults.gl_pathc > 0 ? &m_globResults : nullptr;`
    return nullptr;
}

// 釋放 SearchString 的結果
void CMofPacking::DeleteSearchData() {
    if (!m_pNfsHandle) return;
    // 根據 nfs.h, nfs_glob_free 只需要 NfsGlobResults*
    // 反編譯的C碼 `nfs_glob_free(m_pNfsHandle, &m_globResults)` 是錯誤的。
    nfs_glob_free(&m_globResults);
}

// 將字串轉為小寫
char* CMofPacking::ChangeString(char* str) {
    if (str) {
        return _strlwr(str); // _strlwr 修改原字串並返回它
        // 或者使用C++標準方法：
        // std::string temp_str = str;
        // std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), ::tolower);
        // strcpy(str, temp_str.c_str()); // 如果需要修改原char*
        // return str;
    }
    return nullptr;
}