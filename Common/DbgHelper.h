/**
 * @brief Debug Helper
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
#include "../Base/BaseDefine.h"
#include "../Base/GlobalErrno.h"
#include "../Base/GlobalType.h"
#if defined(_MSC) && defined(_CRTDBG_MAP_ALLOC)
    #include <crtdbg.h>
#endif

//================================================================================
// Define export macro
//================================================================================
#define DBG_OUTPUTLOG_FILE static_cast<const char *>(__FILE__)
#define DBG_OUTPUTLOG_LINE __LINE__
#if   defined(_MSC)
    #define DBG_OUTPUTLOG_FUNC static_cast<const char *>(__FUNCSIG__)
#elif defined(_GCC)
    #define DBG_OUTPUTLOG_FUNC static_cast<const char *>(__PRETTY_FUNCTION__)
#endif

// Redefine new, delete
#if defined(_MSC) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
    #ifdef new
        #undef new
    #endif
    #ifdef delete
        #undef delete
    #endif

    #pragma warning(push)
    #pragma warning(disable : 4595)
    inline void *__cdecl operator new(size_t size, const char *file, int line) { return ::_malloc_dbg(size, _NORMAL_BLOCK, file, line); }
    inline void *__cdecl operator new[](size_t size, const char *file, int line) { return operator new(size, file, line); }
    inline void *__cdecl operator new(size_t size) { return operator new(size, __FILE__, __LINE__); }
    inline void *__cdecl operator new[](size_t size) { return operator new(size, __FILE__, __LINE__); }
    #if defined(_MSVC_LANG)
        #if _MSVC_LANG < 201700L
            inline void *__cdecl operator new(size_t size, const std::nothrow_t &) { return operator new(size, __FILE__, __LINE__); }
            inline void *__cdecl operator new[](size_t size, const std::nothrow_t &) { return operator new(size, __FILE__, __LINE__); }
        #else
            inline void *__cdecl operator new(size_t size, const std::nothrow_t &) noexcept { return operator new(size, __FILE__, __LINE__); }
            inline void *__cdecl operator new[](size_t size, const std::nothrow_t &) noexcept { return operator new(size, __FILE__, __LINE__); }
            inline void *__cdecl operator new(size_t size, std::align_val_t) { return operator new(size, __FILE__, __LINE__); }
            inline void *__cdecl operator new[](size_t size, std::align_val_t) { return operator new(size, __FILE__, __LINE__); }
        #endif
    #else
        #error("Undefined macro _MSVC_LANG.")
    #endif
    inline void __cdecl operator delete(void *p) { ::_free_dbg(p, _NORMAL_BLOCK); }
    inline void __cdecl operator delete[](void *p) { operator delete(p); }
    inline void __cdecl operator delete(void *p, const char *file, int line) { operator delete(p); }
    inline void __cdecl operator delete[](void *p, const char *file, int line) { operator delete(p); }
    inline void __cdecl operator delete(void *p, const std::nothrow_t &) { operator delete(p); }
    inline void __cdecl operator delete[](void *p, const std::nothrow_t &) { operator delete(p); }
    #pragma warning(pop)
    #define new new(__FILE__, __LINE__)
#endif

// ASSERT
#ifdef _DEBUG
    #define DBG_ASSERT(expr)        ((expr) ? void(0) : DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, 0x0100 | ESL_FATAL, #expr, 0))
#else
    #define DBG_ASSERT(expr)
#endif

// VERIFY
#ifdef _DEBUG
    #define DBG_VERIFY(expr)        ((expr) ? void(0) : DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, 0x0200 | ESL_FATAL, #expr, 0))
#else
    #define DBG_VERIFY(expr)        (expr)
#endif

// Output errno infomation
#ifdef _DEBUG
    #define DBG_PERROR(type, str)   DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, 0x0400 | type, str, 0)
#else
    #define DBG_PERROR(type, str)   DbgOutputLog(nullptr,            0,                  nullptr,            0x0400 | type, str, 0)
#endif

// Output custom infomation
#ifdef _DEBUG
    #define DBGLOG_DEBUG(fmt, ...)      DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, ESL_DEBUG,      fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output debug log
    #define DBGLOG_INFOMATION(fmt, ...) DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, ESL_INFOMATION, fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output infomation log
    #define DBGLOG_WARNING(fmt, ...)    DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, ESL_WARNING,    fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output warning log
    #define DBGLOG_ERROR(fmt, ...)      DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, ESL_ERROR,      fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output error log
    #define DBGLOG_FATAL(fmt, ...)      DbgOutputLog(DBG_OUTPUTLOG_FILE, DBG_OUTPUTLOG_LINE, DBG_OUTPUTLOG_FUNC, ESL_FATAL,      fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output fatal log
#else
    #define DBGLOG_DEBUG(fmt, ...)                                                                                                                                          // Output debug log
    #define DBGLOG_INFOMATION(fmt, ...) DbgOutputLog(nullptr,            0,                  nullptr,            ESL_INFOMATION, fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output infomation log
    #define DBGLOG_WARNING(fmt, ...)    DbgOutputLog(nullptr,            0,                  nullptr,            ESL_WARNING,    fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output warning log
    #define DBGLOG_ERROR(fmt, ...)      DbgOutputLog(nullptr,            0,                  nullptr,            ESL_ERROR,      fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output error log
    #define DBGLOG_FATAL(fmt, ...)      DbgOutputLog(nullptr,            0,                  nullptr,            ESL_FATAL,      fmt, VA_COUNT(__VA_ARGS__), ##__VA_ARGS__) // Output fatal log
#endif

//================================================================================
// Define export type
//================================================================================
/**
 * @brief Debug log infomation handling function (Thread safe)
 *
 * @param logTime       Log time (Format: "yyyyMMdd")
 * @param logContent    Log content
 * @param logLength     Log content length (Without terminator '\0')
 */
