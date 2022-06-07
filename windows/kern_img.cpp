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
	
    return result;
}
