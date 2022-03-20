project(kernel_block)

add_library(${PROJECT_NAME} STATIC kernel_block.cpp
    kernel_block_live.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
    )

if (DEFINED LIVE_KERNEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        LIVE_KERNEL=${LIVE_KERNEL})
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${ANDROID_TOOL_BASE}/common/krw_util/krw_util
        ${ANDROID_TOOL_BASE}/linux/rwDrv/include)
    if (ARCH STREQUAL "arm64")
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${ANDROID_TOOL_BASE}/common/hde_armv8)
    endif()
endif()
