#include "kern_img.h"

void kernel_xnu::target_set_known_offsets()
{
    kern_off_map["proc.p_list"] = 0;
    kern_off_map["proc.task"] = sizeof(void*) * 2;
    
    // temporary set to get around offsets and stuff
    kern_off_map["proc.p_pid"] = 0x68;
    kern_off_map["proc.p_ublock"] = 0x20;
    
    kern_off_map["ublock.u_proc"] = 0x00;
    kern_off_map["ublock.u_task"] = sizeof(size_t);
    kern_off_map["ublock.u_ucred"] = 0x20;

    kern_off_map["ucred_t.cr_posix"] = 0x18;
    
    kern_off_map["posix_cred.cr_uid"] = 0;
    kern_off_map["posix_cred.cr_ruid"] = sizeof(uid_t);
}
