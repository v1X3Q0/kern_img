#ifndef KERNEL_BLOCK_H
#define KERNEL_BLOCK_H

#include <map>
#include <string>
#include <vector>
#include <bgrep_e.h>
#include <localUtil.h>
// #include <ibeSet.h>

#define SAFE_LIVEKERN(x) \
    if (live_kernel == true) \
    { \
        x; \
    }

#define KSYM_V(GLOBAL) \
    kern_sym_map[ # GLOBAL ]

#define KSYM_VS(GLOBAL) \
    kern_sym_fetch( # GLOBAL )

#define SAFE_LIVE_FREE(x) \
    if (live_kernel == true) \
    { \
        SAFE_FREE(x); \
    }

#define RESOLVE_REL(x) \
    ((size_t)x - (size_t)binBegin)

#define R_KA(x) \
    ((size_t)x + (size_t)ANDROID_KERNBASE)

#define UNRESOLVE_REL(x) \
    ((size_t)x + (size_t)binBegin)

struct named_kmap;
struct real_kmap;

typedef struct real_kmap
{
    size_t kva;
    uint8_t* alloc_base;
    size_t alloc_size;
    std::vector<struct named_kmap*> child_list;
    size_t ref_counter;
} real_kmap_t;

typedef struct
{
    std::string alloc_name;
    uint8_t* alloc_base;
    size_t alloc_size;
} kmap_block_t;

typedef struct named_kmap 
{
    kmap_block_t kmap_stats;
    size_t kva;
    real_kmap_t* owner;
} named_kmap_t;

class kernel_block
{
public:
    size_t resolveRel(size_t rebase);
    // under the condition that we have a live kernel, translation routine.

    int get_bitness() { return bitness; };
    bool is_live_kernel() { return live_kernel; };
    int kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, bool volatile_region, void** out_img_off);
    int kernel_search(search_set* getB, size_t img_var, size_t img_var_sz, void** out_img_off);

    int kernel_search_seq(size_t img_var, size_t img_var_sz, uint8_t* byte_search, size_t search_sz,
        size_t offset, size_t step, bool match, void** out_img_off);
    size_t get_kernimg_sz() { return kern_sz; };
    size_t get_binbegin() { return binBegin; };
    int check_kmap(std::string kmap_name, named_kmap_t** named_kmap_out);
    int check_kmap_force(std::string kmap_name, named_kmap_t** named_kmap_out);

    int kstruct_offset(std::string kstruct_name, uint64_t* kstruct_off_out);
    int kern_sym_fetch(std::string kstruct_name, uint64_t* kstruct_off_out);

    int kern_sym_insert(std::string ksym_name, uint64_t symaddr);
    int kern_off_insert(std::string koff_name, uint64_t offval);

    int live_kern_addr(size_t target_kernel_address, size_t size_kernel_buf, void** out_live_addr);

    // to be used for actual construction
    template <typename kern_dist>
    static kern_dist* allocate_kern_img(const char* kern_file, int bitness_a=(sizeof(void*) * 8))
    {
        kern_dist* result = 0;
        void* binBegin_l = 0;
        size_t kernSz_l = 0;

        SAFE_BAIL(block_grab(kern_file, &binBegin_l, &kernSz_l) == -1);

        result = new kern_dist((uint32_t*)binBegin_l, kernSz_l, bitness_a);
        SAFE_BAIL(result->parseAndGetGlobals() == -1);

        goto finish;
    fail:
        SAFE_DEL(result);
    finish:
        return result;
    };

    template <typename kern_dist>
    static kern_dist* grab_live_kernel(void* kern_base)
    {
        kern_dist* result = 0;

        result = new kern_dist((uint32_t*)kern_base);
        SAFE_BAIL(result->parseAndGetGlobals() == -1);

        goto finish;
    fail:
        SAFE_DEL(result);
    finish:
        return result;
    }

    template <typename kern_dist>
    static kern_dist* grab_live_kernel(void* kern_base, kernel_block* kern_pass)
    {
        kern_dist* result = 0;

        result = new kern_dist((uint32_t*)kern_base, kern_pass);
        SAFE_BAIL(result->parseAndGetGlobals() == -1);

        goto finish;
    fail:
        SAFE_DEL(result);
    finish:
        return result;
    }

    virtual int ksym_dlsym(const char* newString, uint64_t* out_address) = 0;
    virtual int parseAndGetGlobals() = 0;
    virtual void insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size) = 0;

    // not live kernel constructors, we have the size and such cause we allocated it
    kernel_block(uint32_t* binBegin_a, size_t kern_sz_a) : binBegin((size_t)binBegin_a), kern_sz(kern_sz_a), live_kernel(false) {};
    kernel_block(uint32_t* binBegin_a, size_t kern_sz_a, int bitness_a) : kernel_block(binBegin_a, kern_sz_a) { bitness = bitness_a; };
protected:
    size_t binBegin;
    const bool live_kernel;
    size_t kern_sz;
    int bitness;
#ifdef __METALKIT__
public:
#endif
    // private constructors for internal use only, live we just give it begin
    kernel_block(uint32_t* binBegin_a) : binBegin((size_t)binBegin_a), live_kernel(true) {};
#ifdef __METALKIT__
protected:
#endif
    // kernel_block(uint32_t* binBegin_a, size_t kern_sz_a) : binBegin((size_t)binBegin_a), kern_sz((size_t)kern_sz_a), live_kernel(false) {};
    kernel_block(const char* kern_file) : live_kernel(false), bitness(sizeof(void*) * 8) {};

    // this vector is for validating that a request is for a block that is already mapped
    std::vector<real_kmap_t*> kmap_list;
    // meanwhile, this is for tracking allocations, so that a request can be done quickly
    std::map<std::string, named_kmap_t*> named_alloc_list;

    // set the offsets for generic kernel pointer types that are shared among all kernels
    void set_known_offsets();
    virtual void target_set_known_offsets() = 0;
    virtual int dyn_kmap_find(std::string kmap_nanme, named_kmap_t** block_out) = 0;


    // map kernel block, result map in kmap_ret
    int map_kernel_block(std::string block_name, size_t kva, size_t kb_size, named_kmap_t** kmap_ret);
    int consolidate_kmap_allocation(size_t kva, size_t kb_size, real_kmap_t** kmap_ret);
    int map_save_virt(size_t kva, size_t kb_size, void** virt_ret);

    int volatile_map(size_t kva, size_t kv_size, void** virt_ret, bool volatile_op);
    int volatile_free(size_t kva, void* virt_used, bool volatile_op);

    // virtual int grab_kernel_offsets() = 0;

    std::map<std::string, uint64_t> kern_sym_map;
    std::map<std::string, uint64_t> kern_off_map;
#ifndef NDEBUG
public:
    std::map<std::string, uint64_t>* get_kernoff_map()
    {
        return &kern_off_map;
    }
#endif
};

#endif