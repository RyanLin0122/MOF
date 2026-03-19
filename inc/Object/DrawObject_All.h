#pragma once

// 比較函式 — 用於 qsort 按 Y 座標排序各類繪製物件
int compareCharYpos(const void* a1, const void* a2);
int compareObjectYpos(const void* a1, const void* a2);
int compareItemYpos(const void* a1, const void* a2);
int compareMatchFlagYpos(const void* a1, const void* a2);
int comparePKPlagYpos(const void* a1, const void* a2);

// DrawObject_All — 場景繪製管理器
// 負責將所有角色、物件、道具、旗幟等按 Y 座標排序後繪製
class DrawObject_All {
public:
    DrawObject_All();
    ~DrawObject_All();

    // 準備繪製：更新所有角色、物件、道具等的繪製資料
    void PrepareDrawing();

    // 繪製：按 Y 座標排序的多路合併繪製
    void DrawObject();
};

extern DrawObject_All g_DrawObject_ALL;
