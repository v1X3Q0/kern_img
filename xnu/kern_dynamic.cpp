#include <string>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <TargetConditionals.h>

#include <localUtil.h>
#include <localUtil_xnu.h>
// #include <krw_util.h>
#include "kern_dynamic.h"
#include "kern_static.h"

kern_dynamic::kern_dynamic(uint32_t* binBegin_a) : kernel_xnu(binBegin_a)
{
#if TARGET_OS_OSX
    syskern_static = kernel_block::allocate_kern_img<kern_static>("/System/Library/Kernels/kernel.release.t8101");
#endif
}

int kern_dynamic::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    size_t symtmp = 0;

    FINISH_IF(kern_sym_fetch(newString, &symtmp) == 0);
    SAFE_BAIL(syskern_static == 0);
    SAFE_BAIL(syskern_static->ksym_dlsym(newString, &symtmp) == -1);

finish:
    if (out_address != 0)
    {
        *out_address = symtmp;
    }
    result = 0;
fail:
    return result;
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

    result = 0;
fail:
    SAFE_FREE(mach_header_temp);
    return result;
}

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}
