#ifndef LANGSCORE_DESERIALIZER_H
#define LANGSCORE_DESERIALIZER_H

#include <string>
#include <functional>

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

    void setApplicationFolder(std::string path);
    void setProjectPath(ProjectType type, std::string path);

    Result exec();
    std::string outputPath() const;

    std::function<void(std::string)> process_stdout;

private:
    std::string appPath;
    std::string projectPath;
    ProjectType currentProjectType;
};

}

#endif // LANGSCORE_DESERIALIZER_H
