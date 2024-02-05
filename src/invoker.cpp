#include "invoker.h"
#include "config.h"
#include "utility.hpp"

#include <process.h>
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

static int execProcess(const char* cmd) {
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if(!pipe) {
        throw std::runtime_error("_popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::cout << buffer.data();
    }
    return _pclose(pipe.get());
}
#endif

invoker::invoker()
    : appPath("")
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
    this->_projectPath = std::move(path);
}


ErrorStatus invoker::analyze(){
    config config;
    auto tempPath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
    tempPath.make_preferred();
    if(std::filesystem::exists(tempPath) == false){
        if(std::filesystem::create_directories(tempPath) == false){
            return ErrorStatus(ErrorStatus::Module::INVOKER, 6);
        }
    }
    //return exec({"-i", "\"" + _projectPath.string() + "\"", "-o", "\"" + tempPath.string() + "\""});
    return exec({"-i", convertPath(_projectPath), "-o", convertPath(tempPath)});
}

ErrorStatus langscore::invoker::update()
{
    config config;
    auto tempPath = std::filesystem::path(config.langscoreUpdateDirectorty());
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

ErrorStatus langscore::invoker::packingVXAce(){
    config config;
    auto inputDir  = std::filesystem::path(config.packingInputDirectory());
    auto outputDir = std::filesystem::path(config.gameProjectPath()+u8"/Data/Translate");
    //return exec({"-i", "\""+inputDir.string()+ "\"", "-o", "\""+outputDir.string()+ "\"", "-p"});
    return exec({"-i", convertPath(inputDir), "-o", convertPath(outputDir), "-p"});
}

ErrorStatus langscore::invoker::exec(std::vector<std::string> args)
{
    auto basePath = appPath.empty() ? "./" : appPath;
    config config;
    if(config.projectType() == config::VXAce)
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
