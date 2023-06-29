/**
 * @brief Unique Lock
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
#include "../Base/BaseDefine.h"
#include "../Common/SysHelper.h"
#include "../Common/DbgHelper.h"
#if defined(_WINDOWS)
    #include "../Library/HashLibrary/md5.h"
#elif defined(_LINUX)
    #include <sys/mman.h>
    #include <semaphore.h>
#endif
#include "ThreadSafe.h"

//================================================================================
// Define inside macro
//================================================================================
#define LOCK_STATUS_IDLE  0x00
#define LOCK_STATUS_READ  0x01
#define LOCK_STATUS_WRITE 0x02

#if defined(_LINUX)
    #define INIT_STATUS_NONE       0x00
    #define INIT_STATUS_ATTRINITED 0x01
    #define INIT_STATUS_LOCKINITED 0x02
    #define INIT_STATUS_SEMINITED  0x04
#endif

//================================================================================
// Define inside type
//================================================================================
#if defined(_WINDOWS)
    struct threadsafe_rwlock_t
    {
        struct MmapDatas
        {
            uchar     lockStatus    = LOCK_STATUS_IDLE;
            uint      lockedCount   = 0;
            uint      rwaitingCount = 0;
            uint      wwaitingCount = 0;
            pthread_t writeThreadID = 0;
        };
        HANDLE     mmapFile   = nullptr;
        MmapDatas *mmapDatas  = nullptr;
        HANDLE     innerLock  = nullptr;
        HANDLE     innerEvent = nullptr;
    };
#elif defined(_LINUX)
    struct threadsafe_mutex_t
    {
        struct MmapDatas
        {
            uint                lockedCount = 0;
            pthread_mutexattr_t lockAttr;
            pthread_mutex_t     lockObj;
        };
        uchar      initStatus = INIT_STATUS_NONE;
        pid_t      creatorPid = 0;
        MmapDatas *mmapDatas  = nullptr;
    };
    struct threadsafe_rwlock_t
    {
        struct MmapDatas
        {
            uchar               lockStatus    = LOCK_STATUS_IDLE;
            uint                lockedCount   = 0;
            uint                rwaitingCount = 0;
            uint                wwaitingCount = 0;
            pthread_t           writeThreadID = 0;
            pthread_mutexattr_t innerAttr;
            pthread_mutex_t     innerLock;
            sem_t               innerSem;
        };
        uchar      initStatus = INIT_STATUS_NONE;
        pid_t      creatorPid = 0;
        MmapDatas *mmapDatas  = nullptr;
    };
#endif

//================================================================================
// Implementation export method [ThreadLock]
//================================================================================
/**
 * @brief Construct function
 *
 * @param lockType Lock type
 * @param lockName Lock name (Nullptr: thread lock; Other: process lock)
 */
