#include <elf.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include <localUtil.h>
#include "spare_vmlinux.h"
#include "kern_img.h"

int kernel_linux::findKindInKstr(const char* newString, int* index)
{
    const char* strIter = 0;
    named_kmap_t* ksymstrSec = 0;
    int result = -1;
    int i = 0;

    SAFE_BAIL(check_kmap("__ksymtab_strings", &ksymstrSec) == -1);
    SAFE_BAIL(live_kern_addr(UNRESOLVE_REL(ksymstrSec->kva), ksymstrSec->kmap_stats.alloc_size, (void**)&strIter) == -1);
    // strIter = (const char*)UNRESOLVE_REL(ksymstrSec->sh_offset);

    for (; i < ksyms_count; i++)
    {
        if (strcmp(newString, strIter) == 0)
        {
            goto finish_eval;
        }
        if (*(uint16_t*)(strIter + strlen(strIter)) == 0)
        {
            break;
        }
        strIter = strIter + strlen(strIter) + 1;
    }

    goto fail;
finish_eval:
    *index = i;
finish:
    result = 0;
fail:
    SAFE_LIVE_FREE(strIter);
    return result;
}

int kernel_linux::ksym_dlsym(const char* newString, size_t* out_address)
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

