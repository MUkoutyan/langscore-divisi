#include "divisi_vxace.h"
#include "config.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/csvwriter.h"
#include "../reader/csvreader.h"

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace
{
    constexpr char Script_File_Name[] = "unison.rb";

    std::unordered_map<std::u8string, std::u8string> Help_Text = {
        { u8"en",    u8"The currently selected language is displayed." },
        { u8"es",    u8"Se muestra el idioma actualmente seleccionado." },
        { u8"de",    u8"Die aktuell ausgewählte Sprache wird angezeigt." },
        { u8"fr",    u8"La langue actuellement sélectionnée s'affiche." },
        { u8"it",    u8"Viene visualizzata la lingua attualmente selezionata." },
        { u8"ja",    u8"現在選択中の言語が表示されます。" },
        { u8"ko",    u8"현재 선택한 언어가 표시됩니다." },
        { u8"ru",    u8"Отображается текущий выбранный язык." },
        { u8"zh-sc", u8"显示当前选择的语言。" },
        { u8"zh-tc", u8"顯示當前選擇的語言。" },
    };

    std::unordered_map<std::u8string, std::u8string> Language_Items = {
        { u8"en",    u8"English" },
        { u8"es",    u8"Español" },
        { u8"de",    u8"Deutsch" },
        { u8"fr",    u8"Français" },
        { u8"it",    u8"Italiano" },
        { u8"ja",    u8"日本語" },
        { u8"ko",    u8"한국어" },
        { u8"ru",    u8"Русский язык" },
        { u8"zh-sc", u8"中文(簡体)" },
        { u8"zh-tc", u8"中文(繁体)" },
    };

    const auto nl = u8"\n"s;
    const auto tab = u8"\t"s;
}

divisi_vxace::divisi_vxace()
    : platform_base()
{
    config config;
    this->supportLangs = utility::convert<std::string, std::u8string>(config.languages());
    this->setIgnoreScriptPath(utility::convert<std::string, std::filesystem::path>(config.vxaceIgnoreScripts()));
}

divisi_vxace::~divisi_vxace(){}

void divisi_vxace::setProjectPath(std::filesystem::path path)
{
    this->deserializer.setProjectPath(deserializer::ProjectType::VXAce, std::move(path));

    fs::path outPath = this->deserializer.outputTmpPath();
    if(fs::exists(outPath)){
        fs::remove_all(outPath);
    }
}

std::filesystem::path langscore::divisi_vxace::outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir)
{
    if(dir.empty()){
        config config;
        dir = config.rpgmakerOutputPath();
    }
    const auto& projectPath = this->deserializer.projectPath();

    fs::path to = projectPath / dir;

    if(std::filesystem::exists(to) == false){
        std::filesystem::create_directories(to);
    }

    return to /= fileName;
}

void divisi_vxace::prepareAnalyzeProject()
{
    this->deserializer.exec();

    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    fs::recursive_directory_iterator dataItr(deserializeOutPath);
    for(auto& f : dataItr)
    {
        auto extension = f.path().extension();
        if(extension == ".json"){
            this->dataFileList.emplace_back(f.path());
        }
        else if(extension == ".rb"){
            this->scriptFileList.emplace_back(f.path());
        }
    }

    auto projectPath = this->deserializer.projectPath();
    auto graphicsPath = projectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }

        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        auto relative_path = f.path().lexically_relative(projectPath);
        this->graphicFileList.emplace_back(relative_path.replace_extension(""));
    }

}

void langscore::divisi_vxace::convert()
{    
    this->convertRvData();
    this->convertRvScript();
    this->convertGraphFileNameData();
    this->copyData();
}

void langscore::divisi_vxace::setIgnoreScriptPath(utility::filelist ignoreScriptPath)
{
    this->ignoreScriptPath = std::move(ignoreScriptPath);
    const auto AddIgnore = [&](fs::path file){
        auto result = std::find(this->ignoreScriptPath.begin(), this->ignoreScriptPath.end(), file);
        if(result == this->ignoreScriptPath.end()){
            this->ignoreScriptPath.emplace_back(file);
        }
    };
    AddIgnore(Script_File_Name);
    AddIgnore("unison_custom.rb");
}

