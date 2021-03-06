#ifndef KERN_IMG_H
#define KERN_IMG_H

#include <Windows.h>

#include <kernel_block.h>

// generic linux kernel functionality, these are the parts that are backwards
// compatible with live kernels and non live kernels

class kernel_windows : public kernel_block
{
protected:
    void target_set_known_offsets();
    int dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out);

public:
    virtual kernel_windows* get_syskern_static() = 0;
    using kernel_block::kernel_block;
};

#endif