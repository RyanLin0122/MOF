#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "cltCharKindInfo.h"
#include "cltNPCInfo.h"

// =====================================================================
//  cltQuestionKindInfo — 시험 문제 종류 정보 (考試問題種類資訊)
//  反編譯來源：mofclient.c:325108-325294 (cltQuestionKindInfo::Initialize、
//             Free、GetQuestionKindInfo、GetQuestions、IsAnswer_*、
//             TranslateKindCode)
//  資料來源檔：question.txt（EUC-KR）
//  每筆紀錄固定 12 bytes（GT: `operator new(12 * count)`，mofclient.c:325144）
// =====================================================================
//
//  question.txt 一列七欄（以 \t 分隔；strtok 之分隔字元為 "\t\n"）：
//    [0] 문제 ID         問題 ID（"M0071" 或 "Q0024"）
//                         → cltQuestionKindInfo::TranslateKindCode → +0  (WORD)
//    [1] 답(기획용)      答案（企劃用韓文文字；解析時讀取後直接捨棄）
//    [2] 학년             學年（如 20 / 19 / …）        → atoi → +2  (BYTE)
//                                                          +3  (BYTE，保留欄；memset 為 0)
//    [3] 시험문제 코드   試題碼（題目顯示文字代碼）  → atoi → +4  (WORD)
//    [4] 축약 설명        縮約說明（題目縮寫文字代碼）→ atoi → +6  (WORD)
//    [5] 답_몬스터       答_怪物（"J0758" 或 "0"）
//                         → cltCharKindInfo::TranslateKindCode → +8  (WORD)
//    [6] 답_NPC          答_NPC（"N0234" 或 "0"）
//                         → cltNPCInfo::TranslateKindCode → +10 (WORD)
//
#pragma pack(push, 1)
struct strQuestionKindInfo {
    uint16_t qKind;          // +0  問題 ID（문제 ID 轉碼後的 16-bit kind）
    uint8_t  grade;          // +2  學年（학년）
    uint8_t  _pad;           // +3  保留欄（GT 從不寫入，memset 後恆為 0）
    uint16_t examCode;       // +4  試題碼（시험문제 코드）
    uint16_t shortDesc;      // +6  縮約說明（축약 설명，數字代碼）
    uint16_t ansMonsterKind; // +8  答_怪物（답_몬스터，Jxxxx 轉碼；"0"→0）
    uint16_t ansNpcKind;     // +10 答_NPC （답_NPC，Nxxxx 轉碼；"0"→0）
};                            // sizeof == 12
#pragma pack(pop)
static_assert(sizeof(strQuestionKindInfo) == 12, "strQuestionKindInfo must be 12 bytes");

class cltQuestionKindInfo {
public:
    cltQuestionKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltQuestionKindInfo() { Free(); }

    // 對應：int __thiscall Initialize(char* filename)
    //  GT: mofclient.c:325108
    int  Initialize(char* filename);

    // 對應：void __thiscall Free()
    //  GT: mofclient.c:325196
    void Free();

    // 對應：strQuestionKindInfo* __thiscall GetQuestionKindInfo(uint16_t code)
    //  GT: mofclient.c:325207；步距 6 個 WORD（= 12 bytes）線性掃描。
    strQuestionKindInfo* GetQuestionKindInfo(uint16_t code);

    // 對應：int __thiscall GetQuestions(char grade, uint16_t* outCodes)
    //  GT: mofclient.c:325228；回傳符合學年的題數，並把 qKind 依序寫入 outCodes。
    int  GetQuestions(char grade, uint16_t* outCodes);

    // 對應：strQuestionKindInfo* __thiscall IsAnswer_MonsterKind(uint16_t qKindCode)
    //  GT: mofclient.c:325257；命中題目時回傳 (ansMonsterKind != 0) 的「布林位址」。
    bool IsAnswer_MonsterKind(uint16_t qKindCode);

    // 對應：strQuestionKindInfo* __thiscall IsAnswer_NpcKind(uint16_t qKindCode)
    //  GT: mofclient.c:325268；同上但檢查 ansNpcKind。
    bool IsAnswer_NpcKind(uint16_t qKindCode);

    // 對應：static uint16_t __cdecl TranslateKindCode(char* s)
    //  GT: mofclient.c:325279；需 strlen(s)==5；((toupper(s[0])+31)<<11)|atoi(s+1)；
    //  atoi 結果 ≥ 0x800 時回 0。
    static uint16_t TranslateKindCode(char* s);

    inline const strQuestionKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strQuestionKindInfo* m_items;  // +0 (x86 layout)
    int                  m_count;  // +4 (x86 layout)
};
