#pragma once
#include "../invoker.h"
#include "../include/config.h"
#include "../utility.hpp"
#include "../writer/writerbase.h"
#include "../reader/csvreader.h"
#include "errorstatus.hpp"

namespace langscore
{
	class platform_base
	{
	public:
		constexpr static char8_t Script_File_Name[] = u8"langscore";
		constexpr static char8_t Custom_Script_File_Name[] = u8"langscore_custom";

        enum ValidateErrorType { Error, Warning };


        /*
        * enum ErrorTextCol
        * {
        *     Type = 0,
        *     Summary,
        *     Language,
        *     Details,
        *     File,
        *     Row,
        * };
        */

		enum ValidateSummary : char {
			EmptyCol = 0,       //翻訳文が空
			NotFoundEsc,        //原文にある制御文字が翻訳文に含まれていない
			UnclosedEsc,        //[]で閉じる必要のある制御文字が閉じられていない
			IncludeCR,          //翻訳文にCR改行が含まれている。(マップのみ検出)
			NotEQLang,          //設定した言語とCSVを内の言語列に差異がある
            PartiallyClipped,   //テキストの最後の文字が少し見切れる
            FullyClipped,       //完全に見えない文字がある
            OverTextCount,      //テキストの文字数が多すぎる
            InvalidCSV,         //CSVファイルが不正
		};

        struct ValidateFileInfo {
            std::filesystem::path csvPath;
            config::TextValidateTypeMap textValidateInfos;
        };

        platform_base() :appPath(""), invoker(config{}.projectType()) {}
		virtual ~platform_base() {}

		void setAppPath(std::filesystem::path path){
			appPath = path;
			this->invoker.setApplicationFolder(std::move(path));
		}
		virtual void setProjectPath(std::filesystem::path path) = 0;
		virtual ErrorStatus analyze() = 0;
		virtual ErrorStatus reanalysis() = 0;
		virtual ErrorStatus updatePlugin() = 0;
		virtual ErrorStatus exportCSV() = 0;
		virtual ErrorStatus validate() = 0;
		virtual ErrorStatus packing() = 0;

		//virtual void copyData(langscore::MergeTextMode option = langscore::MergeTextMode::AcceptSource) = 0;

	protected:
		std::filesystem::path appPath;
		langscore::invoker invoker;
		utility::u8stringlist supportLangs;
		std::u8string defaultLanguage;

		utility::filelist basicDataFileList;
		utility::filelist scriptFileList;
		utility::filelist graphicFileList;

        std::filesystem::path currentGameProjectPath;

		std::filesystem::copy_options convertCopyOption(MergeTextMode mode);
		std::vector<std::filesystem::path> exportFolderPath(std::filesystem::path fileName);

		void writeFixedGraphFileNameData();

		void copyFonts(std::filesystem::path fontDestPath);
		static std::unordered_map<std::u8string, std::u8string> Help_Text;
		static std::unordered_map<std::u8string, std::u8string> Language_Items;

		utility::u8stringlist GetScriptFileName(config& config, utility::u8stringlist scriptNameList);

        virtual std::filesystem::path getGameProjectFontDirectory() const { return {}; }


		template<class Writer, class Reader>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, Reader&& reader, std::u8string defaultLanguage, utility::u8stringlist supportLangs, MergeTextMode overwriteMode = MergeTextMode::AcceptSource, bool fillDefaultLanguageColumn = true) {
			return writeFixedTranslateText(Writer{reader}, std::move(path), std::move(defaultLanguage), std::move(supportLangs), overwriteMode, fillDefaultLanguageColumn);
		}
		template<class Writer>
		ErrorStatus writeFixedTranslateText(std::filesystem::path path, const std::unique_ptr<readerbase>& json, std::u8string defaultLanguage, utility::u8stringlist supportLangs, MergeTextMode overwriteMode = MergeTextMode::AcceptSource, bool fillDefaultLanguageColumn = true){
			return writeFixedTranslateText(Writer{json}, std::move(path), std::move(defaultLanguage), std::move(supportLangs), overwriteMode, fillDefaultLanguageColumn);
		}

		template<class Writer>
		ErrorStatus writeFixedTranslateText(Writer writer, std::filesystem::path path, std::u8string defaultLanguage, utility::u8stringlist supportLangs, MergeTextMode overwriteMode, bool fillDefaultLanguageColumn){

			writer.setOverwriteMode(overwriteMode);
			//既に編集済みのCSVがある場合はマージを行う。
			if(std::filesystem::exists(path)) {
				if(writer.merge(path) == false) {
					return ErrorStatus(ErrorStatus::Module::PLATFORM_BASE, 1);
				}
			}

			writer.setFillDefLangCol(fillDefaultLanguageColumn);
            writer.setSupportLanguage(std::move(supportLangs));

			return writer.write(std::move(path), std::move(defaultLanguage), overwriteMode);
		}

        struct ValidateTextInfo 
        {
            TranslateText origin;

            struct Display {
                std::u8string original;
                std::unordered_map<std::u8string, std::u8string> translates;
            };
            Display display;
            
            std::vector<std::u8string> escWithValueChars;   //検出した制御文字とその値
            std::vector<std::u8string> escChars;            //検出した制御文字
        };

        std::vector<ValidateTextInfo> convertValidateTextInfo(std::string fileName, const std::vector<TranslateText>& texts) const;
        void detectConstrolChar(ValidateTextInfo& validateInfo) const;

		bool validateTranslateFileList(std::vector<ValidateFileInfo> csvPathList) const;
        
        bool validateCsvFormat(ValidateFileInfo& fileInfo, const csvreader& wroteCsvReader) const;
		bool validateTextFormat(const std::vector<ValidateTextInfo>& texts, std::filesystem::path path) const;
        std::uint64_t countNumTexts(const std::u8string& multilineText) const;
        bool validateTexts(const std::vector<ValidateTextInfo>& texts, const config::TextValidateTypeMap& validateSizeList, std::filesystem::path path) const;

        //テキストの幅を計測する。戻り値:{textの1文字, {左座標, 右の座標}}
        struct TextHorizontalLength { int left; int right; };
        std::vector<std::pair<std::u8string, TextHorizontalLength>> measureTextWidth(std::u8string text, std::u8string fontPath, int fontSize) const;

	};
}