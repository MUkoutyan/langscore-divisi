#include "writerbase.h"
#include "utility.hpp"
#include <fstream>
#include "scripttextparser.hpp"

using namespace langscore;
using namespace std::string_literals;


writerbase::~writerbase()
{}

#ifdef _DEBUG
void writerbase::ReplaceDebugTextByOrigin(std::vector<TranslateText>& transTexts)
{
	for(auto& txt : transTexts)
	{
		auto origin = txt.original;
		for(auto& tl : txt.translates)
		{
			auto t = tl.second;
			t = origin;
			if(t[0] == u8'\"'){
				t.insert(1, tl.first + u8"-");
			}
			else {
				t = tl.first + u8"-" + t;
			}
			tl.second = t;
		}
	}
}

void writerbase::ReplaceDebugTextByLang(std::vector<TranslateText>& transTexts, std::u8string def_lang)
{
	for(auto& txt : transTexts)
	{
		auto origin = txt.translates[def_lang];
		for(auto& tl : txt.translates)
		{
			auto t = tl.second;
			if(def_lang != tl.first)
			{
				t = origin;
				if(t[0] == u8'\"'){
					t.insert(1, tl.first + u8"-");
				}
				else {
					t = tl.first + u8"-" + t;
				}
			}
			tl.second = t;
		}
	}
}
#endif

void writerbase::writeU8String(std::ofstream& out, std::u8string text)
{
	out.write(reinterpret_cast<const char*>(text.c_str()), text.size());
}

