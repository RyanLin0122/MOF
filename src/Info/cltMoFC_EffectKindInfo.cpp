#include "Info/cltMoFC_EffectKindInfo.h"

// 如需在本檔直接提供全域，也可取消以下註解：
// cltTextFileManager g_clTextFileManager;

cltMoFC_EffectKindInfo::cltMoFC_EffectKindInfo() {
    // 與反編譯碼：memset((char*)this + 4, 0, 0x3FFFC) 行為等價
    std::memset(table_, 0, sizeof(table_));
}

cltMoFC_EffectKindInfo::~cltMoFC_EffectKindInfo() {
    // 反編譯碼僅寫回 vftable，不釋放配置的節點（因此這裡也不釋放，維持行為一致）
    // 若需避免外洩，請自行擴充清理程式，但這會偏離原行為。
}

static inline bool iequals(const char* a, const char* b) {
#if defined(_WIN32)
    return _stricmp(a, b) == 0;
#else
    return strcasecmp(a, b) == 0;
#endif
}

int cltMoFC_EffectKindInfo::Initialize(char* fileName) {
    // 與反編譯碼一致的區域變數/流程控制
    FILE* stream = g_clTextFileManager.fopen(fileName);
    int resultFlag = 0; // v19，只有在「讀到 EOF」的路徑才設為 1
    if (!stream) {
        return 0;
    }

    // 與反編譯碼相同：先讀 3 行（通常是註解/標頭），再嘗試讀第 4 行開始資料
    char line[1024];
    if (std::fgets(line, 1023, stream) &&
        std::fgets(line, 1023, stream) &&
        std::fgets(line, 1023, stream)) {

        if (std::fgets(line, 1023, stream)) {
            const char* DELIM = "\t\n"; // 分隔符：Tab + 換行（與原碼一致）

            while (true) {
                // 逐列解析（欄位對照：韓文 → 中文）
                // 1) "스킬 이펙트 ID"（技能特效ID），如 "E0001"
                char* tok = std::strtok(line, DELIM);
                if (!tok) break;

                uint16_t kind = TranslateKindCode(tok);
                if (kind == 0) break; // 不合法則結束整體讀取（與原始行為一致）

                // 查是否重複。原碼若遇到已存在的索引，會直接 break（整體中止）
                if (kind < TABLE_SIZE && table_[kind] != nullptr) {
                    break;
                }

                // 建立節點（照 0x84 bytes 清零）
                stEffectKindInfo* info = (stEffectKindInfo*)::operator new(sizeof(stEffectKindInfo));
                std::memset(info, 0, sizeof(stEffectKindInfo));
                info->kindCode = kind;

                // 2) "스킬명"（技能名）—原始程式碼有取 token 但不存（僅檢查存在）
                if (!std::strtok(nullptr, DELIM)) {
                    // 欄位缺失 → 中止整體（與原始行為一致）
                    break;
                }

                // 3) "EA 파일명"（EA 檔案名）—存入 info->eaFile
                char* ea = std::strtok(nullptr, DELIM);
                if (!ea) {
                    break;
                }
                std::strncpy(info->eaFile, ea, sizeof(info->eaFile) - 1);
                info->eaFile[sizeof(info->eaFile) - 1] = '\0';

                // 4) "스킬 타입"（技能類型）—轉換為 1..6
                char* typ = std::strtok(nullptr, DELIM);
                if (!typ) {
                    break;
                }
                if (iequals(typ, "ONCE"))       info->skillType = 1;
                else if (iequals(typ, "DIRECT"))    info->skillType = 2;
                else if (iequals(typ, "SUSTAIN"))   info->skillType = 3;
                else if (iequals(typ, "SHOOTUNIT")) info->skillType = 4;
                else if (iequals(typ, "SHOOTNOTEA"))info->skillType = 5;
                else if (iequals(typ, "ITEMONCE"))  info->skillType = 6;
                // 原始碼未處理其他字串（保留為 0）

                // 5) "이펙트 단계"（特效階段）—原始程式碼僅檢查 token 存在，不保存
                if (!std::strtok(nullptr, DELIM)) {
                    // 缺欄 → 整體中止（與原行為一致）
                    ::operator delete(info);
                    break;
                }

                // 寫入表
                table_[kind] = info;

                // 讀下一行；若失敗（EOF），依原始碼將回傳值設為 1
                if (!std::fgets(line, 1023, stream)) {
                    resultFlag = 1; // v19 = 1（完成到 EOF）
                    break;
                }
            }
        }
        else {
            // 連第 4 行都沒有 → 視為完成（v19 = 1）
            resultFlag = 1;
        }
    }

    g_clTextFileManager.fclose(stream);
    return resultFlag;
}

stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(uint16_t code) {
    if (code < TABLE_SIZE) {
        return table_[code];
    }
    return nullptr;
}

stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(char* codeStr) {
    uint16_t k = TranslateKindCode(codeStr);
    return GetEffectKindInfo(k);
}

uint16_t cltMoFC_EffectKindInfo::TranslateKindCode(char* s) {
    // 反編譯碼條件：長度必須 == 5
    if (!s || std::strlen(s) != 5) return 0;

    // v3 = (toupper(s[0]) + 31) << 11 (16-bit 截斷)
    int c0 = std::toupper(static_cast<unsigned char>(s[0]));
    int group = (c0 + 31) << 11;

    // 後四碼數字，必須 < 0x800
    int num = std::atoi(s + 1);
    if (num >= 0x800) return 0;

    uint16_t result = static_cast<uint16_t>(group | num);
    return result;
}