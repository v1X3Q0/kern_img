#ifndef KERN_DYNAMIC_H
#define KERN_DYNAMIC_H

#include <string>

#include "kern_img.h"
#include "kern_static.h"

class kern_dynamic : public kernel_windows
{
public:
    // virtual methods to be implemented, 
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size);
    int ksym_dlsym(const char* newString, size_t* out_address);

    int kva_to_raw(const char* symbol, void** symbol_out);

    kern_dynamic(uint32_t* binBegin_a);
    int register_statics();

private:
    kern_static* syskern_static;
    using kernel_windows::kernel_windows;
};

#endif