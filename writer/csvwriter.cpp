#include "csvwriter.h"
#include "utility.hpp"
#include <filesystem>
#include <fstream>

bool csvwriter::write(std::string _path)
{
    namespace fs = std::filesystem;
    //qDebug() << "CSV Output File : " << path;
//    auto csvFilePath = loadFile.fileName().remove(".json") + ".csv";
    fs::path path(_path);
   
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return false; }
    
    std::vector<std::string> csvHeader = {"original"};
    for(auto& lang : this->useLangList){
        csvHeader.emplace_back(lang);
    }
    csvHeader.emplace_back("memo");
    outputCSVFile << utility::join(csvHeader, ",") << "\n";
    
    for(const auto& text : this->texts)
    {
        std::vector<std::string> rowtext = {text.original};
        for(auto& t : text.translates){
            rowtext.emplace_back(t.second);
        }
        rowtext.emplace_back(text.note);
        
        outputCSVFile << utility::join(rowtext, ",") << "\n";
    }
    
    return true;
}
