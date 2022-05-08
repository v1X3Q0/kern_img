#include "kern_img.h"
#include <mach-o/loader.h>
#include <xnu_types.h>

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

    kern_off_map["ptov_table_entry_t.pa"] = offsetof(ptov_table_entry, pa);
    kern_off_map["ptov_table_entry_t.va"] = offsetof(ptov_table_entry, va);
    kern_off_map["ptov_table_entry_t.len"] = offsetof(ptov_table_entry, len);

    kern_off_map["pmap_t.tte"] = offsetof(pmap, tte);
    kern_off_map["pmap_t.ttep"] = offsetof(pmap, ttep);
    kern_off_map["pmap_t.min"] = offsetof(pmap, min);
    kern_off_map["pmap_t.max"] = offsetof(pmap, max);
    kern_off_map["pmap_t.pmap_pt_attr"] = offsetof(pmap, pmap_pt_attr);

    kern_off_map["ipc_space.is_table"] = 0x20;

    kern_off_map["ipc_entry.size"] = 0x18;

    kern_off_map["ipc_entry.ie_object"] = 0;

    kern_off_map["ipc_port.ip_messages"] = 0x20;

    kern_off_map["ipc_mqueue.imq_messages"] = 0;

    kern_off_map["ipc_kmsg_queue.ikmq_base"] = 0;

    kern_off_map["ipc_kmsg.ikm_header"] = 0x18;
}

int kernel_xnu::dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out)
{
    int result = -1;
    struct mach_header_64* mach_tmp = 0;
    named_kmap_t* mh_block = 0;
    struct load_command* lc_iter = 0;
    struct segment_command_64* lc_seg_tmp = 0;
    struct section_64* lc_sec_tmp = 0;
    std::string curblock_name = "";
    int command_index = 0;
    int sec_index = 0;
    size_t kernel_address_equiv = 0;

    if (live_kernel == true)
    {
        check_kmap("mach_header", &mh_block);
        mach_tmp = (struct mach_header_64*)mh_block->kmap_stats.alloc_base;
    }
    else
    {
        mach_tmp = (struct mach_header_64*)binBegin;
    }

    lc_iter = (load_command*)&mach_tmp[1];

    for (; command_index < mach_tmp->ncmds; command_index++)
    {
        if (lc_iter->cmd == LC_SEGMENT_64)
        {
            lc_seg_tmp = (struct segment_command_64*)lc_iter;
            lc_sec_tmp = (struct section_64*)&lc_seg_tmp[1];

            curblock_name = lc_seg_tmp->segname;
            if (curblock_name == kmap_nanme)
            {
                if (live_kernel == true)
                {
                    kernel_address_equiv = lc_seg_tmp->vmaddr;
                }
                else
                {
                    kernel_address_equiv = lc_seg_tmp->fileoff + binBegin;
                }
                insert_section(curblock_name,
                    kernel_address_equiv, lc_seg_tmp->filesize);
                goto finish;
            }

            for(sec_index = 0; sec_index < lc_seg_tmp->nsects; lc_sec_tmp++, sec_index++)
            {
                curblock_name = std::string(lc_seg_tmp->segname) + "::" + std::string(lc_sec_tmp->sectname);
                if (curblock_name == kmap_nanme)
                {
                    if (live_kernel == true)
                    {
                        kernel_address_equiv = lc_sec_tmp->addr;
                    }
                    else
                    {
                        kernel_address_equiv = lc_sec_tmp->offset + binBegin;
                    }
                    insert_section(curblock_name, kernel_address_equiv, lc_sec_tmp->size);
                    goto finish;
                }
            }
        }
        lc_iter = (struct load_command*)((size_t)lc_iter + lc_iter->cmdsize);
    }

    goto fail;
finish:
    result = 0;
    if (block_out != 0)
    {
        check_kmap(kmap_nanme, block_out);
    }
fail:
    return result;;
}
