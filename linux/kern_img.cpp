#include <elf.h>
#include <string.h>
#include <localUtil.h>

#include "kern_img.h"

int kern_img::findKindInKstr(const char* newString, int* index)
{
    const char* strIter = 0;
    named_kmap_t* ksymstrSec = 0;
    int result = -1;
    int i = 0;

    SAFE_BAIL(check_kmap("__ksymtab_strings", &ksymstrSec) == -1);
    SAFE_BAIL(live_kern_addr(UNRESOLVE_REL(ksymstrSec->kva), ksymstrSec->alloc_size, (void**)&strIter) == -1);
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
