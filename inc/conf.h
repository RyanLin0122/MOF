enum TestMode {
	TEST_IMAGE,
	TEST_EFFECT,
	TEST_FONT,
	UI_BASIC
};

#define CURRENT_MODE (TEST_MODE)
#define TEST_MODE	(0)
#define GAME_MODE	(1)
#define PACK_MODE   (2)

// 設定要執行的測試
const TestMode G_CURRENT_TEST = UI_BASIC;