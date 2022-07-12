#ifndef PARSEANDKERN_H
#define PARSEANDKERN_H

#include <map>
#include <vector>
#include <string>
#include <elf.h>
#include <string.h>
#include <iostream>
#include <dlfcn.h>

#include <localUtil.h>
// #include <ibeSet.h>
#include <drv_share.h>
#include <localUtil_cpp.h>

// #include <jsmn/jsmn.h>
// #include <json.h>
// #include <jsonUtil.h>

#include "spare_vmlinux.h"
#include <kern_img.h>

template <typename Elf_Shdr>
bool cmp_Shdr(std::pair<std::string, Elf_Shdr*>& a,
         std::pair<std::string, Elf_Shdr*>& b);

template <typename Elf_Phdr>
bool cmp_Phdr(std::pair<std::string, Elf_Phdr*>& a,
         std::pair<std::string, Elf_Phdr*>& b);

template <typename size_b, typename Elf_Ehdr, typename Elf_Shdr,
          typename Elf_Phdr, typename Elf_Xword, typename Elf_Word>
class kern_static : public kernel_linux
{
public:
    // couple of helpers for finding stuff
    Elf_Phdr *find_prog(std::string lookupKey);
    Elf_Shdr *find_sect(std::string lookupKey);
    int check_sect(std::string sect_name, Elf_Shdr **sect_out);

    // have to extend these, they are for every kernel. the ksym
    // dlsym is for getting the kernel symbol location, really
    // extending that is just in case a symbol can't or shouldn't
    // be cached, for instance if a symbol table has been found.
    int kdlsym(const char *newString, size_t *out_address);

    // get the index of a kstr in the ksymstr table.
    int findKindInKstr(const char *newString, int *index);

    // int gen_vmlinux_sz(size_t* outSz, size_t headOffset);
    int gen_shstrtab(std::string **out_shstrtab, uint16_t *numSects, uint16_t *shstrtab_index);

    int gen_vmlinux_sz(size_t *outSz, size_t headOffset)
    {
        size_t szTemp = 0;
        std::string *shstrtab_tmp;

        // we are gonna consider that the header size SHOULD just be a page, however
        // under the condition that maybe we got a dummy thicc header, we allow it
        // to be passed in.
        szTemp += headOffset;

        // we are using the whole parsed kernel image as input, so add that.
        szTemp += kern_sz;

        // generate shstrtab
        gen_shstrtab(&shstrtab_tmp, NULL, NULL);

        // adding the shstrtab to the image whole
        szTemp += shstrtab_tmp->size();

        // adding the section list to the array, plus 1 for the null section
        szTemp += ((sect_list.size() + 1) * sizeof(Elf_Shdr));

        *outSz = szTemp;
        return 0;
    }

    void elfConstruction()
    {
        int result = -1;
        Elf64_Ehdr *vmlinuxBase = 0;
        char *kernimgBase = 0;
        size_t vmlinux_sz;
        std::string vmlinux_dir_made;

        kernel_symbol *ksymBase = 0;
        kern_static *parsedKernimg = 0;
        Elf_Phdr *phdrBase = 0;

        std::string *shstrtab_tmp;
        void *vmlinux_iter = 0;

        // alloc outfile
        parsedKernimg->gen_vmlinux_sz(&vmlinux_sz, PAGE_SIZE);

        if ((vmlinux_sz % PAGE_MASK4K) != 0)
        {
            vmlinux_sz = (vmlinux_sz + PAGE_SIZE4K) & ~PAGE_MASK4K;
        }
        result = posix_memalign((void **)&vmlinuxBase, PAGE_SIZE4K, vmlinux_sz);
        SAFE_BAIL(vmlinuxBase == 0);

        // write elf header to the new vmlinux
        elfHeadConstruction(vmlinuxBase);
        vmlinux_iter = (void *)((size_t)vmlinuxBase + sizeof(Elf_Ehdr));

        // write the new program header to the new vmlinux
        phdrBase = (Elf_Phdr *)vmlinux_iter;
        // insert_phdr(PT_LOAD, PF_X | PF_W | PF_R, PAGE_SIZE4K, ANDROID_KERNBASE, ANDROID_KERNBASE,
        //     parsedKernimg->get_kernimg_sz(), parsedKernimg->get_kernimg_sz(), 0x10000);
        // patch_and_write_phdr((Elf64_Phdr*)vmlinux_iter, &g_phArray);
        vmlinux_iter = (void *)((size_t)vmlinuxBase + PAGE_SIZE);

        // write the kernel image itself to the new vmlinux
        kernimgBase = (char *)vmlinux_iter;
        memcpy(vmlinux_iter, (void *)parsedKernimg->get_binbegin(), parsedKernimg->get_kernimg_sz());
        vmlinux_iter = (void *)((size_t)vmlinux_iter + parsedKernimg->get_kernimg_sz());

        // write the new shstrtab to vmlinux
        parsedKernimg->gen_shstrtab(&shstrtab_tmp, &vmlinuxBase->e_shnum, &vmlinuxBase->e_shstrndx);
        memcpy(vmlinux_iter, shstrtab_tmp->data(), shstrtab_tmp->size());
        vmlinux_iter = (void *)((size_t)vmlinux_iter + shstrtab_tmp->size());

        // patch the section header and write it to the binary, adjusting for
        // the program header and the elf header
        vmlinuxBase->e_shoff = ((size_t)vmlinux_iter - (size_t)vmlinuxBase);
        parsedKernimg->patch_and_write(vmlinuxBase, (Elf_Shdr *)vmlinux_iter, phdrBase, (size_t)kernimgBase - (size_t)vmlinuxBase);
    }

