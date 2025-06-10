#include "invoker.h"
#include "utility.hpp"

#include <iostream>
#include <fstream>
#include <array>
#include "../rvcnv/rvcnv_hash.cpp"
#include "md5.h"

using namespace langscore;

static std::string convertPath(std::filesystem::path path) {
    auto result = path.string();
    if(result.find(' ') != std::string::npos) {
        return "\"" + result + "\"";
    }
    return result;
}

#ifdef WIN32
#include <process.h>
#include <Windows.h>
static int execProcess(const char* cmd) 
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

    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdError = writePipe;
    startupInfo.hStdOutput = writePipe;

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    if(!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL,TRUE, 0, NULL, NULL, &startupInfo, &processInfo))               
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
static int execProcess(const char* cmd) {
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
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
    if(std::filesystem::is_directory(path) == false) {
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
    if(std::filesystem::exists(tempPath) == false){
        if(std::filesystem::create_directories(tempPath) == false){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 6);
        }
    }
    //return exec({"-i", "\"" + _projectPath.string() + "\"", "-o", "\"" + tempPath.string() + "\""});
    return exec({"-i", convertPath(_projectPath), "-o", convertPath(tempPath)});
}

ErrorStatus langscore::invoker::reanalysis(std::filesystem::path updateDir)
{
    //config config;
    //auto tempPath = std::filesystem::path(config.langscoreUpdateDirectorty());
    auto tempPath = updateDir;
    tempPath.make_preferred();
    if(std::filesystem::exists(tempPath) == false){
        if(std::filesystem::create_directories(tempPath) == false){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 7);
        }
    }
    //return exec({"-i", "\""+_projectPath.string()+"\"", "-o", "\""+tempPath.string()+"\""});
    return exec({"-i", convertPath(_projectPath), "-o", convertPath(tempPath)});
}

ErrorStatus langscore::invoker::recompressVXAce(){
    return exec({"-i", convertPath(_projectPath), "-c"});
}

ErrorStatus langscore::invoker::packingVXAce(std::filesystem::path destGameTranslateFolder, std::filesystem::path packingDir)
{
    config config;
    ErrorStatus result = Status_Success;
    if(config.enableLanguagePatch()) 
    {
        auto allLanguage = config.allLanguages();
        //patchPath内のディレクトリを検索
        for(const auto& entry : std::filesystem::directory_iterator(packingDir)) {
            if(entry.is_directory()) 
            {
                auto dirName = entry.path().filename().string();
                if(std::ranges::find_if(allLanguage, [&](const auto& l){ return l.name == dirName; }) != allLanguage.end()) 
                {
                    if(exec({"-i", convertPath(entry), "-o", convertPath(destGameTranslateFolder / dirName), "-p"}) != Status_Success) 
                    {
                        return ErrorStatus(ErrorStatus::Module::INVOKER, 2);
                    }
                }
            }
        }
        return Status_Success;

    }
    else {
        return exec({"-i", convertPath(packingDir), "-o", convertPath(destGameTranslateFolder), "-p"});
    }
}

ErrorStatus langscore::invoker::exec(std::vector<std::string> args)
{
    auto basePath = appPath.empty() ? "./" : appPath;
    if(projectType == config::VXAce)
    {
        auto rvcnvPath = (basePath / "rvcnv.exe");

        rvcnvPath.make_preferred();
        if(std::filesystem::exists(rvcnvPath) == false){
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

        auto process = "\"" + rvcnvPath.string() + "\"" + " " + utility::join(args, std::string(" "));

        auto ret = execProcess(process.c_str());
        if(ret == -1){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 4);
        }
    }
    else //if(currentProjectType == None)
    {
        return ErrorStatus(ErrorStatus::Module::INVOKER, 1);
    }


    return Status_Success;
}
