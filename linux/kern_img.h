#ifndef KERN_IMG_H
#define KERN_IMG_H

#include <kernel_block.h>
#include <elf.h>

// generic linux kernel functionality, these are the parts that are backwards
// compatible with live kernels and non live kernels

class kern_img : public kernel_block
{
protected:
    using kernel_block::kernel_block;
    size_t ksyms_count;
    std::vector<std::pair<std::string, Elf64_Shdr*>> sect_list;

    // generic grab routines
    int grab_sinittext();

    // generic base routines
    int base_inits();
    int base_ksymtab();

public:
    virtual void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) = 0;

    int findKindInKstr(const char* newString, int* index);

    size_t get_ksyms_count() { return ksyms_count; };
    unsigned int kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen);
    unsigned long kallsyms_lookup_name(const char *name);
    unsigned long kallsyms_sym_address(int idx);
};

#endif