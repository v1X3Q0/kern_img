project(kernel_block)

set(LOCAL_INCLUDES
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
)

if (ARCH MATCHES "arm64" OR ARCH MATCHES "arm64e" OR ARCH MATCHES "aarch64")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_armv8)
elseif (ARCH MATCHES "AMD64" OR ARCH MATCHES "i386" OR ARCH MATCHES "x86_64")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_x86)
endif()

set(SOURCES_LIST
    kernel_block.cpp)

if (DEFINED NOLIVE)
    list(APPEND SOURCES_LIST kernel_block_nolive.cpp)
else()
    list(APPEND SOURCES_LIST kernel_block_live.cpp)
    list(APPEND LOCAL_INCLUDES
        ${ANDROID_TOOL_BASE}/common/krw_util/krw_util
        ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include
        )
endif()


if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Metalkit")
    list(APPEND SOURCES_LIST kernel_search.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_INCLUDES})
