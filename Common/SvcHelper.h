/**
 * @brief Service Helper
 *
 * @author WindEagle <fy516a@gmail.com>
 * @version 1.0.0
 * @date 2020-01-01 00:00
 * @copyright Copyright (c) 2020-2023 ZyTech Team
 * @par Changelog:
 * Date                 Version     Author          Description
 */
#ifndef OFW_SVCHELPER_H
#define OFW_SVCHELPER_H

//################################################################################
// Include header file
//################################################################################
#include "../base/basedefine.h"
#include "../base/typedefine.h"
#include "../base/funcdefine.h"

//################################################################################
// Define export macro
//################################################################################
// Service status
#define OFW_SVC_S_ERROR        0x0000 // Failed to obtain service status
#define OFW_SVC_S_NOTINSTALLED 0x1000 // Not installed
#define OFW_SVC_S_INSTALLED    0x2000 // Installed
#define OFW_SVC_S_DISABLED     0x0100 // [CONFIG] Disabled
#define OFW_SVC_S_DEMAND       0x0200 // [CONFIG] Demand
#define OFW_SVC_S_AUTORUN      0x0400 // [CONFIG] Auto run
#define OFW_SVC_S_STOPED       0x0001 // [STATUS] Stoped
#define OFW_SVC_S_STOPPING     0x0002 // [STATUS] Stopping
#define OFW_SVC_S_STARTING     0x0004 // [STATUS] Starting
#define OFW_SVC_S_RUNNING      0x0008 // [STATUS] Running

//################################################################################
// Define export type
//################################################################################
namespace ofw
{
    /**
     * @brief The service worker function
     * 
     * @return Exit code
     */
    typedef std::function<int()> TSvcWorkerFunc;
}

//################################################################################
// Define export method
//################################################################################
namespace ofw
{
    /**
     * @brief [THREAD_SAFE|ERROR_CODE] Get service status
     *
     * @param svcname Target service name
     * @return Service status (OFW_SVC_S_ERROR: failed; Other: successed)
     */
    int get_service_status(const char * svcname);

    /**
     * @brief [THREAD_SAFE|ERROR_CODE] Install service
     *
     * @param appfile     The file path of the service program
     * @param execparams  Parameters for the application to run in service mode
     * @param svcname     Service name
     * @param dispname    Service display name (Only valid in Windows environment)
     * @param description Service description
     * @param depends     Service dependencies (Recommend: "")
     * @param username    Service runs account name (Recommend: "")
     * @param password    Service runs account password (Recommend: "")
     * @return Whether the service is installed successfully
     */
    bool service_install(const char * appfile, const char * execparams, const char * svcname, const char * dispname, const char * description, const char * depends, const char * username, const char * password);

    /**
     * @brief [THREAD_SAFE|ERROR_CODE] Uninstall service
     *
     * @param svcname Service name
     * @return Whether the service is uninstalled successfully
     */
    bool service_uninstall(const char * svcname);

    /**
     * @brief [THREAD_SAFE|ERROR_CODE] Start service
     *
     * @param svcname Service name
     * @return Whether the start was successful
     */
    bool service_start(const char * svcname);

    /**
     * @brief [THREAD_SAFE|ERROR_CODE] Stop service
     *
     * @param svcname Service name
     * @return Whether the stop was successful
     */
    bool service_stop(const char * svcname);
}

#endif
