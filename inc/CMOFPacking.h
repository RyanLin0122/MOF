#ifndef CMOFPACKING_H
#define CMOFPACKING_H

#include "nfs.h"    // 包含 NfsHandle, NfsGlobResults, 以及所有 nfs_* API 函數
#include <windows.h> // 為了 WIN32_FIND_DATAA, HANDLE 等 (已由 nfs.h 間接包含)
#include <string.h>  // 為了 memset, _strlwr (或使用 <cstring>)
#include <stdio.h>   // 為了 _sprintf (或使用 <cstdio>)
#include <cstring>   // C++版本的 string.h
#include <cstdio>    // C++版本的 stdio.h


// CMofPacking 類別定義
class CMofPacking {
public:
	CMofPacking();
	virtual ~CMofPacking(); // 虛擬解構函式，因為反編譯碼顯示了 scalar deleting destructor

	bool Init(); // 初始化函數
	bool PackFileOpen(const char* packFileName); // 開啟NFS封裝檔案
	bool PackFileClose(); // 關閉NFS封裝檔案

	bool AddFile(const char* filePathInPack); // 新增檔案至封裝 (若已存在則先移除再建立)
	bool RemoveFile(const char* filePathInPack); // 從封裝中移除檔案

	// DataPacking 返回值：
	// 0: FindFirstFileA 失敗 (目錄不存在或為空)
	// 1: 找到 "mof.ini"，處理終止
	// 2: 目錄正常處理完畢
	int DataPacking(const char* directoryPath); // 將指定目錄下的所有檔案打包進NFS

	void FileReadBackGroundLoading(const char* filePathInPack); // 背景載入式讀取檔案至內部緩衝區 m_backgroundLoadBuffer
	char* FileRead(const char* filePathInPack); // 讀取檔案至動態分配的內部緩衝區 m_pReadBuffer

	void DeleteBuffer();  // 釋放 m_pReadBuffer
	void DeleteBuffer1(); // 釋放 m_pBuffer1 (此緩衝區在提供的C碼中似乎未被用於載入)

	int GetBufferSize() const; // 取得上次讀取操作的位元組數 (m_nReadBytes)

	NfsGlobResults* SearchString(const char* pattern); // 在NFS中搜尋符合模式的檔案名稱
	void DeleteSearchData(); // 釋放 SearchString 的結果

	char* ChangeString(char* str); // 將字串轉為小寫 (輔助函數)

	// m_backgroundLoadBufferField (偏移量292，大小0x7A1200)
	// 用於 FileReadBackGroundLoading 的固定大小緩衝區。
	// 根據 `memset((char *)this + 292, 0, 0x7A1200u);`，它緊隨其前的成員之後。
	// 前述成員 (vtable, ptrs, char[256], int, NfsGlobResults) 總大小恰好為 292 位元組 (假設32位元指標和size_t)。
	char  m_backgroundLoadBufferField[0x7A1200];
private:
	// 成員變數的順序和類型基於對反編譯C碼的分析
	// 實際記憶體佈局由編譯器決定，但這裡的順序參考了初始化和memset操作的偏移

	NfsHandle* m_pNfsHandle;          // 指向NFS檔案系統控制代碼的指標 (對應 this+1 * sizeof(DWORD))
	void* m_pReadBuffer;         // 由FileRead分配，用於儲存完整檔案內容 (對應 this+2 * sizeof(DWORD))
	void* m_pBuffer1;            // 另一個緩衝區指標，DeleteBuffer1會釋放它 (對應 this+3 * sizeof(DWORD))

	// m_tempPathBuffer (偏移量16，大小256位元組)
	// 在CMofPacking::CMofPacking中由 memset((char *)this + 16, 0, 0x100u) 初始化。
	// 在DataPacking中，本地變數FileName, Buffer, v13的大小均為256，此成員變數可能用於類似目的，或為未使用。
	// 在AddFile中，本地變數v5大小為2048。
	// 此處保留一個256位元組的成員，如反編譯碼所示。
	char       m_tempPathBuffer[256];

	int            m_nReadBytes;      // 記錄上次讀取操作的位元組數 (對應 this+68 * sizeof(DWORD))
	NfsGlobResults m_globResults;     // 儲存nfs_glob的搜尋結果 (其首地址對應 this+276)
	// m_globResults.internal_callback_error_flag 對應 this+72 * sizeof(DWORD)

// m_isLoadingFlag 對應 C 碼中的 byte_7A1324[(_DWORD)this]
// 這是一個執行緒不安全的標記，指示是否有檔案正在被讀取。
// 其在記憶體中的確切位置相對於其他成員，從C碼難以完美對應到C++的標準佈局中，
// 特別是考慮到 m_backgroundLoadBufferField 的 memset 從 this+292 開始。
// 這裡將其聲明為一個bool成員，C++編譯器會為其分配位置。
	bool           m_isLoadingFlag;
};

#endif // CMOFPACKING_H