#pragma once
#include "test.h"

#include "test_base.cpp"
#include "test_writer.cpp"
#include "test_vxace.cpp"
#include "test_mvmz.cpp"


int main(int argc, char** argv)
{
	langscore::config::attachConfigFile(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	IUTEST_INIT(&argc, argv);
	auto result = IUTEST_RUN_ALL_TESTS();
	ClearGenerateFiles();
	return result;
}