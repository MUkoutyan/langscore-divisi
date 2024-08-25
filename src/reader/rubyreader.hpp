//javascriptreaderのruby版のクラスを作成
#include "readerbase.h"
#include "utility.hpp"
#include "csvreader.h"
#include "config.h"
#include "scripttextparser.hpp"
#include <filesystem>
#include <ranges>

namespace langscore
{

	class rubyreader: public readerbase
	{
	public:
		rubyreader(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
			: readerbase(std::move(langs), std::move(scriptFileList))
		{
			this->setComment(u8"#", u8"=begin", u8"=end");

			config config;
			const auto lsAnalyzePath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
			auto filePathPair = plaincsvreader{lsAnalyzePath / "Scripts/_list.csv"}.getPlainCsvTexts();
			for(const auto& row : filePathPair)
			{
				auto info = PluginInfo{
					utility::cnvStr<std::u8string>(row[1]),
					utility::cnvStr<std::u8string>(row[0]) + u8".rb",
					true, u8"", {}
				};
				pluginInfoList.emplace_back(std::move(info));
			}

			for(auto& path : this->scriptFileList)
			{
				auto scriptTexts = this->parse(path);
				this->texts.insert(this->texts.end(), scriptTexts.begin(), scriptTexts.end());

				scriptTranslatesMap.emplace_back(path.filename().u8string(), std::move(scriptTexts));
			}
		}
		~rubyreader() override {}


		void applyIgnoreScripts(const std::vector<config::ScriptData>& scriptInfoList)
		{
			if(this->texts.empty()){ return; }
			using namespace std::string_literals;
			namespace fs = std::filesystem;

			config config;
			auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());

			for(auto& t : this->texts){
				if(t.translates.find(def_lang) == t.translates.end()){ continue; }
				t.translates[def_lang] = t.original;
				t.scriptLineInfo.swap(t.original);
			}

			//無視する行の判定
			rubyreader reader({def_lang}, {});
			utility::u8stringlist ignoreRowName;
			for(auto& scriptInfo : pluginInfoList)
			{
				auto fileName = fs::path(scriptInfo.filename).filename().u8string();
                auto scriptName = scriptInfo.name;

                if(scriptName == u8"langscore" || scriptName == u8"langscore_custom") {
                    std::erase_if(this->texts, [fileName = utility::removeExtension(fileName)](const auto& x) {
                        return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
                    });
                    continue;
                }

                auto result = std::find_if(scriptInfoList.cbegin(), scriptInfoList.cend(), [&](const auto& x) {
                    return x.filename == fileName;
                });

                if(result == scriptInfoList.end()) {
                    continue;
                }

				if(result->ignore == false){
					for(const auto& textInfo : result->texts)
					{
						if(textInfo.disable){ continue; }
						if(textInfo.ignore){ continue; }
                        //スクリプト内の行を無視する。
						auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
						ignoreRowName.emplace_back(std::move(name));
					}
				}
				else{
                    //スクリプト自体を無視する。
					std::erase_if(this->texts, [&fileName](const auto& x){
						return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
					});
				}
			}

            if(ignoreRowName.empty() == false)
			{
                //無視する行の削除。
				std::erase_if(this->texts, [&ignoreRowName](const auto& t){
					return std::find(ignoreRowName.cbegin(), ignoreRowName.cend(), t.original) != ignoreRowName.cend();
				});
			}
		}

	private:

		std::vector<TranslateText> parse(std::filesystem::path path)
		{
			//パース自体はあくまでコンストラクタに渡されたファイルパスをそのまま読み込ませるべきなので、
			//ここでスクリプト名によるフィルター等は行わない。
			return convertScriptToCSV(path);
		}

		std::u8string GetScriptName(std::u8string fileName) const
		{
			using namespace std::string_literals;

			auto result = std::ranges::find_if(pluginInfoList, [&fileName](const auto& x){
				return x.filename == fileName;
			});
			if(result == pluginInfoList.end()){ return u8""s; }
			auto scriptName = (*result).name;
			if(scriptName == u8"langscore"){ return u8""s; }
			if(scriptName == u8"langscore_custom"){ return u8""s; }
			return scriptName;
		}

		ProgressNextStep checkLineComment(TextCodec& lineText) const override
		{
			//先頭が行コメントなら無視する
			auto lineCommentBegin = lineText.find(this->lineComment);
			if(lineCommentBegin == TextCodec::npos){
				return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
			}

			//検出位置が0なら文字列が入る余地が無いのでコメントとして扱う
			if(lineCommentBegin == 0){
				return ProgressNextStep::NextLine;
			}

			//文字列中の場合はコメントとして扱わない ("#{number}"など)
			if(findBeginEnclose(lineText, lineCommentBegin) != TextCodecChar{}){
				return ProgressNextStep::Throught;
			}

			std::fill_n(lineText.begin() + lineCommentBegin, lineText.size() - lineCommentBegin, TextCodecChar(' '));

			if(isValidProgramLine(lineText) == false){
				return ProgressNextStep::NextLine;
			}
			return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
		}

		ScriptTextParser::DataType findStrings(std::u8string line) const
		{
			if(line.empty()) { return {}; }

			using namespace std::string_view_literals;
			ScriptTextParser parser;
			//行内の文字を抽出。マルチバイト文字を1文字とカウントするための形式。
			auto wordList = parser.ConvertWordList(line);

			//文字列が無ければ無視
			if(std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x) { return std::get<0>(x) == u8"\""sv; }) == wordList.cend() &&
				std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x) { return std::get<0>(x) == u8"'"sv; }) == wordList.cend())
			{
				return {};
			}

			ScriptTextParser::DataType transTextList;
			size_t strStart = 0;
			bool findDq = false;
			bool findSq = false;
			size_t col = 0;
			size_t index = 1;
			bool beforeEsc = false;
			for(auto& strView : wordList)
			{
				auto& str = std::get<0>(strView);
				if(beforeEsc == false)
				{
					if(str == u8"\"" && findSq == false) {
						if(findDq == false) {
							col = index;
							strStart = std::get<1>(strView) + 1;
						}
						else {
							auto endPos = std::get<1>(strView);
							parser.convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
						}
						findDq = !findDq;
					}
					else if(str == u8"'" && findDq == false) {
						if(findSq == false) {
							col = index;
							strStart = std::get<1>(strView) + 1;
						}
						else {
							auto endPos = std::get<1>(strView);
							parser.convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
						}
						findSq = !findSq;
					}
				}

				beforeEsc = str == u8"\\";

				index++;
			}
			return transTextList;
		}
	};

}