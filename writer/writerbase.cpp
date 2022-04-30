#include "writerbase.h"

#include <QJsonObject>
#include <QJsonArray>

namespace {

static const QStringList ignoreKeys = {
    "json_class", "@note", "@character_name", "@self_switch_ch", "@switches", "@title1_name"
};
static const std::map<QString, std::vector<QString>> ignoreForClassKeys = {
    {"RPG::UsableItem::Damage", {"@formula"}},
    {"RPG::Event",  {"@name"}},
    {"RPG::SE",     {"@name"}},
    {"RPG::ME",     {"@name"}},
    {"RPG::BGM",    {"@name"}},
    {"RPG::Map",    {"@battleback1_name", "@battleback2_name"}},
    {"RPG::Troop",  {"@name"}}
};

}

writerbase::writerbase(std::vector<QLocale::Language> locales, QJsonDocument json)
    : useLangList(std::move(locales))
{
    json2tt(std::move(json));
}

writerbase::~writerbase(){
}

void writerbase::addText(QString original, QString note)
{
    if(original.isEmpty()){ return; }
    
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
//        result->note += QString("同じテキストが複数で使用されています");
    }
}

void writerbase::json2tt(QJsonDocument json)
{
    if(json.isArray())
    {
        convertJArray(json.array());
    }
    else if(json.isObject())
    {
        convertJObject(json.object());
    }
}

bool writerbase::checkEventCommandCode(const QJsonObject &obj)
{
    for(auto s=obj.begin(); s!=obj.end(); ++s)
    {
        if(s.key() == "@code"){
            //許可するコード
            auto v = s.value().toInt();
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

void writerbase::convertJArray(QJsonArray arr, QString parentClass, QString arrayinKey)
{
    for(auto s=arr.begin(); s!=arr.end(); ++s)
    {
        if(s->isArray()){ 
            convertJArray(s->toArray(), parentClass, arrayinKey);
            continue; 
        }
        else if(s->isObject()){
            convertJObject(s->toObject());
        }
        else if(s->isString()){
            addText(s->toString(), parentClass+"=>"+arrayinKey);
        }
    }
}

void writerbase::convertJObject(QJsonObject root)
{
    QString currentClassName = "";
    bool hasSpecIgnoreKeys = false;
    for(auto s=root.begin(); s!=root.end(); ++s)
    {
        if(s.key() == "json_class"){
            currentClassName = s.value().toString();
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
        if(val.isArray()){ 
            convertJArray(val.toArray(), currentClassName, key); 
            continue;
        }
        else if(val.isObject()){
            convertJObject(val.toObject());
            continue;
        }
        else if(val.isString() == false){ continue; }
        
        if(ignoreKeys.contains(key)){ continue; }
        if(hasSpecIgnoreKeys){
            const auto& fields = ignoreForClassKeys.at(currentClassName);
            if(std::find(fields.cbegin(), fields.cend(), key) != fields.cend()){
                continue;
            }
        }
        
        auto valStr = s->toString();
        if(valStr.isEmpty()){ continue; }
        
        if(valStr.contains('\n')){
            valStr = "\"" + valStr + "\"";
        }
        
        addText(s->toString(), currentClassName + ":" + key);
    }
}
