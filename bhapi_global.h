#ifndef BHAPI_GLOBAL_H
#define BHAPI_GLOBAL_H



#if defined(BHAPI_LIBRARY)
#  define BHAPISHARED_EXPORT Q_DECL_EXPORT
#else
#  define BHAPISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BHAPI_GLOBAL_H
