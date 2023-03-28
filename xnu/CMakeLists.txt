cmake_minimum_required(VERSION 3.0.0)
project(kern_xnu)

# so in mac and windows case, they have access to their static
# kernel's but android doesn't. so kern_static will be omitted
# from the android build. linux also seems to follow this
# convention
set(SOURCES_LIST
    kern_img.cpp
    )

list(APPEND SOURCES_LIST kern_static.cpp)

if (NOT DEFINED NOLIVE)
    list(APPEND SOURCES_LIST kern_dynamic.cpp)
endif()

set(LOCAL_INCLUDES ${KERN_IMG_INCLUDES})

list(APPEND LOCAL_INCLUDES .)

# just a definition for static variants
list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/xnu/kernelresolver)

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOCAL_INCLUDES})

