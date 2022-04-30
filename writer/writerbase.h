#ifndef WRITERBASE_H
#define WRITERBASE_H

#include <map>
#include <QString>
#include <QLocale>
#include <QJsonDocument>

class writerbase
{
public:
    
    struct TranslateText 
    {
        QString original;
        QString note;
        std::map<QLocale::Language, QString> translates;
    };
    
    writerbase(std::vector<QLocale::Language> locales, QJsonDocument json);
    virtual ~writerbase();
    
    virtual bool write(QString writePath) = 0;
    
protected:
    std::vector<QLocale::Language> useLangList;
    std::vector<TranslateText> texts;
    
    void addText(QString original, QString note = "");
    void json2tt(QJsonDocument json);
    
    bool checkEventCommandCode(const QJsonObject& obj);
    void convertJArray(QJsonArray arr, QString parentClass = "", QString arrayinKey = "");
    void convertJObject(QJsonObject root);
    
};

#endif // WRITERBASE_H
