#pragma once
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include "Text/Huffman.h"

// 文件头结构体
struct Header {
    char filename[32];          // 文件名 (32字节)
    unsigned int file_offset;   // 文件在包中的偏移位置 (4字节)
    unsigned int compressed_size;   // 压缩后大小 (4字节)
    unsigned int original_size;     // 原始文件大小 (4字节)
    // 总共44字节
};

class cltTextFileManager {
private:
    Header headers[1024];       // 文件头数组，最多1024个文件，每个44字节 = 45056字节
    char archive_filename[256]; // 存档文件名，256字节
    // 总共: 45056 + 256 = 45312 字节 (0xB100)
    // 但代码中使用0xB000 (45056)，说明只初始化headers部分

public:
    // 构造函数
    cltTextFileManager();
    
    // 析构函数
    ~cltTextFileManager() = default;
    
    // 初始化，加载文件包
    int Initialize(char* filename);
    
    // 打开文件（解压到临时文件）
    FILE* fopen(char* filename);
    
    // 关闭文件（删除临时文件）
    void fclose(FILE* stream);
    
    // 根据文件名获取文件头
    Header* GetHeaderByFileName(char* filename);
};
