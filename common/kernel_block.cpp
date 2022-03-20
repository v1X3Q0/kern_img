#include <string.h>

#include <localUtil.h>
#include <ibeSet.h>
#include <bgrep_e.h>

#include "kernel_block.h"

#ifndef LIVE_KERNEL
// only for live kernel definitions
int kernel_block::live_kern_addr(size_t target_kernel_address, size_t size_kernel_buf, void** out_live_addr)
{
    int result = -1;
    *out_live_addr = (void*)target_kernel_address;
    result = 0;
    return result;
}
#endif

int kernel_block::kstruct_offset(std::string kstruct_name, size_t* kstruct_off_out)
{
    int result = -1;
    
    SAFE_BAIL(kern_off_map.find(kstruct_name) == kern_off_map.end());
    *kstruct_off_out = kern_off_map[kstruct_name];
    result = 0;
fail:
    return result;
}

int kernel_block::check_kmap(std::string kmap_name, named_kmap_t** named_kmap_out)
{
    int result = -1;

    auto kmap_saved = named_alloc_list.find(kmap_name);
    SAFE_BAIL(kmap_saved == named_alloc_list.end());

    if (named_kmap_out != 0)
    {
        *named_kmap_out = kmap_saved->second;
    }
    result = 0;
fail:
    return result;
}

int kernel_block::kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, bool volatile_region, void** out_img_off)
{
    int result = -1;
    void* img_var_local = (void*)img_var;

#ifdef LIVE_KERNEL
    SAFE_BAIL(volatile_map(img_var, img_var_sz, &img_var_local, false) == -1);
#endif

    SAFE_BAIL(getB->findPattern((uint8_t*)img_var_local, img_var_sz, (void**)out_img_off) == -1);
#ifdef LIVE_KERNEL
    *out_img_off = (void**)((size_t)(*out_img_off) - (size_t)img_var_local + (size_t)img_var);
#endif
    result = 0;
fail:
#ifdef LIVE_KERNEL
    volatile_free(img_var, img_var_local, volatile_region);
#endif
    return result;
}

int kernel_block::kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, void** out_img_off)
{
    return kernel_search(getB, img_var, img_var_sz, true, out_img_off);
}

size_t kernel_block::resolveRel(size_t rebase)
{
    return rebase - binBegin;
}

