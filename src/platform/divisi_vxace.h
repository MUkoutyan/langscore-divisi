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
		void writeFixedGraphFileNameData();
		void rewriteScriptList(bool& replaceScript);

		void fetchActorTextFromMap(const utility::filelist& list, const utility::u8stringlist& csvFolder, const std::unordered_map<std::filesystem::path, std::unique_ptr<jsonreaderbase>>&);

		bool validateTranslateFileList(utility::filelist csvPathList) const;
		bool validateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const;
		std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> findEscChars(std::u8string text) const;

		utility::u8stringlist formatSystemVariable(std::filesystem::path path);
		
		std::filesystem::path exportFolderPath(std::filesystem::path fileName) override;

	};
}