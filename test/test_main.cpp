#include <iutest.hpp>
#include "config.h"

using namespace std::string_literals;

IUTEST(Langscore_Config, LoadConfig)
{
	langscore::config config;

	utility::stringlist actual = {"ja", "en", "zh-sc"};
	auto expected = config.languages();
	for(int i = 0; i < actual.size(); ++i){
		IUTEST_ASSERT_EQ(expected[i], actual[i]);
	}
}

int main(int argc, char** argv)
{
	IUTEST_INIT(&argc, argv);
	return IUTEST_RUN_ALL_TESTS();
}