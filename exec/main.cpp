#include "divisi.h"
#include "../src/serialize_base.h"
#include "../src/reader/csvreader.h"
#include <iostream>

struct ARGS
{
	std::filesystem::path appPath;
	std::filesystem::path gameProjPath;
	std::filesystem::path configFile;
	bool analyze = false;
	bool write = false;
	langscore::OverwriteTextMode overwriteMode = langscore::OverwriteTextMode::LeaveOld;
};

ARGS analyzeOption(int argc, const char* argv[])
{
	ARGS args;
	args.appPath = argv[0];
	for(int i = 1; i < argc; ++i)
	{
		std::string_view str = argv[i];
		if(str.find("-i") != std::string_view::npos){
			++i;
			args.gameProjPath = argv[i];
		}
		else if(str.find("-c") != std::string_view::npos){
			++i;
			args.configFile = argv[i];
		}
		else if(str.find("--analyze") != std::string_view::npos){
			args.analyze = true;
		}
		else if(str.find("--write")){
			args.write = true;
		}
		else if(str.find("--leaveold")){
			args.overwriteMode = langscore::OverwriteTextMode::LeaveOld;
		}
		else if(str.find("--leaveoldnonblank")){
			args.overwriteMode = langscore::OverwriteTextMode::LeaveOldNonBlank;
		}
		else if(str.find("--overwritenew")){
			args.overwriteMode = langscore::OverwriteTextMode::OverwriteNew;
		}
		else if(str.find("--both")){
			args.overwriteMode = langscore::OverwriteTextMode::Both;
		}
	}

	return args;
}

int main(int argc, const char* argv[])
{
	if(argc < 2){
		return -1;
	}

	auto args = analyzeOption(argc, argv);
	if(args.analyze == false && args.write == false){
		return 0;
	}

	langscore::divisi divisi(args.appPath);

	if(args.analyze){
		divisi.prepareAnalyzeProject(argv[1]);
	}

	divisi.exec();

	return 0;
}