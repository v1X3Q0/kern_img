#ifdef LIVE_KERNEL

#include <string>
#include <mach-o/loader.h>
#include <mach/mach.h>

#include <localUtil.h>
#include <localUtil_xnu.h>
// #include <krw_util.h>
#include "kern_dynamic.h"

int kern_dynamic::ksym_dlsym(const char* newString, size_t* out_address)
{
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

    result = 0;
fail:
    SAFE_FREE(mach_header_temp);
    return result;
}

void kern_dynamic::target_set_known_offsets()
{
    kern_off_map["proc.p_list"] = 0;
    kern_off_map["proc.task"] = sizeof(void*) * 2;
    
    // temporary set to get around offsets and stuff
    kern_off_map["proc.p_pid"] = 0x44;
}

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}

#endif // LIVE_KERNEL
