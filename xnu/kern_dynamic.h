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

    kern_dynamic(uint32_t* binBegin_a);

private:
    kernel_xnu* syskern_static;

    // optionally, keep a pointer to a static image to use
    // if we are on a target where we can use the system
    // kernel for symbols that we don't want to find with
    // heuristics.
};

#endif