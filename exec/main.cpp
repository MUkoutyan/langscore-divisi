#include "divisi.h"
#include "../src/serialize_base.h"
#include "../src/reader/csvreader.h"
#include <iostream>
#include "versioninfo.h"
#include "config.h"

struct ARGS
{
	std::filesystem::path appPath;
	std::filesystem::path configFile;
	std::filesystem::path gameProjectPath;
    bool createConfigFile = false;
	bool analyze = false;
	bool update = false;
	bool write = false;
	bool validate = false;
	bool packing = false;
    bool outputTestScript = false;
	langscore::MergeTextMode overwriteMode = langscore::MergeTextMode::AcceptSource;
    langscore::config::ProjectType projectType = langscore::config::ProjectType::None;
};

std::filesystem::path getFilePathFromArgs(int& i, const int argc, const char* argv[])
{
    std::string path = argv[i];

    if(path.empty()) { return ""; }
    //"で括られていなければそのまま解釈。
    if(path[0] != '\"') {
        return path;
    }
    //"で括られている場合でも、取得した文字列が"で終わるなら
    //空白文字が無いパターンなので、そのまま解釈。
    if(*(path.rbegin()) == '\"') {
        path.erase(path.size() - 1, 1);
        path.erase(path.begin());
        return path;
    }

    i += 1;
    for(; i < argc; ++i) {
        std::string nextPath = argv[i];
        path += " " + nextPath;
        if(*(nextPath.rbegin()) == '\"') {
            break;
        }
    }

    if(*(path.rbegin()) == '\\' || *(path.rbegin()) == '/') {
        path.erase(path.size() - 1, 1);
    }
    path.erase(path.begin());
    return path;
}

ARGS analyzeOption(int argc, const char* argv[])
{
	ARGS args;
	args.appPath = argv[0];
	std::cout << "Options : ";
	for(int i = 1; i < argc; ++i)
	{
		std::string_view str = argv[i];
		std::cout << str << " ";
		if(str == "-c"){
			++i;	//次の要素を読み込む
            args.configFile = getFilePathFromArgs(i, argc, argv);
			std::cout << args.configFile << " ";

		}
		else if(str.find("--analyze") != std::string_view::npos){
			args.analyze = true;
		}
		else if(str.find("--update") != std::string_view::npos){
			args.update = true;
		}
		else if(str.find("--write") != std::string_view::npos){
			args.write = true;
		}
		else if(str.find("--validate") != std::string_view::npos){
			args.validate = true;
		}
		else if(str.find("--packing") != std::string_view::npos){
			args.packing = true;
		}
        else if(str.find("--createConfigFile") != std::string_view::npos) {
            args.createConfigFile = true;
            ++i;	//次の要素を読み込む
            args.gameProjectPath = getFilePathFromArgs(i, argc, argv);
			std::cout << args.gameProjectPath << " ";
        }
        
	}
	std::cout << std::endl;
	return args;
}

int main(int argc, const char* argv[])
{
	std::cout << "Langscore Divisi Version " << VER_FILEVERSION_STR << std::endl;
	std::cout << "Build on " << __DATE__ << " " << __TIME__ << std::endl;
	if(argc < 2){
#ifdef _DEBUG
		std::cerr << "Need Argments..." << std::endl;
#endif
		return -1;
	}

	const auto args = analyzeOption(argc, argv);

	langscore::divisi divisi(args.appPath, args.configFile);

	ErrorStatus result;
    if(args.createConfigFile) {
        result = divisi.createConfig(args.gameProjectPath);
    }

	if(args.analyze){
		result = divisi.analyze();
	}
	if(args.update){
		result = divisi.update();
	}
	if(args.write){
		result = divisi.write();
	}
	if(args.validate){
		result = divisi.validate();
	}
	if(args.packing){
		result = divisi.packing();
	}
	if(result.invalid()){
		std::cerr << result.toStr() << std::endl;
		return result.val();
	}
	

	return 0;
}