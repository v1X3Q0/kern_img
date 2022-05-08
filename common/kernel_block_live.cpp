#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "kernel_block.h"
#include <localUtil.h>

#include <krw_util.h>

void kernel_block::set_known_offsets()
{
    kern_off_map["list_entry.next"] = 0;
    kern_off_map["list_entry.prev"] = sizeof(size_t);

    kern_off_map["list_entry<T>.next"] = 0;
    kern_off_map["list_entry<T>.prev"] = sizeof(size_t);

}

int kernel_block::live_kern_addr(size_t target_kernel_address, size_t size_kernel_buf, void** out_live_addr)
{
    int result = -1;
    void* newKernelAddress = 0;

    newKernelAddress = calloc(size_kernel_buf, 1);
    SAFE_BAIL(newKernelAddress == 0);
    SAFE_BAIL(kRead(newKernelAddress, size_kernel_buf, (size_t)target_kernel_address) == -1);

    if (out_live_addr != 0)
    {
        *out_live_addr = newKernelAddress;
    }

    result = 0;
    goto finish;
fail:
    SAFE_FREE(newKernelAddress);
finish:
    return result;
}

int kernel_block::consolidate_kmap_allocation(size_t kva, size_t kb_size, real_kmap_t** kmap_ret)
{
    int result = -1;
    std::vector<real_kmap_t*> collision_kmaps;
    real_kmap_t* real_alloc_save = 0;
    size_t new_kbase = 0;
    char* new_alloc = 0;
    size_t new_alloc_sz = 0;
    char* new_alloc_tmp = 0;
    size_t bytes_read = 0;

    // under the case that we have a new block that overlaps multiple blocks, we consolidate on each
    // iteration.
    for (auto i = kmap_list.begin(); i != kmap_list.end(); i++)
    {
        size_t cur_kva = (*i)->kva;
        size_t cur_ksz = (*i)->alloc_size;

        // check if our new "section" overlaps with any other sections. if it
        // does, we need to mark the allocation as a collision, and then copy
        // it over later to the newly allocated block.
        if (OVERLAP_REGIONS(kva, kb_size, cur_kva, cur_ksz))
        {
            if (CASE_OVERLAP_R1_IN_R2(kva, kb_size, cur_kva, cur_ksz))
            {
                // region 1 is encompassed, insert our new named block
                if ((kva + kb_size) <= (cur_kva + cur_ksz))
                {
                    real_alloc_save = *i;
                    goto finish;
                }
                // there is an overlap, realloc, copy and break;
                else
                {
                    // difference between where the new end will be and the old end was, added to orig size
                    // new_alloc_sz = ((kva + kb_size) - (cur_kva + cur_ksz)) + cur_ksz;
                    // new_kbase = cur_kva;
                    collision_kmaps.push_back(*i);
                }
            }
            // if it ends in the old block, then the difference is just between the bases, and added to
            // the orig size
            else if (CASE_OVERLAP_R1_ENDS_R2(kva, kb_size, cur_kva, cur_ksz))
            {
                // new_alloc_sz = (cur_kva - kva) + cur_ksz;
                // new_kbase = kva;
                collision_kmaps.push_back(*i);
            }
            // if it encompasses r2, then its size is just set to new sizes
            else if (CASE_OVERLAP_R1_EATS_R2(kva, kb_size, cur_kva, cur_ksz))
            {
                // new_alloc_sz = kb_size;
                // new_kbase = kva;
                collision_kmaps.push_back(*i);
            }
        }
    }

    real_alloc_save = (real_kmap_t*)calloc(1, sizeof(real_kmap_t));

    // should it not have overlapped, then we set the new kva and size to prepare for a read.
    if (collision_kmaps.size() == 0)
    {
        new_kbase = kva;
        new_alloc_sz = kb_size;
    }
    else
    {
        new_kbase = std::min<size_t>(kva, collision_kmaps.front()->kva);
        new_alloc_sz = std::max(kva + kb_size, collision_kmaps.back()->kva + collision_kmaps.back()->alloc_size) -
            std::min<size_t>(kva, collision_kmaps.front()->kva);
    }
    new_alloc = (char*)calloc(new_alloc_sz, 1);

    real_alloc_save->kva = new_kbase;
    real_alloc_save->alloc_size = new_alloc_sz;
    real_alloc_save->alloc_base = (uint8_t*)new_alloc;
    // real_alloc_save->ref_counter++;

    // if no collisions, then we have a new region, so proceed to allocate it and
    // map the kernel memory.
    if (collision_kmaps.size() == 0)
    {
        SAFE_BAIL(kRead(new_alloc, new_alloc_sz, new_kbase) == -1);
        goto finish;
    }
    
    // calculate the new real allocation's size, sinze we have a sorted vector
    // of elements that will be in the new real allocation, we can just take
    // those and assume that the front's beginning is the beginning, and the
    // back's end is the end, and use that in a comparator with the new kva
    if (kva < collision_kmaps.front()->kva)
    {
        SAFE_BAIL(kRead((void*)new_alloc_tmp, collision_kmaps.front()->kva - kva, kva) == -1);
        new_alloc_tmp += collision_kmaps.front()->kva - kva;
        bytes_read += collision_kmaps.front()->kva - kva;
    }

    for (auto i = collision_kmaps.begin(); i != collision_kmaps.end(); i++)
    {
        real_kmap_t* current_kmap = *i;
        size_t tmp_targ = 0;
        size_t tmp_targ_sz = 0;
        
        // first memcpy the block, change the adjust the pointer for the
        // real_kmap, and then for all of the child named_kmap's
        memcpy(new_alloc_tmp, current_kmap->alloc_base, current_kmap->alloc_size);
        for (auto j = current_kmap->child_list.begin(); j != current_kmap->child_list.end(); j++)
        {
            named_kmap_t* current_named = *j;
            current_named->kmap_stats.alloc_base = (uint8_t*)(
            // first the offset into the old allocation
                (current_named->kva - current_kmap->kva) +
            // add that to the base difference to the new allocation
                (current_kmap->kva - new_kbase) +
            // and then add to the new base
                new_alloc);
            current_named->owner = real_alloc_save;
            real_alloc_save->child_list.push_back(current_named);
        }
        new_alloc_tmp += current_kmap->alloc_size;
        bytes_read += current_kmap->alloc_size;
        
        // check if the next block doesn't border the current block. if so,
        // kread for the difference
        if ((i + 1) != collision_kmaps.end())
        {
            real_kmap_t* next_kmap = *(i + 1);

            if ((current_kmap->kva + current_kmap->alloc_size) < next_kmap->kva)
            {
                tmp_targ_sz = next_kmap->kva - (current_kmap->kva + current_kmap->alloc_size);
                tmp_targ = current_kmap->kva + current_kmap->alloc_size;
            }
        }
        // you have copied the last region, now all that's left is to see if the
        // new map exceeds the bounds of what we have
        else if ((i + 1) == collision_kmaps.end())
        {
            if ((kva + kb_size) > (collision_kmaps.back()->kva + collision_kmaps.back()->alloc_size))
            {
                tmp_targ_sz = (kva + kb_size) - (collision_kmaps.back()->kva + collision_kmaps.back()->alloc_size);
                tmp_targ = collision_kmaps.back()->kva + collision_kmaps.back()->alloc_size;
            }
        }
        if (tmp_targ != 0)
        {
            SAFE_BAIL(kRead(new_alloc_tmp,
                tmp_targ_sz,
                tmp_targ) == -1);
            new_alloc_tmp += tmp_targ_sz;
            bytes_read += tmp_targ_sz;
        }
    }

    // now that you've finished using the collision map, empty it
    for (auto i = collision_kmaps.begin(); i != collision_kmaps.end(); i++)
    {
        for (auto j = kmap_list.begin(); j != kmap_list.end(); j++)
        {
            if (*j == *i)
            {
                kmap_list.erase(j);
                break;
            }
        }
    }

finish:
    result = 0;
    if (kmap_ret != 0)
    {
        *kmap_ret = real_alloc_save;
        real_alloc_save->ref_counter++;
    }
    if (new_alloc != 0)
    {
        kmap_list.push_back(real_alloc_save);
    }
fail:

    return result;
}

