#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include <ibeSet.h>
#include <localUtil.h>

// #include <krw_util.h>

#include "kern_dynamic.h"

#include "spare_vmlinux.h"

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}

// routine to be used for dynamic use, the relocation table will fill these up,
// maybe someday i can see how they are filled in static use as well.
int kern_dynamic::base_ksymtab_kcrctab_ksymtabstrings()
{
#define TARGET_KSYMTAB_SEARCH_STR followStr
    int result = -1;
    named_kmap_t* head_text_shdr = 0;
    named_kmap_t* init_text_shdr = 0;
    named_kmap_t* text_shdr = 0;
    size_t kBuffer = 0;
    size_t searchSz = 0;
    char searchStr[] = "module.sig_enforce";
    char followStr[] = "nomodule";
    symsearch* tmpSymSearch = 0;
    uint16_t poststr_block = 0;
    binary_ss module_ss((uint8_t*)followStr, sizeof(followStr), 0, 1, true);
    binary_ss nullterm_ss((uint8_t*)&poststr_block, sizeof(poststr_block), 0, 1, true);

    size_t ksymtab_base = 0;
    size_t ksymtab_gpl_base = 0;
    size_t kcrctab_base = 0;
    size_t kcrctab_gpl_base = 0;
    size_t ksymtab_strings_base = 0;

    // check if all 3 already exists
    FINISH_IF((check_kmap("__ksymtab", NULL) == 0) &&
        (check_kmap("__kcrctab", NULL) == 0) &&
        (check_kmap("__ksymtab_strings", NULL) == 0)
        );

    SAFE_BAIL(check_kmap(".head.text", &head_text_shdr) == -1);
    SAFE_BAIL(check_kmap(".init.text", &init_text_shdr) == -1);
    SAFE_BAIL(check_kmap(".text", &text_shdr) == -1);
    
    // if not, begin the search! brute force for our string, with an upper bound
    // limit of the .init.text section. Once we get there we have to stop
    // reading or kernel panic.

    // skip a section by starting at the .text, though if we can't guarantee
    // alignment.... may have to do .head.text, which should only be an
    // additional page or so.
    searchSz = init_text_shdr->kva - head_text_shdr->kva;
    SAFE_BAIL(kernel_search(&module_ss, head_text_shdr->kva, searchSz, true, (void**)&kBuffer) == -1);

    kBuffer = kBuffer + sizeof(TARGET_KSYMTAB_SEARCH_STR);
    BIT_PAD(kBuffer, size_t, 8);

    SAFE_BAIL(live_kern_addr(kBuffer, sizeof(symsearch) * 3, (void**)&tmpSymSearch) == -1);

    // index 0 and 1 are each the ksymtab and ksymtab_gpl respectively
    // index 2 bases the kcrc, but its end is the same as its entry. The end of it is the 
    // crcgpl, which is referenced by the gpl ksymtab
    ksymtab_base = (size_t)tmpSymSearch[0].start;
    ksymtab_gpl_base = (size_t)tmpSymSearch[1].start;
    kcrctab_base = (size_t)tmpSymSearch[2].start;
    kcrctab_gpl_base = (size_t)tmpSymSearch[1].crcs;
    ksymtab_strings_base = (size_t)tmpSymSearch[2].crcs;

    insert_section("__ksymtab", ksymtab_base, ksymtab_gpl_base - ksymtab_base);
    insert_section("__ksymtab_gpl", ksymtab_gpl_base, kcrctab_base - ksymtab_gpl_base);
    insert_section("__kcrctab", kcrctab_base, kcrctab_gpl_base - kcrctab_base);
    insert_section("__kcrctab_gpl", kcrctab_gpl_base, ksymtab_strings_base - kcrctab_gpl_base);

    SAFE_BAIL(kernel_search(&nullterm_ss, ksymtab_strings_base, init_text_shdr->kva - ksymtab_strings_base, true, (void**)&kBuffer) == -1);
    insert_section("__ksymtab_strings", ksymtab_strings_base, kBuffer - ksymtab_strings_base + 1);

    ksyms_count = (kcrctab_base - ksymtab_base) / sizeof(kernel_symbol);
finish:
    result = 0;
fail:
    SAFE_LIVE_FREE(tmpSymSearch)
    return result;
}

