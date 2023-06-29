/**
 * @brief Encoding Helper
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
#include <locale.h>
#include "EncodingHelper.h"

//================================================================================
// Define inside type
//================================================================================
/**
 * @brief Set application default encoding
 */
class EncodingHelperPrivate final
{
private:
    /**
     * @brief System previous encoding
     */
    char *previousEncoding;

public:
    /**
     * @brief Construct function
     */
    EncodingHelperPrivate() noexcept
    {
        previousEncoding = setlocale(LC_CTYPE, "zh_CN.UTF-8");
    }

    /**
     * @brief Destruct function
     */
    ~EncodingHelperPrivate()
    {
        if (previousEncoding) setlocale(LC_CTYPE, previousEncoding);
    }
};

//================================================================================
// Initialize global variable
//================================================================================
/**
 * @brief Auto execute private class
 */
static EncodingHelperPrivate __EncodingHelperPrivate;