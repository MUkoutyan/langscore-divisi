#include "csvwriter.h"
#include <QFile>

#include <QDebug>

bool csvwriter::write(QString path)
{
    qDebug() << "CSV Output File : " << path;
//    auto csvFilePath = loadFile.fileName().remove(".json") + ".csv";
    QFile outputCSVFile(path);
    
    auto openMode = QIODevice::ReadWrite | QIODevice::Text;
    if(outputCSVFile.exists()){
        openMode |= QIODevice::Truncate;
    }
    
    if(outputCSVFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false){ return false; }
    QTextStream csvWriter(&outputCSVFile);
    csvWriter.setEncoding(QStringConverter::Utf8);
    csvWriter.setGenerateByteOrderMark(true);
    
    QStringList csvHeader = {"original"};
    for(auto& lang : this->useLangList){
        QLocale l(lang);
        csvHeader.append(l.name());
    }
    csvHeader.append("memo");
    csvWriter << csvHeader.join(",") << "\n";
    
    for(const auto& text : this->texts)
    {
        QStringList rowtext = {text.original};
        for(auto& t : text.translates){
            rowtext.append(t.second);
        }
        rowtext.append(text.note);
        csvWriter << rowtext.join(",") << "\n";
    }
    
    return true;
}
