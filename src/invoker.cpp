#include "invoker.h"
#include "utility.hpp"

#include <iostream>
#include <fstream>
#include <array>
#include "../rvcnv/rvcnv_hash.cpp"
#include "md5.h"

using namespace langscore;

namespace
{
    //OS のパスの文字列型（Windows は std::wstring、それ以外は UTF-8 の std::string）
    using NativeString = std::filesystem::path::string_type;

    //プロセスを起動する直前に、引数を OS のコマンドラインにする。
    //path::string() は ANSI コードページに変換するため、その文字コードに無い文字を含むパスを渡せない。native() をそのまま使う
    NativeString ToCommandLine(const std::vector<std::filesystem::path>& args)
    {
        using Char = std::filesystem::path::value_type;
        NativeString commandLine;
        for(const auto& arg : args)
        {
            if(commandLine.empty() == false) { commandLine += Char(' '); }
            const auto& text = arg.native();
            if(text.find(Char(' ')) == NativeString::npos)
            {
                commandLine += text;
                continue;
            }
            //空白を含む引数は " で囲む。閉じの " の直前の \ は " のエスケープと解釈されるため 2 つにする
            commandLine += Char('"');
            commandLine += text;
            if(text.ends_with(Char('\\'))) { commandLine += Char('\\'); }
            commandLine += Char('"');
        }
        return commandLine;
    }
}

