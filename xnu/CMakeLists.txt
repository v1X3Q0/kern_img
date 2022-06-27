project(kern_${MY_HOST_COMPILER})

# set(KERN_BLOCK_DYNAMIC kern_${MY_HOST_COMPILER}_dynamic)
# set(KERN_BLOCK_STATIC kern_${MY_HOST_COMPILER}_static)

# add_library(${KERN_BLOCK_DYNAMIC} STATIC kern_dynamic.cpp)
# add_library(${KERN_BLOCK_STATIC} STATIC kern_static.cpp)
add_library(${PROJECT_NAME} STATIC kern_img.cpp kern_dynamic.cpp kern_static.cpp)

set(LOCAL_INCLUDES
    ./
    ../common
    ${ANDROID_TOOL_BASE}/common/local_util
    ${ANDROID_TOOL_BASE}/common/bgrep_c
    ${ANDROID_TOOL_BASE}/common/c_reginst
    ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/${MY_HOST_COMPILER}_dyn_offset
    ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include
)

if (ARCH STREQUAL "arm64" OR ARCH STREQUAL "arm64e")
    list(APPEND LOCAL_INCLUDES ${ANDROID_TOOL_BASE}/common/hde_armv8)
endif()

# target_include_directories(${KERN_BLOCK_DYNAMIC} PRIVATE ${LOCAL_INCLUDES})    
# target_include_directories(${KERN_BLOCK_STATIC} PRIVATE ${LOCAL_INCLUDES})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOCAL_INCLUDES})

# just a definition for static variants
target_include_directories(${PROJECT_NAME} PRIVATE
    ${ANDROID_TOOL_BASE}/xnu/kernelresolver)

# in case i someday need live parsing in this library, likely
# target_include_directories(${PROJECT_NAME} PRIVATE
#     ${ANDROID_TOOL_BASE}/common/krw_util/krw_util
#     ${ANDROID_TOOL_BASE}/${MY_HOST_COMPILER}/${RW_DRIVER}/include)
