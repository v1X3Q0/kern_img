cmake_minimum_required(VERSION 3.0.0)
# project(kern_img VERSION 0.1.0)

if(NOT DEFINED ANDROID_TOOL_BASE)
    message(FATAL_ERROR "need ANDROID_TOOL_BASE")
endif()

set(KERN_IMG_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
    # for now linux is seperately maintained, so dyndir is separate.
    # ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/${MY_HOST_COMPILER}_dyn_offset
    ${DYNDIR}
    ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include
)

if (ARCH MATCHES "arm64" OR ARCH MATCHES "aarch64")
    list(APPEND KERN_IMG_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_armv8)
elseif (ARCH MATCHES "AMD64" OR ARCH MATCHES "x86_64")
    list(APPEND KERN_IMG_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_x86)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/xnu)
endif()

