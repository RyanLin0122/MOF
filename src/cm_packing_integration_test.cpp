#include "cm_packing_integration_test.h"
#include "CMofPacking.h"

// 為了方便，我們可以在測試程式中存取 CMofPacking 的私有成員以進行驗證
// 透過定義一個 friend class 或暫時將成員設為 public。
// 但更簡單的方式是，我們假設可以透過 nfs API 間接驗證其行為。
// 例如，檢查 m_pNfsHandle 是否為 nullptr，可以透過下一個 API 呼叫是否成功來判斷。
// 為了驗證的明確性，我們這裡直接使用 nfs API 來檢查 VFS 內部狀態。

// 簡單的測試結果記錄
static int tests_passed = 0;
static int tests_failed = 0;

// 測試用的 VFS 檔案基礎名稱
const char* INT_TEST_VFS_BASENAME = "integration_test";

// 測試執行的宏，簡化主函式
#define RUN_TEST(test_func) \
    do { \
        std::cout << "Running test: " #test_func "..." << std::endl; \
        try { \
            test_func(); \
            tests_passed++; \
            std::cout << #test_func << " ... PASSED" << std::endl; \
        } catch (const std::exception& e) { \
            tests_failed++; \
            std::cout << #test_func << " ... FAILED: " << e.what() << std::endl; \
        } catch (...) { \
            tests_failed++; \
            std::cout << #test_func << " ... FAILED: Unknown exception" << std::endl; \
        } \
        std::cout << "----------------------------------------" << std::endl; \
    } while(0)

// 輔助函式：清理測試產生的檔案
void cleanup_test_vfs_files(const std::string& base_name) {
    std::filesystem::remove(base_name + ".paki");
	std::filesystem::remove(base_name + ".pak");
	std::filesystem::remove(base_name + ".lock");
}

// 輔助函式：在本地建立一個帶有內容的檔案
void create_local_file(const std::string& path, const std::string& content) {
    std::ofstream outfile(path);
    assert(outfile.is_open());
    outfile << content;
    outfile.close();
}

// --- 測試案例 ---

// 測試1：基本生命週期 (GetInstance, Open, Close, Destroy)
void test_lifecycle() {
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);

    CMofPacking* packer = CMofPacking::GetInstance();
    assert(packer != nullptr);

    // 測試開啟一個新的封裝檔案
    assert(packer->PackFileOpen(INT_TEST_VFS_BASENAME) == true);
    // 檢查底層 nfs handle 是否被建立
    assert(packer->m_pNfsHandle != nullptr); 
    
    // 測試關閉
    assert(packer->PackFileClose() == true);
    assert(packer->m_pNfsHandle == nullptr);

    // 驗證實體檔案被保留
    assert(std::filesystem::exists(std::string(INT_TEST_VFS_BASENAME) + ".paki"));
    assert(std::filesystem::exists(std::string(INT_TEST_VFS_BASENAME) + ".pak"));

    // 測試重新開啟
    assert(packer->PackFileOpen(INT_TEST_VFS_BASENAME) == true);
    assert(packer->m_pNfsHandle != nullptr);
    assert(packer->PackFileClose() == true);

    CMofPacking::DestroyInstance();
    // 檢查單例指標是否被重置 (無法直接檢查，但再次 GetInstance 應產生新物件)
    packer = CMofPacking::GetInstance();
    assert(packer != nullptr);
    CMofPacking::DestroyInstance();

    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
}

// 測試2：新增與移除檔案
void test_add_and_remove_file() {
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
    CMofPacking* packer = CMofPacking::GetInstance();
    assert(packer->PackFileOpen(INT_TEST_VFS_BASENAME) == true);
    
    const char* file1 = "config/settings.ini";
    const char* file2 = "root_file.txt";

    // 1. 新增一個檔案
    assert(packer->AddFile(file1) == true);
    // 使用 nfs API 驗證檔案是否存在
    assert(nfs_file_exists(packer->m_pNfsHandle, file1) == 1);
    
    // 2. 新增第二個檔案
    assert(packer->AddFile(file2) == true);
    assert(nfs_file_exists(packer->m_pNfsHandle, file2) == 1);

    // 3. 測試覆寫 (AddFile 內部會先移除)
    assert(packer->AddFile(file1) == true);
    assert(nfs_file_exists(packer->m_pNfsHandle, file1) == 1);

    // 4. 移除檔案
    assert(packer->RemoveFile(file1) == true);
    assert(nfs_file_exists(packer->m_pNfsHandle, file1) == 0);

    // 5. 移除一個不存在的檔案 (修正後的 RemoveFile 應返回 false)
    assert(packer->RemoveFile("non_existent_file.tmp") == false);

    assert(packer->PackFileClose() == true);
    CMofPacking::DestroyInstance();
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
}

