enum TestMode {
	TEST_IMAGE,
	TEST_EFFECT,
	TEST_FONT,
	UI_BASIC
};

#define CURRENT_MODE         (MINIGAME_DEBUG_MODE)
#define TEST_MODE            (0)
#define GAME_MODE            (1)
#define PACK_MODE            (2)
#define MINIGAME_DEBUG_MODE  (3)

// 設定要執行的測試
const TestMode G_CURRENT_TEST = TEST_IMAGE;