int kernel_block::map_save_virt(size_t kva, size_t kb_size, void** virt_ret)
{
    int result = -1;
    real_kmap_t* out_kmap = 0;

    SAFE_BAIL(consolidate_kmap_allocation(kva, kb_size, &out_kmap) == -1)

    *virt_ret = (kva - out_kmap->kva) + out_kmap->alloc_base;
    result = 0 ;
fail:
    return result;
}

int kernel_block::map_kernel_block(std::string block_name, size_t kva, size_t kb_size, named_kmap_t** kmap_ret)
{
    int result = -1;
    named_kmap_t* named_alloc_save = 0;
    real_kmap_t* real_alloc_save = 0;

    // if we are able to find the section already, then just return the block
    if (named_alloc_list.find(block_name) != named_alloc_list.end())
    {
        *kmap_ret = named_alloc_list[block_name];
        goto finish;
    }
    // if we can't find the block, we are gonna have to see if it is in the bounds
    // of blocks that are already allocated.

    SAFE_BAIL(consolidate_kmap_allocation(kva, kb_size, &real_alloc_save) == -1);
    named_alloc_save = (named_kmap_t*)calloc(1, sizeof(named_kmap_t));
    named_alloc_save->kva = kva;
    named_alloc_save->kmap_stats.alloc_size = kb_size;
    named_alloc_save->kmap_stats.alloc_base = (kva - real_alloc_save->kva) + real_alloc_save->alloc_base;
    // 3 different inserts to track for new named object: track
    // its real allocator owner, have the map that tracks the pull and
    // add to the allocators tracking child list
    named_alloc_save->owner = real_alloc_save;
    named_alloc_list[block_name] = named_alloc_save;
    real_alloc_save->child_list.push_back(named_alloc_save);

finish:
    result = 0;
fail:
    return result;
}

int kernel_block::volatile_map(size_t kva, size_t kv_size, void** virt_ret, bool volatile_op)
{
    int result = -1;

    if (volatile_op == true)
    {
        SAFE_BAIL(live_kern_addr(kva, kv_size, virt_ret) == -1);
    }
    else
    {
        SAFE_BAIL(map_save_virt(kva, kv_size, virt_ret) == -1);
    }

    result = 0;
fail:
    return result;
}

int kernel_block::volatile_free(size_t kva, void* virt_used, bool volatile_op)
{
    if (volatile_op == true)
    {
        SAFE_FREE(virt_used);
    }
    return 0;
}
