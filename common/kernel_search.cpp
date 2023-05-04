#include <string.h>

#include <localUtil.h>
// #include <ibeSet.h>
#include <bgrep_e.h>

#include "kernel_block.h"

int kernel_block::kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, bool volatile_region, void** out_img_off)
{
    int result = -1;
    void* img_var_local = (void*)img_var;

    SAFE_LIVEKERN(SAFE_BAIL(volatile_map(img_var, img_var_sz, &img_var_local, volatile_region) == -1))

    SAFE_BAIL(getB->findPattern((uint8_t*)img_var_local, img_var_sz, (void**)out_img_off) == -1);
    SAFE_LIVEKERN(*out_img_off = (void**)((size_t)(*out_img_off) - (size_t)img_var_local + (size_t)img_var))
    result = 0;
fail:
    SAFE_LIVEKERN(volatile_free(img_var, img_var_local, volatile_region))
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

int kernel_block::check_kmap_force(std::string kmap_name, named_kmap_t** named_kmap_out)
{
    int result = 0;
    named_kmap_t* found_block = 0;

    if (live_kernel == true)
    {
        FINISH_IF(check_kmap(kmap_name, &found_block) == 0);
        FINISH_IF(dyn_kmap_find(kmap_name, &found_block) == 0);
    }
    else
    {

    }

finish:
    result = 0;
    if (named_kmap_out != 0)
    {
        *named_kmap_out = found_block;
    }
fail:
    return result;
}