ThreadLock::ThreadLock(const LockType lockType, const char *lockName) noexcept : _lockType(lockType), _isMultiProcess(lockName), _lockInstance(nullptr)
{
    switch (this->_lockType)
    {
        case LockType::Mutex:
        {
#if defined(_WINDOWS)
            HANDLE lock_object = nullptr;

            if (this->_isMultiProcess)
            {
                MD5 md5_object;
                md5_object.add(lockName, strlen(lockName));

                lock_object = CreateMutex(NULL, FALSE, md5_object.getHash().c_str());
                if (!lock_object || lock_object == INVALID_HANDLE_VALUE) PERROR("Failed to create mutex lock for cross-process:");
            }
            else
            {
                lock_object = CreateMutex(NULL, FALSE, NULL);
                if (!lock_object || lock_object == INVALID_HANDLE_VALUE) PERROR("Failed to create mutex lock for cross-thread:");
            }

            this->_lockInstance = lock_object;
#elif defined(_LINUX)
            threadsafe_mutex_t *lock_object = new threadsafe_mutex_t();

            lock_object->creatorPid = SELF_PROCESS_ID;

            if (this->_isMultiProcess)
            {
                lock_object->mmapDatas = (threadsafe_mutex_t::MmapDatas *)mmap(NULL, sizeof(threadsafe_mutex_t::MmapDatas), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
                if (lock_object->mmapDatas == MAP_FAILED) PERROR("Failed to map shared memory for mutex lock:");

                {
                    threadsafe_mutex_t::MmapDatas default_datas;
                    memcpy(lock_object->mmapDatas, &default_datas, sizeof(threadsafe_mutex_t::MmapDatas));
                }
            }
            else
            {
                lock_object->mmapDatas = new threadsafe_mutex_t::MmapDatas();
            }

            if (pthread_mutexattr_init(&lock_object->mmapDatas->lockAttr) != 0) PERROR("Failed to initialize mutex lock attribute:");
            lock_object->initStatus |= INIT_STATUS_ATTRINITED;

            if (pthread_mutexattr_setpshared(&lock_object->mmapDatas->lockAttr, this->_isMultiProcess ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE) != 0) PERROR("Failed to set mutex lock attribute:");
            if (pthread_mutexattr_settype(&lock_object->mmapDatas->lockAttr, PTHREAD_MUTEX_ERRORCHECK_NP) != 0) PERROR("Failed to set mutex lock attribute:");

            if (pthread_mutex_init(&lock_object->mmapDatas->lockObj, &lock_object->mmapDatas->lockAttr) != 0) PERROR("Failed to initialize mutex lock:");
            lock_object->initStatus |= INIT_STATUS_LOCKINITED;

            this->_lockInstance              = lock_object;
#endif
        }
        break;
        case LockType::RwLock:
        {
#if defined(_WINDOWS)
            threadsafe_rwlock_t *lock_object = new threadsafe_rwlock_t();

            if (this->_isMultiProcess)
            {
                bool is_creator = false;
                MD5  md5_object;
                md5_object.add(lockName, strlen(lockName));

                lock_object->mmapFile = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(threadsafe_rwlock_t::MmapDatas), md5_object.getHash().append("_MMAP").c_str());
                if (!lock_object->mmapFile || lock_object->mmapFile == INVALID_HANDLE_VALUE) PERROR("Failed to open shared memory for read/write lock:");
                is_creator = (GetLastError() != ERROR_ALREADY_EXISTS);

                lock_object->mmapDatas = (threadsafe_rwlock_t::MmapDatas *)MapViewOfFile(lock_object->mmapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(threadsafe_rwlock_t::MmapDatas));
                if (!lock_object->mmapDatas) PERROR("Failed to map shared memory for read/write lock:");

                if (is_creator)
                {
                    threadsafe_rwlock_t::MmapDatas default_datas;
                    memcpy(lock_object->mmapDatas, &default_datas, sizeof(threadsafe_rwlock_t::MmapDatas));
                }

                lock_object->innerLock = CreateMutex(NULL, FALSE, md5_object.getHash().append("_LOCK").c_str());
                if (!lock_object->innerLock || lock_object->innerLock == INVALID_HANDLE_VALUE) PERROR("Failed to create inner lock for read/write lock:");

                lock_object->innerEvent = CreateEvent(NULL, TRUE, FALSE, md5_object.getHash().append("_EVENT").c_str());
                if (!lock_object->innerEvent || lock_object->innerEvent == INVALID_HANDLE_VALUE) PERROR("Failed to create inner event for read/write lock:");
            }
            else
            {
                lock_object->mmapDatas = new threadsafe_rwlock_t::MmapDatas();

                lock_object->innerLock = CreateMutex(NULL, FALSE, NULL);
                if (!lock_object->innerLock || lock_object->innerLock == INVALID_HANDLE_VALUE) PERROR("Failed to create inner lock for read/write lock:");

                lock_object->innerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                if (!lock_object->innerEvent || lock_object->innerEvent == INVALID_HANDLE_VALUE) PERROR("Failed to create inner event for read/write lock:");
            }

            this->_lockInstance = lock_object;
#elif defined(_LINUX)
            threadsafe_rwlock_t *lock_object = new threadsafe_rwlock_t();

            lock_object->creatorPid = SELF_PROCESS_ID;

            if (this->_isMultiProcess)
            {
                lock_object->mmapDatas = (threadsafe_rwlock_t::MmapDatas *)mmap(NULL, sizeof(threadsafe_rwlock_t::MmapDatas), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
                if (lock_object->mmapDatas == MAP_FAILED) PERROR("Failed to map shared memory for read/write lock:");

                {
                    threadsafe_rwlock_t::MmapDatas default_datas;
                    memcpy(lock_object->mmapDatas, &default_datas, sizeof(threadsafe_rwlock_t::MmapDatas));
                }
            }
            else
            {
                lock_object->mmapDatas = new threadsafe_rwlock_t::MmapDatas();
            }

            if (pthread_mutexattr_init(&lock_object->mmapDatas->innerAttr) != 0) PERROR("Failed to initialize inner lock attribute for read/write lock:");
            lock_object->initStatus |= INIT_STATUS_ATTRINITED;

            if (pthread_mutexattr_setpshared(&lock_object->mmapDatas->innerAttr, this->_isMultiProcess ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE) != 0) PERROR("Failed to set inner lock shared attribute for read/write lock:");
            if (pthread_mutexattr_settype(&lock_object->mmapDatas->innerAttr, PTHREAD_MUTEX_TIMED_NP) != 0) PERROR("Failed to set inner lock type attribute for read/write lock:");

            if (pthread_mutex_init(&lock_object->mmapDatas->innerLock, &lock_object->mmapDatas->innerAttr) != 0) PERROR("Failed to initialize inner lock for read/write lock:");
            lock_object->initStatus |= INIT_STATUS_LOCKINITED;

            if (sem_init(&lock_object->mmapDatas->innerSem, 1, 0) != 0) PERROR("Failed to create inner semaphore for read/write lock:");
            lock_object->initStatus |= INIT_STATUS_SEMINITED;

            this->_lockInstance = lock_object;
#endif
        }
        break;
    }
}

/**
 * @brief Destruct function
 */
ThreadLock::~ThreadLock()
{
    if (!this->_lockInstance) return;

    switch (this->_lockType)
    {
        case LockType::Mutex:
        {
#if defined(_WINDOWS)
            ::CloseHandle((HANDLE)this->_lockInstance);
#elif defined(_LINUX)
            threadsafe_mutex_t *lock_object = (threadsafe_mutex_t *)this->_lockInstance;

            if (lock_object->mmapDatas)
            {
                if (lock_object->creatorPid == SELF_PROCESS_ID)
                {
                    if (lock_object->initStatus & INIT_STATUS_LOCKINITED) pthread_mutex_destroy(&lock_object->mmapDatas->lockObj);
                    if (lock_object->initStatus & INIT_STATUS_ATTRINITED) pthread_mutexattr_destroy(&lock_object->mmapDatas->lockAttr);
                }
                if (this->_isMultiProcess)
                    munmap(lock_object->mmapDatas, sizeof(threadsafe_mutex_t::MmapDatas));
                else
                    delete lock_object->mmapDatas;
                lock_object->mmapDatas = nullptr;
            }

            delete lock_object;
#endif
        }
        break;
        case LockType::RwLock:
        {
#if defined(_WINDOWS)
            threadsafe_rwlock_t *lock_object = (threadsafe_rwlock_t *)this->_lockInstance;

            if (lock_object->innerEvent)
            {
                ::CloseHandle(lock_object->innerEvent);
                lock_object->innerEvent = nullptr;
            }

            if (lock_object->innerLock)
            {
                ::CloseHandle(lock_object->innerLock);
                lock_object->innerLock = nullptr;
            }

            if (lock_object->mmapDatas)
            {
                if (this->_isMultiProcess)
                    ::UnmapViewOfFile(lock_object->mmapDatas);
                else
                    delete lock_object->mmapDatas;
                lock_object->mmapDatas = nullptr;
            }

            if (lock_object->mmapFile)
            {
                ::CloseHandle(lock_object->mmapFile);
                lock_object->mmapFile = nullptr;
            }

            delete lock_object;
#elif defined(_LINUX)
            threadsafe_rwlock_t *lock_object = (threadsafe_rwlock_t *)this->_lockInstance;

            if (lock_object->mmapDatas)
            {
                if (lock_object->creatorPid == SELF_PROCESS_ID)
                {
                    if (lock_object->initStatus & INIT_STATUS_LOCKINITED) pthread_mutex_destroy(&lock_object->mmapDatas->innerLock);
                    if (lock_object->initStatus & INIT_STATUS_ATTRINITED) pthread_mutexattr_destroy(&lock_object->mmapDatas->innerAttr);
                    if (lock_object->initStatus & INIT_STATUS_SEMINITED) sem_destroy(&lock_object->mmapDatas->innerSem);
                }
                if (this->_isMultiProcess)
                    munmap(lock_object->mmapDatas, sizeof(threadsafe_rwlock_t::MmapDatas));
                else
                    delete lock_object;
                lock_object->mmapDatas = nullptr;
            }

            delete lock_object;
#endif
        }
        break;
    }

    this->_lockInstance = nullptr;
}

/**
 * @brief Relock
 */
void ThreadLock::_reLock(const LockMode lockMode) noexcept
{
    if (!this->_lockInstance) return;

    switch (this->_lockType)
    {
        case LockType::Mutex:
        {
#if defined(_WINDOWS)
            WaitForSingleObject((HANDLE)this->_lockInstance, INFINITE);
#elif defined(_LINUX)
            threadsafe_mutex_t *lock_object = (threadsafe_mutex_t *)this->_lockInstance;
            pthread_mutex_lock(&lock_object->mmapDatas->lockObj);
            lock_object->mmapDatas->lockedCount++;
#endif
        }
        break;
        case LockType::RwLock:
        {
#if defined(_WINDOWS)
            threadsafe_rwlock_t *lock_object = (threadsafe_rwlock_t *)this->_lockInstance;
            bool                 wait_return = false;

            if (lockMode == LockMode::Read)
            {
                while (true)
                {
                    WaitForSingleObject(lock_object->innerLock, INFINITE);
                    if (wait_return) lock_object->mmapDatas->rwaitingCount--;

                    if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_IDLE)
                    {
                        lock_object->mmapDatas->lockStatus = LOCK_STATUS_READ;
                        lock_object->mmapDatas->lockedCount++;
                        ReleaseMutex(lock_object->innerLock);
                        break;
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_READ)
                    {
                        if (lock_object->mmapDatas->wwaitingCount > 0)
                        {
                            lock_object->mmapDatas->rwaitingCount++;
                            ResetEvent(lock_object->innerEvent);
                            SignalObjectAndWait(lock_object->innerLock, lock_object->innerEvent, INFINITE, FALSE);
                            wait_return = true;
                        }
                        else
                        {
                            lock_object->mmapDatas->lockedCount++;
                            ReleaseMutex(lock_object->innerLock);
                            break;
                        }
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE)
                    {
                        lock_object->mmapDatas->rwaitingCount++;
                        ResetEvent(lock_object->innerEvent);
                        SignalObjectAndWait(lock_object->innerLock, lock_object->innerEvent, INFINITE, FALSE);
                        wait_return = true;
                    }
                }
            }
            else if (lockMode == LockMode::Write)
            {
                pthread_t current_threadid = SELF_NATIVE_THREAD_ID;

                while (true)
                {
                    WaitForSingleObject(lock_object->innerLock, INFINITE);
                    if (wait_return) lock_object->mmapDatas->wwaitingCount--;

                    if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_IDLE)
                    {
                        lock_object->mmapDatas->lockStatus = LOCK_STATUS_WRITE;
                        lock_object->mmapDatas->lockedCount++;
                        lock_object->mmapDatas->writeThreadID = current_threadid;
                        ReleaseMutex(lock_object->innerLock);
                        break;
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE && lock_object->mmapDatas->writeThreadID == current_threadid)
                    {
                        lock_object->mmapDatas->lockedCount++;
                        ReleaseMutex(lock_object->innerLock);
                        break;
                    }
                    else
                    {
                        lock_object->mmapDatas->wwaitingCount++;
                        ResetEvent(lock_object->innerEvent);
                        SignalObjectAndWait(lock_object->innerLock, lock_object->innerEvent, INFINITE, FALSE);
                        wait_return = true;
                    }
                }
            }
#elif defined(_LINUX)
            threadsafe_rwlock_t *lock_object = (threadsafe_rwlock_t *)this->_lockInstance;
            bool                 wait_return = false;

            if (lockMode == LockMode::Read)
            {
                while (true)
                {
                    pthread_mutex_lock(&lock_object->mmapDatas->innerLock);
                    if (wait_return) lock_object->mmapDatas->rwaitingCount--;

                    if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_IDLE)
                    {
                        lock_object->mmapDatas->lockStatus = LOCK_STATUS_READ;
                        lock_object->mmapDatas->lockedCount++;
                        pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                        break;
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_READ)
                    {
                        if (lock_object->mmapDatas->wwaitingCount > 0)
                        {
                            lock_object->mmapDatas->rwaitingCount++;
                            pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                            sem_wait(&lock_object->mmapDatas->innerSem);
                            wait_return = true;
                        }
                        else
                        {
                            lock_object->mmapDatas->lockedCount++;
                            pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                            break;
                        }
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE)
                    {
                        if (lock_object->mmapDatas->writeThreadID == SELF_NATIVE_THREAD_ID) DBGLOG_FATAL("Thread deadlock.");
                        lock_object->mmapDatas->rwaitingCount++;
                        pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                        sem_wait(&lock_object->mmapDatas->innerSem);
                        wait_return = true;
                    }
                }
            }
            else if (lockMode == LockMode::Write)
            {
                pthread_t current_threadid = SELF_NATIVE_THREAD_ID;

                while (true)
                {
                    pthread_mutex_lock(&lock_object->mmapDatas->innerLock);
                    if (wait_return) lock_object->mmapDatas->wwaitingCount--;

                    if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_IDLE)
                    {
                        lock_object->mmapDatas->lockStatus = LOCK_STATUS_WRITE;
                        lock_object->mmapDatas->lockedCount++;
                        lock_object->mmapDatas->writeThreadID = current_threadid;
                        pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                        break;
                    }
                    else if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE && lock_object->mmapDatas->writeThreadID == current_threadid)
                    {
                        lock_object->mmapDatas->lockedCount++;
                        pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                        break;
                    }
                    else
                    {
                        lock_object->mmapDatas->wwaitingCount++;
                        pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
                        sem_wait(&lock_object->mmapDatas->innerSem);
                        wait_return = true;
                    }
                }
            }
