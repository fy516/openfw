/**
 * @brief Logging File Manage
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
#include <thread>

//================================================================================
// Define preset type
//================================================================================
class ZYLoggingFilePrivate;

//================================================================================
// Define export type
//================================================================================
/**
 * @brief Logging File Manage
 */
class ZYLoggingFile final
{
    friend class ZYLoggingFilePrivate;

public:
    /**
     * @brief File naming rule
     */
    enum NAMING_RULE
    {
        NR_FIXED = 0, // Fixed name
        NR_DATE  = 1  // Date suffix name
    };

    /**
     * @brief Hex or Binary argument (Used to format argument)
     */
    struct HexOrBitArg
    {
        const void *datas  = nullptr; // Byte datas
        int         length = 0;       // Datas length (Units: Hex=bytes; Binary=bits)
        HexOrBitArg(const void *d, const int l) : datas(d), length(l) {}
    };

    /**
     * @brief Safe mutex
     */
    struct SafeMutex
    {
        int safeLevel; // Safe level (Use object safe level macros)
#if defined(_LINUX)
        mutable pthread_mutex_t     mutexLock; // Mutex lock
        mutable pthread_mutexattr_t mutexAttr; // Mutex lock attribute
#elif defined(_WINDOWS)
#endif
    };

private:
    char *      _dirPath;    // File directory path
    char *      _fileName;   // File name (No file suffix name)
    NAMING_RULE _namingRule; // File naming rule
    SafeMutex * _safeLock;   // Safe mutex

public:
    /**
     * @brief Construct function
     *
     * @param safeLevel  Object safe level (Use object safe level macros)
     * @param dirPath    File directory path (Must end with '\\0')
     * @param fileName   File name (Must end with '\\0'; No file suffix name)
     * @param namingRule File naming rule
     */
    ZYLoggingFile(const int safeLevel, const char *dirPath, const char *fileName, const NAMING_RULE namingRule) noexcept;

    /**
     * @brief Destruct function
     */
    ~ZYLoggingFile();

    /**
     * @brief Output one line
     *
     * @param logLevel   Log level (Use execute status level macros)
     * @param logContent Log content (Must end with '\\0')
     * @param ...        Log format content (Format parameters)
     */
    void outputLine(const int logLevel, const char *logContent) const noexcept;

    /**
     * @brief Output one line
     *
     * @param logLevel  Log level (Use execute status level macros)
     * @param fmtString Format string (Must end with '\\0'; Format: "%%"="%", "%X|%x"=Hex string, %B|%b"=Binary string, Other=Reference sprintf() specifier)
     * @param ...       Format arguments ("%X|%x|%B|%b" must use std::make_unique<::HexOrBitArg>("123", 3).get() type argument)
     */
    void outputLine(const int logLevel, const char *fmtString, ...) const noexcept;
};