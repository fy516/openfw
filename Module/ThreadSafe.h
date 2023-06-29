/**
 * @brief Thread Safe
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

//================================================================================
// Define preset type
//================================================================================
class LockGuard;

//================================================================================
// Define export type
//================================================================================
/**
 * @brief Thread lock
 */
class ThreadLock final
{
    friend class LockGuard;

public:
    /**
     * @brief Lock type
     */
    enum LockType
    {
        Mutex, // Mutex lock
        RwLock // Read write lock
    };

    /**
     * @brief Lock mode
     */
    enum LockMode
    {
        Read, // Read lock
        Write // Write lock
    };

private:
    /**
     * @brief Lock type
     */
    LockType _lockType;

    /**
     * @brief Whether used to multi process
     */
    bool _isMultiProcess;

    /**
     * @brief Lock instance
     */
    void *_lockInstance;

public:
    /**
     * @brief Construct function
     *
     * @param lockType Lock type
     * @param lockName Lock name (Nullptr: thread lock; Other: process lock)
     */
    ThreadLock(const LockType lockType, const char *lockName = nullptr) noexcept;

    /**
     * @brief Destruct function
     */
    ~ThreadLock();

private:
    /**
     * @brief Relock
     */
    void _reLock(const LockMode lockMode) noexcept;

    /**
     * @brief Unlock
     */
    void _unLock() noexcept;
};

/**
 * @brief Lock guard
 */
class LockGuard final
{
    /**
     * @brief Disabled heap create
     */
    void *operator new(size_t)   = delete;
    void *operator new[](size_t) = delete;

    /**
     * @brief Disabled heap destroy
     */
    void operator delete(void *)   = delete;
    void operator delete[](void *) = delete;

private:
    /**
     * @brief Lock instance
     */
    ThreadLock *_lockInstance;

    /**
     * @brief Lock mode
     */
    ThreadLock::LockMode _lockMode;

    /**
     * @brief Whether to locked
     */
    bool _isLocked;

public:
    /**
     * @brief Construct function
     *
     * @param lockInstance Lock instance
     * @param lockMode     Lock mode
     * @param lockNow      Wether to lock it now
     */
    LockGuard(ThreadLock *lockInstance, const ThreadLock::LockMode lockMode, const bool lockNow) noexcept;

    /**
     * @brief Destruct function
     */
    ~LockGuard();

    /**
     * @brief Relock
     *
     * @param lockMode Lock mode
     */
    void reLock(const ThreadLock::LockMode lockMode) noexcept;

    /**
     * @brief Unlock
     */
    void unLock() noexcept;
};