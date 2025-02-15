#pragma once
#include <map>

namespace langscore
{
    enum class JsonKey : size_t
    {
        Languages,
        LanguageName,
        Enable,
        Disable,
        FontName,
        FontSize,
        FontPath,
        Global,
        Local,
        Project,
        Analyze,
        Write,
        Name,
        Ignore,
        IgnorePoints,
        Row,
        Col,
        WriteType,
        Text,
        IgnorePictures,
        DefaultLanguage,
        TmpDir,
        UsCustomFuncComment,
        ExportDirectory,
        ExportByLang,
        RPGMakerOutputPath,
        RPGMakerBasicData,
        RPGMakerScripts,
        OverwriteLangscore,
        OverwriteLangscoreCustom,
        PackingInputDir,
        PackingEnablePerLang,
        PackingPerLangOutputDir,

        ApplicationVersion,
        ConfigVersion,
        AttachLsTransType,
        ExportAllScriptStrings,
        EnableLanguagePatch,
        IsFirstExported,

        Validate,
        ValidateTextMode,
        ValidateSizeList,
        ValidateTextLength,
        ValidateTextWidth,
        ValidateCSVList,
        ValidateTextCategory,
        ControlCharList,

        NumKeys,
    };


    #define MAKE_KEYVALUE(k) {JsonKey::k, #k}

    static std::map<JsonKey, const char*> jsonKeys = {
        MAKE_KEYVALUE(Languages),
        MAKE_KEYVALUE(LanguageName),
        MAKE_KEYVALUE(Enable),
        MAKE_KEYVALUE(Disable),
        MAKE_KEYVALUE(FontName),
        MAKE_KEYVALUE(FontSize),
        MAKE_KEYVALUE(FontPath),
        MAKE_KEYVALUE(Global),
        MAKE_KEYVALUE(Local),
        MAKE_KEYVALUE(Project),
        MAKE_KEYVALUE(Analyze),
        MAKE_KEYVALUE(Write),
        MAKE_KEYVALUE(Name),
        MAKE_KEYVALUE(Ignore),
        MAKE_KEYVALUE(IgnorePoints),
        MAKE_KEYVALUE(Row),
        MAKE_KEYVALUE(Col),
        MAKE_KEYVALUE(WriteType),
        MAKE_KEYVALUE(Text),
        MAKE_KEYVALUE(IgnorePictures),
        MAKE_KEYVALUE(DefaultLanguage),
        MAKE_KEYVALUE(TmpDir),
        MAKE_KEYVALUE(UsCustomFuncComment),
        MAKE_KEYVALUE(ExportDirectory),
        MAKE_KEYVALUE(ExportByLang),
        MAKE_KEYVALUE(RPGMakerOutputPath),
        MAKE_KEYVALUE(RPGMakerBasicData),
        MAKE_KEYVALUE(RPGMakerScripts),
        MAKE_KEYVALUE(OverwriteLangscore),
        MAKE_KEYVALUE(OverwriteLangscoreCustom),
        MAKE_KEYVALUE(PackingInputDir),
        MAKE_KEYVALUE(PackingEnablePerLang),
        MAKE_KEYVALUE(PackingPerLangOutputDir),
        MAKE_KEYVALUE(ApplicationVersion),
        MAKE_KEYVALUE(ConfigVersion),
        MAKE_KEYVALUE(AttachLsTransType),
        MAKE_KEYVALUE(ExportAllScriptStrings),
        MAKE_KEYVALUE(EnableLanguagePatch),
        MAKE_KEYVALUE(IsFirstExported),
        MAKE_KEYVALUE(Validate),
        MAKE_KEYVALUE(ValidateTextMode),
        MAKE_KEYVALUE(ValidateSizeList),
        MAKE_KEYVALUE(ValidateTextLength),
        MAKE_KEYVALUE(ValidateTextWidth),
        MAKE_KEYVALUE(ValidateCSVList),
        MAKE_KEYVALUE(ValidateTextCategory),
        MAKE_KEYVALUE(ControlCharList)
    };

    static const char* configKey(JsonKey key)
    {
        if(jsonKeys.find(key) == jsonKeys.end()) { return nullptr; }
        return jsonKeys[key];
    }
}