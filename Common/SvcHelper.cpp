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
//################################################################################
// Include header file
//################################################################################
#include "svchelper.h"
#include "enchelper.h"
#include "filehelper.h"
#include "loghelper.h"
#include "strhelper.h"
#include "syshelper.h"
#ifdef _LINUX
    #include <sys/file.h>
    #include <sys/wait.h>
#endif

//################################################################################
// Define inside macro
//################################################################################
#ifdef _LINUX
    //============================================================
    // Service control command type
    //============================================================
    #define LINUX_SVCCTRL_TYPE_UNAVAILABLE 0 // No command available
    #define LINUX_SVCCTRL_TYPE_UNAUTH      1 // Unauthorized access
    #define LINUX_SVCCTRL_TYPE_SYSTEMCTL   2 // The "systemctl" command is available
    #define LINUX_SVCCTRL_TYPE_SERVICE     3 // The "service" command is available

    //============================================================
    // Set the suffix of the Linux command to silent mode
    //============================================================
    #define LINUX_COMMAND_SILENT_SUFFIX " >/dev/null 2>&1"

    //============================================================
    // Linux systemctl service file template
    //============================================================
    #define LINUX_SERVICE_FILE_TEMPLATE_1                                     \
        "# It's not recommended to modify this file in-place, because it\n"   \
        "# will be overwritten during upgrades.  If you want to customize,\n" \
        "# the best way is to use the \"systemctl edit\" command.\n"          \
        "\n"                                                                  \
        "[Unit]\n"                                                            \
        "Description=[SVC_DISPLAY_NAME]\n"                                    \
        "After=syslog.target[SVC_DEPENDENCIES]\n"                             \
        "\n"                                                                  \
        "[Service]\n"                                                         \
        "Type=forking\n"                                                      \
        "PIDFile=/run/[SVC_NAME].pid\n"                                       \
        "ExecStart=[APP_PATH_AND_PARAMS]\n"                                   \
        "ExecStop=/bin/kill -s TERM $MAINPID\n"                               \
        "Restart=always\n"                                                    \
        "\n"                                                                  \
        "[Install]\n"                                                         \
        "WantedBy=multi-user.target\n"

    //============================================================
    // Linux chkconfig service file template
    //============================================================
    #define LINUX_SERVICE_FILE_TEMPLATE_2                                                                            \
        "#! /bin/sh\n"                                                                                               \
        "\n"                                                                                                         \
        "### BEGIN INIT INFO\n"                                                                                      \
        "# Provides:          [SVC_NAME]\n"                                                                          \
        "# Required-Start:    $remote_fs $syslog\n"                                                                  \
        "# Required-Stop:     $remote_fs $syslog\n"                                                                  \
        "# Should-Start:      $named autofs\n"                                                                       \
        "# Default-Start:     2 3 4 5\n"                                                                             \
        "# Default-Stop:      \n"                                                                                    \
        "# Description:       [SVC_DISPLAY_NAME]\n"                                                                  \
        "### END INIT INFO\n"                                                                                        \
        "\n"                                                                                                         \
        "set -e\n"                                                                                                   \
        "\n"                                                                                                         \
        "# Service name\n"                                                                                           \
        "SVC_NAME=[SVC_NAME]\n"                                                                                      \
        "# Application path\n"                                                                                       \
        "APP_PATH=[APP_PATH]\n"                                                                                      \
        "# Application execute parameters\n"                                                                         \
        "APP_PARAMS=[APP_PARAMS]\n"                                                                                  \
        "# PID file path\n"                                                                                          \
        "PID_FILE=/run/${SVC_NAME}.pid\n"                                                                            \
        "\n"                                                                                                         \
        "start()\n"                                                                                                  \
        "{\n"                                                                                                        \
        "    echo -n \"[....] Starting $SVC_NAME: \"\n"                                                              \
        "    [ -d $PID_DIR ] || mkdir $PID_DIR\n"                                                                    \
        "    # Return\n"                                                                                             \
        "    #   0 if daemon has been started\n"                                                                     \
        "    #   1 if daemon was already running\n"                                                                  \
        "    #   other if daemon could not be started or a failure occured\n"                                        \
        "    start-stop-daemon --start --quiet --pidfile \"$PID_FILE\" --exec \"$APP_PATH\" -- $APP_PARAMS\n"        \
        "    case \"$?\" in\n"                                                                                       \
        "        0,1)\n"                                                                                             \
        "            echo -e '[\033[32m ok \033[0m]'\n"                                                              \
        "            ;;\n"                                                                                           \
        "        *)\n"                                                                                               \
        "            echo -e '\033[31mfailed!\033[0m'\n"                                                             \
        "            ;;\n"                                                                                           \
        "    esac\n"                                                                                                 \
        "}\n"                                                                                                        \
        "stop()\n"                                                                                                   \
        "{\n"                                                                                                        \
        "    echo -n \"[....] Stopping $SVC_NAME: \"\n"                                                              \
        "    # Return\n"                                                                                             \
        "    #   0 if daemon has been stopped\n"                                                                     \
        "    #   1 if daemon was already stopped\n"                                                                  \
        "    #   other if daemon could not be stopped or a failure occurred\n"                                       \
        "    start-stop-daemon --stop --quiet --retry=TERM/30/KILL/5 --pidfile \"$PID_FILE\" --exec \"$APP_PATH\"\n" \
        "    case \"$?\" in\n"                                                                                       \
        "        0,1)\n"                                                                                             \
        "            echo -e '[\033[32m ok \033[0m]'\n"                                                              \
        "            ;;\n"                                                                                           \
        "        *)\n"                                                                                               \
        "            echo -e '\033[31mfailed!\033[0m'\n"                                                             \
        "            ;;\n"                                                                                           \
        "    esac\n"                                                                                                 \
        "}\n"                                                                                                        \
        "restart()\n"                                                                                                \
        "{\n"                                                                                                        \
        "    stop\n"                                                                                                 \
        "    start\n"                                                                                                \
        "}\n"                                                                                                        \
        "reload()\n"                                                                                                 \
        "{\n"                                                                                                        \
        "    echo -n \"Reloading $SVC_NAME :\"\n"                                                                    \
        "    start-stop-daemon --stop --quiet --signal USR2 --pidfile \"$PID_FILE\" --exec \"$APP_PATH\"\n"          \
        "\n"                                                                                                         \
        "    killproc -p ${PID_FILE} bathtub-server -USR2\n"                                                         \
        "    RET_VAL=$?\n"                                                                                           \
        "    echo\n"                                                                                                 \
        "}\n"                                                                                                        \
        "status()\n"                                                                                                 \
        "{\n"                                                                                                        \
        "    # Return\n"                                                                                             \
        "    #   0 if daemon is running\n"                                                                           \
        "    #   1 if daemon is not running and the pid file exists.\n"                                              \
        "    #   2 if daemon is not running.\n"                                                                      \
        "    #   other if unable to determine status.\n"                                                             \
        "    start-stop-daemon --status --quiet --pidfile \"$PID_FILE\" --exec \"$APP_PATH\"\n"                      \
        "    case \"$?\" in\n"                                                                                       \
        "        0)\n"                                                                                               \
        "            echo -e \"[\033[32m ok \033[0m] $SVC_NAME is running.\"\n"                                      \
        "            ;;\n"                                                                                           \
        "        *)\n"                                                                                               \
        "            echo -e \"[\033[31mFAIL\033[0m] $SVC_NAME is not running ... \033[31m\"'failed!'\"\033[0m\"\n"  \
        "            ;;\n"                                                                                           \
        "    esac\n"                                                                                                 \
        "}\n"                                                                                                        \
        "case \"$1\" in\n"                                                                                           \
        "    start)\n"                                                                                               \
        "        start\n"                                                                                            \
        "        ;;\n"                                                                                               \
        "    stop)\n"                                                                                                \
        "        stop\n"                                                                                             \
        "        ;;\n"                                                                                               \
        "    reload)\n"                                                                                              \
        "        reload\n"                                                                                           \
        "        ;;\n"                                                                                               \
        "    restart)\n"                                                                                             \
        "        restart\n"                                                                                          \
        "        ;;\n"                                                                                               \
        "    status)\n"                                                                                              \
        "        status\n"                                                                                           \
        "        ;;\n"                                                                                               \
        "    *)\n"                                                                                                   \
        "        echo \"Usage: $0 {start|stop|reload|restart|status}\"\n"                                            \
        "        exit 1\n"                                                                                           \
        "        ;;\n"                                                                                               \
        "esac\n"

