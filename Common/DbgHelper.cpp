/**
 * @brief Debug Helper
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
#include "FuncHelper.h"
#include "SysHelper.h"
#include <mutex>
#include <stdarg.h>
#include "../Library/DebugBreak/debugbreak.h"
#if defined(_QT_FRAMEWORK_USED)
    #include <QApplication>
#endif
#include "DbgHelper.h"

//================================================================================
// Define inside macro
//================================================================================
/**
 * @brief Debug log step length
 */
#define DBGLOG_STRING_STEP_LENGTH 128

//================================================================================
// Define inside type
//================================================================================
#if (defined(_MSC) && defined(_CRTDBG_MAP_ALLOC)) || defined(_QT_FRAMEWORK_USED)
class DebugHelperPrivate final
{
private:
    #if defined(_QT_FRAMEWORK_USED)
    static void OutputRedirection(QtMsgType type, const QMessageLogContext & context, const QString & msg) noexcept
    {
        QByteArray local_message = msg.toLocal8Bit();
        switch (type)
        {
            case QtDebugMsg:
                DbgOutputLog(context.file, context.line, context.function, ESL_DEBUG,      "%s", 1, local_message.constData());
                break;
            case QtInfoMsg:
                DbgOutputLog(context.file, context.line, context.function, ESL_INFOMATION, "%s", 1, local_message.constData());
                break;
            case QtWarningMsg:
                DbgOutputLog(context.file, context.line, context.function, ESL_WARNING,    "%s", 1, local_message.constData());
                break;
            case QtCriticalMsg:
                DbgOutputLog(context.file, context.line, context.function, ESL_ERROR,      "%s", 1, local_message.constData());
                break;
            case QtFatalMsg:
                DbgOutputLog(context.file, context.line, context.function, ESL_FATAL,      "%s", 1, local_message.constData());
                break;
        }
    }
    #endif
public:
    DebugHelperPrivate() noexcept
    {
    #if defined(_MSC) && defined(_CRTDBG_MAP_ALLOC)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDERR);
        _CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    #endif
    #if defined(_QT_FRAMEWORK_USED)
        qInstallMessageHandler(DebugHelperPrivate::OutputRedirection);
    #endif
    }
    ~DebugHelperPrivate() {}
};
#endif

//================================================================================
// Initialize inside variable
//================================================================================
#if (defined(_MSC) && defined(_CRTDBG_MAP_ALLOC)) || defined(_QT_FRAMEWORK_USED)
static DebugHelperPrivate __DebugHelperPrivate;
#endif

/**
 * @brief Inner mutex
 */
static std::mutex __InnerMutex;

/**
 * @brief Debug log infomation handling function
 */
static dbg_log_handle_t __DbgLogHandle = nullptr;

//================================================================================
// Implementation inside method
//================================================================================
/**
 * @brief Format debug log
 *
 * @param outString     Output string
 * @param fmtString     Format string (Must end with '\\0'; Format: "%%"="%", "%X|%x"=Hex string, %B|%b"=Binary string, Other=Reference sprintf() specifier)
 * @param fmtArgs       Format arguments ("%X|%x|%B|%b" must be use a parameter in the format of std::make_unique<dbg_log_datas_t>("xxx", 3).get())
 */
