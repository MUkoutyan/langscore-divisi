#pragma once
#include "readerbase.h"
//readerbaseを継承したspeciftranstextクラスを宣言

namespace langscore
{
//ファイル読み込みを行わず、直接翻訳テキストを指定するためのクラス
class speciftranstext : public readerbase
{
public:
	speciftranstext(std::vector<TranslateText> texts)
		: readerbase({}){
		this->texts = std::move(texts);
	}
    speciftranstext(std::vector<std::u8string> useLangs, std::vector<TranslateText> texts)
        : readerbase({}) {
        this->useLangList = std::move(useLangs);
        this->texts = std::move(texts);
    }
	virtual ~speciftranstext() {}
};
}