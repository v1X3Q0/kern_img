add_library(kern_linux STATIC kern_img.cpp ki_grab.cpp pak_dyn.cpp
    pak_kallsym.cpp)

target_include_directories(kern_linux PRIVATE ../common)

target_include_directories(kern_linux PRIVATE
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/hde_armv8
    ${ANDROID_TOOL_BASE}/common/c_reginst)
