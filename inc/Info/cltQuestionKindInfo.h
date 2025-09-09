#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "cltCharKindInfo.h"
#include "cltNPCInfo.h"

// 依反編譯還原：每筆資料恰為 12 bytes
// 版面配置（offset / 型別 / 說明）對應 question.txt：
//  0  WORD  問題Kind（由 "M0071" 或 "Q0024" 透過 TranslateKindCode 轉為 16-bit）
//  2  BYTE  學年（學年；第3欄，值如 20/19/...）
//  3  BYTE  （保留/補齊，未使用；以 0 置入以對齊）
//  4  WORD  試題碼（"시험문제 코드"）
//  6  WORD  縮約說明（"축약 설명"；此處是數字代碼）
//  8  WORD  答_怪物（字串如 "J0758" -> 轉碼；若為 "0" 則為 0）
// 10  WORD  答_NPC  （字串如 "N0234" -> 轉碼；若為 "0" 則為 0）
#pragma pack(push, 1)
struct strQuestionKindInfo {
	uint16_t qKind;          // 問題ID轉碼（Mxxxx/Qxxxx）
	uint8_t  grade;          // 學年
	uint8_t  _pad;           // 對齊用
	uint16_t examCode;       // 試題碼
	uint16_t shortDesc;      // 縮約說明（數字代碼）
	uint16_t ansMonsterKind; // 答_怪物（Jxxxx -> 轉碼；"0" 則為 0）
	uint16_t ansNpcKind;     // 答_NPC（Nxxxx -> 轉碼；"0" 則為 0）
};
#pragma pack(pop)

class cltQuestionKindInfo {
public:
	cltQuestionKindInfo() : m_items(nullptr), m_count(0) {}
	~cltQuestionKindInfo() { Free(); }

	// 對應：int __thiscall Initialize(char* filename)
	int  Initialize(char* filename);

	// 對應：void __thiscall Free()
	void Free();

	// 對應：strQuestionKindInfo* __thiscall GetQuestionKindInfo(uint16_t code)
	strQuestionKindInfo* GetQuestionKindInfo(uint16_t code);

	// 對應：int __thiscall GetQuestions(char grade, uint16_t* outCodes)
	// 回傳筆數，並把符合 grade 的 qKind 依序寫入 outCodes
	int  GetQuestions(char grade, uint16_t* outCodes);

	// 更直白的布林版（對應 IDA 那兩個 IsAnswer_* 函式的語義）
	bool IsAnswer_MonsterKind(uint16_t qKindCode);
	bool IsAnswer_NpcKind(uint16_t qKindCode);

	// 對應：static uint16_t __cdecl TranslateKindCode(char* s)
	static uint16_t TranslateKindCode(char* s);

	// 便利
	inline const strQuestionKindInfo* data() const { return m_items; }
	inline int size() const { return m_count; }

private:
	strQuestionKindInfo* m_items;
	int                  m_count;
};