// 測試3：讀寫驗證
void test_read_write() {
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
    CMofPacking* packer = CMofPacking::GetInstance();
    assert(packer->PackFileOpen(INT_TEST_VFS_BASENAME) == true);

    const char* file_in_pack = "data/story.txt";
    const std::string content = "This is a test story that will be written and read back.";

    // 為了寫入，我們需要手動使用 nfs API，因為 CMofPacking 沒有提供 Write 介面
    int fd = nfs_file_create(packer->m_pNfsHandle, file_in_pack);
    assert(fd >= 0);
    int bytes_written = nfs_file_write(packer->m_pNfsHandle, fd, content.c_str(), content.length());
    assert(bytes_written == (int)content.length());
    nfs_file_close(packer->m_pNfsHandle, fd);

    // 現在使用 CMofPacking 的 FileRead 來讀回
    char* buffer = packer->FileRead(file_in_pack);
    assert(buffer != nullptr);
    
    // 驗證讀回的內容和大小
    assert(packer->GetBufferSize() == (int)content.length());
    assert(strncmp(buffer, content.c_str(), content.length()) == 0);

    packer->DeleteBuffer();
    assert(packer->m_pReadBuffer == nullptr);

    assert(packer->PackFileClose() == true);
    CMofPacking::DestroyInstance();
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
}

// 測試4：檔案搜尋 (Globbing)
void test_glob_search() {
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
    CMofPacking* packer = CMofPacking::GetInstance();
    assert(packer->PackFileOpen(INT_TEST_VFS_BASENAME) == true);

    // 新增一些用於測試的檔案
    packer->AddFile("image.jpg");
    packer->AddFile("photo.jpg");
    packer->AddFile("document.pdf");
    packer->AddFile("archive.zip");

    // 搜尋所有 .jpg 檔案
    NfsGlobResults* results = packer->SearchString("*.jpg");
    assert(results != nullptr);
    assert(results->gl_pathc == 2);

    // 為了穩定比較，對結果進行排序
    std::vector<std::string> found_files;
    for (size_t i = 0; i < results->gl_pathc; ++i) {
        found_files.push_back(results->gl_pathv[i]);
    }
    std::sort(found_files.begin(), found_files.end());
    
    assert(found_files[0] == "image.jpg");
    assert(found_files[1] == "photo.jpg");

    packer->DeleteSearchData();
    // 驗證結果是否被清空
    assert(packer->m_globResults.gl_pathc == 0);
    assert(packer->m_globResults.gl_pathv == nullptr);

    assert(packer->PackFileClose() == true);
    CMofPacking::DestroyInstance();
    cleanup_test_vfs_files(INT_TEST_VFS_BASENAME);
}

// 測試5：打包整個目錄
void test_directory_packing() {
    const std::string dir_name = "temp_pack_dir";
    const std::string pack_name = "dir_pack_test";
    
    // 準備環境：建立本地目錄和檔案
    std::filesystem::remove_all(dir_name); // 清理舊的測試目錄
    cleanup_test_vfs_files(pack_name);
    
    std::filesystem::create_directory(dir_name);
    std::filesystem::create_directory(dir_name + "/subdir");
    create_local_file(dir_name + "/root.txt", "root file");
    create_local_file(dir_name + "/image.png", "png data");
    create_local_file(dir_name + "/subdir/nested.dat", "nested data");

    // 開始打包
    CMofPacking* packer = CMofPacking::GetInstance();
    assert(packer->PackFileOpen(pack_name.c_str()) == true);
    
    // 呼叫 DataPacking，注意路徑結尾需要斜線
    int result = packer->DataPacking((dir_name + "\\").c_str());
    assert(result == 2); // 2 表示正常完成

    // 驗證封裝內的檔案
    // 注意：DataPacking 內部會將檔名轉為小寫
    assert(nfs_file_exists(packer->m_pNfsHandle, "root.txt") == 1);
    assert(nfs_file_exists(packer->m_pNfsHandle, "image.png") == 1);
    assert(nfs_file_exists(packer->m_pNfsHandle, "nested.dat") == 1); // 假設路徑被扁平化處理
    assert(nfs_file_exists(packer->m_pNfsHandle, "non_existent.file") == 0);
    
    assert(packer->PackFileClose() == true);
    
    // 測試 mof.ini 中斷條件
    std::filesystem::create_directory(dir_name + "/stop_test");
    create_local_file(dir_name + "/stop_test/a.txt", "a");
    create_local_file(dir_name + "/stop_test/mof.ini", "stop");
    create_local_file(dir_name + "/stop_test/z.txt", "z");
    
    assert(packer->PackFileOpen(pack_name.c_str()) == true);
    result = packer->DataPacking((dir_name + "\\stop_test\\").c_str());
    assert(result == 1); // 應返回 1 表示找到 mof.ini
    
    // 驗證 z.txt 沒有被打包進去
    assert(nfs_file_exists(packer->m_pNfsHandle, "z.txt") == 0);
    
    // 清理
    assert(packer->PackFileClose() == true);
    CMofPacking::DestroyInstance();
    cleanup_test_vfs_files(pack_name);
    std::filesystem::remove_all(dir_name);
}

// --- 主函式 ---
int run_cmofpacking_tests()
{
    RUN_TEST(test_lifecycle);
    RUN_TEST(test_add_and_remove_file);
    RUN_TEST(test_read_write);
    RUN_TEST(test_glob_search);
    RUN_TEST(test_directory_packing);

    std::cout << "========================================" << std::endl;
    std::cout << "All tests completed." << std::endl;
    std::cout << "Tests Passed: " << tests_passed << std::endl;
    std::cout << "Tests Failed: " << tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    if (tests_failed > 0) {
        std::cerr << "THERE WERE TEST FAILURES!" << std::endl;
        return 1;
    }

    std::cout << "All integration tests passed successfully!" << std::endl;
    return 0;
}