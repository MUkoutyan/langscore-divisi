﻿#include "rbscriptwriter.h"
#include "config.h"
#include "utility.hpp"
#include "scriptRegex.hpp"
#include <fstream>
#include <format>

#include "csvwriter.h"
#include "../reader/csvreader.h"

static std::mutex _mutex;
static bool processing = false;
static std::condition_variable cond;


using namespace langscore;
using namespace std::string_literals;

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

        auto scriptName = (*result)[1];
        scriptNameMap.emplace(path, scriptName);
        auto transTextList = convertScriptToCSV(path);

        std::copy(transTextList.begin(), transTextList.end(), std::back_inserter(this->texts));
        scriptTranslates[path] = std::move(transTextList);
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

bool langscore::rbscriptwriter::write(std::filesystem::path filePath, OverwriteTextMode overwriteMode)
{
    using namespace std::literals::string_literals;

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

    const auto GetScriptName = [this](auto filePath){
        const auto& scriptName = scriptNameMap[filePath];
        if(scriptName.empty()){ return u8""s; }
        if(scriptName == u8"langscore"){ return u8""s; }
        if(scriptName == u8"langscore_custom"){ return u8""s; }
        return scriptName;
    };
    //======================================

    std::ofstream outFile(filePath);
    outFile << nl;
    outFile << "def " << functionName << nl;
    outFile << nl;
    outFile << tab << "$data_langscore_scripts ||= LSCSV.to_hash(\"Data/Translates/Scripts.lscsv\")" << nl;
    outFile << nl;

    for(auto& path : scriptTranslates)
    {
        if(path.second.empty()){ continue; }
        auto scriptName = GetScriptName(path.first);
        if(scriptName.empty()){ continue; }

        auto functionName = funcName(path.first.filename().stem().string());

        outFile << tab << functionName << tab << "#" << utility::toString(scriptName) << nl;
    }
    outFile << "end" << nl << nl;


    config config;
    auto funcComment = config.usScriptFuncComment();

    for(auto& path : scriptTranslates)
    {
        if(path.second.empty()){ continue; }
        auto scriptName = GetScriptName(path.first);
        if(scriptName.empty()){ continue; }
        outFile << functionComment(utility::toString(scriptName));
        outFile << functionDef(path.first.filename().stem().string());

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

writerbase::ProgressNextStep rbscriptwriter::checkCommentLine(TextCodec line)
{		
    //コメントのみの行かをチェック
    {
        auto check_comment_line = line;
        auto pos = std::find_if(check_comment_line.begin(), check_comment_line.end(), [](auto c){
            return c != '\t' && c != ' ';
        });
        check_comment_line.erase(check_comment_line.begin(), pos);
        if(check_comment_line.empty() == false && check_comment_line[0] == '#'){
            return ProgressNextStep::Continue;
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

    if(line == u8"=begin"){
        rangeComment = true;
        return ProgressNextStep::Continue;
    }
    else if(rangeComment)
    {
        if(line == u8"=end"){ rangeComment = false; }
        return ProgressNextStep::Continue;
    }

    if(line.empty()){ ProgressNextStep::Continue; }

    return ProgressNextStep::Throught;
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