int kern_dynamic::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    const char* kstrBase = 0;
    const char* kstrIter = 0;
    named_kmap_t* ksymSec = 0;
    named_kmap_t* ksymgplSec = 0;
    named_kmap_t* ksymstrSec = 0;
    kernel_symbol* ksymIter = 0;

    // get dependencies, we need the ksymtab_str for comparison and the ksymtab has the
    // out value for us.
    SAFE_BAIL(check_kmap("__ksymtab", &ksymSec) == -1);
    SAFE_BAIL(check_kmap("__ksymtab_gpl", &ksymgplSec) == -1);
    SAFE_BAIL(check_kmap("__ksymtab_strings", &ksymstrSec) == -1);

    ksymIter = (kernel_symbol*)ksymSec->kmap_stats.alloc_base;
    kstrBase = (const char*)ksymstrSec->kmap_stats.alloc_base;
    // SAFE_BAIL(live_kern_addr(UNRESOLVE_REL(ksymSec->sh_offset), ksymSec->sh_size + ksymgplSec->sh_size, (void**)&ksymIter) == -1);
    // SAFE_BAIL(live_kern_addr(UNRESOLVE_REL(ksymstrSec->sh_offset), ksymstrSec->sh_size, (void**)&kstrBase) == -1);
    // strIter = (const char*)UNRESOLVE_REL(ksymstrSec->sh_offset);

    for (int i = 0; i < ksyms_count; i++)
    {
        // resolve against the base of the ksymtab so we can add to the kstrbase
        kstrIter = (const char*)((ksymIter[i].name - (size_t)ksymstrSec->kva) + (size_t)kstrBase);
        if (strcmp(newString, kstrIter) == 0)
        {
            if (out_address != 0)
            {
                *out_address = ksymIter[i].value;
            }
            goto found;
        }
    }
    goto fail;

found:
    result = 0;
fail:
    SAFE_LIVE_FREE(ksymIter);
    SAFE_LIVE_FREE(kstrBase);
    return result;
}

int kern_dynamic::parseAndGetGlobals()
{
    int result = -1;

    SAFE_BAIL(base_ksymtab_kcrctab_ksymtabstrings() == -1);
    SAFE_BAIL(grab_task_struct_offs() == -1);

    result = 0;
fail:
    return result;

}

int kern_dynamic::grab_task_struct_offs()
{
    int result = -1;
    size_t init_task = 0;
    void* init_task_mapped = 0;
    size_t* memberIter = 0;
    size_t pushable_tasks = 0;
    size_t tasks = 0;

    SAFE_BAIL(ksym_dlsym("init_task", &init_task) == -1);
    SAFE_BAIL(live_kern_addr(init_task, PAGE_SIZE, &init_task_mapped) == -1);

    memberIter = (size_t*)init_task_mapped;
    for (int i = 0; i < PAGE_SIZE; i += 8)
    {
        int curIter = i / sizeof(size_t);
        
        if (
            (memberIter[curIter] == memberIter[curIter + 1]) &&
            (memberIter[curIter + 2] == memberIter[curIter + 3]) &&
            (memberIter[curIter] != 0) &&
            (memberIter[curIter + 2] != 0)
            )
        {
            // we are at task->pushable_tasks.prio_list, so the base of a
            // plist_node is at current - 8, the size of prio, plist_node's
            // first member. then subtract the size of another list to get
            // the offset for the tasks structure.
            pushable_tasks = i - sizeof(size_t) * 1;
            tasks = i - sizeof(size_t) * 3;
            goto found;
        }
    }
    goto fail;

found:
    kern_off_map["task_struct.tasks"] = tasks;
    kern_off_map["task_struct.pushable_tasks"] = pushable_tasks;

    result = 0;
fail:
    SAFE_LIVE_FREE(init_task_mapped);
    return result;
}
