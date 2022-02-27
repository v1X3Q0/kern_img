add_library(kernel_block STATIC kernel_block.cpp
    kernel_block_live.cpp)

target_include_directories(kernel_block PRIVATE
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/hde_armv8
    ${ANDROID_TOOL_BASE}/common/c_reginst)
