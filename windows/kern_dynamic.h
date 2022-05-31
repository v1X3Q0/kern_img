#ifndef KERN_DYNAMIC_H
#define KERN_DYNAMIC_H

#include <string>

#include "kern_img.h"

class kern_dynamic : public kernel_windows
{
public:
    // virtual methods to be implemented, 
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size);
    int ksym_dlsym(const char* newString, size_t* out_address);

private:
    using kernel_windows::kernel_windows;
};

#endif