#endif
        }
        break;
    }
}

/**
 * @brief Unlock
 */
void ThreadLock::_unLock() noexcept
{
    if (!this->_lockInstance) return;

    switch (this->_lockType)
    {
        case LockType::Mutex:
        {
#if defined(_WINDOWS)
            ReleaseMutex((HANDLE)this->_lockInstance);
#elif defined(_LINUX)
            threadsafe_mutex_t *lock_object = (threadsafe_mutex_t *)this->_lockInstance;
            lock_object->mmapDatas->lockedCount--;
            if (lock_object->mmapDatas->lockedCount == 0) pthread_mutex_unlock(&lock_object->mmapDatas->lockObj);
#endif
        }
        break;
        case LockType::RwLock:
        {
#if defined(_WINDOWS)
            threadsafe_rwlock_t *lock_object      = (threadsafe_rwlock_t *)this->_lockInstance;
            pthread_t            current_threadid = SELF_NATIVE_THREAD_ID;

            WaitForSingleObject(lock_object->innerLock, INFINITE);
            if (lock_object->mmapDatas->lockedCount > 0) lock_object->mmapDatas->lockedCount--;
            if (lock_object->mmapDatas->lockedCount == 0)
            {
                if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE) lock_object->mmapDatas->writeThreadID = 0;
                lock_object->mmapDatas->lockStatus = LOCK_STATUS_IDLE;
                if (lock_object->mmapDatas->wwaitingCount > 0 || lock_object->mmapDatas->rwaitingCount > 0) SetEvent(lock_object->innerEvent);
            }
            ReleaseMutex(lock_object->innerLock);
#elif defined(_LINUX)
            threadsafe_rwlock_t *lock_object      = (threadsafe_rwlock_t *)this->_lockInstance;
            pthread_t            current_threadid = SELF_NATIVE_THREAD_ID;

            pthread_mutex_lock(&lock_object->mmapDatas->innerLock);
            if (lock_object->mmapDatas->lockedCount > 0) lock_object->mmapDatas->lockedCount--;
            if (lock_object->mmapDatas->lockedCount == 0)
            {
                if (lock_object->mmapDatas->lockStatus == LOCK_STATUS_WRITE) lock_object->mmapDatas->writeThreadID = 0;
                lock_object->mmapDatas->lockStatus = LOCK_STATUS_IDLE;
                if (lock_object->mmapDatas->wwaitingCount > 0 || lock_object->mmapDatas->rwaitingCount > 0) sem_post(&lock_object->mmapDatas->innerSem);
            }
            pthread_mutex_unlock(&lock_object->mmapDatas->innerLock);
#endif
        }
        break;
    }
}

