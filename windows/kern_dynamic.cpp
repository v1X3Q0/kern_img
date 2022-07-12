#include <string>
#include <Windows.h>

#include <localUtil_windows.h>

#include <Windows_dyn_offset.h>

#include "kern_dynamic.h"

kern_dynamic::kern_dynamic(uint32_t* binBegin_a) : kernel_windows(binBegin_a)
{
    syskern_static = kernel_block::allocate_kern_img<kern_static>(KERNEL_PATH);
}

kern_dynamic::kern_dynamic(uint32_t* binBegin_a, kernel_block* kern_tmp) : kernel_windows(binBegin_a)
{
    syskern_static = (kern_static*)kern_tmp;
}

int kern_dynamic::kva_to_raw(const char *symbol, void** symbol_out)
{
    int result = -1;
    size_t symTmp = 0;
    IMAGE_SECTION_HEADER* section_64_static = 0;

    // on the read file
    SAFE_BAIL(syskern_static->ksym_dlsym(symbol, &symTmp) == -1);
    SAFE_BAIL(section_with_sym((uint8_t*)syskern_static->get_binbegin(), symTmp, &section_64_static) == -1);

    symTmp = symTmp - section_64_static->VirtualAddress + section_64_static->PointerToRawData;

    result = 0;
    if (symbol_out != 0)
    {
        *symbol_out = (void*)symTmp;
    }
fail:
    return result;
}

int kern_dynamic::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    size_t symtmp = 0;
    named_kmap_t* mh_base = 0;

    FINISH_IF(kern_sym_fetch(newString, &symtmp) == 0);
    SAFE_BAIL(syskern_static == 0);
    symtmp = redlsym((uint8_t*)syskern_static->get_binbegin(), newString, FALSE);
    SAFE_BAIL(symtmp == 0);
    symtmp = symtmp + binBegin;

finish:
    result = 0;
    if (out_address != 0)
    {
        *out_address = symtmp;
    }
fail:
    return result;
}

int kern_dynamic::register_statics()
{
    std::map<std::string, size_t>* static_syms = 0;
    static_syms = syskern_static->kern_sym_map_fetch();
    auto i = static_syms->begin();
    size_t resolved_addr = 0;

    for (; i != static_syms->end(); i++)
    {
        ksym_dlsym(i->first.data(), &resolved_addr);
        kern_sym_insert(i->first, resolved_addr);
    }
    return 0;
}

int kern_dynamic::parseAndGetGlobals()
{
    int result = -1;
    uint8_t* nt_header_map = 0;
    size_t nt_header_size = 0;
    size_t opt_header_size = 0;

    opt_header_size = sizeof(IMAGE_DOS_HEADER) + NT_HEADER_SIZE + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER);
    opt_header_size = PAGE_SIZE4K;

    SAFE_BAIL(live_kern_addr(binBegin, opt_header_size, (void**)&nt_header_map) == -1);
    SAFE_BAIL(nt_headsize(nt_header_map, &nt_header_size) == -1);

    insert_section("nt_header", binBegin, nt_header_size);

    set_known_offsets();
    target_set_known_offsets();

    register_statics();

    dyn_offsets(this);

    result = 0;
fail:
    SAFE_FREE(nt_header_map);
    return result;
}

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}
