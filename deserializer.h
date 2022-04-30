#ifndef LANGSCORE_DESERIALIZER_H
#define LANGSCORE_DESERIALIZER_H

#include <QObject>

namespace langscore
{

class deserializer : public QObject
{
    Q_OBJECT
public:

    struct Result {
    public:
        Result(int code = 0) : code(code), specMsg(""){}
        int val() const noexcept { return code; }
        void setSpecMsg(QString m){ specMsg = std::move(m); }
        QString toStr() const;
    private:
        int code;
        QString specMsg;
    };

    enum ProjectType {
        None,
        VXAce,
        MV,
        MZ,
    };

    deserializer();
    ~deserializer();

    void setApplicationFolder(QString path);
    void setProjectPath(ProjectType type, QString path);

    Result exec();
    QString outputPath() const;

signals:
    void recvStdOut(QString);

private:
    QString appPath;
    QString projectPath;
    ProjectType currentProjectType;
};

}

#endif // LANGSCORE_DESERIALIZER_H
