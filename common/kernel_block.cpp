#include <string.h>

#include <localUtil.h>

#include "kernel_block.h"

void kernel_block::set_known_offsets()
{
    kern_off_map["list_entry.next"] = 0;
    kern_off_map["list_entry.prev"] = sizeof(size_t);

    kern_off_map["list_entry<T>.next"] = 0;
    kern_off_map["list_entry<T>.prev"] = sizeof(size_t);

}

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

int kernel_block::kern_off_insert(std::string koff_name, size_t offval)
{
    kern_off_map[koff_name] = offval;
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

