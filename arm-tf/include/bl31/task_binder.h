#ifndef __ASSEMBLER__

#ifndef TASK_BINDER_H
#define TASK_BINDER_H

#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/plat_arm.h>

#define CCAEXT_SHADOWTASK_MAX 0x1
#define CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER 170
#define CCAEXT_SHADOWTASK_MAX_TOTALDATASET 170


#define BITMAP_REALTASK_ADDR(phys_addr) ((( (phys_addr&0xffffff000) - 0x8a0000000 ) >> 9 ) + 0xa0f00000)
#define bitmap_realtask_config(phys_addr, bit) *(uint64_t*)BITMAP_REALTASK_ADDR(phys_addr) = bit
#define bitmap_realtask_check(phys_addr, bit) (*(uint64_t*)BITMAP_REALTASK_ADDR(phys_addr) == bit)

			/*
			* CAGE-Extension-Note: We input this address based on PCIe configuration on this prototype, 
			*					   modify this in your environment.
			*/

#define SELF_CONFIG_FPGA_DATA_ADDR 0x51000000
#define SELF_CONFIG_XDMA1_CHANNEL_ADDR 0x50140000
#define SELF_CONFIG_XDMA1_USER_ADDR 0x50100000 //Not used

struct dataset{
    bool isused;
    uint64_t stub_addr;
    uint64_t real_addr;
    uint64_t set_size;
};

struct shadowtask{
    bool is_available;
    uint64_t stub_app_id;
    struct dataset total_databuffer[CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER];
    struct dataset total_dataset[CCAEXT_SHADOWTASK_MAX_TOTALDATASET];
    uint64_t gpu_gpt_mempart_startaddr;
};

extern struct shadowtask shadowtask_total[CCAEXT_SHADOWTASK_MAX];

bool check_and_tag_bitmap_realtask(uint64_t startaddr, uint64_t required_size, uint64_t bit);
uint64_t find_available_realtask_region(uint64_t size);
void init_bitmap_realtask_page(void);
void init_ccaext_shadowtask(void);
void print_all_shadowtask(void);
struct shadowtask *add_ccaext_shadowtask(uint64_t stub_app_id);
struct shadowtask *find_ccaext_shadowtask(uint64_t stub_app_id);
void destroy_ccaext_shadowtask(struct shadowtask *cur_shadowtask);
void record_each_dataset(uint64_t stub_app_id,uint64_t stub_addr,uint64_t real_addr,uint64_t set_size);
void record_each_databuffer(uint64_t stub_app_id,uint64_t stub_addr,uint64_t real_addr,uint64_t set_size);
void protect_total_dataset(uint64_t stub_app_id);
uint64_t get_dataset_real_addr(struct shadowtask *cur_shadowtask, uint64_t stub_addr);
uint64_t get_databuffer_real_addr(struct shadowtask *cur_shadowtask, uint64_t stub_addr);
uint64_t get_databuffer_stub_addr(struct shadowtask *cur_shadowtask, uint64_t real_addr);

void sha256(uint32_t *ctx, const void *in, size_t size);
void sha256_final(uint32_t *ctx, const void *in, size_t remain_size, size_t tot_size);
void sha256_block_data_order(uint32_t *ctx, const void *in, size_t num);
void hmac_sha256(void *out, const void *in, size_t size);
int check_code_integrity();
uint64_t eval_opt1(uint64_t gb_size);
uint64_t eval_opt2(bool isopt,uint64_t memsize,uint64_t numgpc);

#endif
#endif