#endif

//################################################################################
// Define inside class
//################################################################################
#ifdef _WINDOWS
//============================================================
// SC_HANDLE auto release class
//============================================================
class SC_HANDLE__EX
{
private:
    SC_HANDLE m_handle;

public:
    SC_HANDLE__EX & operator=(SC_HANDLE handle)
    {
        if (handle != m_handle)
        {
            if (m_handle) ::CloseServiceHandle(m_handle);
            m_handle = handle;
        }
        return *this;
    }
    operator SC_HANDLE() { return m_handle; }

public:
    SC_HANDLE__EX(SC_HANDLE handle = nullptr) : m_handle(handle) {}
    ~SC_HANDLE__EX()
    {
        if (m_handle) ::CloseServiceHandle(m_handle);
    }
};

//============================================================
// LPENUM_SERVICE_STATUS auto release class
//============================================================
class LPENUM_SERVICE_STATUS__EX
{
private:
    LPENUM_SERVICE_STATUS m_point;

public:
    LPENUM_SERVICE_STATUS__EX & operator=(LPENUM_SERVICE_STATUS point)
    {
        if (point != m_point)
        {
            if (m_point) ::HeapFree(::GetProcessHeap(), 0, m_point);
            m_point = point;
        }
        return *this;
    }
    operator LPENUM_SERVICE_STATUS() { return m_point; }

public:
    LPENUM_SERVICE_STATUS__EX(LPENUM_SERVICE_STATUS point = nullptr) : m_point(point) {}
    ~LPENUM_SERVICE_STATUS__EX()
    {
        if (m_point) ::HeapFree(::GetProcessHeap(), 0, m_point);
    }
};
#endif

