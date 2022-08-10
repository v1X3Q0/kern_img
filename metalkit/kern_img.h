#ifndef KERN_IMG_H
#define KERN_IMG_H

#include <kernel_block.h>

// generic linux kernel functionality, these are the parts that are backwards
// compatible with live kernels and non live kernels

class kernel_metalkit : public kernel_block
{
protected:
    // necessary routines
    void target_set_known_offsets() {};
    int dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out) { return 0; };

public:
    using kernel_block::kernel_block;

    // necessary routines
    int ksym_dlsym(const char* newString, size_t* out_address) { return 0; };
    int parseAndGetGlobals() { return 0; };
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) {};
};

#endif