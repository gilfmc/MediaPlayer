#ifndef OMDB_GLOBAL_H
#define OMDB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(OMDB_LIBRARY)
#  define OMDBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define OMDBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // OMDB_GLOBAL_H
