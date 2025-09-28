#ifndef TESTS_WINDOWS_H
#define TESTS_WINDOWS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int BOOL;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef int INT;
typedef short SHORT;
typedef unsigned short USHORT;
typedef char CHAR;
typedef float FLOAT;
typedef void VOID;
typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HWND;
typedef void* HGLRC;
typedef void* HDC;
typedef void* FARPROC;
typedef void* PROC;
typedef void* LPVOID;
typedef const char* LPCSTR;
typedef void* PVOID;
typedef unsigned long ULONG_PTR;
typedef long LONG_PTR;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef int INT32;
typedef unsigned int UINT32;
typedef ULONG_PTR DWORD_PTR;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define WINAPI
#define APIENTRY
#define CALLBACK
#define WINGDIAPI
#define WINBASEAPI
#define WINUSERAPI
#define __stdcall
#define __cdecl
#define WINAPI_FAMILY_PARTITION(...) 1
#define WINAPI_PARTITION_DESKTOP 1
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name;
#endif

typedef struct _POINT {
    LONG x;
    LONG y;
} POINT;

typedef struct _RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef struct _INIT_ONCE {
    void* Ptr;
} INIT_ONCE, *PINIT_ONCE;

#define INIT_ONCE_STATIC_INIT { (void*)0 }

typedef BOOL (*PINIT_ONCE_FN)(PINIT_ONCE, PVOID, PVOID*);

static inline BOOL InitOnceExecuteOnce(
    PINIT_ONCE init_once,
    PINIT_ONCE_FN init_fn,
    PVOID parameter,
    PVOID* context)
{
    (void)parameter;
    if (init_fn) {
        return init_fn(init_once, parameter, context);
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif

#endif /* TESTS_WINDOWS_H */
