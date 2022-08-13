#pragma once
#include "../invoker.h"
#include "../include/config.h"
#include "../utility.hpp"
#include "../writer/writerbase.h"

namespace langscore
{
	class platform_base
	{
	public:
		platform_base() :appPath("") {}
		virtual ~platform_base() {}

		void setAppPath(std::filesystem::path path){
			appPath = path;
			this->invoker.setApplicationFolder(std::move(path));
		}
		virtual void setProjectPath(std::filesystem::path path) = 0;
		virtual bool analyze() = 0;
		virtual bool write() = 0;

		//virtual void copyData(langscore::OverwriteTextMode option = langscore::OverwriteTextMode::LeaveOld) = 0;

	protected:
		std::filesystem::path appPath;
		invoker invoker;
		utility::u8stringlist supportLangs;
		std::u8string defaultLanguage;

		std::vector<std::filesystem::path> dataFileList;
		std::vector<std::filesystem::path> scriptFileList;
		std::vector<std::filesystem::path> graphicFileList;

		std::filesystem::copy_options convertCopyOption(OverwriteTextMode mode);
		virtual std::filesystem::path exportFolderPath(std::filesystem::path fileName, std::filesystem::path dir = "") = 0;

		template<typename Writer, typename TsData>
		void writeAnalyzeTranslateText(std::filesystem::path path, TsData texts, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld, bool isDebug = true)
		{
			//最終的な出力先にCSVが存在するか
			Writer writer(supportLangs, std::move(texts));
			writer.write(path, overwriteMode);
		}

		template<typename Writer, typename TsData>
		void writeFixedTranslateText(std::filesystem::path path, TsData texts, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld)
		{
			//最終的な出力先にCSVが存在するか
			Writer writer(supportLangs, std::move(texts));
			writer.setOverwriteMode(overwriteMode);
			//既に編集済みのCSVがある場合はマージを行う。
			if(overwriteMode == OverwriteTextMode::LeaveOld || overwriteMode == OverwriteTextMode::LeaveOldNonBlank)
			{
				const auto csvFileInProject = exportFolderPath(path.filename());
				if(std::filesystem::exists(csvFileInProject)){
					if(writer.merge(csvFileInProject) == false){
						return;
					}
				}
			}

			writer.write(path, overwriteMode);
		}
	};
}