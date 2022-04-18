#ifndef KERN_STATIC_H
#define KERN_STATIC_H

#include <string>
#include "kern_img.h"

class kern_static : public kernel_xnu
{
public:
    // virtual methods to be implemented, 
    int ksym_dlsym(const char* newString, size_t* out_address);
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) {};
    int insert_sections();

private:
    using kernel_xnu::kernel_xnu;
    // task strut offsets, for now its easier for us to just assume this routine
    // is for dynamic only.

};

#endif