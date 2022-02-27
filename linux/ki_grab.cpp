#include <elf.h>
#include <localUtil.h>

#include "spare_vmlinux.h"
#include "kern_img.h"

int kern_img::grab_sinittext()
{
    int result = -1;
    hde_t tempInst = {0};
    uint32_t* binBegMap = 0;
    
    SAFE_BAIL(live_kern_addr(binBegin, sizeof(*binBegMap), (void**)&binBegMap) == -1);
    SAFE_BAIL(parseInst(*binBegMap, &tempInst) == -1);
    KSYM_V(_sinittext) = tempInst.immLarge + binBegin;

    result = 0;
fail:
    SAFE_LIVE_FREE(binBegMap);
    return result;
}

int kern_img::base_inits()
{
    int result = -1;
    instSet getB;
    uint32_t* text_start = 0;
    named_kmap_t* head_map = 0;
    uint32_t nonzeroLook = 0;
    binary_ss nonzero_ss((uint8_t*)&nonzeroLook, sizeof(nonzeroLook), 0x40, sizeof(nonzeroLook), false);

    FINISH_IF((check_kmap(".head.text", NULL) == 0) &&
        (check_kmap(".text", NULL) == 0) &&
        (check_kmap(".init.text", NULL) == 0));

    insert_section(".head.text", (size_t)binBegin, 0);
    
    // SO originally i searched for the first sub sp operation. HOWEVER it seems like on different
    // devices and kernels the first routine may not even start with a sub, but rather an stp.
    // if this is the case.... well gonna be harder to detect. so another option is either looking for
    // page, or first nonzero word after 0x40, gonna stick with the latter.

    // getB.addNewInst(cOperand::createASI<size_t, size_t, saveVar_t>(SP, SP, getB.checkOperand(0)));
    // SAFE_BAIL(kernel_search(&getB, binBegin, PAGE_SIZE * 4, &text_start) == -1);

    SAFE_BAIL(kernel_search(&nonzero_ss, binBegin, PAGE_SIZE * 2, (void**)&text_start) == -1);
    SAFE_BAIL(check_kmap(".head.text", &head_map) == -1);
    head_map->alloc_size = (size_t)text_start - (size_t)binBegin;
    insert_section(".text", (size_t)text_start, 0);

    insert_section(".init.text", KSYM_V(_sinittext), 0);

finish:
    result = 0;
fail:
    return result;
}

int kern_img::base_ksymtab()
{
    // here is asspull city.... gonna look for a hella regex. in execution, the routine
    // _request_firmware has a call to kmem_cache_alloc_trace(kmalloc_caches[0][7], 0x14080C0u, 0x20uLL);
    // where args 2 and 3 are the gfp flags and size. because i believe them to be measurable enough,
    // as well as arguments, lets give them a looksie....

    int result = -1;
    size_t ksymtabTmp = 0;
    named_kmap_t* crcSec = 0;

    // check if ksymtab already exists
    FINISH_IF(check_kmap("__ksymtab", NULL) == 0);

    // grab the base that i need
    SAFE_BAIL(check_kmap("__kcrctab", &crcSec) == -1);

    SAFE_BAIL(ksyms_count == 0);
    ksymtabTmp = (UNRESOLVE_REL(crcSec->kva) - sizeof(kernel_symbol) * ksyms_count);
    insert_section("__ksymtab", ksymtabTmp, 0);

    // instSet getB;
    // size_t start_kernelOff = 0;
    // uint32_t* modverAddr = 0;
    // size_t modverOff = 0;

    // getB.addNewInst(cOperand::createMWI<size_t, size_t>(1, 0x80c0));
    // getB.addNewInst(cOperand::createB<saveVar_t>(getB.checkOperand(0)));
    // SAFE_BAIL(getB.findPattern(__primary_switched_g, PAGE_SIZE, &start_kernel_g) == -1);

    // getB.getVar(0, &start_kernelOff);
    // start_kernel_g = (uint32_t*)(start_kernelOff + (size_t)start_kernel_g);

    // getB.clearInternals();
    // getB.addNewInst(cOperand::createADRP<saveVar_t, saveVar_t>(getB.checkOperand(0), getB.checkOperand(1)));
    // getB.addNewInst(cOperand::createADRP<saveVar_t, saveVar_t>(getB.checkOperand(2), getB.checkOperand(3)));
    // getB.addNewInst(cOperand::createASI<saveVar_t, saveVar_t, saveVar_t>(getB.checkOperand(0), getB.checkOperand(0), getB.checkOperand(4)));
    // getB.addNewInst(cOperand::createASI<saveVar_t, saveVar_t, saveVar_t>(getB.checkOperand(2), getB.checkOperand(2), getB.checkOperand(5)));
    // getB.addNewInst(cOperand::createLI<saveVar_t, size_t, size_t, size_t>(getB.checkOperand(6), X31,  0x39, 0x3));

finish:
    result = 0;
fail:
    return result;
}

