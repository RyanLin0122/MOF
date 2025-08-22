#include "Text/Huffman.h"


// 初始化位流
void _Huffman_InitBitstream(huff_bitstream_t* bitstream, uint8_t* data, unsigned int size) {
    bitstream->data = data;
    bitstream->bit_pos = 0;
    bitstream->remaining = size;
}

// 写入指定位数的数据
void _Huffman_WriteBits(huff_bitstream_t* bitstream, unsigned int value, unsigned int bits) {
    uint8_t* current_byte = bitstream->data;
    int remaining_bytes = bitstream->remaining;
    int bit_position = bitstream->bit_pos;
    int bits_written = 0;

    if (bits > 0) {
        do {
            if (remaining_bytes <= 0) break;

            // 计算当前位的值 (从最高位开始)
            bool bit_value = ((1 << (bits - 1)) & value) != 0;

            // 清除目标位置的位，然后设置新值
            *current_byte = (*current_byte & ~(1 << (7 - bit_position))) +
                (bit_value << (7 - bit_position));

            value <<= 1;  // 左移准备下一位
            bit_position = (bit_position + 1) & 7;

            if (bit_position == 0) {
                current_byte++;
                remaining_bytes--;
            }

            bits_written++;
        } while (bits_written < bits);
    }

    bitstream->bit_pos = bit_position;
    bitstream->data = current_byte;
    bitstream->remaining = remaining_bytes;
}

// 读取指定位数的数据
unsigned int _Huffman_ReadBits(huff_bitstream_t* bitstream, unsigned int bits) {
    unsigned int result = 0;
    uint8_t* current_byte = bitstream->data;
    int bit_position = bitstream->bit_pos;
    int remaining_bytes = bitstream->remaining;

    for (unsigned int i = 0; i < bits; i++) {
        if (remaining_bytes <= 0) break;

        bool bit_value = (*current_byte & (1 << (7 - bit_position))) != 0;
        bit_position = (bit_position + 1) & 7;
        result = bit_value + 2 * result;

        if (bit_position == 0) {
            remaining_bytes--;
            current_byte++;
        }
    }

    bitstream->remaining = remaining_bytes;
    bitstream->data = current_byte;
    bitstream->bit_pos = bit_position;
    return result;
}

// 统计频率并排序
void _Huffman_Hist(uint8_t* input, huff_sym_t* symbols, unsigned int size) {
    // 初始化符号表
    for (int i = 0; i < 256; i++) {
        symbols[i].symbol = i;
        symbols[i].frequency = 0;
        symbols[i].code = 0;
        symbols[i].code_length = 0;
    }

    // 统计频率
    for (unsigned int i = 0; i < size; i++) {
        symbols[input[i]].frequency++;
    }

    // 冒泡排序 - 按频率升序排列
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i < 255; i++) {
            if (symbols[i].frequency > symbols[i + 1].frequency) {
                // 交换整个结构
                huff_sym_t temp = symbols[i];
                symbols[i] = symbols[i + 1];
                symbols[i + 1] = temp;
                swapped = true;
            }
        }
    } while (swapped);
}

// 构建Huffman树并写入树结构
void _Huffman_MakeTree(huff_sym_t* symbols, huff_bitstream_t* bitstream,
    unsigned int code, unsigned int depth,
    unsigned int start, unsigned int end) {
    if (start == end) {
        // 叶子节点
        _Huffman_WriteBits(bitstream, 1, 1);
        _Huffman_WriteBits(bitstream, symbols[start].symbol, 8);
        symbols[start].code = code;
        symbols[start].code_length = depth;
        return;
    }

    // 内部节点
    _Huffman_WriteBits(bitstream, 0, 1);

    unsigned int next_depth = depth + 1;

    while (true) {
        _Huffman_WriteBits(bitstream, 0, 1);

        // 计算总权重
        unsigned int total_weight = 0;
        if (start <= end) {
            for (unsigned int i = start; i <= end; i++) {
                total_weight += symbols[i].frequency;
            }
        }

        // 找到分割点
        unsigned int left_weight = 0;
        unsigned int split_point = start;

        for (unsigned int i = start; i < end; i++) {
            left_weight += symbols[i].frequency;
            split_point++;
            if (left_weight > total_weight / 2) break;
        }

        if (left_weight > 0) {
            _Huffman_WriteBits(bitstream, 1, 1);
            _Huffman_MakeTree(symbols, bitstream, 2 * code, next_depth, start, split_point - 1);
        }
        else {
            _Huffman_WriteBits(bitstream, 0, 1);
        }

        if (total_weight == left_weight) break;

        _Huffman_WriteBits(bitstream, 1, 1);
        next_depth++;
        start = split_point;
        depth++;
        code = 2 * code + 1;

        if (split_point == end) {
            // 处理右子树为单个节点的情况
            _Huffman_WriteBits(bitstream, 1, 1);
            _Huffman_WriteBits(bitstream, symbols[end].symbol, 8);
            symbols[end].code = code;
            symbols[end].code_length = depth;
            return;
        }
    }

    _Huffman_WriteBits(bitstream, 0, 1);
}