//################################################################################
// Implementation inside method
//################################################################################
#ifdef _WINDOWS
/**
 * @brief Stop all dependent services
 *
 * @param manager  Source serivce manager
 * @param service  Source service handle
 * @return Whether the stop was successful
 */
static bool __stdcall __stop_depend_services(SC_HANDLE__EX & manager, SC_HANDLE__EX & service)
{
    // Define inside variable
    SC_HANDLE__EX             svc_handle;                      // Dependent service handle
    LPENUM_SERVICE_STATUS__EX svc_depends;                     // Dependent service list
    DWORD                     bytes_needed;                    // Service status bytes needed
    DWORD                     depends_count;                   // Depends count
    ENUM_SERVICE_STATUS       enum_status;                     // Enum service status
    SERVICE_STATUS_PROCESS    svc_status;                      // Service status
    DWORD                     start_time   = ::GetTickCount(); // Start time
    DWORD                     stop_timeout = 30000;            // Define stop timeout

    // Pass a zero-length buffer to get the required buffer size.
    // If the Enum call succeeds, then there are no dependent services, so do nothing.
    if (::EnumDependentServicesA(service, SERVICE_ACTIVE, svc_depends, 0, &bytes_needed, &depends_count)) return true;

    // Unexpected error
    if (::GetLastError() != ERROR_MORE_DATA) return false;

    // Allocate a buffer for the dependencies.
    svc_depends = (LPENUM_SERVICE_STATUS)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, bytes_needed);
    if (!svc_depends) return false;

    try
    {
        // Enumerate the dependencies.
        if (!::EnumDependentServicesA(service, SERVICE_ACTIVE, svc_depends, bytes_needed, &bytes_needed, &depends_count)) return false;

        // Stop all the dependencies.
        for (DWORD depends_index = 0; depends_index < depends_count; depends_index++)
        {
            enum_status = *(svc_depends + depends_index);

            // Open the service.
            svc_handle = ::OpenServiceA(manager, enum_status.lpServiceName, SERVICE_QUERY_STATUS | SERVICE_STOP);
            if (!svc_handle) return false;

            // Send a stop code.
            if (!::ControlService(svc_handle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&svc_status)) return false;

            // Wait for the service to stop.
            while (svc_status.dwCurrentState != SERVICE_STOPPED)
            {
                ::Sleep(svc_status.dwWaitHint);
                if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;
                if (svc_status.dwCurrentState == SERVICE_STOPPED) break;
                if (::GetTickCount() - start_time > stop_timeout) return false;
            }
        }
    }
    catch (...)
    {
        return false;
    }

    // Return execute result
    return true;
}
#endif
#ifdef _LINUX
/**
 * @brief Check service control type
 *
 * @return Execute status (LINUX_SVCCTRL_EXEC_UNAVAILABLE: unavailable: LINUX_SVCCTRL_EXEC_FAILED; failed: LINUX_SVCCTRL_EXEC_SUCCESSED: successed)
 */
static int __check_service_ctrltype()
{
    // The "systemctl" command is supported
    if (::system("systemctl --version" LINUX_COMMAND_SILENT_SUFFIX) == 0) return (::system("sudo -n systemctl --version" LINUX_COMMAND_SILENT_SUFFIX) == 0 ? LINUX_SVCCTRL_TYPE_SYSTEMCTL : LINUX_SVCCTRL_TYPE_UNAUTH);

    // The "service" command is supported
    if (::system("service --version" LINUX_COMMAND_SILENT_SUFFIX) == 0) return (::system("sudo -n service --version" LINUX_COMMAND_SILENT_SUFFIX) == 0 ? LINUX_SVCCTRL_TYPE_SERVICE : LINUX_SVCCTRL_TYPE_UNAUTH);

    // No command available
    return LINUX_SVCCTRL_TYPE_UNAVAILABLE;
}

/**
 * @brief Execute service control command
 *
 * @param svcname Target service name
 * @param command Service control command
 * @return Execute status (LINUX_SVCCTRL_EXEC_UNAVAILABLE: unavailable: LINUX_SVCCTRL_EXEC_FAILED; failed: LINUX_SVCCTRL_EXEC_SUCCESSED: successed)
 */
static bool __exec_svc_ctrlcmd(const char * svcname, const char * command)
{
    // Define temporary variables
    std::unique_ptr<char[]> ctrl_cmd(new char[::strlen(command) - 2 /* %s */ + ::strlen(svcname) + sizeof(LINUX_COMMAND_SILENT_SUFFIX) /* Silent suffix */]); // Service control command

    // Initialize smart pointer memory
    ::memset(ctrl_cmd.get(), 0, (::strlen(command) - 2 /* %s */ + ::strlen(svcname) + sizeof(LINUX_COMMAND_SILENT_SUFFIX) /* Silent suffix */) * sizeof(char));

    // Generate service control command
    ::sprintf(ctrl_cmd.get(), command, svcname);
    ::memcpy(ctrl_cmd.get() + ::strlen(command) - 2 + ::strlen(svcname), LINUX_COMMAND_SILENT_SUFFIX, sizeof(LINUX_COMMAND_SILENT_SUFFIX));

    // Execute service control command
    if (::system(ctrl_cmd.get()) == 0) return true;

    // No command executed successfully
    return false;
}
#endif