static void __DbgFormatLog(std::string & outString, char ** fmtString, va_list fmtArgs)
{
    char * specifier_begin = *fmtString;
    char * specifier_end   = nullptr;

    if (!specifier_begin) return;

    while ((specifier_begin = strchr(specifier_begin, '%')))
    {
        if (specifier_begin[1] == '%')
        {
            specifier_begin = specifier_begin + 2;
            continue;
        }

        specifier_end = specifier_begin + 1;
        while (*specifier_end != '\0' && !strchr("diuoxXfFeEgGaAcspn%Bb", *specifier_end)) specifier_end++;

        if (*specifier_end == '%')
        {
            specifier_begin = specifier_end;
            continue;
        }
        if (*specifier_end == '\0') specifier_end = nullptr;

        break;
    }

    if (specifier_end)
    {
        if (*specifier_end == 'X' || *specifier_end == 'x' || *specifier_end == 'B' || *specifier_end == 'b')
        {
            if (specifier_begin != *fmtString)
            {
                char old_char = specifier_begin[0];

                specifier_begin[0] = '\0';
                {
                    size_t                  add_length  = snprintf(NULL, 0, *fmtString, 0);
                    std::unique_ptr<char[]> temp_strptr = std::make_unique<char[]>(add_length + 1);

                    snprintf(temp_strptr.get(), add_length + 1, *fmtString, 0);
                    if (outString.capacity() - outString.size() < add_length) outString.reserve(outString.capacity() + MAX(add_length, DBGLOG_STRING_STEP_LENGTH));
                    outString += temp_strptr.get();
                }
                specifier_begin[0] = old_char;
            }

            if (*specifier_end == 'X' || *specifier_end == 'x')
            {
                const char *      HEX_TABLE_UPPER = "0123456789ABCDEF";
                const char *      HEX_TABLE_LOWER = "0123456789abcdef";
                dbg_log_datas_t * hex_arg         = va_arg(fmtArgs, dbg_log_datas_t *);
                const char *      hex_bytes_pos   = hex_arg->datas;
                size_t            add_length      = (hex_arg->length == 0 ? 0 : (hex_arg->length * 3 - 1));

                if (outString.capacity() - outString.size() < add_length) outString.reserve(outString.capacity() + MAX(add_length, DBGLOG_STRING_STEP_LENGTH));

                for (uint loop_idx = 0; loop_idx < hex_arg->length; loop_idx++)
                {
                    if (loop_idx != 0)
                    {
                        hex_bytes_pos++;
                        outString += ' ';
                    }
                    outString += (*specifier_end == 'X' ? HEX_TABLE_UPPER[(*hex_bytes_pos & 0xf0) >> 4] : HEX_TABLE_LOWER[(*hex_bytes_pos & 0xf0) >> 4]);
                    outString += (*specifier_end == 'X' ? HEX_TABLE_UPPER[(*hex_bytes_pos & 0x0f) >> 0] : HEX_TABLE_LOWER[(*hex_bytes_pos & 0x0f) >> 0]);
                }
            }
            else if (*specifier_end == 'B' || *specifier_end == 'b')
            {
                dbg_log_datas_t * bit_arg         = va_arg(fmtArgs, dbg_log_datas_t *);
                const char *      bit_bytes_pos   = bit_arg->datas;
                uint              bit_bytes_count = (bit_arg->length / 8) + (bit_arg->length % 8 == 0 ? 0 : 1);
                size_t            add_length      = bit_arg->length + (bit_bytes_count - 1);

                if (outString.capacity() - outString.size() < add_length) outString.reserve(outString.capacity() + MAX(add_length, DBGLOG_STRING_STEP_LENGTH));

                for (uint loop_bytes_idx = 0; loop_bytes_idx < bit_bytes_count; loop_bytes_idx++)
                {
                    if (loop_bytes_idx != 0)
                    {
                        bit_bytes_pos++;
                        outString += ' ';
                    }

                    for (uint loop_bits_idx = 0; loop_bits_idx < 8 && loop_bytes_idx * 8 + loop_bits_idx < bit_arg->length; loop_bits_idx++)
                    {
                        outString += ((*bit_bytes_pos >> (7 - loop_bits_idx)) & 0x01 ? '1' : '0');
                    }
                }
            }
        }
        else
        {
            char old_char = specifier_end[1];

            specifier_end[1] = '\0';
            {
                size_t                  add_length  = vsnprintf(NULL, 0, *fmtString, fmtArgs);
                std::unique_ptr<char[]> temp_strptr = std::make_unique<char[]>(add_length + 1);

                vsnprintf(temp_strptr.get(), add_length + 1, *fmtString, fmtArgs);
                if (outString.capacity() - outString.size() < add_length) outString.reserve(outString.capacity() + MAX(add_length, DBGLOG_STRING_STEP_LENGTH));
                outString += temp_strptr.get();
            }
            specifier_end[1] = old_char;
        }

        *fmtString = specifier_end + 1;
    }
    else
    {
        size_t                  add_length  = vsnprintf(NULL, 0, *fmtString, fmtArgs);
        std::unique_ptr<char[]> temp_strptr = std::make_unique<char[]>(add_length + 1);

        vsnprintf(temp_strptr.get(), add_length + 1, *fmtString, fmtArgs);
        if (outString.capacity() - outString.size() < add_length) outString.reserve(outString.capacity() + MAX(add_length, DBGLOG_STRING_STEP_LENGTH));
        outString += temp_strptr.get();

        *fmtString = nullptr;
    }
}

