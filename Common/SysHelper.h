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
#pragma once

//================================================================================
// Include head file
//================================================================================
#include "../Base/BaseDefine.h"
#include "../Base/GlobalType.h"

//================================================================================
// Define export macro
//================================================================================
// Environment variable
#if   defined(_WINDOWS)
    // Current user name
    #define ENV_USER_NAME getenv("USERNAME")
    // Current user directory path
    #define ENV_HOME_PATH getenv("USERPROFILE")
    // Current temporary directory path
    #define ENV_TEMP_PATH getenv("TEMP")
#elif defined(_LINUX)
    // Current user name
    #define ENV_USER_NAME getenv("USER")
    // Current user directory path
    #define ENV_HOME_PATH getenv("HOME")
    // Current temporary directory path
    #define ENV_TEMP_PATH "/tmp"
#endif

// Current process id
#if   defined(_WINDOWS)
    #define SELF_PROCESS_ID ((pid_t)::GetCurrentProcessId())
#elif defined(_LINUX)
    #define SELF_PROCESS_ID ((pid_t)getpid())
#endif

// Current thread id
#if   defined(_WINDOWS)
    #define SELF_THREAD_ID ((pthread_t)::GetCurrentThreadId())
#elif defined(_LINUX)
    #define SELF_THREAD_ID ((pthread_t)pthread_self())
#endif

// Current native thread id
#if   defined(_WINDOWS)
    #define SELF_NATIVE_THREAD_ID SELF_THREAD_ID
#elif defined(_LINUX)
    #include <sys/syscall.h>
    #define SELF_NATIVE_THREAD_ID ((pthread_t)syscall(__NR_gettid))
#endif

// Check the thread is same
#if   defined(_WINDOWS)
    #define IsSameThread(tid1, tid2) ((tid1) == (tid2))
#elif defined(_LINUX)
    #define IsSameThread(tid1, tid2) pthread_equal((tid1), (tid2))
#endif

// Check the thread is current thread
#define IsSelfThread(tid) IsSameThread((tid), SELF_THREAD_ID)

// Check the native thread is same
#define IsSameNativeThread(tid1, tid2) ((tid1) == (tid2))

// Check the native thread is current native thread
#define IsSelfNativeThread(tid) IsSameNativeThread((tid), SELF_NATIVE_THREAD_ID)

// Processor to do yield (Only allow higher thread to employ processor, and only support hyperthreading technology)
#if   defined(_WIN32)
    #include <Windows.h>
    #ifndef YieldProcessor
        #pragma intrinsic(_mm_pause)
        #define __atomic_yield _mm_pause
    #else
        #define __atomic_yield YieldProcessor
    #endif
#elif defined(__cplusplus)
    #include <thread>
    static inline void __atomic_yield() { std::this_thread::yield(); }
#elif defined(__SSE2__)
    #include <emmintrin.h>
    static inline void __atomic_yield() { _mm_pause(); }
#elif   (defined(__GNUC__) || defined(__clang__)) && \
        (defined(__x86_64__) || defined(__i386__) || defined(__arm__) || defined(__armel__) || defined(__ARMEL__) || \
        defined(__aarch64__) || defined(__powerpc__) || defined(__ppc__) || defined(__PPC__))
    #if defined(__x86_64__) || defined(__i386__)
        static inline void __atomic_yield() { __asm__ volatile ("pause" ::: "memory"); }
    #elif defined(__aarch64__)
        static inline void __atomic_yield() { __asm__ volatile("wfe"); }
    #elif (defined(__arm__) && __ARM_ARCH__ >= 7)
        static inline void __atomic_yield() { __asm__ volatile("yield" ::: "memory"); }
    #elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
        static inline void __atomic_yield() { __asm__ __volatile__ ("or 27,27,27" ::: "memory"); }
    #elif defined(__armel__) || defined(__ARMEL__)
        static inline void __atomic_yield() { __asm__ volatile ("nop" ::: "memory"); }
    #endif
#elif defined(__sun)
    #include <synch.h>
    static inline void __atomic_yield() { smt_pause(); }
#elif defined(__wasi__)
    #include <sched.h>
    static inline void __atomic_yield() { sched_yield(); }
#else
    #include <unistd.h>
    static inline void __atomic_yield() { sleep(0); }
#endif
#define SysYieldProcessor __atomic_yield

// Processor switch to other thread (Allow any lower thread to employ processor)
#if   defined(_WIN32)
    #define SysSwitchToThread SwitchToThread
#elif defined(_LINUX)
    #define SysSwitchToThread sched_yield
#endif

// Sleep for seconds
#define SleepForSeconds(tm) std::this_thread::sleep_for(std::chrono::seconds(tm));

// Sleep for milliseconds
#define SleepForMilliseconds(tm) std::this_thread::sleep_for(std::chrono::milliseconds(tm));

// Sleep for microseconds
#define SleepForMicroseconds(tm) std::this_thread::sleep_for(std::chrono::microseconds(tm));

//================================================================================
// Define export type
//================================================================================
/**
 * @brief Memory occupy infomation
 */
struct SysMemoryOccupy
{
    float  usage     = 0.0F; // Usage (Unit: %)
    size_t total     = 0;    // Total (Unit: MB)
    size_t available = 0;    // Available (Unit: MB)
};

/**
 * @brief Disk occupy infomation
 */
struct SysDiskOccupy
{
    float  usage     = 0.0F; // Usage (Unit: %)
    size_t total     = 0;    // Total (Unit: MB)
    size_t available = 0;    // Available (Unit: MB)
};

//================================================================================
// Define export method
//================================================================================
/**
 * @brief Get system processors count
 *
 * @return uint     Processors count
 */
uint GetSysProcessorCount() noexcept;

/**
 * @brief Get system page size
 *
 * @return ulong    Page size (Unit: byte)
 */
ulong GetSysPageSize() noexcept;

/**
 * @brief Get processor usage
 *
 * @return float    Processor usage
 */
float GetSysProcessorUsage() noexcept;

/**
 * @brief Get memory occupy
 *
 * @param[in,out] memOccupy     Memory occupy
 */
void GetSysMemoryOccupy(SysMemoryOccupy &memOccupy) noexcept;

/**
 * @brief Get disk occupy (Get the disk occupy where the target file is located)
 *
 * @param[in,out] diskOccupy    Disk occupy
 * @param         filePath      Target file path
 */
void GetSysDiskOccupy(SysDiskOccupy &diskOccupy, const char *filePath) noexcept;

/**
 * @brief Get environment variable (Only valid within current process)
 *
 * @param envName       Name
 * @return const char*  Value
 */
const char *GetSysCurrentEnv(const char *envName) noexcept;

/**
 * @brief Set environment variable (Only valid within current process)
 *
 * @param envName       Name
 * @return const char*  Value
 */
const char *SetSysCurrentEnv(const char *envName) noexcept;