void divisi_vxace::convertRvData()
{
    std::cout << "convertRvData" << std::endl;

    auto writeRvCsv = [this](fs::path path)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = path.make_preferred().replace_extension(".csv");
        writeTranslateText<csvwriter>(csvFilePath, json, langscore::OverwriteTextMode::LeaveOldNonBlank);

#ifndef _DEBUG
        fs::remove(path);
#endif
    };

    const utility::stringlist ignoreFile = {"Animations.json", "Tilesets.json"};
    for(auto& path : dataFileList)
    {
        auto result = std::find_if(ignoreFile.cbegin(), ignoreFile.cend(), [f = path.filename()](const auto& x){
            return f == x;
        });
        if(result != ignoreFile.cend()){ continue; }

        writeRvCsv(path);
    }
    std::cout << "Finish." << std::endl;
}


void divisi_vxace::convertRvScript()
{
    std::cout << "convertRvScript" << std::endl;

    auto scriptList = scriptFileList;
    auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [this](auto path){
        auto osPath = path.filename();
        auto result = std::find(this->ignoreScriptPath.cbegin(), this->ignoreScriptPath.cend(), osPath);
        return result != this->ignoreScriptPath.cend();
    });
    scriptList.erase(rm_result, scriptList.end());

    //Rubyスクリプトを予め解析してテキストを生成しておく。
    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    auto& scriptTrans = scriptWriter.getScriptTexts();
    auto& transTexts = scriptWriter.curerntTexts();

    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
    }

    //デフォルトスクリプトのVocab.rb内の文字列は予め対応
    auto resourceFolder = this->appPath.parent_path() / "resource";
    csvreader reader;
    auto vocabs = reader.parse(resourceFolder/"vocab.csv");
    for(auto& pathPair : scriptTrans)
    {
        if(pathPair.first.filename() != "Vocab.rb"){ continue; }

        const auto searchFunc = [&](const auto& x){
            return x.memo.find(u8"Vocab.rb") != std::u8string::npos;
        };
        auto begin = std::find_if(transTexts.begin(), transTexts.end(), searchFunc);
        auto end = std::find_if(transTexts.rbegin(), transTexts.rend(), searchFunc).base();

        std::for_each(begin, end, [&](auto& texts)
        {
            auto result = std::find_if(vocabs.begin(), vocabs.end(), [&texts](const auto& x){
                return x.original == texts.original;
            });
            if(result == vocabs.end()){ return; }
            for(const auto& key : this->supportLangs){
                if(result->translates.find(key) != result->translates.end()){
                    texts.translates[key] = result->translates[key];
                }
                else if(key == u8"ja"){
                    texts.translates[key] = result->original;
                }
            }
        });
        break;
    }
    //=================================

    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    auto outputPath = deserializeOutPath / "Scripts";
    if(std::filesystem::exists(outputPath) == false){
        std::filesystem::create_directories(outputPath);
    }

    for(auto& t : transTexts){
        t.memo.swap(t.original);
    }

