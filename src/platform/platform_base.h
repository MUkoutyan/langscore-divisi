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
		virtual void analyze() = 0;
		virtual void write() = 0;

		virtual void convertGraphFileNameData() = 0;
		virtual void copyDataToTemp() = 0;
		virtual void copyData(langscore::OverwriteTextMode option = langscore::OverwriteTextMode::LeaveOld) = 0;

	protected:
		std::filesystem::path appPath;
		invoker invoker;
		utility::u8stringlist supportLangs;
		std::u8string defaultLanguage;

		std::vector<std::filesystem::path> dataFileList;
		std::vector<std::filesystem::path> scriptFileList;
		std::vector<std::filesystem::path> graphicFileList;

		std::filesystem::copy_options convertCopyOption(OverwriteTextMode mode);
		virtual std::filesystem::path outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir = "") = 0;

		template<typename Writer, typename TsData>
		void writeTranslateText(std::filesystem::path path, TsData texts, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld, bool isDebug = true)
		{
			//最終的な出力先にCSVが存在するか
			Writer writer(supportLangs, std::move(texts));
			writer.setOverwriteMode(overwriteMode);
			writer.isDebug = isDebug;
			const auto csvFileInProject = outputProjectDataPath(path.filename());
			if(std::filesystem::exists(csvFileInProject)){
				if(writer.merge(csvFileInProject) == false){
					return;
				}
			}

			writer.write(path, overwriteMode);
		}
	};
}