//################################################################################
// Implementation export method
//################################################################################
/**
 * @brief [THREAD_SAFE|ERROR_CODE] Get service status
 *
 * @param svcname Target service name
 * @return Service status (OFW_SVC_S_ERROR: failed; Other: successed)
 */
int ofw::get_service_status(const char * svcname)
{
    // Define inside variable
    int ret_status = OFW_SVC_S_ERROR; // The value used to return

#ifdef _WINDOWS
    // Get a handle to the SCM database.
    SC_HANDLE__EX svc_manager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!svc_manager) return OFW_SVC_S_ERROR;

    // Open service
    SC_HANDLE__EX svc_handle = ::OpenServiceA(svc_manager, ofw::utf8_to_ansi(svcname).get(), SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (svc_handle)
        ret_status = OFW_SVC_S_INSTALLED;
    else if (::GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
        ret_status = OFW_SVC_S_NOTINSTALLED;
    else
        return OFW_SVC_S_ERROR;

    // Query service config
    if ((ret_status & OFW_SVC_S_INSTALLED) == OFW_SVC_S_INSTALLED)
    {
        // Define temporary variables
        DWORD                  has_size = 0;       // Required buffer size
        DWORD                  buf_size = 0;       // Buffer size
        LPQUERY_SERVICE_CONFIG svc_lpsc = nullptr; // Config information

        // Get the required buffer size
        if (!::QueryServiceConfigA(svc_handle, NULL, 0, &has_size))
        {
            if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) return OFW_SVC_S_ERROR;

            buf_size = has_size;
            svc_lpsc = (LPQUERY_SERVICE_CONFIG)::LocalAlloc(LMEM_FIXED, buf_size);
        }

        // Query service config
        if (!::QueryServiceConfigA(svc_handle, svc_lpsc, buf_size, &has_size))
        {
            if (svc_lpsc) ::LocalFree(svc_lpsc);
            return OFW_SVC_S_ERROR;
        }

        // Define return value
        if (svc_lpsc->dwStartType == SERVICE_DISABLED)
            ret_status |= OFW_SVC_S_DISABLED;
        else if (svc_lpsc->dwStartType == SERVICE_DEMAND_START)
            ret_status |= OFW_SVC_S_DEMAND;
        else if (svc_lpsc->dwStartType == SERVICE_AUTO_START)
            ret_status |= OFW_SVC_S_AUTORUN;

        // Release config informaton
        if (svc_lpsc) ::LocalFree(svc_lpsc);
    }

    // Query service status
    if ((ret_status & OFW_SVC_S_INSTALLED) == OFW_SVC_S_INSTALLED)
    {
        // Define temporary variables
        SERVICE_STATUS svc_status = {}; // Service status

        // Query service status
        if (!::QueryServiceStatus(svc_handle, &svc_status)) return OFW_SVC_S_ERROR;

        // Define return value
        if (svc_status.dwCurrentState == SERVICE_STOPPED)
            ret_status |= OFW_SVC_S_STOPED;
        else if (svc_status.dwCurrentState == SERVICE_STOP_PENDING)
            ret_status |= OFW_SVC_S_STOPPING;
        else if (svc_status.dwCurrentState == SERVICE_START_PENDING)
            ret_status |= OFW_SVC_S_STARTING;
        else if (svc_status.dwCurrentState == SERVICE_RUNNING)
            ret_status |= OFW_SVC_S_RUNNING;
    }
#endif
#ifdef _LINUX
    // Define temporary variables
    int svcctrl_type = __check_service_ctrltype(); // Service control command type

    // Check service control command type
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAVAILABLE, ENOENT, return OFW_SVC_S_ERROR);
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAUTH,      EPERM,  return OFW_SVC_S_ERROR);

    // Check service installed
    {
        const char * cmd_systemctl = "systemctl list-unit-files --type=service | grep -E \"(^|\\n|\\t| )%s\\.service\"";
        const char * cmd_service   = "service --status-all | grep -E \"(^|\\n|\\t| )%s($|\\n|\\t| )\"";

        if (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL && __exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? cmd_systemctl : cmd_service))
            ret_status |= OFW_SVC_S_INSTALLED;
        else
            ret_status |= OFW_SVC_S_NOTINSTALLED;
    }

    // Check service start type
    if ((ret_status & OFW_SVC_S_INSTALLED) == OFW_SVC_S_INSTALLED)
    {
        const char * cmd_systemctl = "systemctl list-unit-files --type=service --state=enabled | grep -E \"(^|\\n|\\t| )%s\\.service\"";
        const char * cmd_service   = "chkconfig -c %s";

        if (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL && __exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? cmd_systemctl : cmd_service))
            ret_status |= OFW_SVC_S_AUTORUN;
        else
            ret_status |= OFW_SVC_S_DEMAND;
    }

    // Check service status
    if ((ret_status & OFW_SVC_S_INSTALLED) == OFW_SVC_S_INSTALLED)
    {
        const char * cmd_systemctl = "systemctl list-units --type=service --state=running | grep -E \"(^|\\n|\\t| )%s\\.service\"";
        const char * cmd_service   = "service %s status";

        if (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL && __exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? cmd_systemctl : cmd_service))
            ret_status |= OFW_SVC_S_RUNNING;
        else
            ret_status |= OFW_SVC_S_STOPED;
    }
