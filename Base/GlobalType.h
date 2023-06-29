/**
 * @brief Global Type
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

//================================================================================
// Define export macro
//================================================================================
// Define type
#if defined(_MSC)
typedef unsigned char      uchar;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef long long          longlong;
typedef unsigned long long ulonglong;
typedef SSIZE_T            ssize_t;
typedef DWORD              pid_t;
typedef DWORD              pthread_t;
#elif defined(_GCC)
typedef unsigned char      uchar;
typedef long long          longlong;
typedef unsigned long long ulonglong;
#endif
