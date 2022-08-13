#include "rbscriptwriter.h"
#include "config.h"
#include "utility.hpp"
#include <fstream>
#include <regex>
#include <mutex>
#include <future>
#include <format>

#include "csvwriter.h"
#include "../reader/csvreader.h"

static std::mutex _list_mutex;
static std::mutex _mutex;
static bool processing = false;
static std::condition_variable cond;

using namespace langscore;

namespace
{
    const auto nl = '\n';
    const auto tab = '\t';
}

rbscriptwriter::rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
    : writerbase(std::move(langs), std::vector<TranslateText>{})
{
    utility::u8stringlist scriptNameList;
    csvreader scriptList;
    config config;
    const auto lsAnalyzePath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
    auto scriptListCsv = scriptList.parsePlain(lsAnalyzePath / "Scripts/_list.csv");

    for(auto& path : scriptFileList)
    {
        auto fileName = path.filename().stem().u8string();
        auto result = std::find_if(scriptListCsv.cbegin(), scriptListCsv.cend(), [&fileName](const auto& x){
            return x[0] == fileName;
        });
        if(result == scriptListCsv.cend()){ continue; }
        scriptNameMap.emplace(path, (*result)[1]);

        ConvertScriptToCSV(path, (*result)[1]);
    }
}


bool langscore::rbscriptwriter::merge(std::filesystem::path filePath)
{
    if(std::filesystem::exists(filePath) == false){ return true; }

    switch(overwriteMode)
    {
        case OverwriteTextMode::OverwriteNew:
            return true;
            break;
    }
    return false;
}

bool langscore::rbscriptwriter::write(std::filesystem::path path, OverwriteTextMode overwriteMode)
{
    using namespace std::literals::string_literals;
    auto scriptName = scriptNameMap[path];

    //======================================
    const auto funcName = [](auto str)
    {
        for(auto i = str.find(" "); i != decltype(str)::npos; i = str.find(" ")){
            str.replace(i, 1, "_");
        }
        return "Langscore.translate_" + str;
    };

    const auto functionDef = [&](auto str){
        return decltype(str)("def " + funcName(str) + nl);
    };
    const auto functionComment = [&](auto str)
    {
        return decltype(str)("\t#========================================") + nl +
                             tab + "# " + str + nl +
                             tab + "#========================================" + nl;
    };
    constexpr char functionName[] = "Langscore.Translate_Script_Text";
    //======================================

    std::ofstream outFile(path);
    outFile << nl;
    outFile << "def " << functionName << nl;
    outFile << nl;
    outFile << tab << "$data_langscore_scripts ||= LSCSV.to_hash(\"Data/Translates/Scripts.lscsv\")" << nl;
    outFile << nl;

    for(auto& path : scriptTranslates){
        if(path.second.empty()){ continue; }
        outFile << tab << funcName(utility::toString(scriptName)) << nl;
    }
    outFile << "end" << nl << nl;


    config config;
    auto funcComment = config.usScriptFuncComment();

    for(auto& path : scriptTranslates)
    {
        if(path.second.empty()){ continue; }
        auto fsFilename = path.first.filename().stem();
        auto filename = utility::toString(fsFilename.u8string());
        outFile << functionComment(filename);
        outFile << functionDef(filename);

        if(scriptName == u8"Vocab"){
            WriteVocab(outFile, path.second);
        }
        else
        {
            for(auto& line : path.second)
            {
                auto parsed = utility::split(line.memo, u8':');
                auto filepath = std::vformat(funcComment, std::make_format_args(utility::toString(parsed[0]), utility::toString(parsed[1]), utility::toString(parsed[2])));
                outFile << tab << "#" + filepath << nl;
                outFile << tab << "#original : " << utility::toString(line.original) << nl;
                outFile << tab << "#Langscore.translate_for_script(\"" << utility::toString(line.memo) << "\")" << nl;
                outFile << nl;
            }
        }
        outFile << "end " << nl;
        outFile << nl;
    }


    return false;
}

bool rbscriptwriter::ConvertScriptToCSV(std::filesystem::path path, std::u8string scriptName)
{
    size_t lineCount = 0;

    //スマートにしたい
    std::regex parseStrDq(R"((".*?")(?!^\s*#))");
    std::regex parseStrSq(R"(('.*?')(?!^\s*#))");

    std::vector<TranslateText> transTextList;

    const auto ConvertFromMatch = [&](const std::smatch& matchList, size_t col_diff)
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
                if(line[line.size() - 1] != '\"'){ line.insert(line.size(), "\""); }
            }

            auto lineCountStr = std::to_string(lineCount);
            std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());

            //+2は (" の分
            auto colCountStr = std::to_string(matchList.position(i) + col_diff + 2);
            std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());

            std::u8string u8line(line.begin(), line.end());

            _list_mutex.lock();
            TranslateText t = {
                u8line,
                this->useLangs
            };
            t.memo = scriptName + u8":" + u8lineCount + u8":" + u8ColCountStr;

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
        //コメントのみの行かをチェック
        {
            auto check_comment_line = line;
            auto pos = std::find_if(check_comment_line.begin(), check_comment_line.end(), [](auto c){
                return c != '\t' && c != ' ';
            });
            check_comment_line.erase(check_comment_line.begin(), pos);
            if(check_comment_line.empty() == false && check_comment_line[0] == '#'){
                continue;
            }
        }
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

        const auto RegexMatch = [&](const std::regex& rgx)
        {
            std::smatch matchList;
            int col_diff_tmp = 0;
            while(std::regex_search(line, matchList, rgx))
            {
                ConvertFromMatch(matchList, col_diff_tmp);
                auto length_diff = matchList.position(0) + matchList.length(0);
                line = {line.begin() + length_diff, line.end()};
                col_diff_tmp += length_diff;
            }
        };
        RegexMatch(parseStrDq);
        RegexMatch(parseStrSq);
    }

    std::copy(transTextList.begin(), transTextList.end(), std::back_inserter(this->texts));
    scriptTranslates[path] = std::move(transTextList);
    return true;
}

