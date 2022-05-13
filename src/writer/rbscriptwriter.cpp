#include "rbscriptwriter.h"
#include "../../utility.hpp"
#include <fstream>
#include <regex>
#include <mutex>
#include <future>
#include <format>

#include "csvwriter.h"


static std::mutex _list_mutex;
static std::mutex _mutex;
static bool processing = false;
static std::condition_variable cond;

using namespace langscore;

rbscriptwriter::rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
    : writerbase(std::move(langs), std::vector<TranslateText>{})
{
    for(auto& path : scriptFileList){
        ConvertScriptToCSV(path);
    }
}

bool langscore::rbscriptwriter::write(std::filesystem::path path, OverwriteTextMode overwriteMode)
{
    using namespace std::literals::string_literals;
    std::ofstream outFile(path);

    const auto toString = [](const std::u8string& str){
        return std::string(str.begin(), str.end());
    };

    const auto nl = '\n';
    const auto tab = '\t';
    const auto functionComment = [&](auto str)
    {
        return decltype(str)("\t#========================================") + nl +
                             tab+"# " + str + nl +
                             tab+"#========================================" + nl;
    };
    constexpr char functionName[] = "Langscore_Translate_Script_Text";

    outFile << nl;
    outFile << "def " << functionName << nl;
    outFile << nl;
    outFile << tab << "$data_langscore_scripts ||= LSCSV.to_hash(\"Data/Scripts.lscsv\")" << nl;
    outFile << nl;
    for(auto& path : scriptTranslates)
    {
        if(path.second.empty()){ continue; }
        outFile << functionComment(toString(path.first.filename().u8string()));

        for(auto& line : path.second)
        {
            auto parsed = utility::split(line.original, u8':');
            auto filepath = std::format("project://Scripts/{0}#{1},{2}", toString(parsed[0]), toString(parsed[1]), toString(parsed[2]));
            outFile << tab << "#" + filepath << nl;
            outFile << tab << "#Langscore.translate(\"" << toString(line.memo) << "\")" << nl;
            outFile << nl;
        }
        outFile << nl;
    }

    outFile << "end";

    return false;
}

bool rbscriptwriter::ConvertScriptToCSV(std::filesystem::path path)
{
    auto fileName = path.filename().u8string();

    size_t lineCount = 0;

    //スマートにしたい
    std::regex parseStrDq(R"((".+?")(?!^\s*#))");
    std::regex parseStrSq(R"(('.+?')(?!^\s*#))");

    std::vector<TranslateText> transTextList;

    const auto ConvertFromMatch = [&](const std::smatch& matchList)
    {
        for(size_t i = 0; i < matchList.size(); ++i)
        {
            auto& m = matchList[i];
            auto line = m.str();
            //文字列に,も改行もなければダブルクォーテーションを削除
            if(line.find(',') == std::string::npos &&
               line.find('\n') == std::string::npos)
            {
                auto start = line[0];
                if(start == '\"' || start == '\''){
                    line.erase(0, 1);
                }
                auto end = line[line.size() - 1];
                if(end == '\"' || end == '\''){
                    line.erase(line.size() - 1, 1);
                }
            }
            else if(line.empty() == false)
            {
                //, \nが含まれている場合は念のため""括りされているかチェック
                if(line[0] != '\"'){ line.insert(0, "\""); }
                if(line[line.size() - 1] != '\"'){ line.insert(line.size() - 1, "\""); }
            }

            auto lineCountStr = std::to_string(lineCount);
            std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());

            auto colCountStr = std::to_string(matchList.position(i));
            std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());

            std::u8string u8line(line.begin(), line.end());

            _list_mutex.lock();
            TranslateText t = {
                fileName + u8":" + u8lineCount + u8":" + u8ColCountStr,
                this->useLangs
            };
            t.memo = u8line;

            auto dup_result = std::find_if(transTextList.begin(), transTextList.end(), [&t](const auto& x){
                return x.original == t.original;
            });
            if(dup_result == transTextList.end()){
                transTextList.emplace_back(std::move(t));
            }
            _list_mutex.unlock();
        }
    };

    std::ifstream loadFile(path);
    if(loadFile.is_open() == false){ return false; }

    bool rangeComment = false;
    while(loadFile.eof() == false)
    {
        lineCount++;
        std::string line;
        std::getline(loadFile, line);

        //行頭スペースの削除
        auto pos = std::find_if(line.begin(), line.end(), [](auto c){
            return c != '\t' && c != ' ';
        });
        line.erase(line.begin(), pos);
        //途中のコメントを削除
        auto begin_cm = line.begin();
        for(; begin_cm != line.end(); ++begin_cm){
            if(*begin_cm == '#'){
                auto next = (begin_cm + 1);
                if(next != line.end() && *next != '{'){
                    break;
                }
            }
        }
        if(begin_cm != line.end()){
            line.erase(begin_cm, line.end());
        }

        if(line == "=begin"){
            rangeComment = true;
            continue;
        }
        else if(rangeComment)
        {
            if(line == "=end"){ rangeComment = false; }
            else{ continue; }
        }

        if(line.empty()){ continue; }
        if(line[0] == '#'){ continue; }

        {
            std::smatch matchList;
            if(std::regex_search(line, matchList, parseStrDq)){
                ConvertFromMatch(matchList);
            }
        }
        {
            std::smatch matchList;
            if(std::regex_search(line, matchList, parseStrSq)){
                ConvertFromMatch(matchList);
            }
        }
    }

    scriptTranslates[path] = std::move(transTextList);
    return true;
}
