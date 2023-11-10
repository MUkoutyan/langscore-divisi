#pragma once

#include "../serialize_base.h"
#include "config.h"
#include "scripttextparser.hpp"
#include <vector>
#include <filesystem>

namespace langscore
{
	//スクリプトファイルを読み込んで解析するクラスの基底クラス
	class readerbase
	{
	public:
		//スクリプト名とそのスクリプトの中身をパースした結果を格納する型

		readerbase(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
		virtual ~readerbase();

		std::vector<TranslateText>&  curerntTexts() & { return texts; }
		std::vector<TranslateText>&& curerntTexts() && { return std::move(texts); }
		ScriptPackage&  curerntScriptTransMap() & { return scriptTranslatesMap; }
		ScriptPackage&& curerntScriptTransMap() && { return std::move(scriptTranslatesMap); }
		const std::vector<std::u8string>& curerntUseLangList() & { return useLangList; }
		std::vector<std::u8string>&& curerntUseLangList() && { return std::move(useLangList); }
		const std::vector<PluginInfo>& curerntPluginInfoList() & { return pluginInfoList; }
		std::vector<PluginInfo>&& curerntPluginInfoList() && { return std::move(pluginInfoList); }

	protected:

		using TextCodec = std::u8string;
		using TextCodecChar = TextCodec::value_type;
		using TextCodecView = std::basic_string_view<TextCodecChar>;
		//行を解析した後に行う動作
		enum class ProgressNextStep
		{
			NextLine,	//次の行を読み込む
			Break,		//ファイルの読み込みを終える
			Throught	//行の解析処理を行う
		};

		std::vector<PluginInfo> pluginInfoList;
		std::vector<std::u8string> useLangList;	//翻訳対象となる言語の一覧(bcp47)
		std::vector<std::filesystem::path> scriptFileList;
		std::vector<TranslateText> texts;	//スクリプトファイル内で検出した翻訳文のリスト
		ScriptPackage scriptTranslatesMap;	//スクリプトファイルのパスとその中身をパースした結果を格納するマップ

		TextCodec lineComment;
		TextCodec rangeCommentBegin;
		TextCodec rangeCommentEnd;

		virtual ScriptTextParser::DataType findStrings(std::u8string line) const { return {}; };

		//lineComment, rangeComment~のセット関数を定義
		void setComment(TextCodec line, TextCodec begin, TextCodec end);

		//コメントされた箇所を検出して除外する。
		//複数行にまたがる範囲コメントを検出した場合inRangeCommentにはtrueが含まれる。
		ProgressNextStep replaceCommentToSpace(TextCodec& lineText, bool& inRangeComment) const;
		virtual ProgressNextStep checkRangeComment(TextCodec& lineText, bool& inRangeComment, size_t pos) const;
		virtual ProgressNextStep checkLineComment(TextCodec& lineText) const;

		virtual std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path) const;
		TextCodecChar findBeginEnclose(TextCodecView text, size_t endPos) const;
		bool isValidProgramLine(TextCodecView text) const;
	};
}