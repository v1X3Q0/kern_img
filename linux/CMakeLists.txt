cmake_minimum_required(VERSION 3.0.0)
project(kern_linux)

# so in mac and windows case, they have access to their static
# kernel's but android doesn't. so kern_static will be omitted
# from the android build. linux also seems to follow this
# convention
set(SOURCES_LIST kern_img.cpp)

list(APPEND SOURCES_LIST pak_kallsym.cpp)

# ===========================================
# ============ toolchain arch ===============
# ===========================================
if (ARCH MATCHES "arm64" OR ARCH MATCHES "arm64e" OR ARCH MATCHES "aarch64")
    set(COMP_DEFS SUPPORT_AARCH64=1)
# I believe windows does AMD64 and linux does x86_64
elseif(ARCH MATCHES "AMD64" OR ARCH MATCHES "i386" OR ARCH MATCHES "x86_64")
    set(COMP_DEFS SUPPORT_X86_64=1)
endif()

if (NOT ${MY_HOST_COMPILER} MATCHES "linux")
    set(NOLIVE 1)
endif()

if (NOT DEFINED NOLIVE)
    list(APPEND SOURCES_LIST kern_dynamic.cpp)
endif()

set(LOCAL_INCLUDES ${KERN_IMG_INCLUDES})

list(APPEND LOCAL_INCLUDES .)

if (NOT ${MY_HOST_COMPILER} MATCHES "linux")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/local_util/linux_compat)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES_LIST})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMP_DEFS})

target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_INCLUDES})