#endif

    // Return execute result
    return ret_status;
}

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
bool ofw::service_install(const char * appfile, const char * execparams, const char * svcname, const char * dispname, const char * description, const char * depends, const char * username, const char * password)
{
    // Check parameters for validity
    OFW_CHECK(appfile && *appfile != '\0', EINVAL, return false);
    OFW_CHECK(svcname && *svcname != '\0', EINVAL, return false);
    OFW_CHECK(dispname && *dispname != '\0', EINVAL, return false);

    // Check service status
    {
        // Define temporary variables
        int svc_status = ofw::get_service_status(svcname);

        // Check if the service is error
        OFW_CHECK(svc_status != OFW_SVC_S_ERROR, ENOENT, return false);

        // Check if the service is installed
        if ((svc_status & OFW_SVC_S_INSTALLED) == OFW_SVC_S_INSTALLED) return true;
    }

#ifdef _WINDOWS
    // Define temporary variables
    std::unique_ptr<char[]> svc_run_cmd(nullptr); // The command that the service runs

    // Convert application path to the command that the service runs
    {
        // Convert "\"" in application path to "\\\""
        std::unique_ptr<char[]> app_path = ofw::str_replace(appfile, "\"", "\\\"");

        // Reset service run command string space
        svc_run_cmd.reset(new char[::strlen(app_path.get()) + (execparams && *execparams != '\0' ? (::strlen(execparams) + 1 /*space*/) : 0) + 2 /*Double quotes*/ + 1 /*Terminator*/]);
        ::memset(svc_run_cmd.get(), 0, (::strlen(app_path.get()) + (execparams && *execparams != '\0' ? (::strlen(execparams) + 1 /*space*/) : 0) + 2 /*Double quotes*/ + 1 /*Terminator*/) * sizeof(char));

        // Generate a string for the service run command
        ::sprintf(svc_run_cmd.get(), (execparams && *execparams != '\0' ? "\"%s\" %s" : "\"%s\"%s"), app_path.get(), execparams && *execparams != '\0' ? execparams : "");
    }

    // Install service
    {
        // Get a handle to the SCM database.
        SC_HANDLE__EX svc_manager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
        if (!svc_manager) return false;

        // Create service
        SC_HANDLE__EX svc_handle = ::CreateServiceA(
            svc_manager,
            ofw::utf8_to_ansi(svcname).get(),
            ofw::utf8_to_ansi(dispname).get(),
            SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            ofw::utf8_to_ansi(svc_run_cmd.get()).get(),
            NULL,
            NULL,
            depends ? ofw::utf8_to_ansi(depends).get() : NULL,
            username ? ofw::utf8_to_ansi(username).get() : NULL,
            password ? ofw::utf8_to_ansi(password).get() : NULL);
        if (!svc_handle) return false;

        // Config service description
        if (description)
        {
            // Define temporary variables
            SERVICE_DESCRIPTION     svc_desc;                                        // Service description
            std::unique_ptr<char[]> str_desc = ofw::utf8_to_ansi(description); // Description string

            // Initialize service description
            ::memset(&svc_desc, 0, sizeof(svc_desc));
            svc_desc.lpDescription = str_desc.get();

            // Change service description
            if (!::ChangeServiceConfig2A(svc_handle, SERVICE_CONFIG_DESCRIPTION, &svc_desc)) OFW_PERROR(OFW_ERR_L_WARNING, description);
        }
    }
#endif
#ifdef _LINUX
    // Install service
    {
        // Define temporary variables
        int         svcctrl_type     = __check_service_ctrltype(); // Service control command type
        std::string svc_file_content = "";                   // Contents of the service file

        // Check service control command type
        OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAVAILABLE, ENOENT, return false);
        OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAUTH, EPERM, return false);

        // Replace the template label
        {
            // Replace "[APP_PATH]"            in service file template to "Application path"
            // Replace "[APP_PARAMS]"          in service file template to "Application params"
            // Replace "[APP_PATH_AND_PARAMS]" in service file template to "Application path and params"
            {
                std::unique_ptr<char[]> app_path = ofw::str_replace(appfile, "\"", "\\\"");
                std::unique_ptr<char[]> app_params = ofw::str_replace(execparams, "\"", "\\\"");
                std::unique_ptr<char[]> app_path_and_params(new char[::strlen(app_path.get()) + (execparams && *execparams != '\0' ? (::strlen(execparams) + 1 /*space*/) : 0) + 2 /*Double quotes*/ + 1 /*Terminator*/]);

                // Initialize smart pointer memory
                ::memset(app_path_and_params.get(), 0, (::strlen(app_path.get()) + (execparams && *execparams != '\0' ? (::strlen(execparams) + 1 /*space*/) : 0) + 2 /*Double quotes*/ + 1 /*Terminator*/) * sizeof(char));

                // Generate a string for the service run command
                ::sprintf(app_path_and_params.get(), (execparams && *execparams != '\0' ? "\"%s\" %s" : "\"%s\"%s"), app_path.get(), execparams && *execparams != '\0' ? execparams : "");

                // Set service file template
                svc_file_content = (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? LINUX_SERVICE_FILE_TEMPLATE_1 : LINUX_SERVICE_FILE_TEMPLATE_2);

                // Replace "[APP_PATH]"
                {
                    std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[APP_PATH]", app_path.get());
                    svc_file_content                 = conv_str.get();
                }

                // Replace "[APP_PARAMS]"
                {
                    std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[APP_PARAMS]", app_params.get());
                    svc_file_content                 = conv_str.get();
                }

                // Replace "[APP_PATH_AND_PARAMS]"
                {
                    std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[APP_PATH_AND_PARAMS]", app_path_and_params.get());
                    svc_file_content                 = conv_str.get();
                }
            }

            // Convert "[SVC_NAME]" in service file template to "Service display name"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_NAME]", svcname);
                svc_file_content                 = conv_str.get();
            }

            // Convert "[SVC_DISPLAY_NAME]" in service file template to "Service display name"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_DISPLAY_NAME]", dispname);
                svc_file_content                 = conv_str.get();
            }

            // Convert "[SVC_DESCRIPTION]" in service file template to "Service display name"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_DESCRIPTION]", description);
                svc_file_content                 = conv_str.get();
            }

            // Convert "[SVC_DEPENDENCIES]" in service file template to "Service dependencies"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_DEPENDENCIES]", depends);
                svc_file_content                 = conv_str.get();
            }

            // Convert "[SVC_ACCT_NAME]" in service file template to "Service runs account name"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_ACCT_NAME]", username);
                svc_file_content                 = conv_str.get();
            }

            // Convert "[SVC_ACCT_PASSWORD]" in service file template to "Service runs account password"
            {
                std::unique_ptr<char[]> conv_str = ofw::str_replace(svc_file_content.c_str(), "[SVC_ACCT_PASSWORD]", password);
                svc_file_content                 = conv_str.get();
            }
        }

        // Write service file and enable service auto start
        {
            std::unique_ptr<char[]> svc_path(new char[::strlen(svcname) + (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? sizeof("/usr/lib/systemd/system/%s.service") : sizeof("/etc/init.d/%s")) - 2 /* %s */]);

            // Initialize smart pointer memory
            ::memset(svc_path.get(), 0, (::strlen(svcname) + (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? sizeof("/usr/lib/systemd/system/%s.service") : sizeof("/etc/init.d/%s")) - 2 /* %s */) * sizeof(char));

            // Generate service file path
            ::sprintf(svc_path.get(), (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? "/usr/lib/systemd/system/%s.service" : "/etc/init.d/%s"), svcname);

            // Write service file
            if (ofw::file_write(svc_path.get(), OFW_FILE_O_CREAT | OFW_FILE_O_WRONLY | OFW_FILE_O_TRUNC, svc_file_content.c_str(), svc_file_content.length()) == -1) return false;

            // Add execute permission to service script
            if (!__exec_svc_ctrlcmd(svc_path.get(), "chmod 755 %s") ||
                (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL && !__exec_svc_ctrlcmd(svcname, "systemctl enable %s")) ||
                (svcctrl_type == LINUX_SVCCTRL_TYPE_SERVICE && !__exec_svc_ctrlcmd(svcname, "chkconfig %s on")))
            {
                __exec_svc_ctrlcmd(svc_path.get(), "rm -f %s");
                return false;
            }
        }
    }
