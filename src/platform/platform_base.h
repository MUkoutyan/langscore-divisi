#pragma once
#include "../project_deserializer.h"
#include "../../utility.hpp"
#include "../writer/writerbase.h"

namespace langscore
{
	class platform_base
	{
	public:
		platform_base(utility::u8stringlist supportLangs)
			: supportLangs(std::move(supportLangs)), defaultLanguage(this->supportLangs[0]) {}
		virtual ~platform_base() {}

		void setAppPath(std::filesystem::path path){
			appPath = path;
			this->deserializer.setApplicationFolder(std::move(path));
		}
		virtual void setProjectPath(std::filesystem::path path) = 0;
		virtual void prepareAnalyzeProject() = 0;

		virtual void convertGraphFileNameData() = 0;
		virtual void copyData(std::filesystem::copy_options option) = 0;
		virtual void convert(std::filesystem::copy_options option = std::filesystem::copy_options::none) = 0;

		virtual void setIgnoreScriptPath(utility::filelist ignoreScriptPath) = 0;
	protected:
		std::filesystem::path appPath;
		deserializer deserializer;
		utility::filelist ignoreScriptPath;
		utility::u8stringlist supportLangs;
		std::u8string defaultLanguage;

		std::vector<std::filesystem::path> dataFileList;
		std::vector<std::filesystem::path> scriptFileList;
		std::vector<std::filesystem::path> graphicFileList;

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