//================================================================================
// Implementation export method
//================================================================================
/**
 * @brief Set debug error infomation handling function (Thread safe)
 *
 * @param errHandle     Debug error infomation handling function
 */
void DbgSetHandle(const dbg_log_handle_t errHandle) noexcept
{
    std::lock_guard<std::mutex> inner_locker(__InnerMutex);

    __DbgLogHandle = errHandle;
}

/**
 * @brief Output debug log (Thread safe; Direct use is not recommended)
 *
 * @param filePath      File path (Debug mode: DBG_OUTPUTLOG_FILE; Release mode: nullptr)
 * @param fileLine      File line (Debug mode: DBG_OUTPUTLOG_LINE; Release mode: 0)
 * @param fileFunc      File function (Debug mode: DBG_OUTPUTLOG_FUNC; Release mode: nullptr)
 * @param logType       Log type (0x0100: ASSERT; 0x0200: VERIFY; 0x0400: PERROR; Other: use execute status level)
 * @param fmtString     Format string (Must end with '\\0'; Format: "%%"="%", "%X|%x"=Hex string, %B|%b"=Binary string, Other=Reference sprintf() specifier)
 * @param fmtArgsCount  Format arguments count
 * @param ...           Format arguments ("%X|%x|%B|%b" must be use a parameter in the format of std::make_unique<dbg_log_datas_t>("xxx", 3).get())
 */
