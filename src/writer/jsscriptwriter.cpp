#include "jsscriptwriter.h"
#include "config.h"
#include "utility.hpp"
#include "scripttextparser.hpp"
#include <fstream>
#include <format>

#include "csvwriter.h"
#include "../reader/javascriptreader.hpp"
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

//jsscriptwriter::jsscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
//    : writerbase(std::move(langs), std::vector<TranslateText>{})
//    , scriptFileList(std::move(scriptFileList))
//{
//}


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
    //langscore_custom�̏����o��
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
    auto scriptInfoList = config.rpgMakerScripts();
    //auto scriptTranslatesMap = scriptTranslatesMap;

    //for(auto& pair : scriptTranslatesMap){
    //    std::get<1>(pair) = javascriptreader::applyIgnoreScripts(scriptInfoList, std::get<1>(pair));
    //}

    std::erase_if(this->scriptTranslatesMap, [this, &scriptInfoList](const auto& x){
        return std::get<1>(x).empty();
    });

    for(auto& pair : scriptTranslatesMap)
    {
        const auto& fileName = std::get<0>(pair);
        auto scriptName = fileName;
        auto functionName = utility::cnvStr<std::string>(funcName(fileName));
        outFile << tab << functionName << tab << "//" << utility::toString(scriptName) << nl;
    }
    outFile << "}" << nl << nl;

    auto funcComment = config.usScriptFuncComment();


    for(auto& pair : scriptTranslatesMap)
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

void langscore::jsscriptwriter::WriteVocab(std::ofstream& file, std::vector<TranslateText> texts)
{
    std::vector<std::tuple<std::u8string, std::u8string, bool>> translates = {
        { u8"%s �̌o���l���l���I", u8"ObtainExp", false },
        { u8"%s��%s�����΂����I", u8"Substitute", false },
        { u8"%s���o���I", u8"Emerge", false },
        { u8"%s����", u8"PartyName", false },
        { u8"%s�� %s �̃_���[�W��^�����I", u8"EnemyDamage", false },
        { u8"%s�ɂ͌����Ȃ������I", u8"ActionFailure", false },
        { u8"%s�Ƀ_���[�W��^�����Ȃ��I", u8"EnemyNoDamage", false },
        { u8"%s��%s�� %s �񕜂����I", u8"ActorRecovery", false },
        { u8"%s��%s�� %s �񕜂����I", u8"EnemyRecovery", false },
        { u8"%s��%s�� %s �������I", u8"ActorGain", false },
        { u8"%s��%s�� %s �������I", u8"EnemyGain", false },
        { u8"%s��%s�� %s �������I", u8"ActorLoss", false },
        { u8"%s��%s�� %s �������I", u8"EnemyLoss", false },
        { u8"%s��%s���オ�����I", u8"BuffAdd", false },
        { u8"%s��%s�����������I", u8"DebuffAdd", false },
        { u8"%s��%s�����ɖ߂����I", u8"BuffRemove", false },
        { u8"%s��%s�� %s �D�����I", u8"EnemyDrain", false },
        { u8"%s�̏����I", u8"Victory", false },
        { u8"%s�̔����I", u8"CounterAttack", false },
        { u8"%s�� %s �̃_���[�W���󂯂��I", u8"ActorDamage", false },
        { u8"%s��%s %s �ɏオ�����I", u8"LevelUp", false },
        { u8"%s��%s�� %s �D��ꂽ�I", u8"ActorDrain", false },
        { u8"%s��%s���g�����I", u8"UseItem", false },
        { u8"%s�̓_���[�W���󂯂Ă��Ȃ��I", u8"ActorNoDamage", false },
        { u8"%s�͕s�ӂ����ꂽ�I", u8"Surprise", false },
        { u8"%s�͐���������I", u8"Preemptive", false },
        { u8"%s�͐킢�ɔs�ꂽ�B", u8"Defeat", false },
        { u8"%s�͍U�������킵���I", u8"Evasion", false },
        { u8"%s�͓����o�����I", u8"EscapeStart", false },
        { u8"%s�͖��@��ł��������I", u8"MagicEvasion", false },
        { u8"%s�͖��@�𒵂˕Ԃ����I", u8"MagicReflection", false },
        { u8"%s����ɓ��ꂽ�I", u8"ObtainItem", false },
        { u8"%s���o�����I", u8"ObtainSkill", false },
        { u8"������ %s\\\\G ��ɓ��ꂽ�I", u8"ObtainGold", false },
        { u8"�����������邱�Ƃ͂ł��Ȃ������I", u8"EscapeFailure", false },
        { u8"�ǂ̃t�@�C���ɃZ�[�u���܂����H", u8"SaveMessage", false },
        { u8"�ǂ̃t�@�C�������[�h���܂����H", u8"LoadMessage", false },
        { u8"��߂�", u8"ShopCancel", false },
        { u8"�t�@�C��", u8"File", false },
        { u8"�~�X�I�@%s�Ƀ_���[�W��^�����Ȃ��I", u8"EnemyNoHit", false },
        { u8"�~�X�I�@%s�̓_���[�W���󂯂Ă��Ȃ��I", u8"ActorNoHit", false },
        { u8"��S�̈ꌂ�I�I", u8"CriticalToEnemy", false },
        { u8"���p����", u8"ShopSell", false },
        { u8"�����Ă��鐔", u8"Possession", false },
        { u8"����%s�܂�", u8"ExpNext", false },
        { u8"���݂̌o���l", u8"ExpTotal", false },
        { u8"�ɍ��̈ꌂ�I�I", u8"CriticalToActor", false },
        { u8"�w������", u8"ShopBuy", false },
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
