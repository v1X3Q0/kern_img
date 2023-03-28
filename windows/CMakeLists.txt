cmake_minimum_required(VERSION 3.0.0)
project(kern_windows)

# so in mac and windows case, they have access to their static
# kernel's but android doesn't. so kern_static will be omitted
# from the android build. linux also seems to follow this
# convention
set(SOURCES_LIST
    kern_img.cpp
    )

list(APPEND SOURCES_LIST kern_static.cpp)

if (NOT ${MY_HOST_COMPILER} MATCHES "windows")
    set(NOLIVE 1)
endif()

if (NOT DEFINED NOLIVE)
    list(APPEND SOURCES_LIST kern_dynamic.cpp)
endif()

set(LOCAL_INCLUDES ${KERN_IMG_INCLUDES})

list(APPEND LOCAL_INCLUDES .)

if (NOT ${MY_HOST_COMPILER} MATCHES "windows")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/local_util/windows_compat)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOCAL_INCLUDES})

