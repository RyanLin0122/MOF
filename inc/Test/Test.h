#pragma once
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <d3d9.h>
#include "Test/nfs_test.h"
#include "Test/cm_packing_integration_test.h"
#include "Test/CompTest.h"
#include "Test/ImageTest.h"
#include "Test/ImageDrawTest.h"
#include "Test/EffectSystemTest.h"
#include "Test/FontSystemTest.h"
#include "FileSystem/CMOFPacking.h"
#include "Sound/COgg.h"
#include "Text/cltTextFileManager.h"
#include "global.h"

int Run_Test();

void text_manager_test();
void dump_all_text_files();

void ogg_play_test();

int InitTest();
void UpdateTest(float fElapsedTime);
void Render_Test();
void CleanupTest();