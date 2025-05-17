#include "test.h"
#include "test_base.cpp"
#include "test_writer.cpp"
#include "test_vxace.cpp"
#include "test_mvmz.cpp"
#include "test_validation.cpp"

#include <gtest/gtest.h>


int main(int argc, char** argv)
{
	std::cout << TEST_DATA_SRC << std::endl;
	if(fs::exists(std::string(BINARYT_DIRECTORY) + "/data") == false) 
    {
        auto destPath = fs::path(std::string(BINARYT_DIRECTORY) + "/data");
        try {
            std::cout << "Copying files..." << std::endl;
            fs::copy(TEST_DATA_SRC, destPath, fs::copy_options::recursive);
            std::cout << "done." << std::endl;
        }
        catch(const fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
            return -1;
        }
	}

	langscore::config::attachConfigFile(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");
	
    // GoogleTestの初期化
    ::testing::InitGoogleTest(&argc, argv);

    // Google Test の出力を確認
    std::cout << "Running tests with arguments: ";
    for(int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    // テストの実行
    int result = RUN_ALL_TESTS();
	ClearGenerateFiles();
	return result;
}