#include "jsscriptwriter.h"
#include "config.h"
#include "utility.hpp"
#include "scripttextparser.hpp"
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

jsscriptwriter::jsscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
    : writerbase(std::move(langs), std::vector<TranslateText>{})
    , scriptFileList(std::move(scriptFileList))
{

    utility::u8stringlist scriptNameList;
    csvreader scriptList;
    config config;
    const auto lsAnalyzePath = std::filesystem::path(config.langscoreAnalyzeDirectorty());

    nlohmann::json pluginsList;
    utility::u8stringlist includedScriptList;
    {
        auto pluginsPath = std::filesystem::path(config.gameProjectPath()) / u8"js/plugins.js"s;
        std::ifstream loadFile(pluginsPath.generic_string());
        std::string fileStr{std::istreambuf_iterator<char>(loadFile), std::istreambuf_iterator<char>()};
        auto pluginStrList = utility::split(fileStr, '\n');
        for(auto& line : pluginStrList)
        {
            auto pos = line.find("\"name\"");
            if(pos == std::string::npos){ continue; }
            pos += 6;
            auto name_start = line.find(":\"", pos) + 2;
            auto name_end = line.find("\",", name_start);
            includedScriptList.emplace_back(utility::cnvStr<std::u8string>(line.substr(name_start, (name_end - name_start))));
        }
    }

    for(auto& path : this->scriptFileList)
    {
        auto fileName = path.filename().stem().u8string();

        if(std::find(includedScriptList.begin(), includedScriptList.end(), fileName) == includedScriptList.end()){
            continue;
        }

        //更新時の場合はここでlangscoreスクリプトが含まれている可能性がある
        if(fileName == u8"Langscore"){ continue; }
        else if(fileName == u8"Langscore_Custom"){ continue; }

        auto transTextList = convertScriptToCSV(path);

        std::copy(transTextList.begin(), transTextList.end(), std::back_inserter(this->texts));
        scriptTranslates.emplace_back(fileName, std::move(transTextList));
    }
}


bool langscore::jsscriptwriter::merge(std::filesystem::path filePath)
{
    if(std::filesystem::exists(filePath) == false){ return true; }

    switch(overwriteMode)
    {
        case MergeTextMode::AcceptTarget:
            return true;
            break;
    }
    return false;
}

ErrorStatus langscore::jsscriptwriter::write(std::filesystem::path filePath, MergeTextMode overwriteMode)
{
    //langscore_customの書き出し
    using namespace std::literals::string_literals;

    //======================================
    const auto funcName = [](auto str)
    {
        using Str = decltype(str);
        using Char = Str::value_type;
        for(auto i = str.find(Char(" ")); i != decltype(str)::npos; i = str.find(Char(" "))){
            str.replace(i, 1, (Char*)"_");
        }
        return Str(std::add_pointer_t<Char>("Langscore.translate_") + str);
    };
    const auto functionDef = [&](auto str){
        return decltype(str)("Langscore."+funcName(str) + " = function(){" + nl);
    };
    const auto functionComment = [&](auto str)
    {
        return decltype(str)("\t//========================================") + nl +
            tab + "// " + str + nl +
            tab + "//========================================" + nl;
    };
    constexpr char functionName[] = "Langscore.Translate_Script_Text";

    //======================================

    std::ofstream outFile(filePath);
    outFile << nl;
    outFile << "Langscore." << functionName << " = function(){" + nl;
    outFile << nl;

    config config;
    auto scriptInfoList = config.vxaceScripts();
    auto scriptTranslatesTmp = scriptTranslates;

    for(auto& pair : scriptTranslatesTmp){
        std::get<1>(pair) = this->acceptIgnoreScripts(scriptInfoList, std::get<1>(pair));
    }

    auto rm_result = std::remove_if(scriptTranslatesTmp.begin(), scriptTranslatesTmp.end(), [this, &scriptInfoList](const auto& x){
        return std::get<1>(x).empty();
    });
    scriptTranslatesTmp.erase(rm_result, scriptTranslatesTmp.end());

    for(auto& pair : scriptTranslatesTmp)
    {
        const auto& fileName = std::get<0>(pair);
        auto scriptName = fileName;
        auto functionName = utility::cnvStr<std::string>(funcName(fileName));
        outFile << tab << functionName << tab << "//" << utility::toString(scriptName) << nl;
    }
    outFile << "}" << nl << nl;

    auto funcComment = config.usScriptFuncComment();

    for(auto& pair : scriptTranslatesTmp)
    {
        const auto& fileName = std::get<0>(pair);
        auto scriptName = fileName;
        outFile << functionComment(utility::toString(scriptName));
        outFile << functionDef(utility::cnvStr<std::string>(fileName));

        const auto& list = std::get<1>(pair);
        for(auto& line : list)
        {
            auto parsed = utility::split(line.original, u8':');
            auto filepath = std::vformat(funcComment, std::make_format_args(utility::toString(parsed[0]), utility::toString(parsed[1]), utility::toString(parsed[2])));
            outFile << tab << "//" + filepath << nl;
            outFile << tab << "//original : " << utility::toString(line.scriptLineInfo) << nl;
            outFile << tab << "//Langscore.translate_for_script(\"" << utility::toString(line.original) << "\")" << nl;
            outFile << nl;
        }
        outFile << "} " << nl;
        outFile << nl;
    }


    return Status_Success;
}