//================================================================================
// Implementation export method [LockGuard]
//================================================================================
/**
 * @brief Construct function
 *
 * @param lockInstance Lock instance
 * @param lockMode     Lock mode (None: the current lock do not to lock)
 * @param lockNow      Wether to lock it now
 */
LockGuard::LockGuard(ThreadLock *lockInstance, const ThreadLock::LockMode lockMode, const bool lockNow) noexcept : _lockInstance(lockInstance), _lockMode(ThreadLock::LockMode::Read), _isLocked(false)
{
    if (lockNow) this->reLock(lockMode);
}

/**
 * @brief Destruct function
 */
LockGuard::~LockGuard()
{
    if (this->_isLocked) this->_lockInstance->_unLock();
}

/**
 * @brief Relock
 *
 * @param lockMode Lock mode
 */
void LockGuard::reLock(const ThreadLock::LockMode lockMode) noexcept
{
    if (this->_isLocked)
    {
        if (lockMode == this->_lockMode) return;
        this->_lockInstance->_unLock();
    }
    this->_lockMode = lockMode;
    this->_isLocked = true;
    this->_lockInstance->_reLock(lockMode);
}

/**
 * @brief Unlock
 */
void LockGuard::unLock() noexcept
{
    this->_isLocked = false;
    this->_lockInstance->_unLock();
}