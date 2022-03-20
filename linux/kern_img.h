#ifndef KERN_IMG_H
#define KERN_IMG_H

#include <kernel_block.h>
#include <elf.h>

// generic linux kernel functionality, these are the parts that are backwards
// compatible with live kernels and non live kernels

class kernel_linux : public kernel_block
{
protected:
    size_t ksyms_count;

public:
    using kernel_block::kernel_block;
    virtual void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) = 0;
    // virtual int kcrc_index(std::string symbol, uint32_t* kcrc);

    int findKindInKstr(const char* newString, int* index);

    size_t get_ksyms_count() { return ksyms_count; };
    unsigned int kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen);
    unsigned long kallsyms_lookup_name(const char *name);
    unsigned long kallsyms_sym_address(int idx);
};

#endif