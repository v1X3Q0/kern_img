cmake_minimum_required(VERSION 3.0.0)
project(kern_img VERSION 0.1.0)

if(NOT DEFINED ANDROID_TOOL_BASE)
    message(FATAL_ERROR "need ANDROID_TOOL_BASE")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

# if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/linux)
# endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${MY_HOST_COMPILER})
