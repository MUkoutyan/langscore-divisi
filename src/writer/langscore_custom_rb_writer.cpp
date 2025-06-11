#include "langscore_custom_rb_writer.h"
#include "config.h"
#include "utility.hpp"
#include "scripttextparser.hpp"
#include <fstream>
#include <mutex>

#ifdef __cpp_lib_format
#include <format>
#endif

#include "csvwriter.h"
#include "../reader/csvreader.h"
#include "../reader/rubyreader.hpp"

// static std::mutex _mutex;
// static bool processing = false;
// static std::condition_variable cond;


using namespace langscore;
using namespace std::string_literals;

namespace
{
    const auto nl = '\n';
    const auto tab = '\t';

}

bool langscore::langscore_custom_rb_writer::merge(std::filesystem::path filePath)
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

ErrorStatus langscore_custom_rb_writer::write(std::filesystem::path filePath, std::u8string defaultLanguage, MergeTextMode overwriteMode)
{
    //langscore_customの書き出し
    using namespace std::literals::string_literals;

    //======================================
    const auto funcName = [](auto str)
    {
        using Str  = decltype(str);
        using Char = Str::value_type;
        str = utility::removeExtension(str);
        for(auto i = str.find(Char(' ')); i != decltype(str)::npos; i = str.find(Char(' '))){
            str.replace(i, 1, (Char*)"_");
        }
        return Str(std::add_pointer_t<Char>("Langscore.translate_") + str);
    };
    //Langscore_Custom内のRuby関数の書き出し
    const auto functionDef = [&](auto str){
        str = utility::removeExtension(str);
        return decltype(str)("def " + funcName(str) + nl);
    };
    //スクリプト名を目印として書き出す
    const auto functionComment = [&](auto str)
    {
        return decltype(str)("\t#========================================") + nl +
                             tab + "# " + str + nl +
                             tab + "#========================================" + nl;
    };
    constexpr char functionName[] = "Langscore.Translate_Script_Text";

    //======================================

    std::ofstream outFile(filePath);
    outFile << nl;
    outFile << "def " << functionName << nl;
    outFile << nl;

    config config;
    std::erase_if(scriptTranslatesMap, [this](const auto& x){
        return std::get<1>(x).empty() || GetScriptName(std::get<0>(x)).empty();
    });

    for(auto& pair : scriptTranslatesMap)
    {
        const auto& fileName = std::get<0>(pair);
        auto scriptName = GetScriptName(fileName);
        auto functionName = utility::cnvStr<std::string>(funcName(fileName));
        outFile << tab << functionName << tab << "#" << utility::toString(scriptName) << nl;
    }
    outFile << "end" << nl << nl;

    auto funcComment = config.usScriptFuncComment();

    for(auto& pair : scriptTranslatesMap)
    {
        const auto& fileName = std::get<0>(pair);
        auto scriptName = GetScriptName(fileName);
        outFile << functionComment(utility::toString(scriptName));
        outFile << functionDef(utility::cnvStr<std::string>(fileName));

        const auto& list = std::get<1>(pair);
        if(scriptName == u8"Vocab"){
            WriteVocab(outFile, list);
        }
        else
        {
            for(auto& line : list)
            {
                auto parsed = utility::split(line.scriptLineInfo, u8':');
                auto arg1 = utility::toString(parsed[0]);
                auto arg2 = utility::toString(parsed[1]);
                auto arg3 = utility::toString(parsed[2]);
#ifdef __cpp_lib_format
                auto filepath = std::vformat(funcComment, std::make_format_args(arg1, arg2, arg3));
                outFile << tab << "#" + filepath << nl;
#endif
                outFile << tab << "#original : " << utility::toString(line.original) << nl;
                outFile << tab << "#Langscore.translate_for_script(\"" << utility::toString(line.scriptLineInfo) << "\")" << nl;
                outFile << nl;
            }
        }
        outFile << "end " << nl;
        outFile << nl;
    }


    return Status_Success;
}


void langscore_custom_rb_writer::WriteVocab(std::ofstream& file, std::vector<TranslateText> texts)
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
        auto original = t.translates[def_lang];
        if(original.empty()) {
            original = t.original;
        }

        auto result = std::find_if(translates.begin(), translates.end(), [&original](const auto& x){
            return std::get<0>(x) == original && std::get<2>(x) == false;
        });
        if(result != translates.end())
        {
            auto varName = std::get<1>(*result);
            auto lvalue = "Vocab::" + utility::toString(varName) + ".replace";
            std::string space(maxVarLength - lvalue.length(), ' ');
            file << tab << "# " << utility::toString(t.scriptLineInfo) << "(" << utility::toString(t.original) << ")" << nl;
            file << tab << lvalue << space << " Langscore.translate_for_script(\"" << utility::toString(t.scriptLineInfo) << "\")";
            file << nl;
            std::get<2>(*result) = true;
        }
    }
}

std::u8string langscore_custom_rb_writer::GetScriptName(std::u8string fileName)
{
    using namespace std::string_literals;

    auto result = std::ranges::find_if(pluginInfoList, [&fileName](const auto& x){
        return x.filename == fileName;
    });
    if(result == pluginInfoList.end()){ return u8""s; }
    auto scriptName = (*result).name;
    if(scriptName == u8"langscore"){ return u8""s; }
    if(scriptName == u8"langscore_custom"){ return u8""s; }
    return scriptName;
}
