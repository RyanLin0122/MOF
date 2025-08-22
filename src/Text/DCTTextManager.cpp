#include "Text/DCTTextManager.h"
#include <algorithm>

// 静态成员定义
char DCTTextManager::temp_string_buffer[10240];

// 构造函数
DCTTextManager::DCTTextManager() : max_string_id(0), string_buffer(nullptr) {
    // 初始化字符串指针数组为空
    memset(string_pointers, 0, sizeof(string_pointers));
}

// 析构函数
DCTTextManager::~DCTTextManager() {
    Free();
}

// 初始化函数
int DCTTextManager::Initialize(char* filename) {
    // 设置分隔符
    const char delimiters[] = "\t\r\n";
    
    // 清零字符串指针数组 (262140字节 = 65535 * 4)
    memset(string_pointers, 0, sizeof(string_pointers));
    //max_string_id = 0;
    
    // 从文件管理器获取文件头信息
    Header* header = g_clTextFileManager.GetHeaderByFileName(filename);
    if (!header) {
        return 0;
    }
    
    // 分配字符串缓冲区内存
    string_buffer = operator new(header->original_size);
    if (!string_buffer) {
        return 0;
    }
    
    FILE* file = nullptr;
    
    // 特殊处理MoFTexts.txt文件
    if (strcmp(filename, "MoFTexts.txt") == 0) {
        char local_path[264];
        sprintf(local_path, "MofData/%s", filename);
        
        // 首先尝试从本地路径打开
        file = fopen(local_path, "rb");
        
        // 如果本地文件不存在，则从文件包中打开
        if (!file) {
            file = g_clTextFileManager.fopen(filename);
        }
    } else {
        // 其他文件直接从文件包中打开
        file = g_clTextFileManager.fopen(filename);
    }
    
    if (!file) {
        Free();
        return 0;
    }
    
    // 读取文件内容
    char buffer[10240];
    
    // 跳过前3行（可能是头信息或注释）
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);
    
    char* current_string_pos = (char*)string_buffer;
    bool success = false;
    
    // 处理文件内容
    while (fgets(buffer, sizeof(buffer), file)) {
        // 解析每一行
        char* token = strtok(buffer, delimiters);
        
        if (!token) {
            continue;
        }
        
        // 检查是否为数字（字符串ID）
        if (IsDigit(token)) {
            int string_id = atoi(token);
            
            // 验证ID范围和是否已存在
            if (string_id >= 0 && string_id < 65535 && string_pointers[string_id] == nullptr) {
                // 获取字符串内容
                char* string_content = strtok(nullptr, delimiters);
                
                if (string_content) {
                    // 检查字符串是否包含引号或错误标记
                    if (strchr(string_content, '\"') == nullptr && 
                        strstr(string_content, "#NAME?") == nullptr) {
                        
                        // 设置字符串指针
                        string_pointers[string_id] = current_string_pos;
                        
                        // 更新最大字符串ID
                        if ((unsigned int)string_id > max_string_id) {
                            max_string_id = string_id;
                        }
                        
                        // 复制字符串内容
                        size_t string_length = strlen(string_content) + 1;
                        strncpy(current_string_pos, string_content, string_length - 1);
                        current_string_pos[string_length - 1] = '\0';
                        current_string_pos += string_length;
                    }
                }
            }
        }
    }
    
    success = true;
    
    // 关闭文件
    if (file) {
        g_clTextFileManager.fclose(file);
    }
    
    // 如果加载失败，清理内存
    if (!success) {
        Free();
        return 0;
    }
    
    return 1; // 成功
}

// 释放内存
void DCTTextManager::Free() {
    if (string_buffer) {
        operator delete(string_buffer);
        string_buffer = nullptr;
    }
    
    // 清空字符串指针数组
    //memset(string_pointers, 0, sizeof(string_pointers));
    //max_string_id = 0;
}

// 根据ID获取文本
char* DCTTextManager::GetText(int string_id) {
    // 检查ID范围和有效性
    if (string_id < 0 || string_id >= 65535 || string_pointers[string_id] == nullptr) {
        // 返回默认字符串（ID=0的字符串）
        return string_pointers[0];
    }
    
    return string_pointers[string_id];
}

// 获取解析后的文本（处理转义字符）
char* DCTTextManager::GetParsedText(int string_id, int ignore_newline, char* output_buffer) {
    // 清空临时缓冲区
    temp_string_buffer[0] = '\0';
    
    // 获取原始文本
    char* source_text = GetText(string_id);
    if (!source_text) {
        return temp_string_buffer;
    }
    
    char* dest = temp_string_buffer;
    unsigned char* src = (unsigned char*)source_text;
    char* text_end = source_text + strlen(source_text) - 1;
    
    // 处理字符串中的每个字符
    while (*src) {
        if (src > (unsigned char*)text_end) {
            break;
        }
        
        // 处理双字节字符（DBCS）
        if (src < (unsigned char*)text_end && IsDBCSLeadByte(*src)) {
            // 复制双字节字符
            strncpy(dest, (const char*)src, 2);
            dest += 2;
            src += 2;
        }
        // 处理转义换行符
        else if (strncmp((const char*)src, "\\n", 2) == 0) {
            if (!ignore_newline) {
                *dest++ = '\n';  // 转换为实际换行符
            }
            src += 2;
        }
        // 普通字符
        else {
            *dest++ = *src++;
        }
    }
    
    // 添加字符串结束符
    *dest = '\0';
    
    // 如果提供了输出缓冲区，复制结果
    if (output_buffer) {
        strcpy(output_buffer, temp_string_buffer);
    }
    
    return temp_string_buffer;
}

// 获取最大字符串ID
unsigned int DCTTextManager::GetLastestStringID() {
    return max_string_id;
}

// 辅助函数：判断字符串是否为数字
bool DCTTextManager::IsDigit(const char* str) {
    if (!str || *str == '\0') {
        return false;
    }
    
    // 允许负号开头
    //if (*str == '-') {
    //    str++;
    //}
    
    // 检查剩余字符是否都是数字
    while (*str) {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }
    
    return true;
}

/*
使用示例：

// 全局文本文件管理器实例
cltTextFileManager g_clTextFileManager;

int main() {
    // 初始化文件管理器
    if (g_clTextFileManager.Initialize("texts.pak")) {
        
        DCTTextManager textManager;
        
        // 加载文本数据
        if (textManager.Initialize("strings.txt")) {
            
            // 获取文本
            char* text = textManager.GetText(100);
            if (text) {
                printf("String ID 100: %s\n", text);
            }
            
            // 获取解析后的文本（处理转义字符）
            char* parsed_text = textManager.GetParsedText(100, 0);
            if (parsed_text) {
                printf("Parsed String ID 100: %s\n", parsed_text);
            }
            
            // 获取最大字符串ID
            printf("Max String ID: %u\n", textManager.GetLastestStringID());
        }
    }
    
    return 0;
}
*/