    void elfHeadConstruction(Elf_Ehdr *elfHead)
    {
        memset(elfHead, 0, sizeof(Elf_Ehdr));
        memcpy(&elfHead->e_ident[EI_MAG0], ELFMAG, sizeof(uint32_t));
        if (sizeof(size_b) == 4)
        {
            elfHead->e_ident[EI_CLASS] = ELFCLASS32;
            elfHead->e_machine = EM_ARM;
        }
        else
        {
            elfHead->e_ident[EI_CLASS] = ELFCLASS64;
            elfHead->e_machine = EM_AARCH64;
            elfHead->e_entry = ANDROID_KERNBASE;
        }
        elfHead->e_ident[EI_DATA] = ELFDATA2LSB;
        elfHead->e_ident[EI_VERSION] = EV_CURRENT;

        elfHead->e_type = ET_DYN;
        elfHead->e_version = EV_CURRENT;
        elfHead->e_phoff = sizeof(Elf_Ehdr);
        // elfHead->e_shoff
        elfHead->e_flags = 0x602;
        elfHead->e_ehsize = sizeof(Elf_Ehdr);
        elfHead->e_phentsize = sizeof(Elf_Phdr);
        // elfHead->e_phnum
        elfHead->e_shentsize = sizeof(Elf_Shdr);
        // elfHead->e_shnum
        // elfHead->e_shstrndx
    }

    // patch the out binary, section table base at vmlinux_cur and ph base at phBase
    // int patch_and_write(Elf_Ehdr* vmlinux_base, Elf_Shdr* vmlinux_cur, Elf_Phdr* phBase, size_t offset);

    int patch_and_write(Elf_Ehdr *vmlinux_base, Elf_Shdr *vmlinux_cur, Elf_Phdr *phBase, size_t offset)
    {
        int result = -1;
        Elf_Shdr nullSec = {0};
        Elf_Phdr *phdrTemp = 0;
        int phdrCount = 0;
        memcpy(vmlinux_cur, &nullSec, sizeof(Elf_Shdr));
        vmlinux_cur++;
        auto j = sect_list.begin();
        j++;

        for (auto i = sect_list.begin(); i != sect_list.end(); i++)
        {
            // fix the offset and size of the target segment
            i->second->sh_offset += offset;
            if ((i->second->sh_size == 0) && (j != sect_list.end()))
            {
                i->second->sh_size = (j->second->sh_offset + offset) - i->second->sh_offset;
            }
            // push the changes and the segment header
            memcpy(vmlinux_cur, i->second, sizeof(Elf_Shdr));

            // fix the program header if it has a match
            phdrTemp = find_prog(i->first);
            if (phdrTemp != 0)
            {
                phdrTemp->p_offset = i->second->sh_offset;
                phdrTemp->p_filesz = phdrTemp->p_memsz = i->second->sh_size;
                memcpy(phBase, phdrTemp, sizeof(Elf_Phdr));
                phdrCount++;
                phBase++;
            }

            // increment the section header iterator that will be used for copying
            vmlinux_cur++;
            // increment the next iterator that will be used for the size calc
            if (j != sect_list.end())
            {
                j++;
            }
        }

        vmlinux_base->e_phnum = phdrCount;

        result = 0;
        return result;
    }