void DbgOutputLog(const char * filePath, const int fileLine, const char * fileFunc, const int logType, const char * fmtString, const int fmtArgsCount, ...) noexcept
{
    int error_code = errno;
#if defined(_MSC)
    DWORD last_error = ::GetLastError();
#endif
    const char * log_label   = nullptr;
    tm           log_time    = {0, 0, 0, 1, 0, 0, 0, 0, 0};
    std::string  log_content = "";

    if ((logType & 0x0100))
        log_label = "[ASSERT]";
    else if ((logType & 0x0200))
        log_label = "[VERIFY]";
    else if ((logType & ESL_DEBUG))
        log_label = "[DEBUG]";
    else if ((logType & ESL_INFOMATION))
        log_label = "[INFO]";
    else if ((logType & ESL_WARNING))
        log_label = "[WARNING]";
    else if ((logType & ESL_ERROR))
        log_label = "[ERROR]";
    else if ((logType & ESL_FATAL))
        log_label = "[FATAL]";

    {
        char time_str[] = "1900-01-01 00:00:00.000";

        {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> time_now    = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            std::time_t                                                                   time_total  = time_now.time_since_epoch().count();
            std::time_t                                                                   time_stamp  = time_total / 1000LL;
            int                                                                           time_millis = (int)(time_total % 1000LL);

#if defined(_MSC)
            if (localtime_s(&log_time, &time_stamp) == ESV_SUCCESS)
#elif defined(_GCC)
            if (localtime_r(&time_stamp, &log_time))
#endif
            {
                snprintf(time_str, sizeof(log_time), "%04d-%02d-%02d %02d:%02d:%02d.%03d", log_time.tm_year + 1900, log_time.tm_mon + 1, log_time.tm_mday, log_time.tm_hour, log_time.tm_min, log_time.tm_sec, time_millis);
            }
        }

        {
            const char *            fmt_header  = "%-10sTime: %s, ProcessID: %u, ThreadID: %lu, File: %s:%d, Function: %s\r\n%10s";
            pid_t                   process_id  = SELF_PROCESS_ID;
            pthread_t               thread_id   = SELF_NATIVE_THREAD_ID;
            const char *            file_path   = (filePath ? filePath : "-");
            int                     file_line   = (filePath ? fileLine : 0);
            const char *            file_func   = (fileFunc ? fileFunc : "-");
            size_t                  str_length  = snprintf(NULL, 0, fmt_header, log_label, time_str, process_id, thread_id, file_path, file_line, file_func, "");
            std::unique_ptr<char[]> temp_strptr = std::make_unique<char[]>(PATH_MAX + 1);

            snprintf(temp_strptr.get(), str_length + 1, fmt_header, log_label, time_str, process_id, thread_id, file_path, file_line, file_func, "");
            if (log_content.capacity() < str_length) log_content.reserve(log_content.capacity() + MAX(str_length, DBGLOG_STRING_STEP_LENGTH));
            log_content += temp_strptr.get();
        }
    }

    if (fmtArgsCount > 0)
    {
        size_t                  fmt_strlen = strlen(fmtString);
        std::unique_ptr<char[]> fmt_strptr = std::make_unique<char[]>(fmt_strlen + 1);
        char *                  fmt_strval = fmt_strptr.get();
        va_list                 arg_list;

        memcpy(fmt_strval, fmtString, fmt_strlen + 1);

        va_start(arg_list, fmtArgsCount);
        while (arg_list) __DbgFormatLog(log_content, &fmt_strval, arg_list);
        va_end(arg_list);
    }
    else
    {
        log_content += fmtString;
    }

    if ((logType & 0x0400))
    {
        std::unique_ptr<char[]> errno_str = std::make_unique<char[]>(0xFF);
        errno_str[0]                      = '\0';

#if defined(_MSC)
        if (error_code)
            strerror_s(errno_str.get(), 0xFF, error_code);
        else if (last_error)
            ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error_code, 0, (LPTSTR)errno_str.get(), 0xFF, NULL);
        if (errno_str[0] == '\0') sprintf(errno_str.get(), error_code ? "Unknow error code (%d)." : "Unknow error code (%lu).", error_code ? error_code : last_error);
#elif defined(_GCC)
        if (error_code) strerror_r(error_code, errno_str.get(), 0xFF);
        if (errno_str[0] == '\0') sprintf(errno_str.get(), "Unknow error code (%d).", error_code);
#endif

        log_content += " ";
        log_content += errno_str.get();
    }

    {
        std::unique_lock<std::mutex> inner_locker(__InnerMutex);

        if (__DbgLogHandle)
        {
            inner_locker.unlock();

            char date_str[] = "00000000";
            snprintf(date_str, sizeof(log_time), "%04d%02d%02d", log_time.tm_year + 1900, log_time.tm_mon + 1, log_time.tm_mday);
            log_content += "\r\n";
            __DbgLogHandle(date_str, log_content.c_str(), log_content.size());
        }
        else
        {
#if defined(_WINDOWS)
            HANDLE                     output_handle = GetStdHandle(STD_ERROR_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            if (!output_handle || output_handle == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(output_handle, &buffer_info))
            {
                output_handle = nullptr;
            }
#endif

            switch (logType & 0xff)
            {
                case ESL_FATAL:
                case ESL_ERROR:
#if defined(_WINDOWS)
                    if (output_handle) SetConsoleTextAttribute(output_handle, (FOREGROUND_GREEN | FOREGROUND_RED) | (BACKGROUND_RED) | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
                    fprintf(stderr, "%s", log_label);
#elif defined(_LINUX)
                    fprintf(stderr, "\033[41;33m%s\033[0m", log_label);
#endif
                    break;
                case ESL_WARNING:
#if defined(_WINDOWS)
                    if (output_handle) SetConsoleTextAttribute(output_handle, (FOREGROUND_BLUE) | (BACKGROUND_GREEN | BACKGROUND_RED) | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
                    fprintf(stderr, "%s", log_label);
#elif defined(_LINUX)
                    fprintf(stderr, "\033[43;34m%s\033[0m", log_label);
#endif
                    break;
                default:
#if defined(_WINDOWS)
                    if (output_handle) SetConsoleTextAttribute(output_handle, (FOREGROUND_BLUE & FOREGROUND_GREEN & FOREGROUND_RED) | (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED) | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
                    fprintf(stderr, "%s", log_label);
#elif defined(_LINUX)
                    fprintf(stderr, "\033[47;30m%s\033[0m", log_label);
#endif
                    break;
            }
            fflush(stderr);

#if defined(_WINDOWS)
            if (output_handle) SetConsoleTextAttribute(output_handle, buffer_info.wAttributes);
#endif

            fprintf(stderr, "%s\r\n\r\n", log_content.c_str() + strlen(log_label));
            fflush(stderr);
        }
    }

#ifdef _DEBUG
    if ((logType & ESL_WARNING) || (logType & ESL_ERROR) || (logType & ESL_FATAL)) debug_break();
#endif

    if (logType & ESL_FATAL)
    {
#if defined(_MSC)
        ExitProcess(STATUS_FATAL_APP_EXIT);
#elif defined(_GCC)
        abort();
#endif
    }
}
