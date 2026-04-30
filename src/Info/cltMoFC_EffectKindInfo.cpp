#include "Info/cltMoFC_EffectKindInfo.h"

// ---------------------------------------------------------------------------
// 與 mofclient.c (反組譯之 ground truth) 完全等價的還原實作。
// 主要對照位置：
//   cltMoFC_EffectKindInfo::cltMoFC_EffectKindInfo  mofclient.c:259539
//   cltMoFC_EffectKindInfo::~cltMoFC_EffectKindInfo mofclient.c:259557
//   cltMoFC_EffectKindInfo::Initialize              mofclient.c:259564
//   cltMoFC_EffectKindInfo::GetEffectKindInfo (u16) mofclient.c:259682
//   cltMoFC_EffectKindInfo::GetEffectKindInfo (str) mofclient.c:259688
//   cltMoFC_EffectKindInfo::TranslateKindCode       mofclient.c:259697
// ---------------------------------------------------------------------------

cltMoFC_EffectKindInfo::cltMoFC_EffectKindInfo() {
    // GT: memset((char*)this + 4, 0, 0x3FFFC) — 將整張 table 指標清零。
    // 32→64 bit 後 table_ 的 byte 大小不同，但邏輯（全部 nullptr）等價。
    std::memset(table_, 0, sizeof(table_));
}

cltMoFC_EffectKindInfo::~cltMoFC_EffectKindInfo() {
    // GT 僅寫回 vftable，未釋放 table_ 內任何節點（mofclient.c:259559）。
    // 此處刻意不釋放，維持相同記憶體生命週期。
}

int cltMoFC_EffectKindInfo::Initialize(char* fileName) {
    // 與 GT 對齊的局部變數
    char Delimiter[4] = { '\t', '\n', '\0', '\0' };  // GT: strcpy(Delimiter,"\t\n");
    int  v19 = 0;                                    // 回傳值
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    FILE* stream = g_clTextFileManager.fopen(fileName);
    if (!stream) {
        return 0;
    }

    // GT: 先讀 3 行（標頭/欄位名稱），三次任一失敗則整個 if 不進入，
    //     最終 v19 = 0、執行 fclose 後返回 0。
    if (std::fgets(Buffer, 1023, stream)
        && std::fgets(Buffer, 1023, stream)
        && std::fgets(Buffer, 1023, stream))
    {
        // GT: 嘗試讀第 4 行（首列資料）。
        if (std::fgets(Buffer, 1023, stream)) {
            for (;;) {
                // [1] 스킬 이펙트 ID（技能特效ID） — kindCode @ +0
                char* v5 = std::strtok(Buffer, Delimiter);
                if (!v5) break;

                uint16_t v6 = TranslateKindCode(v5);
                if (!v6) break;       // 不合法種別碼 → 中止整體（v19=0）

                // GT: `v8 = *((_DWORD*)v2 + v6 + 1); if (v8) break;`
                //     重複種別碼 → 中止整體，並保留原項目。
                if (table_[v6] != nullptr) break;

                // GT 順序：先 operator new、再寫入 table_[v6]、再 memset、
                //          最後寫入 kindCode。後續任何 strtok 失敗都不會
                //          delete 該節點（mofclient.c:259619-259663），
                //          換言之 table_[v6] 即使欄位殘缺也已經指向該節點。
                stEffectKindInfo* node = static_cast<stEffectKindInfo*>(
                    ::operator new(sizeof(stEffectKindInfo)));
                table_[v6] = node;
                std::memset(node, 0, sizeof(stEffectKindInfo));
                node->kindCode = v6;

                // [2] 스킬명（技能名稱） — GT 僅檢查 token 存在，不寫入。
                if (!std::strtok(nullptr, Delimiter)) break;

                // [3] EA 파일명（EA 檔案名稱） — eaFile @ +2 (strcpy)
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                std::strcpy(node->eaFile, v12);  // GT: strcpy(v11+2, v12)

                // [4] 스킬 타입（技能類型） — skillType @ +130
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                if      (!_stricmp(v13, "ONCE"))       node->skillType = 1;
                else if (!_stricmp(v13, "DIRECT"))     node->skillType = 2;
                else if (!_stricmp(v13, "SUSTAIN"))    node->skillType = 3;
                else if (!_stricmp(v13, "SHOOTUNIT"))  node->skillType = 4;
                else if (!_stricmp(v13, "SHOOTNOTEA")) node->skillType = 5;
                else if (!_stricmp(v13, "ITEMONCE"))   node->skillType = 6;
                // 其他字串：GT 保持 skillType = 0 並繼續解析。

                // [5] 이펙트 단계（特效階段） — GT 僅檢查 token 存在，不寫入。
                if (!std::strtok(nullptr, Delimiter)) break;

                // GT: 讀下一行；失敗則 v19 = 1（goto LABEL_28），成功則繼續。
                if (!std::fgets(Buffer, 1023, stream)) {
                    v19 = 1;
                    break;
                }
            }
        }
        else {
            // GT: 沒有任何資料行（4 次 fgets 失敗）→ v19 = 1（LABEL_28 路徑）
            v19 = 1;
        }
    }

    g_clTextFileManager.fclose(stream);
    return v19;
}

stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(uint16_t code) {
    // GT (mofclient.c:259684): return *((_DWORD *)this + code + 1)
    //   直接偏移讀取，無任何邊界檢查；code == 0xFFFF 時 GT 會讀到
    //   table 末端後一槽（UB，但實際路徑由 TranslateKindCode 限制
    //   最大值為 0xCFFF，故不會發生）。此處保持與 GT 完全一致。
    return table_[code];
}

stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(char* codeStr) {
    // GT: TranslateKindCode → GetEffectKindInfo(uint16_t)
    return GetEffectKindInfo(TranslateKindCode(codeStr));
}

uint16_t cltMoFC_EffectKindInfo::TranslateKindCode(char* codeStr) {
    // GT (mofclient.c:259697-259712)：
    //   if (strlen(a2) != 5) return 0;
    //   v3 = (_toupper(*a2) + 31) << 11;     // 32-bit int 計算
    //   v4 = (uint16_t)_atoi(a2 + 1);        // 結果先截為 uint16
    //   if (v4 < 0x800u) result = (uint16_t)(v3 | v4); else result = 0;
    //   return result;                        // 回傳型別 unsigned __int16
    if (!codeStr || std::strlen(codeStr) != 5) return 0;

    const int v3 = (std::toupper(static_cast<unsigned char>(codeStr[0])) + 31) << 11;
    const uint16_t v4 = static_cast<uint16_t>(std::atoi(codeStr + 1));
    if (v4 < 0x800u) {
        return static_cast<uint16_t>(v3 | v4);   // 高位透過 uint16 cast 截斷
    }
    return 0;
}