typedef void (*dbg_log_handle_t)(const char *logDate, const char *logContent, const size_t logLength);

/**
 * @brief Debug log format datas argument (Used to format "%X|%x|%B|%b")
 */
struct dbg_log_datas_t
{
    const char *datas  = nullptr; // Byte datas
    uint        length = 0;       // Datas length (Units: Hex=bytes; Binary=bits)

    /**
     * @brief Construct function
     *
     * @param datas     Byte datas
     * @param length    Datas length (Units: Hex[%X|%x]=bytes, Binary[%B|%b]=bits)
     */
    dbg_log_datas_t(const void *datas, const int length) : datas(static_cast<const char *>(datas)), length(length) {}
};

//================================================================================
// Define export method
//================================================================================
/**
 * @brief Set debug error infomation handling function (Thread safe)
 *
 * @param errHandle     Debug error infomation handling function
 */
void DbgSetHandle(const dbg_log_handle_t errHandle) noexcept;

/**
 * @brief Output debug log (Thread safe; Direct use is not recommended)
 *
 * @param filePath      File path (Debug mode: DBG_OUTPUTLOG_FILE; Release mode: nullptr)
 * @param fileLine      File line (Debug mode: DBG_OUTPUTLOG_LINE; Release mode: 0)
 * @param fileFunc      File function (Debug mode: DBG_OUTPUTLOG_FUNC; Release mode: nullptr)
 * @param logType       Log type (0x0100: ASSERT; 0x0200: VERIFY; 0x0400: PERROR; Other: use execute status level)
 * @param fmtString     Format string (Must end with '\\0'; Format: "%%"="%", "%X|%x"=Hex string, %B|%b"=Binary string, Other=Reference sprintf() specifier)
 * @param fmtArgsCount  Format arguments count
 * @param ...           Format arguments ("%X|%x|%B|%b" must be use a parameter in the format of std::make_unique<dbg_log_datas_t>("xxx", 3).get())
 */
void DbgOutputLog(const char *filePath, const int fileLine, const char *fileFunc, const int logType, const char *fmtString, const int fmtArgsCount, ...) noexcept;