std::vector<TranslateText> langscore::jsscriptwriter::acceptIgnoreScripts(const std::vector<config::ScriptData>& scriptInfoList, std::vector<TranslateText> transTexts)
{
    if(transTexts.empty()){ return {}; }
    namespace fs = std::filesystem;

    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());

    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
        t.scriptLineInfo.swap(t.original);
    }

    csvreader scriptListReader;
    const auto lsAnalyzePath = fs::path(config.langscoreAnalyzeDirectorty());
    //無視する行の判定
    utility::u8stringlist ignoreRowName;
    for(auto& scriptInfo : scriptInfoList)
    {
        auto fileName = fs::path(scriptInfo.filename).filename().stem().u8string();
        if(fileName == u8"Langscore"s || fileName == u8"Langscore_Custom"s){
            continue;
        }

        if(scriptInfo.ignore == false){
            for(const auto& textInfo : scriptInfo.texts)
            {
                if(textInfo.disable){ continue; }
                if(textInfo.ignore){ continue; }
                auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
                ignoreRowName.emplace_back(std::move(name));
            }
        }
        else{
            auto rm_result = std::remove_if(transTexts.begin(), transTexts.end(), [&fileName](const auto& x){
                return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
            });
            transTexts.erase(rm_result, transTexts.end());
        }
    }
    {
        auto rm_result = std::remove_if(transTexts.begin(), transTexts.end(), [&ignoreRowName](const auto& t){
            return std::find(ignoreRowName.cbegin(), ignoreRowName.cend(), t.original) != ignoreRowName.cend();
        });
        transTexts.erase(rm_result, transTexts.end());
    }

    return transTexts;
}

writerbase::ProgressNextStep jsscriptwriter::checkCommentLine(TextCodec& line)
{
    //コメントのみの行かをチェック
    using Char = TextCodec::value_type;
    //途中のコメントを削除
    auto begin_cm = line.begin();
    bool inString = false;

    if(rangeComment == false)
    {
        for(; begin_cm != line.end(); ++begin_cm)
        {
            auto c = *begin_cm;
            if(inString == false && c == Char('//')){
                auto next = (begin_cm + 1);
                if(next == line.end()){ continue; }
                if(*next == Char('//')){ break; }
                if(*next == Char('*')){
                    rangeComment = true;
                    break;
                }
            }
            else if(c == Char('\"') || c == Char('\'')){
                inString = !inString;
            }
        }
        if(begin_cm != line.end()){
            line.erase(begin_cm, line.end());
        }
    }

    if(rangeComment)
    {
        auto pos = line.find(u8"*/"s);
        if(pos != TextCodec::npos){
            rangeComment = false;
            line.erase(line.begin(), line.begin()+(pos+2));
            //文章が残っていたら続行
            return line.empty() ? ProgressNextStep::Next : ProgressNextStep::Throught;
        }
        return ProgressNextStep::Next;
    }

    //規定の関数
    ScriptTextParser textParser;
    const std::u8string_view transFuncName = u8".lstrans";
    auto pos = line.find(transFuncName);
    for(; pos != TextCodec::npos; pos = line.find(transFuncName))
    {
        auto quotePos = pos + transFuncName.length();
        auto quote = u8'\"';
        constexpr auto sq = u8'\'';
        bool useBracket = false;
        if(line[quotePos] == u8'('){
            quotePos++;
            useBracket = true;
            if(line[quotePos] == sq){
                quote = sq;
            }
        }
        auto endQuotePos = line.find_first_of(quote, quotePos + 1) + 1;
        if(useBracket){ endQuotePos++; }
        //assert(endQuotePos != TextCodec::npos && "There's a weird script!");

        //列数がズレるのが困るので、削除ではなく空白で置換
        //UTF8も考慮して、マルチバイトを考慮した文字数を置換する。
        const auto detectLength = endQuotePos - pos;
        auto detectStr = line.substr(pos, detectLength);
        auto numText = textParser.ConvertWordList(detectStr);
        TextCodec space(numText.size(), u8' ');
        line.replace(pos, detectLength, space);
    }
    if(line.empty()){ ProgressNextStep::Next; }

    return ProgressNextStep::Throught;
}

