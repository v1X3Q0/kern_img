cmake_minimum_required(VERSION 3.0.0)
# project(kern_img VERSION 0.1.0)

if(NOT DEFINED ANDROID_TOOL_BASE)
    message(FATAL_ERROR "need ANDROID_TOOL_BASE")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

# if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/linux)
# endif()

project(kern_${MY_HOST_COMPILER})

set(SOURCES_LIST
    ${MY_HOST_COMPILER}/kern_img.cpp
    ${MY_HOST_COMPILER}/kern_dynamic.cpp
    ${MY_HOST_COMPILER}/kern_static.cpp)

if (${MY_HOST_COMPILER} MATCHES "linux")
    list(APPEND SOURCES_LIST ${MY_HOST_COMPILER}/pak_kallsym.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})

set(LOCAL_INCLUDES
    ${MY_HOST_COMPILER}
    common
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
    # for now linux is seperately maintained, so dyndir is separate.
    # ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/${MY_HOST_COMPILER}_dyn_offset
    ${DYNDIR}
    ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include
)

if (ARCH MATCHES "arm64")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_armv8)
elseif (ARCH MATCHES "AMD64" OR ARCH MATCHES "x86_64")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_x86)
endif()

# just a definition for static variants
if (${MY_HOST_COMPILER} MATCHES "xnu")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/xnu/kernelresolver)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOCAL_INCLUDES})
