project(kern_${MY_HOST_COMPILER})
add_library(${PROJECT_NAME} STATIC kern_dynamic.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../common)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst)

if (DEFINED LIVE_KERNEL)
# in case i someday need live parsing in this library, likely
    # target_include_directories(${PROJECT_NAME} PRIVATE
    #     ${ANDROID_TOOL_BASE}/common/krw_util/krw_util
    #     ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        LIVE_KERNEL=${LIVE_KERNEL})
    if (ARCH STREQUAL "arm64" OR ARCH STREQUAL "arm64e")
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${ANDROID_TOOL_BASE}/common/hde_armv8)
    endif()
endif()