void langscore::rbscriptwriter::WriteVocab(std::ofstream& file, std::vector<TranslateText> texts)
{
    const static std::map<std::u8string, std::u8string> translates = {
        {u8"%s の経験値を獲得！",u8"ObtainExp"},
        {u8"%sが%sをかばった！",u8"Substitute"},
        {u8"%sが出現！",u8"Emerge"},
        {u8"%sたち",u8"PartyName"},
        {u8"%sに %s のダメージを与えた！",u8"EnemyDamage"},
        {u8"%sには効かなかった！",u8"ActionFailure"},
        {u8"%sにダメージを与えられない！",u8"EnemyNoDamage"},
        {u8"%sの%sが %s 回復した！",u8"ActorRecovery"},
        {u8"%sの%sが %s 回復した！",u8"EnemyRecovery"},
        {u8"%sの%sが %s 増えた！",u8"ActorGain"},
        {u8"%sの%sが %s 増えた！",u8"EnemyGain"},
        {u8"%sの%sが %s 減った！",u8"ActorLoss"},
        {u8"%sの%sが %s 減った！",u8"EnemyLoss"},
        {u8"%sの%sが上がった！",u8"BuffAdd"},
        {u8"%sの%sが下がった！",u8"DebuffAdd"},
        {u8"%sの%sが元に戻った！",u8"BuffRemove"},
        {u8"%sの%sを %s 奪った！",u8"EnemyDrain"},
        {u8"%sの勝利！",u8"Victory"},
        {u8"%sの反撃！",u8"CounterAttack"},
        {u8"%sは %s のダメージを受けた！",u8"ActorDamage"},
        {u8"%sは%s %s に上がった！",u8"LevelUp"},
        {u8"%sは%sを %s 奪われた！",u8"ActorDrain"},
        {u8"%sは%sを使った！",u8"UseItem"},
        {u8"%sはダメージを受けていない！",u8"ActorNoDamage"},
        {u8"%sは不意をつかれた！",u8"Surprise"},
        {u8"%sは先手を取った！",u8"Preemptive"},
        {u8"%sは戦いに敗れた。",u8"Defeat"},
        {u8"%sは攻撃をかわした！",u8"Evasion"},
        {u8"%sは逃げ出した！",u8"EscapeStart"},
        {u8"%sは魔法を打ち消した！",u8"MagicEvasion"},
        {u8"%sは魔法を跳ね返した！",u8"MagicReflection"},
        {u8"%sを手に入れた！",u8"ObtainItem"},
        {u8"%sを覚えた！",u8"ObtainSkill"},
        {u8"お金を %s\\G 手に入れた！",u8"ObtainGold"},
        {u8"しかし逃げることはできなかった！",u8"EscapeFailure"},
        {u8"どのファイルにセーブしますか？",u8"SaveMessage"},
        {u8"どのファイルをロードしますか？",u8"LoadMessage"},
        {u8"やめる",u8"ShopCancel"},
        {u8"ファイル",u8"File"},
        {u8"ミス！　%sにダメージを与えられない！",u8"EnemyNoHit"},
        {u8"ミス！　%sはダメージを受けていない！",u8"ActorNoHit"},
        {u8"会心の一撃！！",u8"CriticalToEnemy"},
        {u8"売却する",u8"ShopSell"},
        {u8"持っている数",u8"Possession"},
        {u8"次の%sまで",u8"ExpNext"},
        {u8"現在の経験値",u8"ExpTotal"},
        {u8"痛恨の一撃！！",u8"CriticalToActor"},
        {u8"購入する",u8"ShopBuy"},
    };
    size_t maxVarLength = 0;
    for(const auto& t : translates){ maxVarLength = std::max(maxVarLength, (u8"Vocab::" + t.second + u8".replace").length()); }

    for(auto& t : texts)
    {
        if(translates.find(t.original) != translates.end())
        {
            auto varName = translates.at(t.original);
            auto lvalue = "Vocab::" + utility::toString(varName) + ".replace";
            std::string space(maxVarLength - lvalue.length(), ' ');
            file << tab << lvalue << space << " Langscore.translate_for_script(\"" << utility::toString(t.memo) << "\")";
            file << nl;
        }
    }
}
