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
//================================================================================
// Include head file
//================================================================================
#include "FileHelper.h"
#include "DbgHelper.h"
#include <sys/stat.h>

//================================================================================
// Define inside macro
//================================================================================
/**
 * @brief Path type check
 */
#if defined(_WINDOWS)
    #define __S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
    #define S_ISDIR(mode)           __S_ISTYPE((mode), _S_IFDIR)
    #define S_ISCHR(mode)           __S_ISTYPE((mode), _S_IFCHR)
    #define S_ISFIFO(mode)          __S_ISTYPE((mode), _S_IFIFO)
    #define S_ISREG(mode)           __S_ISTYPE((mode), _S_IFREG)
#endif

//================================================================================
// Implementation inside method
//================================================================================
/**
 * @brief Get full path name (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return  std::unique_ptr<char[]> Full long path (Nullptr: error; End of path separator same as source path)
 */
std::unique_ptr<char[]> __GetAbsolutePath(const char *srcPath)
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = std::make_unique<char[]>(PATH_MAX + 1);
    char *                  path_strval = path_strptr.get();
    size_t                  newpath_len = 0;

    {
#if   defined(_WINDOWS)
        std::unique_ptr<char[]> full_strptr = std::make_unique<char[]>(PATH_MAX + 1);
        char *                  full_strval = path_strptr.get();
        char                    temp_char   = '\0';

        if (GetFullPathNameA(srcPath, PATH_MAX + 1, full_strval, NULL) == 0) return nullptr;

        if (full_strval[0] == PATH_SEPARATOR && full_strval[1] == PATH_SEPARATOR)
        {
            newpath_len = strlen(full_strval);
            memcpy(path_strval, full_strval, newpath_len + 1);
        }
        else
        {
            char *end_ptr = &full_strval[strlen(full_strval)];
            for (char *full_ptr = end_ptr; full_ptr != full_strval; full_ptr--)
            {
                if (*full_ptr != PATH_SEPARATOR) continue;

                temp_char   = full_ptr[1];
                full_ptr[1] = '\0';
                newpath_len = GetLongPathNameA(full_strval, path_strval, PATH_MAX + 1);
                full_ptr[1] = temp_char;
                if (newpath_len == 0) continue;

                if (path_strval[newpath_len - 1] == PATH_SEPARATOR) full_ptr++;
                memcpy(path_strval + newpath_len, full_ptr, end_ptr - full_ptr);
                newpath_len += end_ptr - full_ptr;
                break;
            }
        }

        if (newpath_len == 0) return nullptr;
#elif defined(_LINUX)
        const size_t srcpath_len = strlen(srcPath);

        if ((srcpath_len == 0 || *srcPath != PATH_SEPARATOR) && !getcwd(path_strval, PATH_MAX + 1)) return nullptr;
        newpath_len = strlen(path_strval);

        const char *next_ptr = srcPath;
        const char *end_ptr  = &srcPath[srcpath_len];
        size_t      part_len = 0;
        for (const char *src_ptr = srcPath; src_ptr < end_ptr; src_ptr = next_ptr + 1)
        {
            next_ptr = (const char *)memchr(src_ptr, PATH_SEPARATOR, end_ptr - next_ptr);
            if (!next_ptr) next_ptr = end_ptr;

            part_len = next_ptr - src_ptr;
            switch (part_len)
            {
                case 2:
                    if (src_ptr[0] == '.' && src_ptr[1] == '.')
                    {
                        const char *slash_ptr = (const char *)memrchr(path_strval, PATH_SEPARATOR, newpath_len);
                        if (slash_ptr) newpath_len = slash_ptr - path_strval;
                        continue;
                    }
                    break;
                case 1:
                    if (src_ptr[0] == '.') continue;
                    break;
                case 0:
                    continue;
            }
            path_strval[newpath_len++] = PATH_SEPARATOR;
            memcpy(path_strval + newpath_len, src_ptr, part_len);
            newpath_len += part_len;
        }

        if (newpath_len == 0 || (srcpath_len != 0 && srcPath[srcpath_len - 1] == PATH_SEPARATOR)) path_strval[newpath_len++] = PATH_SEPARATOR;
#endif

        path_strval[newpath_len] = '\0';
    }

    std::unique_ptr<char[]> result_path = std::make_unique<char[]>(newpath_len + 1);
    if (memcpy(result_path.get(), path_strval, newpath_len + 1))
        return result_path;
    else
        return nullptr;
}

//================================================================================
// Implementation export method
//================================================================================
/**
 * @brief Check whether the file or directory exists (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the file or directory is exists
 */
bool IsExists(const char *srcPath) noexcept
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = GetFullLongPath(srcPath);
    if (!path_strptr.get()) return false;

    struct stat path_stat;
    return (stat(path_strptr.get(), &path_stat) == 0);
}

/**
 * @brief Check whether the path is a directory (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the path is a directory
 */
bool IsDirectory(const char *srcPath) noexcept
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = GetFullLongPath(srcPath);
    if (!path_strptr.get()) return false;

    struct stat path_stat;
    return (stat(path_strptr.get(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode));
}

/**
 * @brief Check whether the path is a file (Thread safe)
 *
 * @param   srcPath Source path
 * @return  bool    Whether to the path is a file
 */
bool IsFile(const char *srcPath) noexcept
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = GetFullLongPath(srcPath);
    if (!path_strptr.get()) return false;

    struct stat path_stat;
    return (stat(path_strptr.get(), &path_stat) == 0 && S_ISREG(path_stat.st_mode));
}