#endif

    // Return execute result
    return true;
}

/**
 * @brief [THREAD_SAFE|ERROR_CODE] Uninstall service
 *
 * @param svcname Service name
 * @return Whether the service is uninstalled successfully
 */
bool ofw::service_uninstall(const char * svcname)
{
    // Check parameters for validity
    OFW_CHECK(svcname && *svcname != '\0', EINVAL, return false);

    // Check service status
    {
        // Define inside variable
        int svc_status = ofw::get_service_status(svcname); // The status of the target service

        // Check if the service is error
        OFW_CHECK(svc_status != OFW_SVC_S_ERROR, ENOENT, return false);

        // If the service is not installed, it returns true directly
        if ((svc_status & OFW_SVC_S_NOTINSTALLED) == OFW_SVC_S_NOTINSTALLED) return true;

        // Stop service
        if ((svc_status & OFW_SVC_S_STOPED) != OFW_SVC_S_STOPED && !ofw::service_stop(svcname)) return false;
    }

#ifdef _WINDOWS
    // Get a handle to the SCM database.
    SC_HANDLE__EX svc_manager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!svc_manager) return false;

    // Get a handle to the service.
    SC_HANDLE__EX svc_handle = ::OpenServiceA(svc_manager, ofw::utf8_to_ansi(svcname).get(), DELETE);
    if (!svc_handle) return false;

    // Delete service
    if (!::DeleteService(svc_handle)) return false;
