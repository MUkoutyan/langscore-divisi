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

		readerbase(std::vector<std::filesystem::path> scriptFileList);
		virtual ~readerbase();

		std::vector<TranslateText>&  currentTexts() & { return texts; }
		std::vector<TranslateText>&& currentTexts() && { return std::move(texts); }
		ScriptPackage&  curerntScriptTransMap() & { return scriptTranslatesMap; }
		ScriptPackage&& curerntScriptTransMap() && { return std::move(scriptTranslatesMap); }
		const std::vector<std::u8string>& curerntUseLangList() const & { return useLangList; }
		std::vector<std::u8string>&& curerntUseLangList() && { return std::move(useLangList); }
		const std::vector<PluginInfo>& curerntPluginInfoList() const & { return pluginInfoList; }
		std::vector<PluginInfo>&& curerntPluginInfoList() && { return std::move(pluginInfoList); }

	protected:

		using TextCodec = std::u8string;
		using TextCodecChar = TextCodec::value_type;
		using TextCodecView = std::basic_string_view<TextCodecChar>;

		std::vector<PluginInfo> pluginInfoList;
		std::vector<std::u8string> useLangList;	//翻訳対象となる言語の一覧(bcp47)
		std::vector<std::filesystem::path> scriptFileList;
		std::vector<TranslateText> texts;	//スクリプトファイル内で検出した翻訳文のリスト
		ScriptPackage scriptTranslatesMap;	//スクリプトファイルのパスとその中身をパースした結果を格納するマップ

	};
}