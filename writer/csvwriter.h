#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "writerbase.h"

class csvwriter : public writerbase
{
public:
    using writerbase::writerbase;
    
    bool write(QString path) override;
};

#endif // CSVWRITER_H