#endif
#ifdef _LINUX
    // Define temporary variables
    int         svcctrl_type     = __check_service_ctrltype(); // Service control command type
    std::string svc_file_content = "";                   // Contents of the service file

    // Check service control command type
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAVAILABLE, ENOENT, return false);
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAUTH, EPERM, return false);

    // Disable service auto start
    if (!__exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? "systemctl disable %s" : "chkconfig %s off")) return false;

    // Remove service file
    {
        std::unique_ptr<char[]> svc_path(new char[::strlen(svcname) + (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? sizeof("/usr/lib/systemd/system/%s.service") : sizeof("/etc/init.d/%s")) - 2 /* %s */]);

        // Initialize smart pointer memory
        ::memset(svc_path.get(), 0, (::strlen(svcname) + (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? sizeof("/usr/lib/systemd/system/%s.service") : sizeof("/etc/init.d/%s")) - 2 /* %s */) * sizeof(char));

        // Generate service file path
        ::sprintf(svc_path.get(), (svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? "/usr/lib/systemd/system/%s.service" : "/etc/init.d/%s"), svcname);

        // Remove service file
        if (!__exec_svc_ctrlcmd(svc_path.get(), "rm -f %s")) return false;
    }
#endif

    // Return execute result
    return true;
}

/**
 * @brief [THREAD_SAFE|ERROR_CODE] Start service
 *
 * @param svcname Service name
 * @return Whether the start was successful
 */
bool ofw::service_start(const char * svcname)
{
    // Check parameters for validity
    OFW_CHECK(svcname && *svcname != '\0', EINVAL, return false);

    // Check service status
    {
        // Define temporary variables
        int svc_status = ofw::get_service_status(svcname);

        // Check if the service is error
        OFW_CHECK(svc_status != OFW_SVC_S_ERROR, ENOENT, return false);

        // Check if the service is not installed
        if ((ofw::get_service_status(svcname) & OFW_SVC_S_NOTINSTALLED) == OFW_SVC_S_NOTINSTALLED) return false;

        // Check if the service is already running
        if ((ofw::get_service_status(svcname) & OFW_SVC_S_RUNNING) == OFW_SVC_S_RUNNING) return true;
    }

#ifdef _WINDOWS
    // Define inside variable
    SC_HANDLE__EX          svc_manager;      // Service manager handle
    SC_HANDLE__EX          svc_handle;       // Service handle
    SERVICE_STATUS_PROCESS svc_status;       // Service status and process information
    DWORD                  old_check_point;  // Old check point
    DWORD                  start_tick_count; // Service start tick count
    DWORD                  wait_time;        // Service control wait time
    DWORD                  bytes_needed;     // Service status bytes needed

    // Get a handle to the SCM database
    svc_manager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!svc_manager) return false;

    // Get a handle to the service
    svc_handle = ::OpenServiceA(svc_manager, ofw::ansi_to_utf8(svcname).get(), SERVICE_QUERY_STATUS | SERVICE_START);
    if (!svc_handle) return false;

    // Check the status in case the service is not stopped
    if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;

    // Check if the service is already running
    // It would be possible to stop the service here, but for simplicity this example just returns
    if (svc_status.dwCurrentState != SERVICE_STOPPED && svc_status.dwCurrentState != SERVICE_STOP_PENDING) return true;

    // Save the tick count and initial checkpoint
    start_tick_count = ::GetTickCount();
    old_check_point  = svc_status.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (svc_status.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint
        // A good interval is one-tenth of the wait hint but not less than 1 second and not more than 10 seconds
        wait_time = svc_status.dwWaitHint / 10;
        wait_time = (wait_time < 1000 ? 1000 : (wait_time > 10000 ? 10000 : wait_time));
        ::Sleep(wait_time);

        // Check the status until the service is no longer stop pending
        if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;

        // Continue to wait and check
        if (svc_status.dwCheckPoint > old_check_point)
        {
            start_tick_count = ::GetTickCount();
            old_check_point  = svc_status.dwCheckPoint;
        }
        // Timeout waiting for service to stop
        else if (::GetTickCount() - start_tick_count > svc_status.dwWaitHint)
        {
            return false;
        }
    }

    // Attempt to start the service
    if (!::StartServiceA(svc_handle, 0, NULL)) return false;

    // Check the status until the service is no longer start pending
    if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;

    // Save the tick count and initial checkpoint
    start_tick_count = ::GetTickCount();
    old_check_point  = svc_status.dwCheckPoint;

    while (svc_status.dwCurrentState == SERVICE_START_PENDING)
    {
        // Do not wait longer than the wait hint
        // A good interval is one-tenth the wait hint, but no less than 1 second and no more than 10 seconds
        wait_time = svc_status.dwWaitHint / 10;
        wait_time = (wait_time < 1000 ? 1000 : (wait_time > 10000 ? 10000 : wait_time));
        ::Sleep(wait_time);

        // Check the status again
        if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) break;

        // Continue to wait and check
        if (svc_status.dwCheckPoint > old_check_point)
        {
            start_tick_count = ::GetTickCount();
            old_check_point  = svc_status.dwCheckPoint;
        }
        // No progress made within the wait hint
        else if (::GetTickCount() - start_tick_count > svc_status.dwWaitHint)
        {
            break;
        }
    }

    // Return whether the service is running
    return (svc_status.dwCurrentState == SERVICE_RUNNING);
