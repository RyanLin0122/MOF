#include "Text/cltTextFileManager.h"

// 构造函数
cltTextFileManager::cltTextFileManager() {
    // 空构造函数，所有初始化在Initialize中进行
}

// 初始化函数，加载文件包的文件头信息
int cltTextFileManager::Initialize(char* filename) {
    // 清零整个headers数组 (45056字节)
    memset(headers, 0, 0xB000);
    
    // 保存存档文件名
    strcpy(archive_filename, filename);
    
    // 打开存档文件
    FILE* archive_file = ::fopen(filename, "rb");
    if (archive_file) {
        // 读取所有文件头信息
        fread(headers, 0xB000, 1, archive_file);
        
        // 关闭文件
        ::fclose(archive_file);
        
        return 1; // 成功
    }
    
    return 0; // 失败
}

// 打开指定文件名的文件
FILE* cltTextFileManager::fopen(char* target_filename) {
    // 打开存档文件
    FILE* archive_file = ::fopen(archive_filename, "rb");
    if (!archive_file) {
        return nullptr;
    }
    
    // 查找目标文件
    int file_index = 0;
    Header* current_header = headers;
    
    // 遍历所有文件头
    while (file_index < 1024 && current_header) {
        // 比较文件名（不区分大小写）
        if (_stricmp(current_header->filename, target_filename) == 0) {
            // 找到目标文件
            
            // 移动到文件数据位置
            fseek(archive_file, current_header->file_offset, SEEK_SET);
            
            // 分配内存存储压缩数据
            void* compressed_data = operator new(current_header->compressed_size);
            
            // 读取压缩数据
            fread(compressed_data, current_header->compressed_size, 1, archive_file);
            
            // 分配内存存储解压后的数据
            unsigned char* decompressed_data = (unsigned char*)operator new(current_header->original_size);
            
            // 使用Huffman算法解压
            Huffman_Uncompress((unsigned char*)compressed_data, 
                             decompressed_data, 
                             current_header->compressed_size, 
                             current_header->original_size);
            
            // 释放压缩数据内存
            if (compressed_data) {
                operator delete(compressed_data);
            }
            
            // 创建临时文件
            FILE* temp_file = ::fopen("txt.tmp", "wb");
            if (temp_file) {
                // 将解压后的数据写入临时文件
                fwrite(decompressed_data, current_header->original_size, 1, temp_file);
                ::fclose(temp_file);
                
                // 释放解压数据内存
                if (decompressed_data) {
                    operator delete(decompressed_data);
                }
                
                // 关闭存档文件
                ::fclose(archive_file);
                
                // 以文本模式重新打开临时文件
                return ::fopen("txt.tmp", "rt");
            }
            else {
                // 临时文件创建失败
                GetLastError();
                if (decompressed_data) {
                    operator delete(decompressed_data);
                }
                ::fclose(archive_file);
                return nullptr;
            }
        }
        
        // 继续查找下一个文件
        file_index++;
        current_header = &headers[file_index];
    }
    
    // 文件未找到，关闭存档文件并尝试打开临时文件
    ::fclose(archive_file);
    return ::fopen("txt.tmp", "rt");
}

// 关闭文件
void cltTextFileManager::fclose(FILE* stream) {
    // 关闭文件流
    ::fclose(stream);
    
    // 删除临时文件
    DeleteFileA("txt.tmp");
}

// 根据文件名获取文件头信息
Header* cltTextFileManager::GetHeaderByFileName(char* target_filename) {
    // 尝试打开存档文件以验证其存在
    FILE* archive_file = ::fopen(archive_filename, "rb");
    if (!archive_file) {
        return nullptr;
    }
    ::fclose(archive_file);
    
    // 查找目标文件
    int file_index = 0;
    Header* current_header = headers;
    
    while (file_index < 1024) {
        // 比较文件名（不区分大小写）
        if (_stricmp(current_header->filename, target_filename) == 0) {
            // 找到目标文件，返回头部信息
            return current_header;
        }
        
        // 继续查找下一个文件
        file_index++;
        current_header = &headers[file_index];
    }
    
    // 文件未找到
    return nullptr;
}

