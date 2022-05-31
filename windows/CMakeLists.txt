project(kern_windows)
add_library(${PROJECT_NAME} STATIC kern_img.cpp kern_dynamic.cpp kern_static.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
    ../common
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/local_util)
