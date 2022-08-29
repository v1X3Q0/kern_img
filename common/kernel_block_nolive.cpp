#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "kernel_block.h"
#include <localUtil.h>

int kernel_block::live_kern_addr(size_t target_kernel_address, size_t size_kernel_buf, void** out_live_addr)
{
    int result = -1;

    if (out_live_addr != 0)
    {
        *out_live_addr = (void*)target_kernel_address;
    }
    result = 0;

    return result;
}

int kernel_block::volatile_map(size_t kva, size_t kv_size, void** virt_ret, bool volatile_op)
{
    int result = -1;

    SAFE_BAIL(live_kern_addr(kva, kv_size, virt_ret) == -1);

    result = 0;
fail:
    return result;
}

int kernel_block::volatile_free(size_t kva, void* virt_used, bool volatile_op)
{
    return 0;
}