#endif
#ifdef _LINUX
    // Define temporary variables
    int svcctrl_type = __check_service_ctrltype(); // Service control command type

    // Check service control command type
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAVAILABLE, ENOENT, return false);
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAUTH, EPERM, return false);

    // Start service and return execute result
    return __exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? "systemctl start %s" : "service %s start");
#endif
}

/**
 * @brief [THREAD_SAFE|ERROR_CODE] Stop service
 *
 * @param svcname Service name
 * @return Whether the stop was successful
 */
bool ofw::service_stop(const char * svcname)
{
    // Check parameters for validity
    OFW_CHECK(svcname && *svcname != '\0', EINVAL, return false);

    // Check service status
    {
        // Define temporary variables
        int svc_status = ofw::get_service_status(svcname);

        // Check if the service is error
        OFW_CHECK(svc_status != OFW_SVC_S_ERROR, ENOENT, return false);

        // Check if the service is not installed
        if ((ofw::get_service_status(svcname) & OFW_SVC_S_NOTINSTALLED) == OFW_SVC_S_NOTINSTALLED) return false;

        // Check if the service is already running
        if ((ofw::get_service_status(svcname) & OFW_SVC_S_STOPED) == OFW_SVC_S_STOPED) return true;
    }

#ifdef _WINDOWS
    // Define inside variable
    SC_HANDLE__EX          svc_manager;                     // Service manager handle
    SC_HANDLE__EX          svc_handle;                      // Service handle
    SERVICE_STATUS_PROCESS svc_status;                      // Service status and process information
    DWORD                  bytes_needed;                    // Service status bytes needed
    DWORD                  wait_time;                       // Service control wait time
    DWORD                  start_time   = ::GetTickCount(); // Start time
    DWORD                  stop_timeout = 30000;            // Define stop timeout

    // Get a handle to the SCM database
    if (!(svc_manager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE))) return false;

    // Get a handle to the service
    if (!(svc_handle = ::OpenServiceA(svc_manager, ofw::ansi_to_utf8(svcname).get(), SERVICE_QUERY_STATUS | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS))) return false;

    // Make sure the service is not already stopped
    if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;
    if (svc_status.dwCurrentState == SERVICE_STOPPED) return true;

    // If a stop is pending, wait for it.
    while (svc_status.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint
        // A good interval is one-tenth of the wait hint but not less than 1 second and not more than 10 seconds.
        wait_time = svc_status.dwWaitHint / 10;
        wait_time = (wait_time < 1000 ? 1000 : (wait_time > 10000 ? 10000 : wait_time));
        ::Sleep(wait_time);

        // Check the status again
        if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;

        // Service stopped successfully
        if (svc_status.dwCurrentState == SERVICE_STOPPED) return true;

        // Service stop timed out
        if (::GetTickCount() - start_time > stop_timeout) return false;
    }

    // If the service is running, dependencies must be stopped first.
    __stop_depend_services(svc_manager, svc_handle);

    // Send a stop code to the service.
    if (!::ControlService(svc_handle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&svc_status)) return false;

    // Wait for the service to stop.
    while (svc_status.dwCurrentState != SERVICE_STOPPED)
    {
        ::Sleep(svc_status.dwWaitHint);

        // Check the status again
        if (!::QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&svc_status, sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) return false;

        // Wait timed out
        if (::GetTickCount() - start_time > stop_timeout) return false;
    }

    // Return execute result
    return (svc_status.dwCurrentState == SERVICE_STOPPED);
#endif
#ifdef _LINUX
    // Define temporary variables
    int svcctrl_type = __check_service_ctrltype(); // Service control command type

    // Check service control command type
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAVAILABLE, ENOENT, return false);
    OFW_CHECK(svcctrl_type != LINUX_SVCCTRL_TYPE_UNAUTH, EPERM, return false);

    // Start service and return execute result
    return __exec_svc_ctrlcmd(svcname, svcctrl_type == LINUX_SVCCTRL_TYPE_SYSTEMCTL ? "systemctl stop %s" : "service %s stop");
#endif
}
