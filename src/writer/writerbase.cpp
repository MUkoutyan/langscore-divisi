#include "writerbase.h"


namespace {

static const std::vector<std::string> ignoreKeys = {
    "json_class", "@note", "@character_name", "@self_switch_ch", "@switches", "@title1_name"
};
static const std::map<std::string, std::vector<std::string>> ignoreForClassKeys = {
    {"RPG::UsableItem::Damage", {"@formula"}},
    {"RPG::Event",  {"@name"}},
    {"RPG::SE",     {"@name"}},
    {"RPG::ME",     {"@name"}},
    {"RPG::BGM",    {"@name"}},
    {"RPG::Map",    {"@battleback1_name", "@battleback2_name"}},
    {"RPG::Troop",  {"@name"}}
};

}

writerbase::writerbase(std::vector<std::string> locales, nlohmann::json json)
    : useLangList(std::move(locales))
{
    json2tt(std::move(json));
}

writerbase::~writerbase(){
}

void writerbase::addText(std::string original, std::string note)
{
    if(original.empty()){ return; }
    
    TranslateText t;
    t.original = std::move(original);
    t.note     = std::move(note);
    for(auto& lang : useLangList){
        t.translates[lang] = "";
    }
    auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
        return x.original == t.original;
    });
    
    if(result == texts.end()){
        texts.emplace_back(std::move(t));
    }
    else{
//        result->note += std::string("同じテキストが複数で使用されています");
    }
}

void writerbase::json2tt(nlohmann::json json)
{
    if(json.is_array())
    {
        convertJArray(json.array());
    }
    else if(json.is_object())
    {
        convertJObject(json.object());
    }
}

bool writerbase::checkEventCommandCode(const nlohmann::json& obj)
{
    for(auto s=obj.begin(); s!=obj.end(); ++s)
    {
        if(s.key() == "@code"){
            //許可するコード
            int v = 0;
            s->get_to(v);
            if(v == 401){
                return true;
            }
            else if(v == 231){
                return true;
            }
        }
    }
    return false;
}

void writerbase::convertJArray(nlohmann::json arr, std::string parentClass, std::string arrayinKey)
{
    for(auto s=arr.begin(); s!=arr.end(); ++s)
    {
        if(s->is_array()){
            convertJArray(s->array(), parentClass, arrayinKey);
            continue; 
        }
        else if(s->is_object()){
            convertJObject(s->object());
        }
        else if(s->is_string()){
            std::string text;
            s->get_to(text);
            addText(text, parentClass + "=>" + arrayinKey);
        }
    }
}

void writerbase::convertJObject(nlohmann::json root)
{
    std::string currentClassName = "";
    bool hasSpecIgnoreKeys = false;
    for(auto s=root.begin(); s!=root.end(); ++s)
    {
        if(s.key() == "json_class"){
            s->get_to(currentClassName);
            hasSpecIgnoreKeys = ignoreForClassKeys.find(currentClassName) != ignoreForClassKeys.end();
            break;
        }
    }
    
    
    if(currentClassName == "RPG::EventCommand"){
        if(checkEventCommandCode(root) == false){ return; }
    }
    
    for(auto s=root.begin(); s!=root.end(); ++s)
    {
        const auto& key = s.key();
        const auto& val = s.value();
        if(val.is_array()){ 
            convertJArray(val.array(), currentClassName, key); 
            continue;
        }
        else if(val.is_object()){
            convertJObject(val.object());
            continue;
        }
        else if(val.is_string() == false){ continue; }
        
        if(std::find(ignoreKeys.cbegin(), ignoreKeys.cend(), key) != ignoreKeys.cend()){ continue; }
        if(hasSpecIgnoreKeys){
            const auto& fields = ignoreForClassKeys.at(currentClassName);
            if(std::find(fields.cbegin(), fields.cend(), key) != fields.cend()){
                continue;
            }
        }
        
        std::string valStr;
        s->get_to(valStr);
        if(valStr.empty()){ continue; }
        
        if(valStr.find('\n') != std::string::npos){
            valStr = "\"" + valStr + "\"";
        }
        
        std::string text;
        s->get_to(text);
        addText(text, currentClassName + ":" + key);
    }
}
