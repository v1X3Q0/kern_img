#ifndef KERN_DYNAMIC_H
#define KERN_DYNAMIC_H

#include "kern_img.h"

class kern_dynamic : public kern_img
{
public:
    // virtual methods to be implemented, 
    int ksym_dlsym(const char* newString, size_t* out_address);
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size);

private:
    using kern_img::kern_img;

    // dynamic stores ksymtab, kcrctab and ksymstr in the same struct, so one
    // routine can pull them all.
    int base_ksymtab_kcrctab_ksymtabstrings();

    // task strut offsets, for now its easier for us to just assume this routine
    // is for dynamic only.
    int grab_task_struct_offs();

};

#endif