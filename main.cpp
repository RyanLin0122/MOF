#include <iostream>
#include "nfs_test.h"
#include "nfs_integration_test.h"
int main() {
	std::cout << "Starting Virtual File System Tests..." << std::endl;
	std::cout << "========================================" << std::endl;

	// 執行所有測試
	run_all_tests();
	return print_test_result();
	/*
	test_create_write_read_small();
	test_cross_block_write_read();
	test_multi_channel_isolation();
	test_channel_truncate();
	printf("ALL INTEGRATION TESTS PASSED\n");
	*/
	return 0;
}