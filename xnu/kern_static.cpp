#include <string>
#include <mach-o/loader.h>

#include <localUtil_xnu.h>
#include <kernel_resolver.h>

#include "kern_img.h"
#include "kern_static.h"

int kern_static::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;

    FINISH_IF(kern_sym_fetch(newString, out_address) == 0);
    SAFE_BAIL(find_symbol((struct mach_header_64*)binBegin, newString, (void**)out_address) == -1);

finish:
    result = 0;
fail:
    return result;
}

int kern_static::insert_sections()
{
    struct mach_header_64* mh_temp = (struct mach_header_64*)binBegin;
    struct load_command* lc_iter = 0;
    struct segment_command_64* lc_seg_tmp = 0;
    struct section_64* lc_sec_tmp = 0;
    int command_index = 0;
    int sec_index = 0;

    lc_iter = (load_command*)&mh_temp[1];
    insert_section("mach_header", binBegin, mh_temp->sizeofcmds);

    for (; command_index < mh_temp->ncmds; command_index++)
    {
        if (lc_iter->cmd == LC_SEGMENT_64)
        {
            insert_section(std::string(lc_seg_tmp->segname),
                lc_seg_tmp->fileoff + binBegin, lc_seg_tmp->filesize);
            lc_seg_tmp = (struct segment_command_64*)lc_iter;
            lc_sec_tmp = (struct section_64*)&lc_seg_tmp[1];
            for(sec_index = 0; sec_index < lc_seg_tmp->nsects; sec_index++)
            {
                insert_section(std::string(lc_seg_tmp->segname) + "." + std::string(lc_sec_tmp->sectname),
                    lc_sec_tmp->offset + binBegin, lc_sec_tmp->size);
            }
        }
        lc_iter = (struct load_command*)((size_t)lc_iter + lc_iter->cmdsize);
    }
    return 0;
}

int kern_static::parseAndGetGlobals()
{
    struct mach_header_64* mach_header_tmp = (struct mach_header_64*)binBegin;
    struct thread_command* ut_com = 0;
    _STRUCT_ARM_THREAD_STATE64* tregs = 0;
    size_t targ_pc = 0;

    getloadcommandfrommach(mach_header_tmp, LC_UNIXTHREAD, (struct load_command**)&ut_com);
    tregs = (_STRUCT_ARM_THREAD_STATE64*)&ut_com[1];

    targ_pc = tregs->__pc;
    kern_sym_insert("_start", targ_pc);

    return 0;
}
