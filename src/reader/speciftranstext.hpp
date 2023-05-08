#pragma once
#include "readerbase.h"
//readerbaseを継承したspeciftranstextクラスを宣言

namespace langscore
{
//ファイル読み込みを行わず、直接翻訳テキストを指定するためのクラス
class speciftranstext : public readerbase
{
public:
	speciftranstext(std::vector<std::u8string> langs, std::vector<TranslateText> texts)
		: readerbase(std::move(langs), {}){
		this->texts = std::move(texts);
	}
	virtual ~speciftranstext() {}
};
}