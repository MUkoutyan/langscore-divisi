#pragma once
#include "readerbase.h"
//readerbase���p������speciftranstext�N���X��錾

namespace langscore
{
//�t�@�C���ǂݍ��݂��s�킸�A���ږ|��e�L�X�g���w�肷�邽�߂̃N���X
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