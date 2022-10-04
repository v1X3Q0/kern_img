#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include <ibeSet.h>
#include <localUtil.h>
#include <localUtil_linux.h>

// #include <krw_util.h>

#include "kern_dynamic.h"

#include "spare_vmlinux.h"

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
    map_kernel_block(sec_name, sh_offset, sh_size, NULL);
}

int kern_dynamic::parseAndGetGlobals()
{
    int result = -1;

    gen_kallsymmap(&kern_sym_map);

    // get any symbols that we can find with heuristics
    finddyn(this);
    
    result = 0;
fail:
    return result;

}

int kern_dynamic::ksym_dlsym(const char* newString, uint64_t* out_address)
{
    int result = -1;
    size_t symtmp = 0;
    named_kmap_t* mh_base = 0;
    std::map<std::string, uint64_t>::iterator findres;

    // if we have found it dynamically it will be here
    FINISH_IF(kern_sym_fetch(newString, &symtmp) == 0);

    FINISH_IF(ksym_dlsym_kcrc(this, newString, &symtmp) == 0);

    // if we have not found it dynamically, then check kallsyms cache
    // findres = kallsym_cache.find(newString);
    // SAFE_BAIL(findres == kallsym_cache.end());
    // symtmp = findres->second;

    goto fail;
finish:
    result = 0;
    if (out_address != 0)
    {
        *out_address = symtmp;
    }
fail:
    return result;
}
