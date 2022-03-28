#ifndef KERN_DYNAMIC_H
#define KERN_DYNAMIC_H

#include "kern_img.h"

class kern_dynamic : public kernel_xnu
{
public:
    // virtual methods to be implemented, 
    int ksym_dlsym(const char* newString, size_t* out_address);
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size);

private:
    using kernel_xnu::kernel_xnu;
    void target_set_known_offsets();
    // task strut offsets, for now its easier for us to just assume this routine
    // is for dynamic only.

};

#endif