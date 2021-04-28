// Firmware library 
// fl_def.h

#ifndef FL_DEF_H
#define FL_DEF_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
#define FL_BEGIN_DECLS                extern "C" {
#define FL_END_DECLS                  }
#else
#define FL_BEGIN_DECLS
#define FL_END_DECLS
#endif

#if defined(WIN32)

#define FL_BEGIN_PACK1                __pragma(pack(push,1))
#define FL_END_PACK                   __pragma(pack(pop))

#if defined(FL_DECLARE_EXPORT)
#define FL_DECLARE(type)              __declspec(dllexport) type __stdcall
#define FL_DECLARE_NONSTD(type)       __declspec(dllexport) type __cdecl
#define FL_DECLARE_DATA               __declspec(dllexport)
#else
#define FL_DECLARE(type)              __declspec(dllimport) type __stdcall
#define FL_DECLARE_NONSTD(type)       __declspec(dllimport) type __cdecl
#define FL_DECLARE_DATA               __declspec(dllimport)
#endif

#elif defined(__GNUC__)
// GNUC start
// Nordic semiconductor : nRF5_SDK_xxx\component\802_15_4\api\SysAL\sys_utils.h
#define FL_BEGIN_PACK1                _Pragma("pack(push, 1)")
#define FL_END_PACK                   _Pragma("pack(pop)")

#define FL_DECLARE(type)              type
#define FL_DECLARE_NONSTD(type)       type
#define FL_DECLARE_DATA
// GNUC end

#else // WIN32 end

#define FL_BEGIN_PACK1                _Pragma("push") \
                                      _Pragma("pack(1)")
#define FL_END_PACK                   _Pragma("pop")

#define FL_DECLARE(type)              type
#define FL_DECLARE_NONSTD(type)       type
#define FL_DECLARE_DATA

#endif


#define FL_FALSE                      (0)
#define FL_TRUE                       (1)

#define FL_OK                         (0)
#define FL_ERROR                      (1)

#define FL_BYTE_ORDER_BIG_ENDIAN      (0)
#define FL_BYTE_ORDER_LITTLE_ENDIAN   (1)

#define FL_BYTE_ORDER                 FL_BYTE_ORDER_LITTLE_ENDIAN

typedef unsigned char fl_status_t;
typedef unsigned char fl_bool_t;

#endif
