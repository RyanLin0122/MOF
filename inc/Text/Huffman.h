#include <cstdint>
#include <cstring>

// 结构体定义
struct huff_bitstream_t {
    uint8_t* data;      // 数据指针
    int bit_pos;        // 位位置 (0-7)
    int remaining;      // 剩余字节数
};

struct huff_sym_t {
    int symbol;         // 符号值
    int frequency;      // 频率
    int code;           // Huffman编码
    int code_length;    // 编码长度
};

// 函数声明
void _Huffman_InitBitstream(huff_bitstream_t* bitstream, uint8_t* data, unsigned int size);
void _Huffman_WriteBits(huff_bitstream_t* bitstream, unsigned int value, unsigned int bits);
unsigned int _Huffman_ReadBits(huff_bitstream_t* bitstream, unsigned int bits);
void _Huffman_Hist(uint8_t* input, huff_sym_t* symbols, unsigned int size);
void _Huffman_MakeTree(huff_sym_t* symbols, huff_bitstream_t* bitstream,
    unsigned int code, unsigned int depth,
    unsigned int start, unsigned int end);
void _Huffman_RecoverTree(huff_sym_t* symbols, huff_bitstream_t* bitstream,
    unsigned int code, unsigned int depth, unsigned int* count);
int Huffman_Compress(uint8_t* input, uint8_t* output, unsigned int input_size);
void Huffman_Uncompress(uint8_t* input, uint8_t* output,
    unsigned int input_size, unsigned int output_size);