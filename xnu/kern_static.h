#ifndef KERN_STATIC_H
#define KERN_STATIC_H

#include <string>
#include "kern_img.h"

class kern_static : public kernel_xnu
{
public:
    // virtual methods to be implemented, 
    int ksym_dlsym(const char* newString, uint64_t* out_address);
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) {};
    int insert_sections();

    std::map<std::string, uint64_t>* kern_sym_map_fetch() { return &kern_sym_map; };
    std::map<std::string, uint64_t>* kern_off_map_fetch()  { return &kern_off_map; };

private:
    using kernel_xnu::kernel_xnu;
    // task strut offsets, for now its easier for us to just assume this routine
    // is for dynamic only.

};

#endif