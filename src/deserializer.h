#ifndef LANGSCORE_DESERIALIZER_H
#define LANGSCORE_DESERIALIZER_H

#include <string>
#include <functional>
#include <filesystem>

namespace langscore
{

class deserializer
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

    deserializer();
    ~deserializer();

    void setApplicationFolder(std::filesystem::path path);
    void setProjectPath(ProjectType type, std::filesystem::path path);

    Result exec();
    std::filesystem::path outputPath() const;

    std::function<void(std::string)> process_stdout;

private:
    std::filesystem::path appPath;
    std::filesystem::path projectPath;
    ProjectType currentProjectType;
};

}

#endif // LANGSCORE_DESERIALIZER_H
