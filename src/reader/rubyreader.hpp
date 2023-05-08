//javascriptreaderのruby版のクラスを作成
#include "readerbase.h"
#include "utility.hpp"
#include "csvreader.h"
#include "config.h"
#include "writer/scripttextparser.hpp"
#include <filesystem>
#include <ranges>

#ifdef ENABLE_TEST
#include "iutest.hpp"

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, DetectRubyString);
#endif

namespace langscore
{

	class rubyreader: public readerbase
	{
#ifdef ENABLE_TEST
		IUTEST_FRIEND_TEST(Langscore_Writer, CheckRubyCommentLine);
		IUTEST_FRIEND_TEST(Langscore_Writer, DetectRubyString);
		IUTEST_FRIEND_TEST(Langscore_Writer, DetectStringPositionFromFile);
		IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_WriteScriptCSV);
#endif
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
			for(auto& scriptInfo : scriptInfoList)
			{
				auto fileName = fs::path(scriptInfo.filename).filename().u8string();
				auto scriptName = reader.GetScriptName(fileName);
				if(scriptName.empty()){ continue; }

				if(scriptInfo.ignore == false){
					for(const auto& textInfo : scriptInfo.texts)
					{
						if(textInfo.disable){ continue; }
						if(textInfo.ignore){ continue; }
						auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
						ignoreRowName.emplace_back(std::move(name));
					}
				}
				else{
					std::erase_if(this->texts, [&fileName](const auto& x){
						return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
					});
				}
			}
			{
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
	};

}