void langscore::jsscriptwriter::WriteVocab(std::ofstream& file, std::vector<TranslateText> texts)
{
    std::vector<std::tuple<std::u8string, std::u8string, bool>> translates = {
        { u8"%s の経験値を獲得！", u8"ObtainExp", false },
        { u8"%sが%sをかばった！", u8"Substitute", false },
        { u8"%sが出現！", u8"Emerge", false },
        { u8"%sたち", u8"PartyName", false },
        { u8"%sに %s のダメージを与えた！", u8"EnemyDamage", false },
        { u8"%sには効かなかった！", u8"ActionFailure", false },
        { u8"%sにダメージを与えられない！", u8"EnemyNoDamage", false },
        { u8"%sの%sが %s 回復した！", u8"ActorRecovery", false },
        { u8"%sの%sが %s 回復した！", u8"EnemyRecovery", false },
        { u8"%sの%sが %s 増えた！", u8"ActorGain", false },
        { u8"%sの%sが %s 増えた！", u8"EnemyGain", false },
        { u8"%sの%sが %s 減った！", u8"ActorLoss", false },
        { u8"%sの%sが %s 減った！", u8"EnemyLoss", false },
        { u8"%sの%sが上がった！", u8"BuffAdd", false },
        { u8"%sの%sが下がった！", u8"DebuffAdd", false },
        { u8"%sの%sが元に戻った！", u8"BuffRemove", false },
        { u8"%sの%sを %s 奪った！", u8"EnemyDrain", false },
        { u8"%sの勝利！", u8"Victory", false },
        { u8"%sの反撃！", u8"CounterAttack", false },
        { u8"%sは %s のダメージを受けた！", u8"ActorDamage", false },
        { u8"%sは%s %s に上がった！", u8"LevelUp", false },
        { u8"%sは%sを %s 奪われた！", u8"ActorDrain", false },
        { u8"%sは%sを使った！", u8"UseItem", false },
        { u8"%sはダメージを受けていない！", u8"ActorNoDamage", false },
        { u8"%sは不意をつかれた！", u8"Surprise", false },
        { u8"%sは先手を取った！", u8"Preemptive", false },
        { u8"%sは戦いに敗れた。", u8"Defeat", false },
        { u8"%sは攻撃をかわした！", u8"Evasion", false },
        { u8"%sは逃げ出した！", u8"EscapeStart", false },
        { u8"%sは魔法を打ち消した！", u8"MagicEvasion", false },
        { u8"%sは魔法を跳ね返した！", u8"MagicReflection", false },
        { u8"%sを手に入れた！", u8"ObtainItem", false },
        { u8"%sを覚えた！", u8"ObtainSkill", false },
        { u8"お金を %s\\\\G 手に入れた！", u8"ObtainGold", false },
        { u8"しかし逃げることはできなかった！", u8"EscapeFailure", false },
        { u8"どのファイルにセーブしますか？", u8"SaveMessage", false },
        { u8"どのファイルをロードしますか？", u8"LoadMessage", false },
        { u8"やめる", u8"ShopCancel", false },
        { u8"ファイル", u8"File", false },
        { u8"ミス！　%sにダメージを与えられない！", u8"EnemyNoHit", false },
        { u8"ミス！　%sはダメージを受けていない！", u8"ActorNoHit", false },
        { u8"会心の一撃！！", u8"CriticalToEnemy", false },
        { u8"売却する", u8"ShopSell", false },
        { u8"持っている数", u8"Possession", false },
        { u8"次の%sまで", u8"ExpNext", false },
        { u8"現在の経験値", u8"ExpTotal", false },
        { u8"痛恨の一撃！！", u8"CriticalToActor", false },
        { u8"購入する", u8"ShopBuy", false },
    };
    size_t maxVarLength = 0;
    for(const auto& t : translates){ maxVarLength = std::max(maxVarLength, (u8"Vocab::" + std::get<1>(t) + u8".replace").length()); }

    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());

    for(auto& t : texts)
    {
        auto& original = t.translates[def_lang];
        auto result = std::find_if(translates.begin(), translates.end(), [&original](const auto& x){
            return std::get<0>(x) == original && std::get<2>(x) == false;
        });
        if(result != translates.end())
        {
            auto varName = std::get<1>(*result);
            auto lvalue = "Vocab::" + utility::toString(varName) + ".replace";
            std::string space(maxVarLength - lvalue.length(), ' ');
            file << tab << "// " << utility::toString(t.scriptLineInfo) << nl;
            file << tab << lvalue << space << " Langscore.translate_for_script(\"" << utility::toString(t.original) << "\")";
            file << nl;
            std::get<2>(*result) = true;
        }
    }
}