/**
 * @brief Get full long Path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return  std::unique_ptr<char[]> Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetFullLongPath(const char *srcPath) noexcept
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = std::make_unique<char[]>(PATH_MAX + 1);
    char                   *path_strval = path_strptr.get();

    {
#if   defined(_WINDOWS)
        std::unique_ptr<char[]> full_path = std::make_unique<char[]>(PATH_MAX + 1);
        if (GetFullPathName(srcPath, PATH_MAX + 1, full_path.get(), NULL) == 0) return nullptr;
        if (GetLongPathName(full_path.get(), path_strval, PATH_MAX + 1) == 0) return nullptr;
#elif defined(_LINUX)
        if (!realpath(srcPath, path_strval)) return nullptr;
#endif
    }

    size_t path_len = strlen(path_strval);
    if (path_len != 0 && path_strval[path_len - 1] == PATH_SEPARATOR)
    {
        path_strval[path_len - 1] = '\0';
        path_len--;
    }

    std::unique_ptr<char[]> result_path = std::make_unique<char[]>(path_len + 1);
    if (memcpy(result_path.get(), path_strval, path_len + 1))
        return result_path;
    else
        return nullptr;
}

/**
 * @brief Get application path (Thread safe)
 *
 * @return  std::unique_ptr<char[]> Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetAppPath() noexcept
{
    std::unique_ptr<char[]> app_path = std::make_unique<char[]>(PATH_MAX + 1);
#if   defined(_WINDOWS)
    if (GetModuleFileName(nullptr, app_path.get(), PATH_MAX + 1) == 0) return nullptr;
#elif defined(_LINUX)
    if (readlink("/proc/self/exe", app_path.get(), PATH_MAX + 1) == -1) return nullptr;
#endif
    return GetFullLongPath(app_path.get());
}

/**
 * @brief Get parent directory path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  Full long path (Nullptr: error; Not end with path separator)
 */
std::unique_ptr<char[]> GetParentDirectoryPath(const char *srcPath) noexcept
{
    DBG_ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = GetFullLongPath(srcPath);
    char *                  path_strval = path_strptr.get();
    if (!path_strval) return nullptr;

    char *last_sep = strchr(path_strval, PATH_SEPARATOR);
    if (last_sep && last_sep != path_strval) *last_sep = '\0';

    return path_strptr;
}

/**
 * @brief Get file full name (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; File name with suffix)
 */
std::unique_ptr<char[]> GetFileFullName(const char *srcPath) noexcept
{
    ASSERT(srcPath);

    std::unique_ptr<char[]> path_strptr = GetFullLongPath(srcPath);
    char *                  path_strval = path_strptr.get();
    if (!path_strval) return nullptr;

    char *last_sep = strrchr(path_strval, PATH_SEPARATOR);
    if (!last_sep || *(++last_sep) == '\0') return nullptr;

    std::unique_ptr<char[]> name_strptr = std::make_unique<char[]>(strlen(last_sep) + 1);
    if (!memcpy(name_strptr.get(), last_sep, strlen(last_sep) + 1)) return nullptr;
    return name_strptr;
}

/**
 * @brief Get file short path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; File name without suffix)
 */
std::unique_ptr<char[]> GetFileShortName(const char *srcPath) noexcept
{
    ASSERT(srcPath);

    std::unique_ptr<char[]> full_strptr = GetFileFullName(srcPath);
    char *                  full_strval = full_strptr.get();
    if (!full_strval) return nullptr;

    char *last_sep = strrchr(full_strval, '.');
    if (last_sep) *last_sep = '\0';

    std::unique_ptr<char[]> name_strptr = std::make_unique<char[]>(strlen(full_strval) + 1);
    if (!memcpy(name_strptr.get(), full_strval, strlen(full_strval) + 1)) return nullptr;
    return name_strptr;
}

/**
 * @brief Get file suffix path (Thread safe)
 *
 * @param   srcPath                 Source path
 * @return std::unique_ptr<char[]>  File full name (Nullptr: error; Suffix name without dot symbol)
 */
std::unique_ptr<char[]> GetFileSuffixName(const char *srcPath) noexcept
{
    ASSERT(srcPath);

    std::unique_ptr<char[]> full_strptr = GetFileFullName(srcPath);
    char *                  full_strval = full_strptr.get();
    if (!full_strval) return nullptr;

    char *last_sep = strrchr(full_strval, '.');
    if (last_sep)
    {
        last_sep++;

        std::unique_ptr<char[]> name_strptr = std::make_unique<char[]>(strlen(last_sep) + 1);
        if (!memcpy(name_strptr.get(), last_sep, strlen(last_sep) + 1)) return nullptr;
        return name_strptr;
    }
    else
    {
        std::unique_ptr<char[]> name_strptr = std::make_unique<char[]>(1);
        name_strptr[0]                      = '\0';
        return name_strptr;
    }
}

/**
 * @brief Make directory
 *
 * @param dirPath Directory path
 * @return bool Whether to make directory is successed
 */
bool MakeDirectory(const char *dirPath) noexcept
{
    return true;
}

/**
 * @brief Rename path (Auto distinguish file or directory)
 *
 * @param srcPath Source path
 * @param newPath Target path
 * @return bool Whether to rename directory is successed
 */
bool RenamePath(const char *srcPath, const char *newPath) noexcept
{
    return true;
}

/**
 * @brief Remove path (Auto distinguish file or directory)
 *
 * @param dirPath Directory path
 * @return bool Whether to remove directory is successed
 */
bool RemovePath(const char *dirPath) noexcept
{
    return true;
}