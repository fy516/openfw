/**
 * @brief System Helper
 *
 * @author WindEagle <fy516a@gmail.com>
 * @version 1.0.0
 * @date 2020-01-01 00:00
 * @copyright Copyright (c) 2020-2022 ZyTech Team
 * @par Changelog:
 * Date                 Version     Author          Description
 */
//================================================================================
// Include head file
//================================================================================
#include "DbgHelper.h"
#include "SysHelper.h"
#include "FuncHelper.h"
#include "FileHelper.h"
#include <mutex>
#include <cstring>
#if   defined(_WINDOWS)
#elif defined(_LINUX)
    #include <sys/statfs.h>
#endif

//================================================================================
// Define inside variable
//================================================================================
/**
 * @brief Inner mutex
 */
static std::mutex __InnerMutex;

//================================================================================
// Implementation export method
//================================================================================
/**
 * @brief Get system processors count
 *
 * @return uint     Processors count
 */
uint GetSysProcessorCount() noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

#if   defined(_WINDOWS)
    SYSTEM_INFO sys_info;
    ::GetNativeSystemInfo(&sys_info);
    return (UINT)sys_info.dwNumberOfProcessors;
#elif defined(_LINUX)
    return (uint)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

/**
 * @brief Get system page size
 *
 * @return ulong    Page size (Unit: byte)
 */
ulong GetSysPageSize() noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

#if   defined(_WINDOWS)
    SYSTEM_INFO si;
    ::GetNativeSystemInfo(&si);
    return (ulong)si.dwPageSize;
#elif defined(_LINUX)
    return (ulong)sysconf(_SC_PAGESIZE);
#endif
}

/**
 * @brief Get processor usage
 *
 * @return float    Processor usage
 */
float GetSysProcessorUsage() noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

    static ulonglong previous_idle  = 0;
    static ulonglong previous_total = 0;
    static float     previous_usage = 0.0F;
    ulonglong        current_idle   = 0;
    ulonglong        current_total  = 0;
    float            current_usage  = 0.0F;

#if defined(_WINDOWS)
    {
        FILETIME proc_idle   = {0, 0};
        FILETIME proc_user   = {0, 0};
        FILETIME proc_system = {0, 0};
        if (GetSystemTimes(&proc_idle, &proc_system, &proc_user))
        {
            current_idle  = (((ulonglong)(proc_idle.dwHighDateTime)) << 32) | ((ulonglong)proc_idle.dwLowDateTime);
            current_total = ((((ulonglong)(proc_user.dwHighDateTime)) << 32) | ((ulonglong)proc_user.dwLowDateTime)) + ((((ulonglong)(proc_system.dwHighDateTime)) << 32) | ((ulonglong)proc_system.dwLowDateTime));
        }
    }
#elif defined(_LINUX)
    FILE *file_descriptor = NULL;

    file_descriptor = fopen("/proc/stat", "r");
    if (file_descriptor)
    {
        char file_buffer[256] = "";
        if (fgets(file_buffer, sizeof(file_buffer), file_descriptor))
        {
            char      proc_name[20] = "";
            ulonglong proc_user     = 0;
            ulonglong proc_nice     = 0;
            ulonglong proc_system   = 0;
            ulonglong proc_idle     = 0;
            ulonglong proc_lowait   = 0;
            ulonglong proc_irq      = 0;
            ulonglong proc_softirq  = 0;
            if (sscanf(file_buffer, "%s %llu %llu %llu %llu %llu %llu %llu", proc_name, &proc_user, &proc_nice, &proc_system, &proc_idle, &proc_lowait, &proc_irq, &proc_softirq) == 8 && strcasecmp(proc_name, "cpu") == 0)
            {
                current_idle  = proc_idle;
                current_total = proc_user + proc_nice + proc_system + proc_idle + proc_lowait + proc_irq + proc_softirq;
            }
        }
        fclose(file_descriptor);
    }
#endif

    current_usage = previous_total && current_total && current_idle >= previous_idle && current_total > previous_total ? (1.0F - (float)((double)(current_idle - previous_idle) / (double)(current_total - previous_total))) * 100.0F : previous_usage;

    if (current_total)
    {
        previous_idle  = current_idle;
        previous_total = current_total;
        previous_usage = current_usage != previous_usage ? current_usage : previous_usage;
    }

    return current_usage;
}

/**
 * @brief Get memory occupy
 *
 * @param[in,out] memOccupy     Memory occupy
 */
void GetSysMemoryOccupy(SysMemoryOccupy &memOccupy) noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

#if defined(_WINDOWS)
    ulonglong mem_total     = 0;
    ulonglong mem_available = 0;

    {
        MEMORYSTATUSEX mem_status;
        mem_status.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&mem_status))
        {
            mem_total     = mem_status.ullTotalPhys;
            mem_available = mem_status.ullAvailPhys;
        }
    }

    mem_total     = mem_total / (1024ULL * 1024ULL);
    mem_available = mem_available / (1024ULL * 1024ULL);