#ifdef _DEBUG
    for(auto& txt : transTexts){
        for(auto& tl : txt.translates)
        {
            auto t = tl.second;
            if(def_lang != tl.first)
            {
                t = txt.memo;
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
#endif

    writeTranslateText<csvwriter>(outputPath, transTexts, OverwriteTextMode::LeaveOld, false);
    
    if(fs::exists(outputPath / "unison_custom.rb") == false){
        writeTranslateText<rbscriptwriter>(outputPath / "unison_custom.rb", scriptList, langscore::OverwriteTextMode::LeaveOldNonBlank);
    }

    //unison.rbの出力
    auto outputScriptFilePath = outputPath / Script_File_Name;
    fs::copy(resourceFolder / Script_File_Name, outputScriptFilePath, std::filesystem::copy_options::overwrite_existing);

    auto fileLines = formatSystemVariable(outputScriptFilePath);

    if(fs::exists(outputScriptFilePath))
    {
        std::ofstream outScriptFile(outputScriptFilePath, std::ios_base::trunc);
        for(const auto& l : fileLines){
            outScriptFile << utility::toString(l) << "\n";
        }
    }

    std::cout << "Finish." << std::endl;
}

utility::u8stringlist divisi_vxace::formatSystemVariable(std::filesystem::path path)
{
    std::ifstream inScriptFile(path);

    std::string _linTmp;
    utility::u8stringlist result;
    config config;
    auto defLanguage = config.defaultLanguage();

    const auto findStr = [](const std::u8string& _line, std::u8string_view str){
        return _line.find(str) != std::u8string::npos;
    };

    while(std::getline(inScriptFile, _linTmp))
    {
        std::u8string _line(_linTmp.begin(), _linTmp.end());
        if(findStr(_line, u8"%{SUPPORT_LANGUAGE}%"))
        {
            auto list = this->supportLangs;
            for(auto& t : list){ t = u8"\"" + t + u8"\""; }
            auto langs = utility::join(list, u8","s);
            _line = tab + u8"SUPPORT_LANGUAGE = [" + langs + u8"]";
        }
        else if(findStr(_line, u8"%{DEFAULT_LANGUAGE}%")){
            _line = tab + u8"DEFAULT_LANGUAGE = \"" + utility::cnvStr<std::u8string>(defLanguage) + u8"\"";
        }
        else if(findStr(_line, u8"%{SUPPORT_FONTS}%"))
        {
            auto fonts = config.vxaceFonts();
            _line = tab + u8"LS_FONT = {" + nl;
            for(auto& pair : fonts)
            {
                auto lang = utility::cnvStr<std::u8string>(pair.first);
                auto sizeStr = utility::cnvStr<std::u8string>(std::to_string(pair.second.size));
                _line += tab + tab;
                _line += u8"\"" + lang + u8"\" => {:name => \"" + pair.second.name + u8"\", :size => " + sizeStr + u8"}," + nl;
            }
            _line += tab + u8"}\n";
        }
        else if(findStr(_line, u8"%{TRANSLATE_FOLDER}%"))
        {
            auto outPath = config.rpgmakerOutputPath();
            _line = tab + u8"TRANSLATE_FOLDER = \"./" + utility::cnvStr<std::u8string>(outPath) + u8"\"" + nl;
        }
        else if(findStr(_line, u8"%{UNISON_LSCSV}%"))
        {
            auto resourceFolder = this->appPath.parent_path() / "resource";
            std::stringstream ss;
            std::ifstream lscsv(resourceFolder/"lscsv.rb");
            assert(lscsv.good());
            ss << lscsv.rdbuf();
            lscsv.close();
            _line = utility::cnvStr<std::u8string>(ss.str());
        }
        else if(findStr(_line, u8"%{SYSTEM1}%"))
        {
            _line  = tab + u8"SYSTEM1 = \"" + ::Help_Text[u8"ja"] + u8"\"" + nl;
            _line += tab + u8"$langscore_system = {" + nl;
            _line += tab + tab + u8"SYSTEM1 => {" + nl;
            for(auto& l : this->supportLangs){
                if(::Help_Text.find(l) != ::Help_Text.end()){
                    _line += tab + tab + tab;
                    _line += u8"\"" + l + u8"\" => \"" + ::Help_Text[l] + u8"\"," + nl;
                }
            }
            _line += tab + tab + u8"}" + nl;
            _line += tab + u8"}";
        }
        else if(findStr(_line, u8"%{SYSTEM2}%"))
        {
            _line = u8"\tSYSTEM2 = {\n";
            for(auto& l : this->supportLangs){
                if(::Language_Items.find(l) != ::Language_Items.end()){
                    _line += u8"\t\t\"" + l + u8"\" => \"" + ::Language_Items[l] + u8"\",\n";
                }
            }
            _line += u8"\t}";
        }
        result.emplace_back(std::move(_line));
    }
    inScriptFile.close();

    return result;
}

void divisi_vxace::convertGraphFileNameData()
{
    std::cout << "convertGraphFileNameData" << std::endl;

    std::vector<TranslateText> transTextList;
    for(auto& f : graphicFileList){
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    const auto& projectPath = deserializer.projectPath();
    config config;
    writeTranslateText<csvwriter>(projectPath / config.rpgmakerOutputPath() / "Graphics.csv", transTextList, OverwriteTextMode::LeaveOld, false);
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::copyData(langscore::OverwriteTextMode option)
{
    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    fs::directory_iterator it(deserializeOutPath);
    //翻訳ファイルのコピー
    for(auto& f : it)
    {
        auto srcPath = f.path().filename();
        if(f.is_directory() && srcPath == "Script"){
            continue;
        }
        if(srcPath.extension().string().find(csvwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath);

        auto fsOption = convertCopyOption(option);
        fs::copy(f, to, fsOption);
    }

    //rbスクリプトのコピー
    fs::directory_iterator it2{deserializeOutPath / "Scripts"};
    for(auto& f : it2)
    {
        auto srcPath = f.path().filename();
        //unison以外は無視
        if(srcPath != "unison_custom.rb" && srcPath != Script_File_Name){
            continue;
        }
        if(srcPath.extension().string().find(rbscriptwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath, "Scripts");
        auto fsOption = convertCopyOption(option);
        fs::copy(f, to, fsOption);
    }
}