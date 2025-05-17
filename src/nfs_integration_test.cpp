#include "nfs.h"
#include "nfs_integration_test.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>

// 測試 1：建立檔案、寫入小於一個區塊、讀回比對
void test_create_write_read_small() {
    const char* fname = "int_test1.dat";
    remove(fname);

    // 1. 建檔並分配一條 channel（1 block per stripe）
    NfsIioFile* f = nfs_iio_create(fname);
    assert(f);
    int ch = nfs_iio_allocate_channel(f, 1);
    assert(ch == 0);

    // 2. 寫入「Hello」
    const char* msg = "Hello";
    int nw = nfs_iio_write(f, ch, msg, strlen(msg));
    assert(nw == (int)strlen(msg));

    // 3. 關閉並重開
    nfs_iio_close(f);
    //nfs_iio_destroy(f);

    // 4. 再開新的 handle 來讀
    NfsIioFile* f2 = nfs_iio_open(fname);
    assert(f2);

    // 5. 讀回並比較
    char buf[16] = {};
    int nr = nfs_iio_read(f2, ch, buf, strlen(msg));
    assert(nr == (int)strlen(msg));
    assert(std::memcmp(buf, msg, strlen(msg)) == 0);

    nfs_iio_close(f2);
    //nfs_iio_destroy(f2);
    printf("test_create_write_read_small passed\n");
}

// 測試 2：跨區塊寫入（超過 block size）、邊界對齊與讀回
void test_cross_block_write_read() {
    const char* fname = "int_test2.dat";
    remove(fname);

    NfsIioFile* f = nfs_iio_create(fname);
    assert(f);
    int ch = nfs_iio_allocate_channel(f, 1);
    assert(ch == 0);

    // 構造一個超過 two blocks 的資料（假設 BLOCK_SIZEv = 512）
    int bs = nfs_iio_BLOCK_SIZEv;
    std::vector<char> data(bs * 2 + 100);
    for (int i = 0; i < (int)data.size(); i++) data[i] = (char)(i & 0xFF);

    int nw = nfs_iio_write(f, ch, data.data(), data.size());
    assert(nw == (int)data.size());

    // 讀回
    std::vector<char> buf(data.size());
    int nr = nfs_iio_read(f, ch, buf.data(), buf.size());
    assert(nr == (int)buf.size());
    assert(std::memcmp(buf.data(), data.data(), buf.size()) == 0);

    nfs_iio_close(f);
    nfs_iio_destroy(f);
    printf("test_cross_block_write_read passed\n");
}

// 測試 3：多 channel 交錯寫入、讀取
void test_multi_channel_isolation() {
    const char* fname = "int_test3.dat";
    remove(fname);

    NfsIioFile* f = nfs_iio_create(fname);
    assert(f);
    int ch0 = nfs_iio_allocate_channel(f, 1);
    int ch1 = nfs_iio_allocate_channel(f, 1);
    assert(ch0 == 0 && ch1 == 1);

    // 分別寫入不同內容
    const char* msg0 = "Channel0";
    const char* msg1 = "Channel1-ABCDEFG";
    assert(nfs_iio_write(f, ch0, msg0, strlen(msg0)) == (int)strlen(msg0));
    assert(nfs_iio_write(f, ch1, msg1, strlen(msg1)) == (int)strlen(msg1));

    // 讀回驗證
    char buf0[32] = {}, buf1[32] = {};
    assert(nfs_iio_read(f, ch0, buf0, strlen(msg0)) == (int)strlen(msg0));
    assert(nfs_iio_read(f, ch1, buf1, strlen(msg1)) == (int)strlen(msg1));
    assert(std::memcmp(buf0, msg0, strlen(msg0)) == 0);
    assert(std::memcmp(buf1, msg1, strlen(msg1)) == 0);

    nfs_iio_close(f);
    nfs_iio_destroy(f);
    printf("test_multi_channel_isolation passed\n");
}

// 測試 4：truncate 後的行為－讀回應該為 0
void test_channel_truncate() {
    const char* fname = "int_test4.dat";
    remove(fname);

    NfsIioFile* f = nfs_iio_create(fname);
    assert(f);
    int ch = nfs_iio_allocate_channel(f, 1);
    const char* msg = "ABCDEFG";
    assert(nfs_iio_write(f, ch, msg, strlen(msg)) == (int)strlen(msg));

    // truncate
    assert(nfs_iio_channel_truncate(f, ch) == 0);

    // 再讀，全部應回 0
    char buf[16];
    int nr = nfs_iio_read(f, ch, buf, 16);
    assert(nr == 16);
    for (int i = 0; i < 16; i++) assert(buf[i] == 0);

    nfs_iio_close(f);
    nfs_iio_destroy(f);
    printf("test_channel_truncate passed\n");
}