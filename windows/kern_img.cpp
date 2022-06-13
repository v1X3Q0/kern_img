#include <localUtil.h>
#include <localUtil_windows.h>

#include "kern_img.h"

void kernel_windows::target_set_known_offsets()
{
    kern_off_map["kprocess.DirectoryTableBase"] = 0x28;
    
    kern_off_map["eprocess.Pcb"] = 0;
    kern_off_map["eprocess.UniqueProcessId"] = 0x440;
    kern_off_map["eprocess.ActiveProcessLinks"] = kern_off_map["eprocess.UniqueProcessId"] + sizeof(void*);    
}

int kernel_windows::dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out)
{
    int result = -1;
    named_kmap_t* nt_block = 0;
    uint8_t* nt_tmp = 0;
    IMAGE_SECTION_HEADER* sect_find = 0;

    if (live_kernel == true)
    {
        SAFE_BAIL(check_kmap("nt_header", &nt_block) == -1);
        nt_tmp = nt_block->kmap_stats.alloc_base;
        SAFE_BAIL(get_pesection(nt_tmp, kmap_nanme.data(), &sect_find) == -1);
        insert_section(kmap_nanme, sect_find->VirtualAddress + nt_block->kva, sect_find->Misc.VirtualSize);
    }

finish:
    result = 0;
    if (block_out != 0)
    {
        check_kmap(kmap_nanme, block_out);
    }
fail:
    return result;
}
