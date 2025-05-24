#pragma once
#include "readerbase.h"
//readerbase���p������speciftranstext�N���X��錾

namespace langscore
{
//�t�@�C���ǂݍ��݂��s�킸�A���ږ|��e�L�X�g���w�肷�邽�߂̃N���X
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