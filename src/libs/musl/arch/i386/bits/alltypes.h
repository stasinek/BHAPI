#ifndef ALLTYPES_H
#define ALLTYPES_H

#define __int8  char
#define __int16 short
#define __int32 long
#define __int64 long long

#define int8_t  __int8
#define int16_t __int16
#define int32_t __int32
#define int64_t __int64

#define uint8_t   unsigned __int8
#define uint16_t  unsigned __int16
#define uint32_t  unsigned __int32
#define uint64_t  unsigned __int64

#define size_t    unsigned long
#define uintptr_t unsigned long

#define _Addr int
#define _Int64 long long
#define _Reg int

#if __GNUC__ >= 3
typedef __builtin_va_list va_list;
typedef __builtin_va_list __isoc_va_list;
#else
typedef struct __va_list * va_list;
typedef struct __va_list * __isoc_va_list;
#endif

#ifndef __cplusplus
#ifdef __WCHAR_TYPE__
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef long wchar_t;
#endif
#endif

#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 0
typedef float float_t;
typedef double double_t;
#else
typedef long double float_t;
typedef long double double_t;
#endif

#if !defined(__cplusplus)
typedef struct { _Alignas(8) long long __ll; long double __ld; } max_align_t;
#elif defined(__GNUC__)
typedef struct { __attribute__((__aligned__(8))) long long __ll; long double __ld; } max_align_t;
#else
typedef struct { alignas(8) long long __ll; long double __ld; } max_align_t;
#endif

typedef long time_t;
typedef long suseconds_t;

typedef struct { union { int __i[9]; volatile int __vi[9]; unsigned __s[9]; } __u; } pthread_attr_t;
typedef struct { union { int __i[6]; volatile int __vi[6]; volatile void *volatile __p[6]; } __u; } pthread_mutex_t;
typedef struct { union { int __i[6]; volatile int __vi[6]; volatile void *volatile __p[6]; } __u; } mtx_t;
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; } pthread_cond_t;
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; } cnd_t;
typedef struct { union { int __i[8]; volatile int __vi[8]; void *__p[8]; } __u; } pthread_rwlock_t;
typedef struct { union { int __i[5]; volatile int __vi[5]; void *__p[5]; } __u; } pthread_barrier_t;

#endif