// 恢复Huffman树结构
void _Huffman_RecoverTree(huff_sym_t* symbols, huff_bitstream_t* bitstream,
    unsigned int code, unsigned int depth, unsigned int* count) {
    if (_Huffman_ReadBits(bitstream, 1)) {
        // 叶子节点
        symbols[*count].symbol = _Huffman_ReadBits(bitstream, 8);
        symbols[*count].code = code;
        symbols[*count].code_length = depth;
        (*count)++;
    }
    else {
        // 内部节点
        unsigned int next_code = code;
        unsigned int next_depth = depth + 1;

        while (true) {
            if (_Huffman_ReadBits(bitstream, 1)) {
                _Huffman_RecoverTree(symbols, bitstream, 2 * next_code, next_depth, count);
            }

            if (!_Huffman_ReadBits(bitstream, 1)) break;

            depth++;
            next_depth++;
            next_code = 2 * next_code + 1;

            if (_Huffman_ReadBits(bitstream, 1)) {
                symbols[*count].symbol = _Huffman_ReadBits(bitstream, 8);
                symbols[*count].code = next_code;
                symbols[*count].code_length = depth;
                (*count)++;
                return;
            }
        }
    }
}

// Huffman压缩主函数
int Huffman_Compress(uint8_t* input, uint8_t* output, unsigned int input_size) {
    if (input_size == 0) return 0;

    // 初始化位流 (预留384字节用于树结构)
    huff_bitstream_t bitstream;
    _Huffman_InitBitstream(&bitstream, output, input_size + 384);

    // 符号数组，包含频率和编码信息
    huff_sym_t symbols[256];

    // 统计频率并排序
    _Huffman_Hist(input, symbols, input_size);

    // 构建并写入Huffman树
    _Huffman_MakeTree(symbols, &bitstream, 0, 0, 0, 255);

    // 检查编码长度是否合理 (不超过32位)
    int max_attempts = 0;
    for (int i = 0; i < 256; i++) {
        if (symbols[i].code_length <= 32) {
            max_attempts++;
            if (max_attempts >= 255) break;
        }
    }

    if (max_attempts >= 255) {
        // 重新排序以确保编码效率
        bool swapped;
        do {
            swapped = false;
            for (int i = 0; i < 255; i++) {
                if (symbols[i].code_length > symbols[i + 1].code_length) {
                    huff_sym_t temp = symbols[i];
                    symbols[i] = symbols[i + 1];
                    symbols[i + 1] = temp;
                    swapped = true;
                }
            }
        } while (swapped);

        // 写入压缩数据
        for (unsigned int i = 0; i < input_size; i++) {
            uint8_t symbol = input[i];
            _Huffman_WriteBits(&bitstream, symbols[symbol].code, symbols[symbol].code_length);
        }

        // 计算输出大小
        int output_size = (int)(bitstream.data - output);
        if (bitstream.bit_pos > 0) output_size++;

        return output_size;
    }

    return 0;
}

// Huffman解压缩主函数
void Huffman_Uncompress(uint8_t* input, uint8_t* output,
    unsigned int input_size, unsigned int output_size) {
    if (input_size == 0) return;

    // 初始化位流
    huff_bitstream_t bitstream;
    _Huffman_InitBitstream(&bitstream, input, input_size);

    // 初始化符号数组
    huff_sym_t symbols[256];
    for (int i = 0; i < 256; i++) {
        symbols[i].code_length = 0x7FFFFFFF;  // 最大值标记未使用
    }

    // 恢复Huffman树
    unsigned int symbol_count = 0;
    _Huffman_RecoverTree(symbols, &bitstream, 0, 0, &symbol_count);

    // 按编码长度排序
    bool swapped;
    do {
        swapped = false;
        for (unsigned int i = 0; i < symbol_count - 1; i++) {
            if (symbols[i].code_length > symbols[i + 1].code_length) {
                huff_sym_t temp = symbols[i];
                symbols[i] = symbols[i + 1];
                symbols[i + 1] = temp;
                swapped = true;
            }
        }
    } while (swapped && symbol_count > 1);

    // 解压缩数据
    for (unsigned int output_pos = 0; output_pos < output_size; output_pos++) {
        unsigned int accumulated_code = 0;
        int accumulated_bits = 0;
        unsigned int symbol_index = 0;

        if (symbol_count > 0) {
            for (unsigned int i = 0; i < symbol_count; i++) {
                int required_bits = symbols[i].code_length;
                int additional_bits = required_bits - accumulated_bits;

                if (additional_bits > 0) {
                    unsigned int new_bits = _Huffman_ReadBits(&bitstream, additional_bits);
                    accumulated_code |= new_bits << (32 - additional_bits - accumulated_bits);
                    accumulated_bits = required_bits;
                }

                if (accumulated_code == (symbols[i].code << (32 - required_bits))) {
                    output[output_pos] = symbols[i].symbol;
                    symbol_index = i;
                    break;
                }

                symbol_index++;
            }
        }
    }
}