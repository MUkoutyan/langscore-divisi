#pragma once
#include "../invoker.h"
#include "../include/config.h"
#include "../utility.hpp"
#include "../writer/writerbase.h"
#include "errorstatus.hpp"

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
		virtual ErrorStatus analyze() = 0;
		virtual ErrorStatus update() = 0;
		virtual ErrorStatus write() = 0;
		virtual ErrorStatus validate() = 0;
		virtual ErrorStatus packing() = 0;

		//virtual void copyData(langscore::MergeTextMode option = langscore::MergeTextMode::AcceptSource) = 0;

	protected:
		std::filesystem::path appPath;
		invoker invoker;
		utility::u8stringlist supportLangs;
		std::u8string defaultLanguage;

		utility::filelist basicDataFileList;
		utility::filelist scriptFileList;
		utility::filelist graphicFileList;

		std::filesystem::copy_options convertCopyOption(MergeTextMode mode);
		virtual std::filesystem::path exportFolderPath(std::filesystem::path fileName) = 0;

		void copyFonts();

		constexpr static char8_t Script_File_Name[] = u8"langscore";
		constexpr static char8_t Custom_Script_File_Name[] = u8"langscore_custom";
		static std::unordered_map<std::u8string, std::u8string> Help_Text;
		static std::unordered_map<std::u8string, std::u8string> Language_Items;

		utility::u8stringlist GetScriptFileName(config& config, utility::u8stringlist scriptNameList);


		template<class Writer>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, const std::unique_ptr<jsonreaderbase>& json, MergeTextMode overwriteMode = MergeTextMode::AcceptSource){
			return writeFixedTranslateText(Writer{supportLangs, json}, std::move(path), overwriteMode);
		}
		template<class Writer>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, std::vector<TranslateText> texts, MergeTextMode overwriteMode = MergeTextMode::AcceptSource){
			return writeFixedTranslateText(Writer{supportLangs, std::move(texts)}, std::move(path), overwriteMode);
		}

		template<class Writer>
		ErrorStatus writeFixedTranslateText(Writer writer, std::filesystem::path path, MergeTextMode overwriteMode){

			writer.setOverwriteMode(overwriteMode);
			//既に編集済みのCSVがある場合はマージを行う。
			const auto csvFileInProject = exportFolderPath(path.filename());
			if(std::filesystem::exists(csvFileInProject)){
				if(writer.merge(csvFileInProject) == false){
					return ErrorStatus(ErrorStatus::Module::PLATFORM_BASE, 1);
				}
			}

			return writer.write(path, overwriteMode);
		}
	};
}