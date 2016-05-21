#ifndef DEBUG_H
#define DEBUG_H

#include <QtDebug>

class Debug
{
public:
    Debug();

    static QDebug log();
};

#endif // DEBUG_H
