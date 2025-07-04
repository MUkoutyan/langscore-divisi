#include "divisi.h"
#include "../src/serialize_base.h"
#include "../src/reader/csvreader.h"
#include <iostream>
#include "versioninfo.h"
#include "config.h"

#include <exception>
#include <cstdlib>
#include <csignal>
#include "converter/to_patch_csv.h"

#if defined(_MSC_VER)
// MSVC (VS2022) 用: C++23 の std::stacktrace を利用
#include <stacktrace>
void print_stacktrace() {
    std::cerr << "[Stack Trace]:\n"
        << std::stacktrace::current()
        << std::endl;
}
#else
// clang++ (Ubuntu) 用: POSIX の execinfo.h を利用
#include <execinfo.h>
#include <unistd.h>
void print_stacktrace() {
    constexpr int max_frames = 50;
    void* frames[max_frames];
    int frame_count = backtrace(frames, max_frames);
    // backtrace_symbols() は動的に領域を確保するので free() する必要があります
    char** symbols = backtrace_symbols(frames, frame_count);
    if(symbols != nullptr) {
        std::cerr << "[Stack Trace]:" << std::endl;
        for(int i = 0; i < frame_count; ++i) {
            std::cerr << symbols[i] << std::endl;
        }
        free(symbols);
    }
}
#endif

// シグナルハンドラ（例: SIGSEGV, SIGABRT など）
void crash_handler(int signum) {
    std::cerr << "Crash signal (" << signum << ") received.\n";
    print_stacktrace();
    std::abort();
}

// std::terminate ハンドラ（未捕捉例外発生時）
void _terminate_handler() {
    std::cerr << "std::terminate was called due to an UNCAUGHT EXCEPTION.\n";
    print_stacktrace();
    std::abort();
}

void setup_crash_handlers() {
    // シグナルハンドラの設定
    std::signal(SIGSEGV, crash_handler);
    std::signal(SIGABRT, crash_handler);
    // 必要に応じて SIGFPE, SIGILL, SIGTERM なども追加可能

    // std::terminate ハンドラの設定
    std::set_terminate(_terminate_handler);
}

struct ARGS
{
	std::filesystem::path appPath;
	std::filesystem::path configFile;
	std::filesystem::path gameProjectPath;
    std::filesystem::path convetCsvFolderPath;
    bool createConfigFile = false;
	bool analyze          = false;
	bool reanalysis       = false;
    bool updatePlugin     = false;
	bool exportCSV        = false;
	bool validate         = false;
	bool packing          = false;
    bool convertPatchCSV  = false;
    bool convertBoundCSV  = false;
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

    auto last_char = *(path.rbegin());
    if(last_char == '\\' || last_char == '/') {
        path.erase(path.size() - 1, 1);
    }
    if(*(path.begin()) == '\"' && last_char == '\"') {
        path = path.substr(1, path.size() - 2); // "で括られている場合は除去
    }
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
		else if(str.find("--reanalysis") != std::string_view::npos){
			args.reanalysis = true;
		}
        else if(str.find("--updatePlugin") != std::string_view::npos) {
            args.updatePlugin = true;
        }
		else if(str.find("--exportCSV") != std::string_view::npos){
			args.exportCSV = true;
		}
		else if(str.find("--validate") != std::string_view::npos){
			args.validate = true;
		}
		else if(str.find("--packing") != std::string_view::npos){
			args.packing = true;
		}
        else if(str.find("--convertPatchCSV") != std::string_view::npos) 
        {
            args.convertPatchCSV = true;
            ++i;	//次の要素を読み込む
            args.convetCsvFolderPath = getFilePathFromArgs(i, argc, argv);
            std::cout << args.convetCsvFolderPath << " ";
        }
        else if(str.find("--convertBoundCSV") != std::string_view::npos)
        {
            args.convertBoundCSV = true;
            ++i;	//次の要素を読み込む
            args.convetCsvFolderPath = getFilePathFromArgs(i, argc, argv);
            std::cout << args.convetCsvFolderPath << " ";
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
    setup_crash_handlers();
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

    if(std::filesystem::exists(args.configFile) == false) {
        std::cerr << "Config file does not exist: " << args.configFile << std::endl;
        return -1;
    }

	ErrorStatus result;
    if(args.createConfigFile) {
        result = divisi.createConfig(args.gameProjectPath);
    }

    if(args.convertPatchCSV) {
        langscore::convert_patch_csv{args.convetCsvFolderPath};
        if(result.invalid()) {
            std::cerr << "convertPatchCSV was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
    }
    else if(args.convertBoundCSV) {
        langscore::convert_bound_csv{args.convetCsvFolderPath};
        if(result.invalid()) {
            std::cerr << "convertBoundCSV was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
    }

	if(args.analyze){
		result = divisi.analyze();
        if(result.invalid()) {
            std::cerr << "analyze was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
	}
	if(args.reanalysis){
		result = divisi.reanalysis();
        if(result.invalid()) {
            std::cerr << "reanalysis was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
	}
    if(args.updatePlugin) {
        result = divisi.updatePlugin();
        if(result.invalid()) {
            std::cerr << "reanalysis was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
    }
	if(args.exportCSV){
		result = divisi.exportCSV();
        if(result.invalid()) {
            std::cerr << "exportCSV was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
	}
	if(args.validate){
		result = divisi.validate();
        if(result.invalid()) {
            std::cerr << "validate was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
	}
	if(args.packing){
		result = divisi.packing();
        if(result.invalid()) {
            std::cerr << "packing was invalid." << std::endl;
            std::cerr << result.toStr() << std::endl;
            return result.val();
        }
	}

    std::cout << "Complete." << std::endl;
	return 0;
}