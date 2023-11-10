#pragma once
#include "serialize_base.h"
#include "utility.hpp"
#include <filesystem>
#include <string>
#include <iostream>
#include <cassert>

class ScriptTextParser
{
public:

	using DataType = std::vector<std::tuple<std::u8string, size_t>>;

	//�P�ꂲ�Ƃ̔z��ɕϊ�
	std::vector<std::tuple<std::u8string_view, size_t>> ConvertWordList(std::u8string_view str)
	{
		std::vector<std::tuple<std::u8string_view, size_t>> result;
		result.reserve(wordCountUTF8(str));
		for(size_t i = 0; i < str.size();)
		{
			auto ptr = str.data()[i];
			const size_t length = utility::getUTF8ByteLength(ptr);
			result.emplace_back(str.substr(i, length), i);
			i += length;
		}
		return result;
	}

	size_t wordCountUTF8(std::u8string_view text)
	{
		int words = 0;
		for(auto beg = text.cbegin(); beg != text.cend(); ++beg){
			if((*beg & 0xc0) != 0x80){ words++; }
		}
		return words;
	}

	void convertTranslateTextFromMatch(std::u8string line, size_t col, DataType& transTextList)
	{
		//�������,�����s���Ȃ���΃_�u���N�H�[�e�[�V�������폜
		//if(line.find(',') == std::string::npos &&
		//   line.find('\n') == std::string::npos)
		//{
		//	auto start = line[0];
		//	if(start == '\"' || start == '\''){
		//		line.erase(0, 1);
		//	}
		//	auto end = line[line.size() - 1];
		//	if(end == '\"' || end == '\''){
		//		line.erase(line.size() - 1, 1);
		//	}
		//}
		//else if(line.empty() == false)
		//{
		//	//, \n���܂܂�Ă���ꍇ�͔O�̂���""���肳��Ă��邩�`�F�b�N
		//	if(line[0] != '\"'){ line.insert(0, u8"\""); }
		//	if(line[line.size() - 1] != '\"'){ line.insert(line.size(), u8"\""); }
		//}

		//auto start = line[0];
		//if(start == '\"' || start == '\''){
		//	line.erase(0, 1);
		//}
		//auto end = line[line.size() - 1];
		//if(end == '\"' || end == '\''){
		//	line.erase(line.size() - 1, 1);
		//}

		if(line.empty()){ return; }

		//+1�� " �̕�
		col += 1;
		if(std::find_if(transTextList.cbegin(), transTextList.cend(), [col](const auto& x){
			return std::get<1>(x) == col;
		}) == transTextList.cend()){
			transTextList.emplace_back(line, col);
		}
	}

	//���͂���������Ɍ���̕����񂪂��邩�𔻒肷��֐�
	//DataType findStrings(std::u8string line)
	//{
	//	if(line.empty()){ return {}; }

	//	using namespace std::string_view_literals;
	//	//�s���̕����𒊏o�B�}���`�o�C�g������1�����ƃJ�E���g���邽�߂̌`���B
	//	auto wordList = ConvertWordList(line);

	//	//�����񂪖�����Ζ���
	//	if(std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x){ return std::get<0>(x) == u8"\""sv; }) == wordList.cend() &&
	//	   std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x){ return std::get<0>(x) == u8"'"sv; }) == wordList.cend())
	//	{
	//		return {};
	//	}

	//	DataType transTextList;
	//	size_t strStart = 0;
	//	bool findDq = false;
	//	bool findSq = false;
	//	size_t col = 0;
	//	size_t index = 1;
	//	bool beforeEsc = false;
	//	for(auto& strView : wordList)
	//	{
	//		auto& str = std::get<0>(strView);
	//		if(beforeEsc == false)
	//		{
	//			if(str == u8"\"" && findSq == false){
	//				if(findDq == false){
	//					col = index;
	//					strStart = std::get<1>(strView)+1;
	//				}
	//				else {
	//					auto endPos = std::get<1>(strView);
	//					convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
	//				}
	//				findDq = !findDq;
	//			}
	//			else if(str == u8"'" && findDq == false){
	//				if(findSq == false){
	//					col = index;
	//					strStart = std::get<1>(strView) + 1;
	//				}
	//				else {
	//					auto endPos = std::get<1>(strView);
	//					convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
	//				}
	//				findSq = !findSq;
	//			}
	//		}
	//		
	//		beforeEsc = str == u8"\\";

	//		index++;
	//	}
	//	return transTextList;
	//}

private:
};