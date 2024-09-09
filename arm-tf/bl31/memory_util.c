#include <bl31/memory_util.h>

#include <lib/mmio.h>
#include <arch.h>
#include <common/debug.h>

void fast_memset(void *dst, unsigned long val, size_t size) {
	size_t _size = size / 64 * 64;
	for (unsigned long addr = (unsigned long)dst ; addr < (unsigned long)dst + _size ; addr += 64) {
		((unsigned long*)addr)[0] = val; ((unsigned long*)addr)[1] = val; ((unsigned long*)addr)[2] = val; ((unsigned long*)addr)[3] = val;
		((unsigned long*)addr)[4] = val; ((unsigned long*)addr)[5] = val; ((unsigned long*)addr)[6] = val; ((unsigned long*)addr)[7] = val;
	}
	memset(dst+_size, val, size-_size);
}
void fast_memcpy(void *dst, void *src, size_t size) {
	for (long long remain = size ; remain-32 >= 0 ; dst += 32, src += 32, remain -= 32) {
		((unsigned long*)dst)[0] = ((unsigned long*)src)[0]; ((unsigned long*)dst)[1] = ((unsigned long*)src)[1];
		((unsigned long*)dst)[2] = ((unsigned long*)src)[2]; ((unsigned long*)dst)[3] = ((unsigned long*)src)[3];
	}
}

void fast_memcpy_addr(uint64_t dst, uint64_t src, size_t size) {

	uint64_t cntsize=0;
	while(cntsize<size){
		*(uint64_t*)(dst+cntsize) = *(uint64_t*)(src+cntsize);
		cntsize+=8;
	}
	flush_dcache_range(dst,size);
}

void datafiller(uint64_t startaddr, uint64_t size, uint64_t val){
	if((size&0x7)!=0){
        ERROR("Input size unaligned\n");
		ERROR("startaddr %llx size %llx val %llx\n",startaddr,size,val);
        panic();
    }
    uint64_t curaddr=startaddr;
    uint64_t finaladdr=startaddr+size;
    while(curaddr<finaladdr){
		asm volatile("str %0,[%1]\n"::"r"(val),"r"(curaddr):);

		curaddr+=0x8;
	}

    flush_dcache_range(startaddr,size);
}

void debug_printregion_64(uint64_t pa, uint64_t size){
				NOTICE("Debug:Print the region\n");
				uint64_t debug_start;
				uint64_t debug_cnt;
				debug_start=pa;
				debug_cnt=0;
				while(debug_cnt<size){
					uint64_t debug_curaddr=debug_cnt+debug_start;
					uint64_t debug_output=mmio_read_64(debug_curaddr);
					NOTICE("DEBUG:%llx %llx\n",debug_output,debug_curaddr);
					debug_cnt+=8;
				}
}

void debug_printregion_32(uint64_t pa, uint32_t size){
				NOTICE("Debug:Print the region\n");
				uint64_t debug_start;
				uint32_t debug_cnt;
				debug_start=pa;
				debug_cnt=0;
				while(debug_cnt<size){
					uint64_t debug_curaddr=debug_cnt+debug_start;
					uint32_t debug_output=mmio_read_32(debug_curaddr);
					NOTICE("DEBUG:%x %llx\n",debug_output,debug_curaddr);
					debug_cnt+=4;
				}
}

void dumpmem(uint64_t addr, uint64_t size) {
    uint64_t i = 0;
    NOTICE("=========================================\n");
	NOTICE("Start addr: %llx\n",addr);
    while (i < size ) {
        NOTICE("+0x%llx: %016llx %016llx %016llx %016llx\n", 
        i, 
        *(uint64_t*)(addr + i), *(uint64_t*)(addr + i + 0x8),
        *(uint64_t*)(addr + i + 0x10), *(uint64_t*)(addr + i + 0x18));
        i += 0x20;
    }
    NOTICE("=========================================\n");
}


void handle_tlb_ipa(unsigned long ipa){
	unsigned long chunkipa=ipa>>12;
	asm volatile(
		"dsb ish\n"
		"tlbi ipas2e1is, %0\n"
		"dsb ish\n"
		"tlbi vmalle1is\n"
		"dsb ish\n"
		"isb\n"
		:
		:"r"(chunkipa)
		:
	);
}