#ifdef WIN32
#include <process.h>
#include <Windows.h>
static int execProcess(NativeString commandLine)
{
    SECURITY_ATTRIBUTES securityAttributes;
    ZeroMemory(&securityAttributes, sizeof(securityAttributes));
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = NULL;

    HANDLE readPipe, writePipe;
    if(!CreatePipe(&readPipe, &writePipe, &securityAttributes, 0)) {
        std::cerr << "CreatePipe failed" << std::endl;
        return -1;
    }

    if(!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed" << std::endl;
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return -1;
    }

    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdError = writePipe;
    startupInfo.hStdOutput = writePipe;

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    //CreateProcessW はコマンドラインの文字列を書き換えることがあるため、書き込める領域を渡す
    if(CreateProcessW(NULL, commandLine.data(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo) == FALSE)
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")" << std::endl;
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return -1;
    }
    CloseHandle(writePipe);

    std::array<char, 128> buffer;
    DWORD bytesRead;
    while(ReadFile(readPipe, buffer.data(), buffer.size(), &bytesRead, NULL) && bytesRead > 0) {
        std::cout.write(buffer.data(), bytesRead);
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode;
    if(!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
        std::cerr << "GetExitCodeProcess failed (" << GetLastError() << ")" << std::endl;
        exitCode = -1;
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(readPipe);

    return exitCode;
}

#else

#include <unistd.h>
#include <sys/wait.h>
static int execProcess(NativeString commandLine) {
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(commandLine.c_str(), "r"), pclose);
    if(!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::cout << buffer.data();
    }
    return pclose(pipe.get());
}
#endif

invoker::invoker(config::ProjectType projectType)
    : projectType(projectType)
    , appPath("")
    , _projectPath("")
{
}

invoker::~invoker(){
}

void invoker::setApplicationFolder(std::filesystem::path path){
    appPath = std::move(path);
    appPath = appPath.parent_path();
}

void invoker::setProjectPath(std::filesystem::path path)
{
    std::error_code errorCode;
    if(std::filesystem::is_directory(path, errorCode) == false) {
        path = path.parent_path();
    }
    this->_projectPath = std::move(path);
}


ErrorStatus invoker::analyze(std::filesystem::path analyzeDir)
{
    //config config;
    //auto tempPath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
    auto tempPath = analyzeDir;
    tempPath.make_preferred();
    std::error_code errorCode;
    std::filesystem::create_directories(tempPath, errorCode);
    if(utility::FileSystemFailed(errorCode, "create_directories", tempPath)){
        return ErrorStatus(ErrorStatus::Module::INVOKER, 6);
    }
    return exec({"-i", _projectPath, "-o", tempPath});
}

ErrorStatus langscore::invoker::reanalysis(std::filesystem::path updateDir)
{
    //config config;
    //auto tempPath = std::filesystem::path(config.langscoreUpdateDirectorty());
    auto tempPath = updateDir;
    tempPath.make_preferred();
    std::error_code errorCode;
    std::filesystem::create_directories(tempPath, errorCode);
    if(utility::FileSystemFailed(errorCode, "create_directories", tempPath)){
        return ErrorStatus(ErrorStatus::Module::INVOKER, 7);
    }
    return exec({"-i", _projectPath, "-o", tempPath});
}

ErrorStatus langscore::invoker::recompressVXAce(){
    //rvcnv は Data/Scripts.rvdata2 を書き換える。読み取り専用だと書き込めずに失敗するため、先に外す
    utility::removeReadOnly(_projectPath / "Data" / "Scripts.rvdata2");
    return exec({"-i", _projectPath, "-c"});
}

ErrorStatus langscore::invoker::packingVXAce(std::filesystem::path destGameTranslateFolder, std::filesystem::path packingDir)
{
    config config;
    ErrorStatus result = Status_Success;
    if(config.enableLanguagePatch()) 
    {
        auto allLanguage = config.allLanguages();
        //patchPath内のディレクトリを検索
        std::error_code errorCode;
        const auto entries = utility::ListDirectory(packingDir);
        if(entries.empty() && std::filesystem::is_directory(packingDir, errorCode) == false) {
            return ErrorStatus(ErrorStatus::Module::INVOKER, 9);
        }
        for(const auto& entry : entries) {
            if(entry.is_directory(errorCode))
            {
                const auto dirName = entry.path().filename();
                const auto languageName = utility::toUtf8(dirName);
                if(std::ranges::find_if(allLanguage, [&](const auto& l){ return l.name == languageName; }) != allLanguage.end())
                {
                    if(exec({"-i", entry.path(), "-o", destGameTranslateFolder / dirName, "-p"}) != Status_Success)
                    {
                        return ErrorStatus(ErrorStatus::Module::INVOKER, 2);
                    }
                }
            }
        }
        return Status_Success;

    }
    else {
        return exec({"-i", packingDir, "-o", destGameTranslateFolder, "-p"});
    }
}

ErrorStatus langscore::invoker::exec(std::vector<std::filesystem::path> args)
{
    auto basePath = appPath.empty() ? "./" : appPath;
    if(projectType == config::VXAce)
    {
        auto rvcnvPath = (basePath / "rvcnv.exe");

        rvcnvPath.make_preferred();
        std::error_code errorCode;
        if(std::filesystem::exists(rvcnvPath, errorCode) == false){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 3);
        }

#if !defined(_DEBUG)
        {
            MD5 md5;
            std::ifstream rvcnv_bin(rvcnvPath, std::ios::binary | std::ios::in);
            rvcnv_bin.seekg(0, std::ios_base::end);
            auto size = rvcnv_bin.tellg();
            rvcnv_bin.seekg(0, std::ios_base::beg);
            std::vector<std::uint8_t> bin(size, 0);
            rvcnv_bin.read((char*)bin.data(), size);

            auto hash = md5((void*)bin.data(), bin.size());
            if(hash != rvcnv_hash){
                return ErrorStatus(ErrorStatus::Module::INVOKER, 5);
            }
        }
#endif

        args.insert(args.begin(), rvcnvPath);
        auto ret = execProcess(ToCommandLine(args));
        if(ret == -1){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 4);
        }
        //rvcnv が失敗した（ファイルに書き込めないなど）のに成功として扱うと、プラグインが書き込まれないまま完了してしまう
        if(ret != 0){
            std::cout << "rvcnv failed. exit code : " << ret << std::endl;
            return ErrorStatus(ErrorStatus::Module::INVOKER, 10);
        }
    }
    else //if(currentProjectType == None)
    {
        return ErrorStatus(ErrorStatus::Module::INVOKER, 1);
    }


    return Status_Success;
}
