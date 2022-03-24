#ifndef KERN_IMG_H
#define KERN_IMG_H

#include <kernel_block.h>
#include <stdio.h>
#include <stdint.h>

// generic linux kernel functionality, these are the parts that are backwards
// compatible with live kernels and non live kernels

class kernel_xnu : public kernel_block
{
protected:

public:
    using kernel_block::kernel_block;
};

#endif