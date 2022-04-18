#include "kern_img.h"

void kernel_xnu::target_set_known_offsets()
{
    kern_off_map["proc.p_list"] = 0;
    kern_off_map["proc.task"] = sizeof(void*) * 2;
    
    // temporary set to get around offsets and stuff
    kern_off_map["proc.p_pid"] = 0x44;
}
