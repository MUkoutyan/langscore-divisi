#ifndef LANGSCORE_INVOKER_H
#define LANGSCORE_INVOKER_H

#include <string>
#include <functional>
#include <filesystem>

namespace langscore
{

class invoker
{
public:

    struct Result {
    public:
        Result(int code = 0) : code(code), specMsg(""){}
        int val() const noexcept { return code; }
        void setSpecMsg(std::string m){ specMsg = std::move(m); }
        std::string toStr() const;
    private:
        int code;
        std::string specMsg;
    };

    enum ProjectType {
        None,
        VXAce,
        MV,
        MZ,
    };

    invoker();
    ~invoker();

    void setApplicationFolder(std::filesystem::path path);
    void setProjectPath(ProjectType type, std::filesystem::path path);
    
    ProjectType projectType() const noexcept;

    Result analyze();
    Result recompressVXAce();
    

    std::function<void(std::string)> process_stdout;

private:
    std::filesystem::path appPath;
    std::filesystem::path _projectPath;
    std::vector<std::u8string> langs;
    ProjectType currentProjectType;

    Result exec(std::vector<std::string> args);
};

}

#endif // LANGSCORE_DESERIALIZER_H
