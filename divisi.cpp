#include "divisi.h"
#include "deserializer.h"

#include "writer/csvwriter.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

using namespace langscore;

class divisi::Impl
{
public:
    deserializer deserializer;
};

divisi::divisi(QString appPath)
    : pImpl(std::make_unique<Impl>())
{
    pImpl->deserializer.setApplicationFolder(std::move(appPath));
    auto outPath = pImpl->deserializer.outputPath();
    QDirIterator it(outPath, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()){
        QFile::remove(it.next());
    }
}

divisi::~divisi(){}

void divisi::setProjectPath(QString projectPath)
{
    QDir dirs(projectPath);
    const auto files = dirs.entryList();

    auto type = deserializer::ProjectType::None;
    if(std::find_if(files.cbegin(), files.cend(), [](QString x){
        return x.contains(".rvproj2");
    }) != files.end()){
        type = deserializer::ProjectType::VXAce;
    }
    pImpl->deserializer.setProjectPath(type, projectPath);
}

void divisi::exec()
{
    pImpl->deserializer.exec();

    const auto deserializeOutPath = pImpl->deserializer.outputPath();
    QDirIterator it(deserializeOutPath, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);

//    const QStringList ignoreFile = {"Animations.json", "Tilesets.json"};
//    while (it.hasNext())
//    {
//        auto path = it.next();
        
//        auto result = std::find_if(ignoreFile.cbegin(), ignoreFile.cend(), [&path](const auto& x){
//            return path.contains(x);
//        });
//        if(result != ignoreFile.cend()){ continue; }
        
//        QFile loadFile(path);
//        if(loadFile.open(QIODevice::ReadOnly) == false){ continue; }
        
//        auto data = loadFile.readAll();
//        QJsonDocument json = QJsonDocument::fromJson(data);
        
//        csvwriter writer({QLocale::Japanese, QLocale::English, QLocale::Chinese}, json);
        
//        auto csvFilePath = loadFile.fileName().remove(".json") + ".csv";
//        writer.write(csvFilePath);
//    }
    
    
    qDebug() << "Write Script.";
    
    auto scriptLocalize = deserializeOutPath + "/Scripts.csv";
    QFile csvWrite(scriptLocalize);
    csvWrite.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writer(&csvWrite);
    writer.setEncoding(QStringConverter::Utf8);
    writer.setGenerateByteOrderMark(true);
    
    const QStringList ignoreScriptFile = {u8"TES基本.rb"};
//    csvwriter writer({QLocale::Japanese, QLocale::English, QLocale::Chinese}, json);
    QDirIterator scriptIt(deserializeOutPath, QStringList() << "*.rb", QDir::Files, QDirIterator::Subdirectories);
    while (scriptIt.hasNext())
    {
        auto path = scriptIt.next();
        auto result = std::find_if(ignoreScriptFile.cbegin(), ignoreScriptFile.cend(), [&path](const auto& x){
            return path.contains(x);
        });
        if(result != ignoreScriptFile.cend()){ continue; }
        
        QFile loadFile(path);
        if(loadFile.open(QIODevice::ReadOnly | QIODevice::Text) == false){ continue; }
        
        size_t lineCount = 0;
        bool rangeComment = false;
        while(loadFile.atEnd() == false)
        {
            lineCount++;
            QString line(loadFile.readLine());
            line = line.trimmed();
            
            if(line == "=begin"){ 
                rangeComment = true;
                continue;
            }
            else if(rangeComment){
                if(line == "=end"){
                    rangeComment = false;
                }
                else{ continue; }
            }
            
            if(line.isEmpty()){ continue; }
            if(line[0] == '#'){ continue; }
            
            if(line.contains("\""))
            {
                line.remove(0, line.indexOf("\""));
                line.remove(line.lastIndexOf("\"")+1, line.size());
                
                writer << line << ",,,," << loadFile.fileName() << " Line : " << lineCount << "\n";
            }
        }
    }
    qDebug() << "Finish Script.";
}
