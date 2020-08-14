// fw_lib_def.h

#ifndef FW_LIB_DEF_H
#define FW_LIB_DEF_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
#define FW_LIB_BEGIN_DECLS              extern "C" {
#define FW_LIB_END_DECLS                }
#else
#define FW_LIB_BEGIN_DECLS
#define FW_LIB_END_DECLS
#endif

#if defined(WIN32)

#define FW_LIB_BEGIN_PACK1              __pragma(pack(push,1))
#define FW_LIB_END_PACK                 __pragma(pack(pop))

#if defined(FW_LIB_DECLARE_EXPORT)
#define FW_LIB_DECLARE(type)            __declspec(dllexport) type __stdcall
#define FW_LIB_DECLARE_NONSTD(type)     __declspec(dllexport) type __cdecl
#define FW_LIB_DECLARE_DATA             __declspec(dllexport)
#else
#define FW_LIB_DECLARE(type)            __declspec(dllimport) type __stdcall
#define FW_LIB_DECLARE_NONSTD(type)     __declspec(dllimport) type __cdecl
#define FW_LIB_DECLARE_DATA             __declspec(dllimport)
#endif

#elif defined(__GNUC__)
// GNUC start
// Nordic semiconductor : nRF5_SDK_xxx\component\802_15_4\api\SysAL\sys_utils.h
#define FW_LIB_BEGIN_PACK1            _Pragma("pack(push, 1)")
#define FW_LIB_END_PACK               _Pragma("pack(pop)")

#define FW_LIB_DECLARE(type)          type
#define FW_LIB_DECLARE_NONSTD(type)   type
#define FW_LIB_DECLARE_DATA
// GNUC end

#else // WIN32 end

#define FW_LIB_BEGIN_PACK1              _Pragma("push") \
                                        _Pragma("pack(1)")
#define FW_LIB_END_PACK                 _Pragma("pop")

#define FW_LIB_DECLARE(type)            type
#define FW_LIB_DECLARE_NONSTD(type)     type
#define FW_LIB_DECLARE_DATA

#endif


#define FW_LIB_FALSE                    (0)
#define FW_LIB_TRUE                     (1)

#define FW_LIB_OK                       (0)
#define FW_LIB_ERROR                    (1)

#define FW_LIB_BYTE_ORDER_BIG_ENDIAN    (0)
#define FW_LIB_BYTE_ORDER_LITTLE_ENDIAN (1)

#define FW_LIB_BYTE_ORDER               FW_LIB_BYTE_ORDER_LITTLE_ENDIAN

typedef unsigned char                   fw_lib_status_t;
typedef unsigned char                   fw_lib_bool_t;

#endif
