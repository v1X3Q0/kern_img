#include <localUtil.h>
#include <localUtil_windows.h>

#include "kern_img.h"

void kernel_windows::target_set_known_offsets()
{

}

int kernel_windows::dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out)
{
    int result = -1;
	
    return result;
}

int kernel_windows::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    int livek = 0;
    size_t outAddr = 0;

    if (live_kernel == true)
    {
        livek = 1;
    }

    outAddr = redlsym((UINT8*)binBegin, newString, livek);
    SAFE_BAIL(outAddr == 0);

    result = 0;
    if (out_address != 0)
    {
        *out_address = outAddr;
    }
fail:
    return result;
}
