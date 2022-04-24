#include <string.h>

#include <localUtil.h>
#include <ibeSet.h>
#include <bgrep_e.h>

#include "kernel_block.h"

int kernel_block::kern_sym_fetch(std::string kstruct_name, size_t* ksym_out)
{
    int result = -1;
    
    SAFE_BAIL(kern_sym_map.find(kstruct_name) == kern_sym_map.end());
    *ksym_out = kern_sym_map[kstruct_name];
    result = 0;
fail:
    return result;
}

int kernel_block::kern_sym_insert(std::string ksym_name, size_t symaddr)
{
    kern_sym_map[ksym_name] = symaddr;
    return 0;
}

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

int kernel_block::check_kmap_force(std::string kmap_name, named_kmap_t** named_kmap_out)
{
    int result = 0;
    named_kmap_t* found_block = 0;

    if (live_kernel == false)
    {
        SAFE_BAIL(check_kmap(kmap_name, named_kmap_out) == -1);
        SAFE_BAIL(dyn_kmap_find(kmap_name, &found_block) == -1);
    }
    else
    {

    }

    result = 0;
fail:
    return result;
}

int kernel_block::kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, bool volatile_region, void** out_img_off)
{
    int result = -1;
    void* img_var_local = (void*)img_var;

    SAFE_LIVEKERN(SAFE_BAIL(volatile_map(img_var, img_var_sz, &img_var_local, false) == -1))

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
