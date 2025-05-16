#include <iostream>
#include <nfs_test.h>

int main() {
	std::cout << "Starting Virtual File System Tests..." << std::endl;
	std::cout << "========================================" << std::endl;

	// 執行所有測試
	run_all_tests();
	return print_test_result();
}