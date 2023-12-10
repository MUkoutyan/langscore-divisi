#pragma once

#include "platform_base.h"
#include "reader/jsonreader.hpp"

#ifdef ENABLE_TEST
#include "iutest.hpp"
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_WriteScriptCSV);
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, ValidateLangscoreCustom);
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_Validate);
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_FindEscChar);
#endif

namespace langscore
{
	class divisi_vxace: public platform_base
	{

#ifdef ENABLE_TEST
		IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_WriteScriptCSV);
		IUTEST_FRIEND_TEST(Langscore_Divisi, ValidateLangscoreCustom);
		IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_Validate);
		IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_FindEscChar);
#endif
	public:
		divisi_vxace();
		~divisi_vxace() override;

		void setProjectPath(std::filesystem::path path) override;
		ErrorStatus analyze() override;
		ErrorStatus update() override;
		ErrorStatus write() override;
		ErrorStatus validate() override;
		ErrorStatus packing() override;

	private:
		std::tuple<utility::filelist, utility::filelist, utility::filelist> fetchFilePathList(std::u8string searchDir);

		void writeAnalyzedBasicData();
		void writeAnalyzedRvScript(std::u8string baseDirectory);

		void writeFixedBasicData();
		void writeFixedRvScript();
		void rewriteScriptList(bool& replaceScript);

		void fetchActorTextFromMap(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list, const std::unordered_map<std::filesystem::path, std::unique_ptr<readerbase>>&);
		void adjustCSV(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list);
		bool adjustCSVCore(std::vector<utility::u8stringlist>& plain_csv, bool isRNLine);

		utility::u8stringlist formatSystemVariable(std::filesystem::path path);

	};
}