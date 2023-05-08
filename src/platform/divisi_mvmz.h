#pragma once

#include "platform_base.h"

#ifdef ENABLE_TEST
#include "iutest.hpp"
//class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_WriteScriptCSV);
//class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, ValidateLangscoreCustom);
//class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_Validate);
//class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_FindEscChar);
#endif

namespace langscore
{
	class divisi_mvmz: public platform_base
	{
	public:
		divisi_mvmz();
		~divisi_mvmz() override;

		void setProjectPath(std::filesystem::path path) override;
		ErrorStatus analyze() override;
		ErrorStatus update() override;
		ErrorStatus write() override;
		ErrorStatus validate() override;
		ErrorStatus packing() override;

	private:
		std::tuple<utility::filelist, utility::filelist, utility::filelist> fetchFilePathList(std::u8string searchDir);

		void writeAnalyzedBasicData();
		void writeAnalyzedScript(std::u8string baseDirectory);

		void writeFixedBasicData();
		void writeFixedScript();

		void fetchActorTextFromMap(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list, const std::unordered_map<std::filesystem::path, std::unique_ptr<readerbase>>&);

		void adjustCSV(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list);
		bool adjustCSVCore(std::vector<utility::u8stringlist>& plain_csv);


		std::filesystem::path exportFolderPath(std::filesystem::path fileName) override;

	};
}