/**
 * @brief Service Base Interface
 *
 * @author WindEagle <fy516a@gmail.com>
 * @version 1.0.0
 * @date 2020-01-01 00:00
 * @copyright Copyright (c) 2020-2023 ZyTech Team
 * @par Changelog:
 * Date                 Version     Author          Description
 */
#ifndef OFW_ISERVICE_H
#define OFW_ISERVICE_H

//################################################################################
// Include header file
//################################################################################
#include "../base/basedefine.h"
#include "../base/typedefine.h"
#include "../base/funcdefine.h"

//################################################################################
// Define export type
//################################################################################
namespace ofw
{
    class IServiceDaemon;

    /**
     * @brief Service base interface
     */
    class IServiceBase
    {
    protected:
        /**
         * @brief Construct function
         *
         * @param svcName  Service name
         * @param subTotal Number of child processes
         */
        IServiceBase(const char * svcName, uint subTotal);

        /**
         * @brief Get the current process index
         *
         * @return The current process index (Main process: 0; Child process start with: 1)
         */
        bool processIndex();

        /**
         * @brief Check whether the service is terminated
         *
         * @return Whether the service is terminated
         */
        bool isTerminated();

        /**
         * @brief Service on start event (Used only for the main process)
         *
         * @return Whether the execute was successful
         */
        virtual bool onStart();

        /**
         * @brief The principal execution portion of the service (Used only for child processes)
         * @details The service will automatically stop when this process is complete
         *
         * @param procIndex Child process index (Start with: 1)
         * @return Whether the execute was successful
         */
        virtual bool onExecute(int procIndex);

        /**
         * @brief Service on stop event (Used only for the main process)
         *
         * @return Whether the stop was successful
         */
        virtual bool onStop();

    public:
        /**
         * @brief Destruct function
         */
        virtual ~IServiceBase();

        /**
         * @brief Start the service execution
         *
         * @return Exit code
         */
        int exec();

    private:
        friend class IServiceDaemon;
    };
}

#endif
