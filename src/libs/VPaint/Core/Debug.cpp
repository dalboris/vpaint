#include "Debug.h"

#include <QElapsedTimer>

QDebug Debug::log()
{
    static bool hasStarted = false;
    static QElapsedTimer timer;
    static qint64 elapsed = 0;

    // Get time elapsed since started
    qint64 newElapsed = 0;
    if (!hasStarted)
    {
        hasStarted = true;
        timer.start();
        newElapsed = 0;
    }
    else
    {
        newElapsed = timer.elapsed();
    }

    // Get difference
    qint64 diff = newElapsed - elapsed;
    elapsed = newElapsed;

    return qDebug() << elapsed << diff;
}
