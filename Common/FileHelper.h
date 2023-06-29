/**
 * @brief File Helper
 *
 * @author WindEagle <fy516a@gmail.com>
 * @version 1.0.0
 * @date 2000-01-01 00:00
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

//================================================================================
// Define export macro
//================================================================================
// Path separator
#if   defined(_WINDOWS)
    #define PATH_SEPARATOR '\\'
#elif defined(_LINUX) || defined(_APPLE)
    #define PATH_SEPARATOR '/'
#endif

//================================================================================
// Define export method
//================================================================================
/**
 * @brief Check whether the file or directory exists (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the file or directory is exists
 */
bool IsExists(const char *srcPath) noexcept;

/**
 * @brief Check whether the path is a directory (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the path is a directory
 */
bool IsDirectory(const char *srcPath) noexcept;

/**
 * @brief Check whether the path is a file (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the path is a file
 */
bool IsFile(const char *srcPath) noexcept;

/**
 * @brief Get full long Path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return  std::unique_ptr<char[]> Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetFullLongPath(const char *srcPath) noexcept;

/**
 * @brief Get application path (Thread safe)
 *
 * @return std::unique_ptr<char[]>  Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetAppPath() noexcept;

/**
 * @brief Get parent directory path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetParentDirectoryPath(const char *srcPath) noexcept;

/**
 * @brief Get file full name (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; File name with suffix)
 */
std::unique_ptr<char[]> GetFileFullName(const char *srcPath) noexcept;

/**
 * @brief Get file short path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; File name without suffix)
 */
std::unique_ptr<char[]> GetFileShortName(const char *srcPath) noexcept;

/**
 * @brief Get file suffix path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; Suffix name without dot symbol)
 */
std::unique_ptr<char[]> GetFileSuffixName(const char *srcPath) noexcept;

/**
 * @brief Make directory
 * 
 * @param dirPath Directory path
 * @return bool Whether to make directory is successed
 */
bool MakeDirectory(const char *dirPath) noexcept;

/**
 * @brief Rename path (Auto distinguish file or directory)
 *
 * @param srcPath Source path
 * @param newPath Target path
 * @return bool Whether to rename directory is successed
 */
bool RenamePath(const char *srcPath, const char *newPath) noexcept;

/**
 * @brief Remove path (Auto distinguish file or directory)
 *
 * @param dirPath Directory path
 * @return bool Whether to remove directory is successed
 */
bool RemovePath(const char *dirPath) noexcept;