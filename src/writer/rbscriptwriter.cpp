#include "rbscriptwriter.h"
#include "../../utility.hpp"
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
    for(auto& path : scriptFileList){
        ConvertScriptToCSV(path);
    }
}

bool langscore::rbscriptwriter::write(std::filesystem::path path, OverwriteTextMode overwriteMode)
{
    using namespace std::literals::string_literals;
    std::ofstream outFile(path);

    const auto functionComment = [&](auto str)
    {
        return decltype(str)("\t#========================================") + nl +
                             tab+"# " + str + nl +
                             tab+"#========================================" + nl;
    };
    constexpr char functionName[] = "Langscore.Translate_Script_Text";

    outFile << nl;
    outFile << "def " << functionName << nl;
    outFile << nl;
    outFile << tab << "$data_langscore_scripts ||= LSCSV.to_hash(\"Data/Scripts.lscsv\")" << nl;
    outFile << nl;
    for(auto& path : scriptTranslates)
    {
        if(path.second.empty()){ continue; }
        auto filename = path.first.filename();
        outFile << functionComment(utility::toString(filename.u8string()));

        if(filename == "Vocab.rb"){
            WriteVocab(outFile, path.second);
        }
        else{
            for(auto& line : path.second)
            {
                auto parsed = utility::split(line.original, u8':');
                auto filepath = std::format("project://Scripts/{0}#{1},{2}", utility::toString(parsed[0]), utility::toString(parsed[1]), utility::toString(parsed[2]));
                outFile << tab << "#" + filepath << nl;
                outFile << tab << "#Langscore.translate(\"" << utility::toString(line.memo) << "\")" << nl;
                outFile << nl;
            }
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

    std::copy(transTextList.begin(), transTextList.end(), std::back_inserter(this->texts));
    scriptTranslates[path] = std::move(transTextList);
    return true;
}

void langscore::rbscriptwriter::WriteVocab(std::ofstream& file, std::vector<TranslateText> texts)
{
    const static std::map<std::u8string, std::u8string> translates = {
        {u8"%s の経験値を獲得！",u8"ObtainExp"},
        {u8"%sが%sをかばった！",u8"Sub""stitute"},
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
        if(translates.find(t.memo) != translates.end())
        {
            auto varName = translates.at(t.memo);
            auto lvalue = "Vocab::" + utility::toString(varName) + ".replace";
            std::string space(maxVarLength - lvalue.length(), ' ');
            file << tab << lvalue << space << " Langscore.translate_for_script(\"" << utility::toString(t.memo) << "\")";
            file << nl;
        }
    }
}
