#ifndef LANGSCORE_DIVISI_H
#define LANGSCORE_DIVISI_H

#include <QString>
#include <memory>

namespace langscore
{

class divisi
{
public:
    divisi(QString appPath);
    ~divisi();

    void setProjectPath(QString projectPath);
    void exec();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
