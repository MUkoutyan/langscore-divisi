#include "test.h"
#include "test_base.cpp"
#include "test_writer.cpp"
#include "test_vxace.cpp"
#include "test_mvmz.cpp"
#include "test_validation.cpp"
#include "test_core.cpp"
#include "test_known_issues.cpp"

#include <gtest/gtest.h>


int main(int argc, char** argv)
{
    // GoogleTestの初期化
    ::testing::InitGoogleTest(&argc, argv);

    // Visual Studio のテストエクスプローラー等は --gtest_list_tests の標準出力を解析して
    // テストを列挙するため、一覧表示のときは余計な出力や前処理を挟まない。
    if(::testing::GTEST_FLAG(list_tests)) {
        return RUN_ALL_TESTS();
    }

	std::cout << TEST_DATA_SRC << std::endl;
	if(fs::exists(std::string(BINARY_DIRECTORY) + "/data") == false) 
    {
        auto destPath = fs::path(std::string(BINARY_DIRECTORY) + "/data");
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