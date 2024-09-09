#ifndef __ASSEMBLER__

#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H



#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/plat_arm.h>

#define get_bit_range_value(number, start, end) (( (number) >> (end) ) & ( (1L << ( (start) - (end) + 1) ) - 1) )

void dumpmem(uint64_t addr, uint64_t size);
void fast_memset(void *dst, unsigned long val, size_t size);
void fast_memcpy(void *dst, void *src, size_t size);
void fast_memcpy_addr(uint64_t dst, uint64_t src, size_t size);
void datafiller(uint64_t startaddr, uint64_t size, uint64_t val);
void debug_printregion_64(uint64_t pa, uint64_t size);
void debug_printregion_32(uint64_t pa, uint32_t size);

void handle_tlb_ipa(unsigned long ipa);
#endif
#endif
