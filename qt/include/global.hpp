#pragma once

#include <QtGlobal>

#if !defined(QT_STATIC)
#if defined(QT_BUILD_SIDEASSIST_LIB)
#define Q_SIDEASSIST_EXPORT Q_DECL_EXPORT
#else
#define Q_SIDEASSIST_EXPORT Q_DECL_IMPORT
#endif
#else
#define Q_SIDEASSIST_EXPORT
#endif
