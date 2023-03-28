#ifndef KERN_STATIC_H
#define KERN_STATIC_H

#include <string>
#include "kern_img.h"

#define KERNEL_PATH "C:\\Windows\\System32\\ntoskrnl.exe"

class kern_static : public kernel_windows
{
public:
    // virtual methods to be implemented, 
    int ksym_dlsym(const char* newString, size_t* out_address);
    int parseAndGetGlobals();
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) {};
    int insert_sections();

    std::map<std::string, uint64_t>* kern_sym_map_fetch() { return &kern_sym_map; };
    std::map<std::string, uint64_t>* kern_off_map_fetch()  { return &kern_off_map; };

    kernel_windows* get_syskern_static() { return (kernel_windows*)this; };
private:
    using kernel_windows::kernel_windows;
    // task strut offsets, for now its easier for us to just assume this routine
    // is for dynamic only.

};

#endif