#ifndef WRITERBASE_H
#define WRITERBASE_H

#include <tuple>
#include <map>
#include <filesystem>
#include "nlohmann/json.hpp"

class writerbase
{
public:
    
    struct TranslateText 
    {
        std::u8string original;
        std::u8string note;
        std::map<std::string, std::u8string> translates;
    };
    
    writerbase(std::vector<std::string> locales, const nlohmann::json& json);
    virtual ~writerbase();
    
    virtual bool write(std::filesystem::path writePath) = 0;
    
protected:
    std::vector<std::string> useLangList;
    std::vector<TranslateText> texts;

    void writeU8String(std::ofstream& out, std::u8string text);
    
    void addText(const nlohmann::json& json, std::u8string note = u8"");
    void json2tt(const nlohmann::json& json);
    
    std::tuple<std::u8string, bool> getObjectClass(const nlohmann::json& root);
    bool checkIgnoreKey(const std::u8string& currentClassName, const std::u8string& key, bool hasSpecIgnoreKeys);

    bool checkEventCommandCode(const nlohmann::json& obj);
    void convertJArray(const nlohmann::json& arr, std::u8string parentClass = u8"", std::u8string arrayinKey = u8"");
    void convertJObject(const nlohmann::json& root);
    
};

#endif // WRITERBASE_H