    int parseAndGetGlobals()
    {
        int result = -1;

        vector_pair_sort<std::string, Elf_Shdr*>(&sect_list, cmp_Shdr<Elf_Shdr>);
        vector_pair_sort<std::string, Elf_Phdr*>(&prog_list, cmp_Phdr<Elf_Phdr>);

        result = 0;
    fail:
        return result;
    }

    // insertion function to be used, should be the only interface for adding new values
    // void insert_section(std::string sec_name, uint16_t sh_type, uint64_t sh_flags,
    //     uint64_t sh_addr, uint64_t sh_offset, uint64_t sh_size, uint16_t sh_link,
    //     uint16_t sh_info, uint64_t sh_addralign, uint64_t sh_entsize);
    // void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size);
    void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
    {
        Elf_Shdr *newShdr = 0;
        Elf_Phdr *newPhdr = 0;
        Elf_Word p_type = 0;
        Elf_Xword p_align = 0;
        Elf_Word p_flags = 0;

        uint16_t sh_type = SHT_PROGBITS;
        size_b sh_flags = 0;
        size_b sh_addr = sh_offset;
        // uint64_t sh_offset = 0;
        // uint64_t sh_size = 0;
        uint16_t sh_link = 0;
        uint16_t sh_info = 0;
        size_b sh_addralign = 0;
        size_b sh_entsize = 0;

        if (
            (sec_name != ".symtab") &&
            (sec_name != ".strtab") &&
            (sec_name != ".shstrtab"))
        {
            if (live_kernel == false)
            {
                sh_addr = R_KA(RESOLVE_REL(sh_addr));
            }
            sh_offset = RESOLVE_REL(sh_offset);
            p_type = PT_LOAD;
            p_align = 0x10000;

            sh_flags = SHF_ALLOC;
            p_flags = PF_R;
            // found a text, add executable flag
        }

        newShdr = new Elf_Shdr{
            0,
            sh_type,
            sh_flags,
            sh_addr,
            sh_offset,
            sh_size,
            sh_link,
            sh_info,
            sh_addralign,
            sh_entsize};

        sect_list.push_back({sec_name, newShdr});

        if (p_type == PT_LOAD)
        {
            newPhdr = new Elf_Phdr{
                p_type,
                p_flags,
                sh_offset,
                sh_addr,
                sh_addr,
                sh_size,
                sh_size,
                p_align};

            prog_list.push_back({sec_name, newPhdr});
        }
    }

    int kcrc_index(std::string symbol, uint32_t* kcrc)
    {
        int result = -1;
        Elf_Shdr *kcrctab_sec = 0;
        uint32_t *kcrctab_base = 0;
        SAFE_BAIL(check_sect("__kcrctab", &kcrctab_sec) == -1);

        kcrctab_base = (uint32_t *)UNRESOLVE_REL(kcrctab_sec->sh_offset);

        for (int i = 0; i < kstrtab_sorted.size(); i++)
        {
            if (symbol == kstrtab_sorted[i])
            {
                *kcrc = kcrctab_base[i];
                goto finish;
            }
        }
        for (int i = 0; i < kstrtab_gpl_sorted.size(); i++)
        {
            if (symbol == kstrtab_gpl_sorted[i])
            {
                *kcrc = kcrctab_base[kstrtab_sorted.size() + i];
                goto finish;
            }
        }
        goto fail;

    finish:
        result = 0;
    fail:
        return result;
    }

private:
    // private constructors for internal use only
    using kernel_linux::kernel_linux;
    int populate_kcrc_map();

    // finding sections in the binary
    int base_new_shstrtab();

    std::vector<std::pair<std::string, Elf_Shdr *>> sect_list;
    std::vector<const char *> kstrtab_sorted;
    std::vector<const char *> kstrtab_gpl_sorted;

    std::vector<std::pair<std::string, Elf_Phdr *>> prog_list;
};

kernel_linux* allocate_static_kernel(const char* kern_filename, uint32_t bitness);

#endif