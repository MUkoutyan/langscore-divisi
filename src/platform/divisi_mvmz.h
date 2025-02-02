#pragma once

#include "platform_base.h"

namespace langscore
{
	class divisi_mvmz: public platform_base
	{
	public:
		divisi_mvmz();
		~divisi_mvmz() override;

		void setProjectPath(std::filesystem::path path) override;
		ErrorStatus analyze() override;
		ErrorStatus reanalysis() override;
        ErrorStatus updatePlugin() override;
		ErrorStatus exportCSV() override;
		ErrorStatus validate() override;
		ErrorStatus packing() override;

	private:
		std::tuple<utility::filelist, utility::filelist, utility::filelist> fetchFilePathList(std::u8string searchDir);

		void writeAnalyzedBasicData();
		void writeAnalyzedScript(std::u8string baseDirectory);

		void writeFixedBasicData();
		void writeFixedScript();

		void fetchActorTextFromMap(const std::u8string& rewriteCSVFolder, const utility::filelist& list, const std::unordered_map<std::filesystem::path, std::unique_ptr<readerbase>>&);

		void adjustCSV(const std::u8string& rewriteCSVFolder, const utility::filelist& list);
		bool adjustCSVCore(std::vector<utility::u8stringlist>& plain_csv);

        void writeLangscorePlugin(bool replaceLs);
		void updatePluginInfo();
		utility::u8stringlist formatSystemVariable(std::filesystem::path path);

        std::filesystem::path getGameProjectFontDirectory() const override;

	};
}