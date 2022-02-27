#include <stdint.h>
#include <string.h>

#include <kernel_block.h>

#include "kern_img.h"

#define ARRAY_SIZE(array) \
    (sizeof(array) / sizeof(*array))
#define KSYM_NAME_LEN 128
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// for now i'm commenting out the enabled checkks, cause really its fairly
// arbitrary what i do with the kall_syms_offsets, since its relative to
// the base of the kernel itself. I may just grab the binBegin and add that.
unsigned long kern_img::kallsyms_sym_address(int idx)
{
	const unsigned long* kallsyms_addresses = (unsigned long*)KSYM_V(kallsyms_addresses);
	const int* kallsyms_offsets = (int*)KSYM_V(kallsyms_offsets);
	const unsigned long kallsyms_relative_base = KSYM_V(kallsyms_relative_base);

	// if (!IS_ENABLED(CONFIG_KALLSYMS_BASE_RELATIVE))
	// 	return kallsyms_addresses[idx];

	// /* values are unsigned offsets if --absolute-percpu is not in effect */
	// if (!IS_ENABLED(CONFIG_KALLSYMS_ABSOLUTE_PERCPU))
	// 	return kallsyms_relative_base + (u32)kallsyms_offsets[idx];

	/* ...otherwise, positive offsets are absolute values */
	if (kallsyms_offsets[idx] >= 0)
		return kallsyms_offsets[idx];

	/* ...and negative offsets are relative to kallsyms_relative_base - 1 */
	return kallsyms_relative_base - 1 - kallsyms_offsets[idx];
}

/*
 * Expand a compressed symbol data into the resulting uncompressed string,
 * if uncompressed string is too long (>= maxlen), it will be truncated,
 * given the offset to where the symbol is in the compressed stream.
 */
unsigned int kern_img::kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen)
{
	int len, skipped_first = 0;
	const u8 *tptr, *data;

	const u8* kallsyms_names = (const u8*)KSYM_V(kallsyms_names);
	const u8* kallsyms_token_table = (const u8*)KSYM_V(kallsyms_token_table);
	const u16* kallsyms_token_index = (const u16*)KSYM_V(kallsyms_token_index);
	/* Get the compressed symbol length from the first symbol byte. */
	data = &kallsyms_names[off];
	len = *data;
	data++;

	/*
	 * Update the offset to return the offset for the next symbol on
	 * the compressed stream.
	 */
	off += len + 1;

	/*
	 * For every byte on the compressed symbol data, copy the table
	 * entry for that byte.
	 */
	while (len) {
		tptr = &kallsyms_token_table[kallsyms_token_index[*data]];
		data++;
		len--;

		while (*tptr) {
			if (skipped_first) {
				if (maxlen <= 1)
					goto tail;
				*result = *tptr;
				result++;
				maxlen--;
			} else
				skipped_first = 1;
			tptr++;
		}
	}

tail:
	if (maxlen)
		*result = '\0';

	/* Return to offset to the next symbol. */
	return off;
}

// /* Look for this name: can be of form module:name. */
// unsigned long module_kallsyms_lookup_name(const char *name)
// {
// 	struct module *mod;
// 	char *colon;
// 	unsigned long ret = 0;

// 	/* Don't lock: we're in enough trouble already. */
// 	preempt_disable();
// 	if ((colon = strchr(name, ':')) != NULL) {
// 		if ((mod = find_module_all(name, colon - name, false)) != NULL)
// 			ret = mod_find_symname(mod, colon+1);
// 	} else {
// 		list_for_each_entry_rcu(mod, &modules, list) {
// 			if (mod->state == MODULE_STATE_UNFORMED)
// 				continue;
// 			if ((ret = mod_find_symname(mod, name)) != 0)
// 				break;
// 		}
// 	}
// 	preempt_enable();
// 	return ret;
// }

// here is a temporary solution for kallsyms that i can't find
// not 100% sure what to do about it, but may uncomment the
// above routine later and see what's good.
static inline unsigned long module_kallsyms_lookup_name(const char *name)
{
	return 0;
}

/* Lookup the address for this symbol. Returns 0 if not found. */
unsigned long kern_img::kallsyms_lookup_name(const char *name)
{
	char namebuf[KSYM_NAME_LEN];
	unsigned long i;
	unsigned int off;
	const unsigned long kallsyms_num_syms = KSYM_V(kallsyms_num_syms);


	for (i = 0, off = 0; i < kallsyms_num_syms; i++) {
		off = kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));

		if (strcmp(namebuf, name) == 0)
			return kallsyms_sym_address(i);
	}
	return module_kallsyms_lookup_name(name);
}
