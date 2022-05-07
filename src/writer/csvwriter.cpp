#include "csvwriter.h"
#include "utility.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
bool csvwriter::write(fs::path _path)
{
    if(this->texts.empty()){ return false; }
    fs::path path(_path);
   
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return false; }
    
    std::vector<std::u8string> csvHeader = {u8"original"};
    for(auto& lang : this->useLangList){
        csvHeader.emplace_back(lang.begin(), lang.end());
    }
    csvHeader.emplace_back(u8"memo");
    const std::u8string delimiter(u8",");
    writeU8String(outputCSVFile, utility::join(csvHeader, delimiter));
    outputCSVFile << "\n";
    
    for(const auto& text : this->texts)
    {
        utility::u8stringlist rowtext = {text.original};
        for(auto& t : text.translates){
            rowtext.emplace_back(t.second);
        }
        rowtext.emplace_back(text.note);

        writeU8String(outputCSVFile, utility::join(rowtext, delimiter));
        outputCSVFile << "\n";
    }
    
    return true;
}
