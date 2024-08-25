#pragma once

#include "platform_base.h"
#include "reader/jsonreader.hpp"


namespace langscore
{
	class divisi_vxace: public platform_base
	{
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