cmake_minimum_required(VERSION 3.4.0)

####################################################################################################
# Set Project Infomations
####################################################################################################
# Set Project Name
set(PROJ_NAME             "ZYFramework")
# Set Project Version (Format: MAJOR.MINOR.PATCH)
set(PROJ_VERSION          1.0.0)
# Set Project Company Name
set(PROJ_COMPANY_NAME     "ZyTech Team")
# Set Project File Description
set(PROJ_FILE_DESCRIPTION "ZY Solution Template")
# Set Project Legal Copyright
set(PROJ_LEGAL_COPYRIGHT  "Copyright (C) 2020-{YEAR} ${PROJ_COMPANY_NAME}")
# Set Project Type ("Console": Generate console application; "Desktop": Generate desktop application; "Static": Generate static library; "Shared": Generate dynamic or shared library)
set(PROJ_TYPE             Static)
# Set Project C/C++ Standard (C supper: 90, 99, 11; C++ supper: 11, 14, 17)
set(PROJ_C_STANDARD       11)
set(PROJ_CXX_STANDARD     14)
# Set Projects Output Directory
set(PROJ_OUTDIR           ${CMAKE_CURRENT_SOURCE_DIR}/.bin)

####################################################################################################
# Set Project Libraries
# Replace Mark Symbols List:
#   {SYSTEM|Windows|Linux}: Project target system type (Values: Windows, Linux)
#   {PLATFORM|x86|x64}:     Project target platform type (Values: 86, 64)
#   {MODE|Debug|Release}:   Project target build type (Values: Debug, Release)
####################################################################################################
set(PROJ_LIBRARY_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set(PROJ_LIBRARY_NAMES
    Qt5::Core Qt5::LinguistTools Qt5::Gui Qt5::Qml Qt5::Widgets Qt5::Sql Qt5::SerialPort Qt5::PrintSupport
)

####################################################################################################
# Set Include Directories And Project Files
# Replace Mark Symbols List:
#   {SYSTEM|Windows|Linux}: Project target system type (Values: Windows, Linux)
#   {PLATFORM|x86|x64}:     Project target platform type (Values: x86, x64)
#   {MODE|Debug|Release}:   Project target build type (Values: Debug, Release)
#   {VARIABLE|VarName}:     Variable value (Examples: CMAKE_CURRENT_SOURCE_DIR)
####################################################################################################
set(PROJ_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
set(PROJ_SOURCE_LIST
    ./*.c
    ./*.cpp
)

####################################################################################################
# Overwrite Message Function
# Constants List:
#   CUSTOM_MESSAGE: Custom message
####################################################################################################
if (NOT DEFINED CUSTOM_MESSAGE)
    set(CUSTOM_MESSAGE "CUSTOM_MESSAGE")
    function(message)
        if(ARGC LESS 2)
            return()
        endif()
        list(GET ARGV 0 TMP_MESSAGE_TYPE)
        if(TMP_MESSAGE_TYPE STREQUAL FATAL_ERROR OR TMP_MESSAGE_TYPE STREQUAL SEND_ERROR OR TMP_MESSAGE_TYPE STREQUAL AUTHOR_WARNING OR TMP_MESSAGE_TYPE STREQUAL WARNING OR TMP_MESSAGE_TYPE STREQUAL CUSTOM_MESSAGE)
            list(REMOVE_AT ARGV 0)
            if (TMP_MESSAGE_TYPE STREQUAL CUSTOM_MESSAGE)
                _message(NOTICE "${ARGV}")
            else()
                _message(${TMP_MESSAGE_TYPE} "${ARGV}")
            endif()
        endif()
    endfunction()
endif()

####################################################################################################
# Replace Project Directories And Files Marks Function
# Parameter List:
#   RESULT_NAME:  Return value variable
#   TARGET_MARKS: Will to replace marks (Format: "Mark1[|Mark2[|Mark3[...]]]")
#   USE_GLOB:     Will to replace use globbing-expressions traverse files or folders
#   SRC_LIST:     Will to replace source list
####################################################################################################
function(replace_marks RESULT_NAME TARGET_MARKS USE_GLOB SRC_LIST)
    set(TEMP_RESULT_LIST)
    foreach(TEMP_SRC_ITEM IN LISTS SRC_LIST)
        # Replace Marks
        string(REGEX MATCHALL "\\{([^\\|]+)(\\|[^\\|]*)+\\}" TEMP_ITEM_MARKS "${TEMP_SRC_ITEM}")
        foreach(TEMP_MARK_SRC IN LISTS TEMP_ITEM_MARKS)
            string(REPLACE "{" ""  TEMP_MARK_ITEM "${TEMP_MARK_SRC}")
            string(REPLACE "}" ""  TEMP_MARK_ITEM "${TEMP_MARK_ITEM}")
            string(REPLACE "|" ";" TEMP_MARK_ITEM "${TEMP_MARK_ITEM}")
            list(GET TEMP_MARK_ITEM 0 TEMP_ITEM_SIGN)
            if    (TEMP_ITEM_SIGN STREQUAL "SYSTEM")
                if(TARGET_MARKS MATCHES "SYSTEM")
                    list(GET TEMP_MARK_ITEM 1 TEMP_ITEM_WINDOWS)
                    list(GET TEMP_MARK_ITEM 2 TEMP_ITEM_LINUX)
                    if    (WIN32 AND NOT CYGWIN)
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_WINDOWS}" TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    elseif(UNIX OR CYGWIN)
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_LINUX}"   TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    endif()
                    unset(TEMP_ITEM_LINUX)
                    unset(TEMP_ITEM_WINDOWS)
                endif()
            elseif(TEMP_ITEM_SIGN STREQUAL "PLATFORM")
                if(TARGET_MARKS MATCHES "PLATFORM")
                    list(GET TEMP_MARK_ITEM 1 TEMP_ITEM_X86)
                    list(GET TEMP_MARK_ITEM 2 TEMP_ITEM_X64)
                    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_X86}" TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_X64}" TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    endif()
                    unset(TEMP_ITEM_X64)
                    unset(TEMP_ITEM_X86)
                endif()
            elseif(TEMP_ITEM_SIGN STREQUAL "MODE")
                if(TARGET_MARKS MATCHES "MODE")
                    list(GET TEMP_MARK_ITEM 1 TEMP_ITEM_DEBUG)
                    list(GET TEMP_MARK_ITEM 2 TEMP_ITEM_RELEASE)
                    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_DEBUG}"   TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    else()
                        string(REPLACE "${TEMP_MARK_SRC}" "${TEMP_ITEM_RELEASE}" TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    endif()
                    unset(TEMP_ITEM_RELEASE)
                    unset(TEMP_ITEM_DEBUG)
                endif()
            elseif(TEMP_ITEM_SIGN STREQUAL "VARIABLE")
                if(TARGET_MARKS MATCHES "VARIABLE")
                    list(GET TEMP_MARK_ITEM 1 TEMP_ITEM_VARNAME)
                    string(REPLACE "${TEMP_MARK_SRC}" "${${TEMP_ITEM_VARNAME}}" TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
                    unset(TEMP_ITEM_VARNAME)
                endif()
            else()
                message(FATAL_ERROR "FATAL: Unknown replace mark \"${TEMP_ITEM_SIGN}\".")
            endif()
            unset(TEMP_MARK_ITEM)
        endforeach(TEMP_MARK_SRC)
        if(NOT TEMP_SRC_ITEM STREQUAL "")
            if(USE_GLOB)
                file(GLOB_RECURSE TEMP_SRC_ITEM "${TEMP_SRC_ITEM}")
            endif()
            list(APPEND TEMP_RESULT_LIST "${TEMP_SRC_ITEM}")
        endif()
    endforeach(TEMP_SRC_ITEM)
    set(${RESULT_NAME} "${TEMP_RESULT_LIST}" PARENT_SCOPE)
    unset(TEMP_RESULT_LIST)
endfunction()

####################################################################################################
# Set Default Build Type
####################################################################################################
if (NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

####################################################################################################
# Set Project Info
####################################################################################################
# Set Project Name
project("${PROJ_NAME}")
# Set Project Type
if("${PROJ_TYPE}" STREQUAL "Console")
    set(PROJECT_TYPE "PROJTYPE_CONSOLE")
elseif("${PROJ_TYPE}" STREQUAL "Desktop")
    set(PROJECT_TYPE "PROJTYPE_DESKTOP")
elseif("${PROJ_TYPE}" STREQUAL "Static")
    set(PROJECT_TYPE "PROJTYPE_STATICLIB")
elseif("${PROJ_TYPE}" STREQUAL "Shared")
    set(PROJECT_TYPE "PROJTYPE_SHAREDLIB")
endif()
# Set Project Version
if (DEFINED SOLUTION_VERSION)
    set(PROJ_VERSION "${SOLUTION_VERSION}")
endif()
set(PROJECT_VERSION "${PROJ_VERSION}")
# Set Project Version SubInformation
string(REPLACE "." ";" TEMP_PROJ_VERSION_PACK_LIST "${PROJ_VERSION}")
list(GET TEMP_PROJ_VERSION_PACK_LIST 0 PROJECT_VERSION_MAJOR)
list(GET TEMP_PROJ_VERSION_PACK_LIST 1 PROJECT_VERSION_MINOR)
list(GET TEMP_PROJ_VERSION_PACK_LIST 2 PROJECT_VERSION_PATCH)
unset(TEMP_PROJ_VERSION_PACK_LIST)
# Set Project Company Name
if (DEFINED SOLUTION_COMPANY_NAME)
    set(PROJ_COMPANY_NAME "${SOLUTION_COMPANY_NAME}")
endif()
set(PROJECT_COMPANY_NAME     "${PROJ_COMPANY_NAME}")
# Set Project File Description
if (DEFINED SOLUTION_FILE_DESCRIPTION)
    set(PROJ_FILE_DESCRIPTION "${SOLUTION_FILE_DESCRIPTION}")
endif()
set(PROJECT_FILE_DESCRIPTION "${PROJ_FILE_DESCRIPTION}")
# Set Project Legal Copyright
if (DEFINED SOLUTION_LEGAL_COPYRIGHT)
    set(PROJ_LEGAL_COPYRIGHT "${SOLUTION_LEGAL_COPYRIGHT}")
endif()
string(TIMESTAMP TEMP_CURRENT_YEAR "%Y")
string(REPLACE "{YEAR}" "${TEMP_CURRENT_YEAR}" PROJ_LEGAL_COPYRIGHT "${PROJ_LEGAL_COPYRIGHT}")
unset(TEMP_CURRENT_YEAR)
set(PROJECT_LEGAL_COPYRIGHT  ${PROJ_LEGAL_COPYRIGHT})
# Set Projects Output Directory
if (DEFINED SOLUTION_OUTDIR)
    set(PROJ_OUTDIR "${SOLUTION_OUTDIR}")
endif()

####################################################################################################
# Get Current Operating System
####################################################################################################
# Get Host System
set(PROJ_HOST_SYSTEM   "${CMAKE_HOST_SYSTEM}")
# Get Host System Processor
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "64")
    set(PROJ_HOST_PLATFORM "64")
else()
    set(PROJ_HOST_PLATFORM "86")
endif()
# Get Host Logical Cores
cmake_host_system_information(RESULT PROJ_HOST_CORES_NUMBER     QUERY NUMBER_OF_LOGICAL_CORES)
# Get Host Available Physical Memory
cmake_host_system_information(RESULT PROJ_HOST_MEMORY_AVAILABLE QUERY AVAILABLE_PHYSICAL_MEMORY)
# Get Host Total Physical Memory
cmake_host_system_information(RESULT PROJ_HOST_MEMORY_TOTAL     QUERY TOTAL_PHYSICAL_MEMORY)

####################################################################################################
# Get Target Operating System
####################################################################################################
# Get Target System
set(PROJ_TARGET_SYSTEM "${CMAKE_SYSTEM}")
if    (PROJ_TARGET_SYSTEM MATCHES "Linux")
    set(PROJ_TARGET_SYSTEM "Linux")
elseif(PROJ_TARGET_SYSTEM MATCHES "Windows")
    set(PROJ_TARGET_SYSTEM "Windows")
else()
    message(FATAL_ERROR "FATAL: The application does not support the target operating system.")
endif()
# Get Target Platform
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PROJ_TARGET_PLATFORM "86")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROJ_TARGET_PLATFORM "64")
else()
    message(FATAL_ERROR "FATAL: The application does not support the target platform.")
endif()
# Get Target Build Mode
string(TOUPPER "${CMAKE_BUILD_TYPE}" TEMP_BUILD_TYPE)
if    ("${TEMP_BUILD_TYPE}" STREQUAL "MINSIZEREL")
    set(PROJ_TARGET_MODE "MinSizeRel")
elseif("${TEMP_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
    set(PROJ_TARGET_MODE "RelWithDebInfo")
elseif("${TEMP_BUILD_TYPE}" STREQUAL "RELEASE")
    set(PROJ_TARGET_MODE "Release")
elseif("${TEMP_BUILD_TYPE}" STREQUAL "DEBUG")
    set(PROJ_TARGET_MODE "Debug")
else()
    message(FATAL_ERROR "FATAL: The application does not support the build type.")
endif()
unset(TEMP_BUILD_TYPE)

####################################################################################################
# Set C Compile Environment
####################################################################################################
include(CheckCCompilerFlag)
if    (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    CHECK_C_COMPILER_FLAG("/std:c11"   COMPILER_SUPPORTS_C_VER_11 QUIET)
    CHECK_C_COMPILER_FLAG("/std:c99"   COMPILER_SUPPORTS_C_VER_99 QUIET)
    CHECK_C_COMPILER_FLAG("/std:c90"   COMPILER_SUPPORTS_C_VER_90 QUIET)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    CHECK_C_COMPILER_FLAG("-std=gnu11" COMPILER_SUPPORTS_C_VER_11 QUIET)
    CHECK_C_COMPILER_FLAG("-std=gnu99" COMPILER_SUPPORTS_C_VER_99 QUIET)
    CHECK_C_COMPILER_FLAG("-std=gnu90" COMPILER_SUPPORTS_C_VER_90 QUIET)
else()
    message(FATAL_ERROR "FATAL: Compiler other than MSVC and GNU is not supported.")
endif()
if    (COMPILER_SUPPORTS_C_VER_11 AND PROJ_C_STANDARD EQUAL 11)
    set(PROJ_COMPILE_C_STD 11)
elseif(COMPILER_SUPPORTS_C_VER_99 AND PROJ_C_STANDARD EQUAL 99)
    set(PROJ_COMPILE_C_STD 99)
elseif(COMPILER_SUPPORTS_C_VER_90 AND PROJ_C_STANDARD EQUAL 90)
    set(PROJ_COMPILE_C_STD 90)
else()
    message(FATAL_ERROR "FATAL: Compiler does not support the C ${PROJ_C_STANDARD} standard.")
endif()
set(CMAKE_C_STANDARD ${PROJ_COMPILE_C_STD})
set(CMAKE_C_STANDARD_REQUIRED ON)
if    (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    # Set Compiler Flags
    if    (PROJ_COMPILE_C_STD EQUAL 11)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c11")                                      # Set C standard to 11
    elseif(PROJ_COMPILE_C_STD EQUAL 99)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c99")                                      # Set C standard to 99
    elseif(PROJ_COMPILE_C_STD EQUAL 90)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c90")                                      # Set C standard to 90
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:utf-8 /execution-charset:utf-8")    # Set utf-8 code
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    # Set Compiler Flags
    if    (PROJ_COMPILE_C_STD EQUAL 11)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")                                    # Set C standard to 11
    elseif(PROJ_COMPILE_C_STD EQUAL 99)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")                                    # Set C standard to 99
    elseif(PROJ_COMPILE_C_STD EQUAL 90)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")                                    # Set C standard to 90
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finput-charset=utf-8 -fexec-charset=utf-8")        # Set utf-8 code
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "C compiler flags" FORCE)

####################################################################################################
# Set C++ Compile Environment
####################################################################################################
include(CheckCXXCompilerFlag)
if    (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    CHECK_CXX_COMPILER_FLAG("/std:c++17"   COMPILER_SUPPORTS_CXX_VER_17 QUIET)
    CHECK_CXX_COMPILER_FLAG("/std:c++14"   COMPILER_SUPPORTS_CXX_VER_14 QUIET)
    CHECK_CXX_COMPILER_FLAG("/std:c++11"   COMPILER_SUPPORTS_CXX_VER_11 QUIET)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    CHECK_CXX_COMPILER_FLAG("-std=gnu++17" COMPILER_SUPPORTS_CXX_VER_17 QUIET)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++14" COMPILER_SUPPORTS_CXX_VER_14 QUIET)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_CXX_VER_11 QUIET)
else()
    message(FATAL_ERROR "FATAL: Compiler other than MSVC and GNU is not supported.")
endif()
if    (COMPILER_SUPPORTS_CXX_VER_17 AND PROJ_CXX_STANDARD EQUAL 17)
    set(PROJ_COMPILE_CXX_STD 17)
elseif(COMPILER_SUPPORTS_CXX_VER_14 AND PROJ_CXX_STANDARD EQUAL 14)
    set(PROJ_COMPILE_CXX_STD 14)
elseif(COMPILER_SUPPORTS_CXX_VER_11 AND PROJ_CXX_STANDARD EQUAL 11)
    set(PROJ_COMPILE_CXX_STD 11)
else()
    message(FATAL_ERROR "FATAL: Compiler does not support the C++ ${PROJ_CXX_STANDARD} standard.")
endif()
set(CMAKE_CXX_STANDARD ${PROJ_COMPILE_CXX_STD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if    (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Set Compile Definitions
    add_definitions(-DWIN32_LEAN_AND_MEAN=1)                                                                    # Global macro WIN32_LEAN_AND_MEAN (Used to disable MFC)
    if("${PROJ_TARGET_MODE}" STREQUAL "Debug")
        add_definitions(-D_CRTDBG_MAP_ALLOC=1)                                                                  # Global macro _CRTDBG_MAP_ALLOC (Used to MSVC CrtDbg)
    endif()
    # Set Compiler Flags
    if    (PROJ_COMPILE_CXX_STD EQUAL 17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")                                                    # Set C++ standard to 17
    elseif(PROJ_COMPILE_CXX_STD EQUAL 14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")                                                    # Set C++ standard to 14
    elseif(PROJ_COMPILE_CXX_STD EQUAL 11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")                                                    # Set C++ standard to 11
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8 /execution-charset:utf-8")                    # Set utf-8 code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline")                                                        # Set whether to remove unused code and data: /Zc:inline(Yes) | Empty(No)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")                                                               # Set warning level: /W0(Disable all warning) | /W1 | /W2 | /W3 | /W4 | /Wall(Enable all warning)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX-")                                                              # Set whether to warnings are treated as errors : /WX(Yes)) | /WX-(No)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gd")                                                               # Set default call convention: /Gd(__cdecl) | /Gr(__fastcall) | /Gz(__stdcall) | /Gv(__vectorcall)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")                                                           # Set disabled warnings: /wd4996(Used outdated api)
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MDd")                                              # Set enabled with multi-threading support
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")                                               # Set enabled with multi-threading support
    # Set Linker Flags
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    /SAFESEH:NO")                                   # Set whether to enabled SafeSEH protection mechanism: /SAFESEH(Yes) | /SAFESEH:NO(No)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")                                   # Set whether to enabled SafeSEH protection mechanism: /SAFESEH(Yes) | /SAFESEH:NO(No)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")                                   # Set whether to enabled SafeSEH protection mechanism: /SAFESEH(Yes) | /SAFESEH:NO(No)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Set Compiler Flags
    if    (PROJ_COMPILE_CXX_STD EQUAL 17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17")                                                  # Set C++ standard to 17
    elseif(PROJ_COMPILE_CXX_STD EQUAL 14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")                                                  # Set C++ standard to 14
    elseif(PROJ_COMPILE_CXX_STD EQUAL 11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")                                                  # Set C++ standard to 11
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=utf-8 -fexec-charset=utf-8")                        # Set utf-8 code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")                                                    # Set whether to stop compiling when you encounter the first error: -Wfatal-errors(Yes) | Empty(No)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")                                                             # Set warning level: -w(Disable all warning) | -Wall(Enable all warning)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")                                                           # Set whether to warnings are treated as errors : -Werror(Yes)) | Empty(No)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-variable")                                        # Set disabled with used unused variable errors
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations -Wno-deprecated-declarations")   # Set disabled with used outdated api warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")                                                          # Set enabled with multi-threading support
    # Set Project Libraries
    list(APPEND PROJ_LIBRARY_NAMES "pthread")                                                                   # Add pthread library
endif()
set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS}"           CACHE STRING "C++ compiler flags"         FORCE)
set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}"     CACHE STRING "C++ compiler debug flags"   FORCE)
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE}"   CACHE STRING "C++ compiler release flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}"    CACHE STRING "CMake exe linker flags"     FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}" CACHE STRING "CMake module linker flags"  FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "CMake shared linker flags"  FORCE)

####################################################################################################
# Show Project Infomation
####################################################################################################
message(CUSTOM_MESSAGE "")
if (DEFINED SOLUTION_PROJECT)
    message(CUSTOM_MESSAGE "************************************************************")
    message(CUSTOM_MESSAGE "** Project Name: ${PROJ_NAME}")
    message(CUSTOM_MESSAGE "** Project Type: ${PROJ_TYPE}")
    message(CUSTOM_MESSAGE "** Project C Standard: ${PROJ_C_STANDARD}")
    message(CUSTOM_MESSAGE "** Project C++ Standard: ${PROJ_CXX_STANDARD}")
    message(CUSTOM_MESSAGE "** -------------------------------")
    message(CUSTOM_MESSAGE "** Compile C Standard: ${PROJ_COMPILE_C_STD}")
    message(CUSTOM_MESSAGE "** Compile C++ Standard: ${PROJ_COMPILE_CXX_STD}")
    message(CUSTOM_MESSAGE "************************************************************")
else()
    message(CUSTOM_MESSAGE "************************************************************")
    message(CUSTOM_MESSAGE "** Project Name: ${PROJ_NAME}")
    message(CUSTOM_MESSAGE "** Project Version: ${PROJ_VERSION}")
    message(CUSTOM_MESSAGE "** Project Type: ${PROJ_TYPE}")
    message(CUSTOM_MESSAGE "** Project C Standard: ${PROJ_C_STANDARD}")
    message(CUSTOM_MESSAGE "** Project C++ Standard: ${PROJ_CXX_STANDARD}")
    message(CUSTOM_MESSAGE "** Project Company Name: ${PROJ_COMPANY_NAME}")
    message(CUSTOM_MESSAGE "** Project File Description: ${PROJ_FILE_DESCRIPTION}")
    message(CUSTOM_MESSAGE "** Project Legal Copyright: ${PROJ_LEGAL_COPYRIGHT}")
    message(CUSTOM_MESSAGE "** -------------------------------")
    message(CUSTOM_MESSAGE "** Host System: ${PROJ_HOST_SYSTEM}")
    message(CUSTOM_MESSAGE "** Host Platform: x${PROJ_HOST_PLATFORM}")
    message(CUSTOM_MESSAGE "** Host Cores: ${PROJ_HOST_CORES_NUMBER}")
    message(CUSTOM_MESSAGE "** Host Memory (MB): ${PROJ_HOST_MEMORY_AVAILABLE}/${PROJ_HOST_MEMORY_TOTAL}")
    if (DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    message(CUSTOM_MESSAGE "** Host SDK Version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
    endif()
    message(CUSTOM_MESSAGE "** -------------------------------")
    message(CUSTOM_MESSAGE "** Target System: ${PROJ_TARGET_SYSTEM}")
    message(CUSTOM_MESSAGE "** Target Platform: x${PROJ_TARGET_PLATFORM}")
    message(CUSTOM_MESSAGE "** Target Mode: ${PROJ_TARGET_MODE}")
    message(CUSTOM_MESSAGE "** -------------------------------")
    message(CUSTOM_MESSAGE "** CMake Version: ${CMAKE_VERSION}")
    message(CUSTOM_MESSAGE "** Compile C Standard: ${PROJ_COMPILE_C_STD}")
    message(CUSTOM_MESSAGE "** Compile C++ Standard: ${PROJ_COMPILE_CXX_STD}")
    message(CUSTOM_MESSAGE "** Compile Output Directory: ${PROJ_OUTDIR}")
    message(CUSTOM_MESSAGE "************************************************************")
endif()

####################################################################################################
# Replace Marks Of The Libraries
####################################################################################################
replace_marks(PROJ_LIBRARY_DIRS  "SYSTEM|PLATFORM|MODE" 0 "${PROJ_LIBRARY_DIRS}")
list(APPEND PROJ_LIBRARY_DIRS "${PROJ_OUTDIR}")
replace_marks(PROJ_LIBRARY_NAMES "SYSTEM|PLATFORM|MODE" 0 "${PROJ_LIBRARY_NAMES}")

####################################################################################################
# Set Qt Library Environment
####################################################################################################
# Define Qt Components
set(PROJ_QT_COMPONENTS)
# Get Qt Components From Library Names
foreach(TEMP_LIBRARY_NAME IN LISTS PROJ_LIBRARY_NAMES)
    if(TEMP_LIBRARY_NAME MATCHES "^Qt([0-9]+)::")
        string(REPLACE "::" ";" TEMP_QT_VER_COM "${TEMP_LIBRARY_NAME}")
        # Check Qt Version
        list(GET TEMP_QT_VER_COM 0 TEMP_QT_VER)
        list(GET TEMP_QT_VER_COM 1 TEMP_QT_COM)
        if(NOT PROJ_QT_COMPONENTS)
            string(TOUPPER "${TEMP_QT_VER}" TEMP_QT_VER_UPPER)
            if(DEFINED ENV{${TEMP_QT_VER_UPPER}_DIR})
                set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "$ENV{${TEMP_QT_VER_UPPER}_DIR}")
            endif()
            find_package(QT NAMES ${TEMP_QT_VER} QUIET)
            if(QT_FOUND AND DEFINED QT_VERSION)
                message(CUSTOM_MESSAGE "-- Qt Version: ${QT_VERSION}")
            else()
                message(FATAL_ERROR "FATAL: \"${TEMP_QT_VER}\" not found, you can try to add the installation prefix of \"QT\" to CMAKE_PREFIX_PATH, or try to set \"${TEMP_QT_VER_UPPER}_DIR\" environment variable.")
            endif()
            unset(TEMP_QT_VER_UPPER)
        endif()
        # Check Qt Component
        find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${TEMP_QT_COM} QUIET)
        if(Qt${QT_VERSION_MAJOR}${TEMP_QT_COM}_FOUND)
            if (Qt${QT_VERSION_MAJOR}${TEMP_QT_COM}_INCLUDE_DIRS)
                list(APPEND PROJ_INCLUDE_DIRS "Qt${QT_VERSION_MAJOR}${TEMP_QT_COM}_INCLUDE_DIRS")
            endif()
            if (Qt${QT_VERSION_MAJOR}${TEMP_QT_COM}_PRIVATE_INCLUDE_DIRS)
                list(APPEND PROJ_INCLUDE_DIRS "Qt${QT_VERSION_MAJOR}${TEMP_QT_COM}_PRIVATE_INCLUDE_DIRS")
            endif()
            message(CUSTOM_MESSAGE "-- Qt Component ${TEMP_QT_COM}: OK")
        else()
            message(FATAL_ERROR "FATAL: Qt Component \"${TEMP_QT_COM}\" not found.")
        endif()
        # Append Qt Component To PROJ_QT_COMPONENTS
        list(APPEND PROJ_QT_COMPONENTS "${TEMP_LIBRARY_NAME}")
        # Remove Qt Component From PROJ_LIBRARY_NAMES
        if(TEMP_LIBRARY_NAME MATCHES "^Qt([0-9]+)::LinguistTools$")
            list(REMOVE_ITEM PROJ_LIBRARY_NAMES "${TEMP_LIBRARY_NAME}")
        endif()
        # Destroy Temporary Variable
        unset(TEMP_QT_COM)
        unset(TEMP_QT_VER)
        unset(TEMP_QT_VER_COM)
    endif()
endforeach(TEMP_LIBRARY_NAME)
# Set Qt Compiler Parameters
if(PROJ_QT_COMPONENTS)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    add_definitions(-D_QT_FRAMEWORK_USED=1)
endif()

####################################################################################################
# Set Other Library Environment
####################################################################################################
# Define Other Libraries
set(PROJ_OTHER_LIBRARIES)
# Get Other Libraries From Library Names
foreach(TEMP_LIBRARY_NAME IN LISTS PROJ_LIBRARY_NAMES)
    if(NOT TEMP_LIBRARY_NAME MATCHES "^Qt([0-9]*)::")
        SET(TEMP_LIBRARY_PATH "TEMP_LIBRARY_PATH-NOTFOUND")
        find_library(TEMP_LIBRARY_PATH NAMES "${TEMP_LIBRARY_NAME}" PATHS "${PROJ_LIBRARY_DIRS}")
        if(NOT TEMP_LIBRARY_PATH STREQUAL "TEMP_LIBRARY_PATH-NOTFOUND")
            message(CUSTOM_MESSAGE "-- Library ${TEMP_LIBRARY_NAME}: OK")
        else()
            message(FATAL_ERROR "FATAL: Library \"${TEMP_LIBRARY_NAME}\" not found.")
        endif()
        unset(TEMP_LIBRARY_PATH)
        # Append Other Libraries To PROJ_OTHER_LIBRARIES
        list(APPEND PROJ_OTHER_LIBRARIES "${TEMP_LIBRARY_NAME}")
    endif()
endforeach(TEMP_LIBRARY_NAME)

####################################################################################################
# Replace Marks Of The Include Directories And Project Files
####################################################################################################
replace_marks(PROJ_INCLUDE_DIRS "SYSTEM|PLATFORM|MODE|VARIABLE" 0 "${PROJ_INCLUDE_DIRS}")
replace_marks(PROJ_SOURCE_LIST  "SYSTEM|PLATFORM|MODE|VARIABLE" 1 "${PROJ_SOURCE_LIST}")

####################################################################################################
# Generate Proejct Resource Header File
####################################################################################################
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.h.in")
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.h" NEWLINE_STYLE CRLF)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.lrc")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.lrc" TEMP_LINUX_RC_LIST)
        string(REGEX REPLACE "[\r\n]+" ";" TEMP_LINUX_RC_LIST "${TEMP_LINUX_RC_LIST}")
        foreach(TEMP_LINUX_RC_FILE IN LISTS TEMP_LINUX_RC_LIST)
            execute_process(COMMAND "xxd" -i "${TEMP_LINUX_RC_FILE}" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Resource" OUTPUT_VARIABLE TEMP_LINUX_RC_CONTENT)
            string(REGEX REPLACE "[\r\n]+" "\r\n" TEMP_LINUX_RC_CONTENT "${TEMP_LINUX_RC_CONTENT}")
            file(APPEND "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.h" "${TEMP_LINUX_RC_CONTENT}")
        endforeach(TEMP_LINUX_RC_FILE)
    endif()
endif()

####################################################################################################
# Append Project Resource File
####################################################################################################
if(WIN32 AND NOT CYGWIN)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.manifest")
        list(APPEND PROJ_SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.manifest")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.h" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.rc")
        list(APPEND PROJ_SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.rc")
    endif()
endif()
if(PROJ_QT_COMPONENTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.qrc")
    list(APPEND PROJ_SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Resource.qrc")
endif()

####################################################################################################
# Set Qt UI Files Compile
####################################################################################################
foreach(TEMP_SOURCE_ITEM IN LISTS PROJ_SOURCE_LIST)
    if(TEMP_SOURCE_ITEM MATCHES "\\.ui$")
        # Build Qt Ui Header File (Output To Ui File Source Directory)
        string(REGEX REPLACE "([^/\\]+).ui$" "ui_\\1.h" TEMP_QT_UI_FILE_HEADER "${TEMP_SOURCE_ITEM}")
        set(UIC_OPTIONS)
        add_custom_command(OUTPUT "${TEMP_QT_UI_FILE_HEADER}" COMMAND "${Qt5Widgets_UIC_EXECUTABLE}" ARGS "${UIC_OPTIONS}" -o "${TEMP_QT_UI_FILE_HEADER}" "${TEMP_SOURCE_ITEM}" MAIN_DEPENDENCY "${TEMP_SOURCE_ITEM}" VERBATIM)
        unset(UIC_OPTIONS)
        set_property(SOURCE "${TEMP_QT_UI_FILE_HEADER}" PROPERTY SKIP_AUTOGEN ON)
        # Insert Qt Ui Header File
        list(INSERT PROJ_SOURCE_LIST BEFORE "${TEMP_QT_UI_FILE_HEADER}")
        unset(TEMP_QT_UI_FILE_HEADER)
        # Remove Qt Ui File
        list(REMOVE_ITEM PROJ_SOURCE_LIST "${TEMP_SOURCE_ITEM}")
    endif()
endforeach(TEMP_SOURCE_ITEM)

####################################################################################################
# Set QT TS Files Compile
####################################################################################################
set(TEMP_QT_TS_FILES)
foreach(TEMP_SOURCE_ITEM IN LISTS PROJ_SOURCE_LIST)
    if(TEMP_SOURCE_ITEM MATCHES "\\.ts$")
        list(APPEND      TEMP_QT_TS_FILES "${TEMP_SOURCE_ITEM}")
        list(REMOVE_ITEM PROJ_SOURCE_LIST "${TEMP_SOURCE_ITEM}")
    endif()
endforeach(TEMP_SOURCE_ITEM)
if(TEMP_QT_TS_FILES AND PROJ_QT_COMPONENTS)
    # Update Qt Translation Files
    qt5_create_translation(TEMP_QM_FILES_PATHS "${PROJ_SOURCE_LIST}" "${TEMP_QT_TS_FILES}" OPTIONS -source-language zh_CN -no-obsolete)
    list(INSERT PROJ_SOURCE_LIST BEFORE "${TEMP_QM_FILES_PATHS}")
    # Build Qt Translations Resource File Content
    set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "")
    set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}<RCC>\n")
    set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}  <qresource prefix=\"/Locales/\">\n")
    foreach(TEMP_QT_QM_FILE_PATH IN LISTS TEMP_QM_FILES_PATHS)
        get_filename_component(TEMP_QT_QM_FILE_NAME "${TEMP_QT_QM_FILE_PATH}" NAME)
        set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}    <file>${TEMP_QT_QM_FILE_NAME}</file>\n")
        unset(TEMP_QT_QM_FILE_NAME)
    endforeach(TEMP_QT_QM_FILE_PATH)
    set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}  </qresource>\n")
    set(TEMP_QT_QRC_TRANSLATIONS_CONTENT "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}</RCC>\n")
    # Write Qt Translations Resource File Content
    set(TEMP_QT_QRC_TRANSLATIONS_FILE "${CMAKE_CURRENT_BINARY_DIR}/Locales.qrc")
    if(EXISTS "${TEMP_QT_QRC_TRANSLATIONS_FILE}")
        file(READ "${TEMP_QT_QRC_TRANSLATIONS_FILE}" TEMP_QT_QRC_TRANSLATIONS_CONTENT_OLD)
        if("${TEMP_QT_QRC_TRANSLATIONS_CONTENT}" STREQUAL "${TEMP_QT_QRC_TRANSLATIONS_CONTENT_OLD}")
            unset(TEMP_QT_QRC_TRANSLATIONS_CONTENT)
        endif()
        unset(TEMP_QT_QRC_TRANSLATIONS_CONTENT_OLD)
    endif()
    if(TEMP_QT_QRC_TRANSLATIONS_CONTENT)
        file(WRITE "${TEMP_QT_QRC_TRANSLATIONS_FILE}" "${TEMP_QT_QRC_TRANSLATIONS_CONTENT}")
        message(CUSTOM_MESSAGE "-- Qt Translations: Rebuilt")
        unset(TEMP_QT_QRC_TRANSLATIONS_CONTENT)
    endif()
    # Add Qt Translations Resource File
    list(INSERT PROJ_SOURCE_LIST BEFORE "${TEMP_QT_QRC_TRANSLATIONS_FILE}")
    unset(TEMP_QT_QRC_TRANSLATIONS_FILE)
    unset(TEMP_QM_FILES_PATHS)
endif()
unset(TEMP_QT_TS_FILES)

####################################################################################################
# Set Qt QRC Files Compile
####################################################################################################
foreach(TEMP_SOURCE_ITEM IN LISTS PROJ_SOURCE_LIST)
    if(TEMP_SOURCE_ITEM MATCHES "\\.qrc$")
        # Build Qt QRC CXX File
        qt5_add_resources(TEMP_QT_UI_FILE_CXX "${TEMP_SOURCE_ITEM}")
        # Insert Qt QRC CXX File
        list(INSERT PROJ_SOURCE_LIST BEFORE "${TEMP_QT_UI_FILE_CXX}")
        unset(TEMP_QT_UI_FILE_CXX)
        # Remove Qt QRC File
        list(REMOVE_ITEM PROJ_SOURCE_LIST "${TEMP_SOURCE_ITEM}")
    endif()
endforeach(TEMP_SOURCE_ITEM)

####################################################################################################
# Set Win32 RC Files Compile
####################################################################################################
foreach(TEMP_SOURCE_ITEM IN LISTS PROJ_SOURCE_LIST)
    if(TEMP_SOURCE_ITEM MATCHES "\\.rc$")
        if    (WIN32 AND NOT CYGWIN)
            # Check RC Compiler
            set(TEMP_CMAKE_RC_COMPILER_PATH "TEMP_CMAKE_RC_COMPILER_PATH-NOTFOUND")
            find_path(TEMP_CMAKE_RC_COMPILER_PATH "${CMAKE_RC_COMPILER}")
            if(NOT TEMP_LIBRARY_PATH STREQUAL "TEMP_CMAKE_RC_COMPILER_PATH-NOTFOUND")
                message(CUSTOM_MESSAGE "-- RC Compiler: OK")
            else()
                message(FATAL_ERROR "-- FATAL: RC compiler not found.")
            endif()
            unset(TEMP_CMAKE_RC_COMPILER_PATH)
            break()
        elseif(UNIX OR CYGWIN)
            # Remove Win32 RC File
            list(REMOVE_ITEM PROJ_SOURCE_LIST "${TEMP_SOURCE_ITEM}")
        endif()
    endif()
endforeach(TEMP_SOURCE_ITEM)

####################################################################################################
# Set Project Directories
####################################################################################################
# Set Project Include Directories
include_directories(${PROJ_INCLUDE_DIRS})
# Set Proejct Library Directories
link_directories(${PROJ_LIBRARY_DIRS})
# Set Output Directory (Used to cmake)
set(ARCHIVE_OUTPUT_DIRECTORY "${PROJ_OUTDIR}")
set(LIBRARY_OUTPUT_PATH      "${PROJ_OUTDIR}")
set(RUNTIME_OUTPUT_DIRECTORY "${PROJ_OUTDIR}")
set(EXECUTABLE_OUTPUT_PATH   "${PROJ_OUTDIR}")
# Set Output Directory (Used to visual studio cmake)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${PROJ_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_PATH_DEBUG               "${PROJ_OUTDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${PROJ_OUTDIR}")
set(CMAKE_EXECUTABLE_OUTPUT_PATH_DEBUG            "${PROJ_OUTDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${PROJ_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_PATH_RELEASE             "${PROJ_OUTDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${PROJ_OUTDIR}")
set(CMAKE_EXECUTABLE_OUTPUT_PATH_RELEASE          "${PROJ_OUTDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL     "${PROJ_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_PATH_MINSIZEREL          "${PROJ_OUTDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${PROJ_OUTDIR}")
set(CMAKE_EXECUTABLE_OUTPUT_PATH_MINSIZEREL       "${PROJ_OUTDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJ_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_PATH_RELWITHDEBINFO      "${PROJ_OUTDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJ_OUTDIR}")
set(CMAKE_EXECUTABLE_OUTPUT_PATH_RELWITHDEBINFO   "${PROJ_OUTDIR}")

####################################################################################################
# Compiler Source Code
####################################################################################################
# Set Compile Source Code
if("${PROJ_TYPE}" STREQUAL "Console")                          # Generate command line application
    add_executable("${PROJ_NAME}" "${PROJ_SOURCE_LIST}")
elseif("${PROJ_TYPE}" STREQUAL "Desktop")                      # Generate desktop application
    if    (WIN32 OR CYGWIN)
        add_executable("${PROJ_NAME}" WIN32 "${PROJ_SOURCE_LIST}")
    elseif(UNIX OR CYGWIN)
        add_executable("${PROJ_NAME}" "${PROJ_SOURCE_LIST}")
    elseif(APPLE)
        add_executable("${PROJ_NAME}" MACOSX_BUNDLE "${PROJ_SOURCE_LIST}")
    endif()
elseif("${PROJ_TYPE}" STREQUAL "Static")                      # Generate static library
    add_library("${PROJ_NAME}" STATIC "${PROJ_SOURCE_LIST}")
elseif("${PROJ_TYPE}" STREQUAL "Shared")                      # Generate dynamic or shared library
    add_library("${PROJ_NAME}" SHARED "${PROJ_SOURCE_LIST}")
endif()

####################################################################################################
# Link Target Libraries
####################################################################################################
target_link_libraries("${PROJ_NAME}" PRIVATE "${PROJ_LIBRARY_NAMES}")