#elif defined(_LINUX)
    FILE *    file_descriptor  = NULL;
    char      file_buffer[256] = "";
    char      item_name[35]    = "";
    ulonglong item_value       = 0;
    ulonglong mem_total        = 0;
    ulonglong mem_free         = 0;
    ulonglong mem_available    = 0;
    ulonglong mem_buffers      = 0;
    ulonglong mem_cached       = 0;

    file_descriptor = fopen("/proc/meminfo", "r");
    if (file_descriptor)
    {
        while (fgets(file_buffer, sizeof(file_buffer), file_descriptor))
        {
            if (sscanf(file_buffer, "%s %llu ", item_name, &item_value) == 2)
            {
                if (strncasecmp(item_name, "MemTotal", MIN(sizeof("MemTotal") - 1, strlen(item_name))) == 0)
                    mem_total = item_value;
                else if (strncasecmp(item_name, "MemFree", MIN(sizeof("MemFree") - 1, strlen(item_name))) == 0)
                    mem_free = item_value;
                else if (strncasecmp(item_name, "MemAvailable", MIN(sizeof("MemAvailable") - 1, strlen(item_name))) == 0)
                    mem_available = item_value;
                else if (strncasecmp(item_name, "Buffers", MIN(sizeof("Buffers") - 1, strlen(item_name))) == 0)
                    mem_buffers = item_value;
                else if (strncasecmp(item_name, "Cached", MIN(sizeof("Cached") - 1, strlen(item_name))) == 0)
                    mem_cached = item_value;
            }
            else
                break;

            if (mem_total && (mem_available || (mem_free && mem_buffers && mem_cached))) break;
        }
        fclose(file_descriptor);
    }

    mem_total     = mem_total / 1024ULL;
    mem_available = (mem_available ? mem_available : (mem_free + mem_buffers + mem_cached)) / 1024ULL;
#endif

    memOccupy.usage     = (float)(mem_total && mem_total > mem_available ? ((double)(mem_total - mem_available) / (double)mem_total) : 0.0F) * 100.0F;
    memOccupy.total     = (size_t)mem_total;
    memOccupy.available = (size_t)mem_available;
}

/**
 * @brief Get disk occupy (Get the disk occupy where the target file is located)
 *
 * @param[in,out] diskOccupy    Disk occupy
 * @param         filePath      Target file path
 */
void GetSysDiskOccupy(SysDiskOccupy &diskOccupy, const char *filePath) noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

    ulonglong disk_total     = 0;
    ulonglong disk_available = 0;

#if defined(_WINDOWS)
    {
        char  root_name[PATH_MAX + 1];
        char *root_ptmp = nullptr;

        if (!GetFullLongPath(filePath, root_name, PATH_MAX + 1)) goto _function_end;

        if (root_name[0] == PATH_SEPARATOR && root_name[1] == PATH_SEPARATOR)
        {
            root_ptmp = &root_name[2];
            while (*root_ptmp && (*root_ptmp != PATH_SEPARATOR)) root_ptmp++;
            if (*root_ptmp) root_ptmp++;
        }
        else
            root_ptmp = root_name;

        while (*root_ptmp && (*root_ptmp != PATH_SEPARATOR)) root_ptmp++;
        if (*root_ptmp)
        {
            root_ptmp++;
            *root_ptmp = '\0';
        }

        ULARGE_INTEGER _disk_total;
        ULARGE_INTEGER _disk_free;
        ULARGE_INTEGER _disk_available;
        if (!GetDiskFreeSpaceEx(root_name, &_disk_available, &_disk_total, &_disk_free)) goto _function_end;
        disk_total     = _disk_total.QuadPart / (1024ULL * 1024ULL);
        disk_available = _disk_available.QuadPart / (1024ULL * 1024ULL);
    }

#elif defined(_LINUX)
    {
        struct statfs           disk_info;
        char                    full_path[PATH_MAX + 1];

        if (!GetFullLongPath(filePath, full_path, PATH_MAX + 1)) goto _function_end;

        if (statfs(full_path, &disk_info) == ESV_SUCCESS)
        {
            disk_total     = ((ulonglong)disk_info.f_bsize * (ulonglong)disk_info.f_blocks) / (1024ULL * 1024ULL);
            disk_available = ((ulonglong)disk_info.f_bsize * (ulonglong)disk_info.f_bavail) / (1024ULL * 1024ULL);
        }
    }
#endif

_function_end:
    diskOccupy.usage     = (float)(disk_total && disk_total > disk_available ? ((double)(disk_total - disk_available) / (double)disk_total) : 0.0F) * 100.0F;
    diskOccupy.total     = (size_t)disk_total;
    diskOccupy.available = (size_t)disk_available;
}

/**
 * @brief Get environment variable (Only valid within current process)
 *
 * @param envName       Name
 * @return const char*  Value
 */
bool GetSysCurrentEnv(const char *envName, char* envBuffer, size_t bufLength) noexcept
{
    std::lock_guard<std::mutex> sys_locker(__InnerMutex);

    char *env_value = getenv(envName);

#if defined(_WINDOWS)
    if (env_value && (strcmp(envName, "USERPROFILE") == 0 || strcmp(envName, "TEMP") == 0))
#elif defined(_LINUX)
    if (env_value && strcmp(envName, "HOME") == 0)
#endif
    {
        return GetFullLongPath(env_value, envBuffer, bufLength);
    }
    else
    {
        size_t env_len = strlen(env_value);
        if (bufLength < env_len + 1) DBGLOG_FATAL("Environment variable buffer length is not enough.");
        return memcpy(envBuffer, env_value, env_len + 1);
    }
}

/**
 * @brief Set environment variable (Only valid within current process)
 *
 * @param envName       Name
 * @return const char*  Value
 */
const char *SetSysCurrentEnv(const char *envName) noexcept
{
    int errcode = 0;
    if (!overwrite)
    {
        size_t envsize = 0;
        errcode        = getenv_s(&envsize, NULL, 0, name);
        if (errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}