/*
 * vpaint_global.h - vpaint libray global
 */
#ifndef VPAINT_GLOBAL_H
#define VPAINT_GLOBAL_H

#include <QtGlobal>

#ifndef QT_STATIC
#  if defined(QT_BUILD_VPAINT_LIB)
#    define Q_VPAINT_EXPORT Q_DECL_EXPORT
#  else
#    define Q_VPAINT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_VPAINT_EXPORT
#endif
enum class ShapeType {
    CURVE,
    LINE,
    CIRCLE,
    TRIANGLE,
    RECTANGLE,
    POLYGON,
    NONE,
};
#endif // VPAINT_GLOBAL_H

