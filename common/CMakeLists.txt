project(kernel_block)

set(LOCAL_INCLUDES
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
)

if (ARCH STREQUAL "arm64" OR ARCH STREQUAL "arm64e")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_armv8)
elseif (ARCH STREQUAL "AMD64" OR ARCH STREQUAL "i386" OR ARCH MATCHES "x86_64")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_x86)
endif()

add_library(${PROJECT_NAME} STATIC kernel_block.cpp)
add_library(${PROJECT_NAME}_live STATIC kernel_block_live.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_INCLUDES})
target_include_directories(${PROJECT_NAME}_live PRIVATE ${LOCAL_INCLUDES})

target_include_directories(${PROJECT_NAME}_live PRIVATE
    ${ANDROID_TOOL_BASE}/common/krw_util/krw_util
    ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include)
