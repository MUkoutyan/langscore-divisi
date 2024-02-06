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
		constexpr static char8_t Script_File_Name[] = u8"langscore";
		constexpr static char8_t Custom_Script_File_Name[] = u8"langscore_custom";

		enum ValidateSummary : int {
			EmptyCol = 0,   //翻訳文が空
			NotFoundEsc,    //原文にある制御文字が翻訳文に含まれていない
			UnclosedEsc,    //[]で閉じる必要のある制御文字が閉じられていない
			IncludeCR,      //翻訳文にCR改行が含まれている。(マップのみ検出)
			NotEQLang,      //設定した言語とCSVを内の言語列に差異がある
		};

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
		std::vector<std::filesystem::path> exportFolderPath(std::filesystem::path fileName);

		void writeFixedGraphFileNameData();

		void copyFonts(std::filesystem::path fontDestPath);
		static std::unordered_map<std::u8string, std::u8string> Help_Text;
		static std::unordered_map<std::u8string, std::u8string> Language_Items;

		utility::u8stringlist GetScriptFileName(config& config, utility::u8stringlist scriptNameList);


		template<class Writer, class Reader>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, Reader&& reader, MergeTextMode overwriteMode = MergeTextMode::AcceptSource, bool fillDefLangCol = true){
			return writeFixedTranslateText(Writer{reader}, std::move(path), overwriteMode, fillDefLangCol);
		}
		template<class Writer>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, const std::unique_ptr<readerbase>& json, MergeTextMode overwriteMode = MergeTextMode::AcceptSource, bool fillDefLangCol = true){
			return writeFixedTranslateText(Writer{json}, std::move(path), overwriteMode, fillDefLangCol);
		}

		template<class Writer>
		ErrorStatus writeFixedTranslateText(Writer writer, std::filesystem::path path, MergeTextMode overwriteMode, bool fillDefLangCol){

			writer.setOverwriteMode(overwriteMode);
			//既に編集済みのCSVがある場合はマージを行う。
			if(std::filesystem::exists(path)) {
				if(writer.merge(path) == false) {
					return ErrorStatus(ErrorStatus::Module::PLATFORM_BASE, 1);
				}
			}

			writer.setFillDefLangCol(fillDefLangCol);

			return writer.write(path, overwriteMode);
		}

		bool validateTranslateFileList(utility::filelist csvPathList) const;
		bool validateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const;
		std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> findRPGMakerEscChars(std::u8string text) const;

	};
}