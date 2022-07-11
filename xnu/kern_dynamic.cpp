#include <string>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <TargetConditionals.h>

#include <localUtil.h>
#include <localUtil_xnu.h>
// #include <krw_util.h>
#include <kernel_resolver.h>
#include <Darwin_dyn_offset.h>

#include "kern_dynamic.h"
#include "kern_static.h"

kern_dynamic::kern_dynamic(uint32_t* binBegin_a) : kernel_xnu(binBegin_a)
{
#if TARGET_OS_OSX
    syskern_static = kernel_block::allocate_kern_img<kern_static>("/System/Library/Kernels/kernel.release.t8101");
#endif
}

int kern_dynamic::dresolve_live_symbol(const char *symbol, void** symbol_out)
{
    int result = -1;
    void* symTmp = 0;
    struct section_64* section_64_static = 0;
    const struct section_64* section_64_live = 0;
    named_kmap_t* mh_base = 0;
    struct mach_header_64* mh_dyn = 0;

    // on the read file
    SAFE_BAIL(syskern_static->ksym_dlsym(symbol, (size_t*)&symTmp) == -1);
    SAFE_BAIL(section_with_sym((struct mach_header_64*)syskern_static->get_binbegin(), (size_t)symTmp, &section_64_static) == -1);

    // on the live kernel header
    SAFE_BAIL(check_kmap("mach_header", &mh_base) == -1);
    mh_dyn = (struct mach_header_64*)mh_base->kmap_stats.alloc_base;
    section_64_live = getsectbynamefromheader_64(mh_dyn, section_64_static->segname, section_64_static->sectname);
    SAFE_BAIL(section_64_live == 0);

    result = 0;
    if (symbol_out != 0)
    {
        *symbol_out = (void*)((size_t)symTmp - section_64_static->addr + section_64_live->addr);
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
    SAFE_BAIL(dresolve_live_symbol(newString, (void**)&symtmp) == -1);

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
    struct mach_header_64* mach_header_temp = 0;
    uint8_t* mach_header_map = 0;

    SAFE_BAIL(live_kern_addr(binBegin, sizeof(struct mach_header_64), (void**)&mach_header_temp) == -1);

    insert_section("mach_header", binBegin, mach_header_temp->sizeofcmds);

    set_known_offsets();
    target_set_known_offsets();

#ifdef TARGET_OS_OSX
    register_statics();
#endif

    dyn_offsets(this);

    result = 0;
fail:
    SAFE_FREE(mach_header_temp);
    return result;
}

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}
