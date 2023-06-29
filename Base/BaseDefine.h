/**
 * @brief Base Define
 *
 * @author WindEagle <fy516a@gmail.com>
 * @version 1.0.0
 * @date 2020-01-01 00:00
 * @copyright Copyright (c) 2020-2022 ZyTech Team
 * @par Changelog:
 * Date                 Version     Author          Description
 */
#pragma once

//================================================================================
// Include head file
//================================================================================
#if   defined(_WIN32) && defined(_MSC_VER)
    #include <windows.h>
#elif defined(__linux__) && defined(__GNUC__)
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>
    #include <limits.h>
#else
    #error("The current program only supports GCC and MSVC compilers.")
#endif
#include <memory>

//================================================================================
// Define export macro
//================================================================================
// Operating system
#if   defined(_WIN32)
    #ifndef _WINDOWS
        #define _WINDOWS    1
    #endif
#elif defined(__linux__)
    #ifndef _LINUX
        #define _LINUX      1
    #endif
#elif defined(__APPLE__)
    #ifndef _APPLE
        #define _APPLE      1
    #endif
#endif

// Build compiler
#if   defined(_MSC_VER)
    #ifndef _MSC
        #define _MSC    1
    #endif
#elif defined(__GNUC__)
    #ifndef _GCC
        #define _GCC    1
    #endif
#endif

// Build platform
#if (defined(_MSC) && defined(_WIN64)) || (defined(_GCC) && defined(__x86_64__))
    #if !defined(_X64)
        #define _X64    1
    #endif
#else
    #if !defined(_X86)
        #define _X86    1
    #endif
#endif

// Build type
#if defined(NDEBUG) || defined(_NDEBUG)
    #if !defined(NDEBUG)
        #define NDEBUG  1
    #endif
    #if !defined(_NDEBUG)
        #define _NDEBUG 1
    #endif
    #if defined(DEBUG)
        #undef DEBUG
    #endif
    #if defined(_DEBUG)
        #undef _DEBUG
    #endif
#else
    #if !defined(DEBUG)
        #define DEBUG   1
    #endif
    #if !defined(_DEBUG)
        #define _DEBUG  1
    #endif
#endif

// Cross-Platform
#if   defined(_MSC)
    #define PATH_MAX MAX_PATH
#elif defined(_GCC)
    #ifndef __stdcall
        #define __stdcall   __attribute__((__stdcall__))
    #endif
    #ifndef __cdecl
        #define __cdecl     __attribute__((__cdecl__))
    #endif
    #ifndef CALLBACK
        #define CALLBACK
    #endif
    #ifndef WINAPI
        #define WINAPI
    #endif
    #ifndef FORCEINLINE
        #define FORCEINLINE __attribute__((always_inline))
    #endif
#endif

// Get variable arguments count (GCC must set c++ standard to "std=gnu++11" or later)
#if   defined(_MSC)
    #define VA_COUNT_INTERNAL_EXPAND(x)                                                                                                                                                                                                                                                                                                                                                                                                                     x
    #define VA_COUNT_INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...)     count
    #define VA_COUNT_INTERNAL_EXPAND_ARGS_PRIVATE(...)                                                                                                                                                                                                                                                                                                                                                                                                      VA_COUNT_INTERNAL_EXPAND(VA_COUNT_INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
    #define VA_COUNT_INTERNAL_ARGS_AUGMENTER(...)                                                                                                                                                                                                                                                                                                                                                                                                           unused, __VA_ARGS__
    #define VA_COUNT(...)                                                                                                                                                                                                                                                                                                                                                                                                                                   VA_COUNT_INTERNAL_EXPAND_ARGS_PRIVATE(VA_COUNT_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#elif defined(_GCC)
    #define VA_COUNT_INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
    #define VA_COUNT(...)                                                                                                                                                                                                                                                                                                                                                                                                                                   VA_COUNT_INTERNAL_GET_ARG_COUNT_PRIVATE(0, ##__VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

// Execute status value (Used to system function)
#define ESV_FAILURE -1 // Execute failure
#define ESV_SUCCESS 0  // Execute success

// Execute status level (Used to log level and exception level)
#define ESL_DEBUG      0x0001 // Debug (Debug infomation)
#define ESL_INFOMATION 0x0002 // Infomation (Message infomation)
#define ESL_WARNING    0x0004 // Warning (It can be repaired, and the system can continue to run)
#define ESL_ERROR      0x0008 // Error (It can be repaired, but it does not ensure that the system will work normally)
#define ESL_FATAL      0x0010 // Fatal (It cannot be repaired, and the system cannot continue to run)

// Thread safe level (Used to thread safe level for object)
#define TSL_THREAD  0 // Thread safe
#define TSL_PROCESS 1 // Process safe
