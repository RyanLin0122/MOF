#pragma once
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include "Text/cltTextFileManager.h"

// 全局文本文件管理器声明
extern cltTextFileManager g_clTextFileManager;

class DCTTextManager {
private:
    unsigned int max_string_id;     // 最大字符串ID
    void* string_buffer;            // 字符串缓冲区指针
    char* string_pointers[65535];   // 字符串指针数组，最多65535个字符串
    
    // 静态临时字符串缓冲区（用于GetParsedText）
    static char temp_string_buffer[10240];

public:
    // 构造函数
    DCTTextManager();
    
    // 析构函数
    ~DCTTextManager();
    
    // 初始化，从指定文件加载文本数据
    // 返回值：成功返回非零值，失败返回0
    int Initialize(char* filename);
    
    // 释放内存
    void Free();
    
    // 根据ID获取文本
    // 参数：string_id - 字符串ID
    // 返回值：对应的文本字符串，如果ID无效则返回默认字符串（ID=0的字符串）
    char* GetText(int string_id);
    
    // 获取解析后的文本（处理转义字符）
    // 参数：string_id - 字符串ID
    //       ignore_newline - 是否忽略换行符（1=忽略，0=转换为实际换行）
    //       output_buffer - 输出缓冲区（可选，如果提供则复制结果到此缓冲区）
    // 返回值：解析后的文本字符串
    char* GetParsedText(int string_id, int ignore_newline = 0, char* output_buffer = nullptr);
    
    // 获取最大字符串ID
    unsigned int GetLastestStringID();

private:
    // 辅助函数：判断字符是否为数字
    static bool IsDigit(const char* str);
};
