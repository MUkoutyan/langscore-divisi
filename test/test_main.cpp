#pragma once
#include "test.h"

#include "test_base.cpp"
#include "test_writer.cpp"
#include "test_vxace.cpp"
#include "test_mvmz.cpp"


int main(int argc, char** argv)
{

	std::cout << TEST_DATA_SRC << std::endl;
	if(fs::exists(std::string(BINARYT_DIRECTORY) + "/data")) {
		fs::remove_all(std::string(BINARYT_DIRECTORY) + "/data");
	}
	auto destPath = fs::path(std::string(BINARYT_DIRECTORY) + "/data");
	try {
		fs::copy(TEST_DATA_SRC, destPath, fs::copy_options::recursive);
	}
	catch(const fs::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	langscore::config::attachConfigFile(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");
	
	IUTEST_INIT(&argc, argv);
	auto result = IUTEST_RUN_ALL_TESTS();
	ClearGenerateFiles();
	return result;
}