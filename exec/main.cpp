#include "divisi.h"
#include "../src/serialize_base.h"
#include "../src/reader/csvreader.h"
#include <iostream>

struct ARGS
{
	std::filesystem::path appPath;
	std::filesystem::path configFile;
	bool analyze = false;
	bool write = false;
	bool validate = false;
	bool packing = false;
	langscore::OverwriteTextMode overwriteMode = langscore::OverwriteTextMode::LeaveOld;
};

ARGS analyzeOption(int argc, const char* argv[])
{
	ARGS args;
	args.appPath = argv[0];
	for(int i = 1; i < argc; ++i)
	{
		std::string_view str = argv[i];
		if(str.find("-c") != std::string_view::npos){
			++i;	//次の要素を読み込む
			args.configFile = argv[i];
		}
		else if(str.find("--analyze") != std::string_view::npos){
			args.analyze = true;
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
		else if(str.find("--leaveold") != std::string_view::npos){
			args.overwriteMode = langscore::OverwriteTextMode::LeaveOld;
		}
		else if(str.find("--leaveoldnonblank") != std::string_view::npos){
			args.overwriteMode = langscore::OverwriteTextMode::LeaveOldNonBlank;
		}
		else if(str.find("--overwritenew") != std::string_view::npos){
			args.overwriteMode = langscore::OverwriteTextMode::OverwriteNew;
		}
		else if(str.find("--both") != std::string_view::npos){
			args.overwriteMode = langscore::OverwriteTextMode::Both;
		}
	}
	return args;
}

int main(int argc, const char* argv[])
{
	if(argc < 2){
#ifdef _DEBUG
		std::cerr << "Need Argments..." << std::endl;
#endif
		return -1;
	}

	const auto args = analyzeOption(argc, argv);

	langscore::divisi divisi(args.appPath, args.configFile);

	ErrorStatus result;
	if(args.analyze){
		result = divisi.analyze();
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