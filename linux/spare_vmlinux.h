#ifndef SPARE_VMLINUX_H
#define SPARE_VMLINUX_H

#include <stdio.h>
#include <stdint.h>

#define MAX_PARAM_PREFIX_LEN (64 - sizeof(unsigned long))
#define MODULE_NAME_LEN MAX_PARAM_PREFIX_LEN
typedef struct
{
    unsigned long crc;
    char name[MODULE_NAME_LEN];
} modversion_info;

typedef struct
{
	unsigned long value;
	const char *name;
} kernel_symbol;

typedef void* kernel_param_ops;
typedef void* module;

/* Special one for arrays */
typedef struct
{
	unsigned int max;
	unsigned int elemsize;
	unsigned int *num;
	const kernel_param_ops *ops;
	void *elem;
} kparam_array;

/* Special one for strings we want to copy into */
typedef struct 
{
	unsigned int maxlen;
	char *string;
} kparam_string;

struct kernel_param {
	const char *name;
	module *mod;
	const kernel_param_ops *ops;
	const uint16_t perm;
	int8_t level;
	uint8_t flags;
	union {
		void *arg;
		const kparam_string *str;
		const kparam_array *arr;
	};
};

typedef struct {
	const kernel_symbol *start, *stop;
	const unsigned long *crcs;
	enum {
		NOT_GPL_ONLY,
		GPL_ONLY,
		WILL_BE_GPL_ONLY,
	} licence;
	bool unused;
} symsearch;

#endif