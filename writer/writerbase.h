#ifndef WRITERBASE_H
#define WRITERBASE_H

#include <map>
#include "nlohmann/json.hpp"

class writerbase
{
public:
    
    struct TranslateText 
    {
        std::string original;
        std::string note;
        std::map<std::string, std::string> translates;
    };
    
    writerbase(std::vector<std::string> locales, nlohmann::json json);
    virtual ~writerbase();
    
    virtual bool write(std::string writePath) = 0;
    
protected:
    std::vector<std::string> useLangList;
    std::vector<TranslateText> texts;
    
    void addText(std::string original, std::string note = "");
    void json2tt(nlohmann::json json);
    
    bool checkEventCommandCode(const nlohmann::json& obj);
    void convertJArray(nlohmann::json arr, std::string parentClass = "", std::string arrayinKey = "");
    void convertJObject(nlohmann::json root);
    
};

#endif // WRITERBASE_H
