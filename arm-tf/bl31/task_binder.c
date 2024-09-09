#include <bl31/task_binder.h>
#include <bl31/access.h>
#include <bl31/memory_util.h>


#include <lib/mmio.h>
#include <arch.h>
#include <common/debug.h>

struct shadowtask shadowtask_total[CCAEXT_SHADOWTASK_MAX];
//0xa0f0_0000 - 0xa0f7_ffff, totally 0x8_0000
//since we have 0x1_0000 real task page, so we use 0x8 to represent each page
uint64_t *bitmap_realtask_page = (uint64_t*)0xa0f00000;
uint32_t init_H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};



void init_bitmap_realtask_page(void){
    uint64_t curaddr_realtask=0x8a0000000;
    uint64_t endaddr_realtask=0x8b0000000;
    while(curaddr_realtask<endaddr_realtask){
        bitmap_realtask_config(curaddr_realtask,0x1);
        curaddr_realtask+=0x1000;
    }
    NOTICE("CCA Extension Bitmap for Real Task Region Initialized\n");
}

bool check_and_tag_bitmap_realtask(uint64_t startaddr, uint64_t required_size, uint64_t bit){
    //To check and tag a bitmap region
    uint64_t cursize=0;
    while (cursize<required_size){
        if (bitmap_realtask_check((startaddr+cursize),bit)){
            return false;
        }
        bitmap_realtask_config((startaddr+cursize),bit);
        cursize+=0x1000;
    }
    return true;
}

uint64_t find_available_realtask_region(uint64_t required_size){
    uint64_t failedaddr=0x0;
    uint64_t startaddr=0x8a0000000;
    uint64_t endaddr=0x8b0000000;
    while ((startaddr+required_size)<=endaddr){


        if(bitmap_realtask_check(startaddr,0x1)){
            bool is_satisfied=true;
            uint64_t cursize=0;
            while(cursize<required_size){
                if(bitmap_realtask_check((startaddr+cursize),0x0)){
                    is_satisfied=false;
                    break;
                }
                cursize+=0x1000;
            }
            if (is_satisfied){
                return startaddr;
            }
            else{
                startaddr+=cursize;
            }
        }
        else{
            startaddr+=0x1000;
        }

    }
    return failedaddr;
}

void init_ccaext_shadowtask(void){
    uint32_t cur_index=0;
    uint64_t cur_gpu_gpt_mempart_startaddr=0xa0900000;
    uint64_t cur_gpu_gpt_mempart_size=0x20000;


	while(cur_index<CCAEXT_SHADOWTASK_MAX){
        memset(&shadowtask_total[cur_index],0,sizeof(struct shadowtask));
        shadowtask_total[cur_index].is_available=true;
        shadowtask_total[cur_index].stub_app_id=0;
        int tmp_idx=0;
        while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATASET){
            shadowtask_total[cur_index].total_dataset[tmp_idx].isused=false;
            shadowtask_total[cur_index].total_dataset[tmp_idx].stub_addr=0;
            shadowtask_total[cur_index].total_dataset[tmp_idx].real_addr=0;
            shadowtask_total[cur_index].total_dataset[tmp_idx].set_size=0;
            tmp_idx+=1;
        }
        tmp_idx=0;
        while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER){
            shadowtask_total[cur_index].total_databuffer[tmp_idx].isused=false;
            shadowtask_total[cur_index].total_databuffer[tmp_idx].stub_addr=0;
            shadowtask_total[cur_index].total_databuffer[tmp_idx].real_addr=0;
            shadowtask_total[cur_index].total_databuffer[tmp_idx].set_size=0;
            tmp_idx+=1;
        }

        shadowtask_total[cur_index].gpu_gpt_mempart_startaddr=cur_gpu_gpt_mempart_startaddr;
        init_ccaext_gpu_gpt_mempart(shadowtask_total[cur_index].gpu_gpt_mempart_startaddr);
        cur_gpu_gpt_mempart_startaddr+=cur_gpu_gpt_mempart_size;

        cur_index+=1;
	}
    NOTICE("CCA Extension Task Binder Initialized\n");
}

void print_all_shadowtask(void){
    uint32_t cur_index=0;
	while(cur_index<CCAEXT_SHADOWTASK_MAX){
        NOTICE("Shadowtask %x: \n",cur_index);
        cur_index+=1;
	}
}

struct shadowtask *add_ccaext_shadowtask(uint64_t stub_app_id){
    uint32_t cur_index=0;
    while(cur_index<CCAEXT_SHADOWTASK_MAX){
        if (shadowtask_total[cur_index].is_available==true){
            shadowtask_total[cur_index].is_available=false;
            shadowtask_total[cur_index].stub_app_id=stub_app_id;
            return &shadowtask_total[cur_index];
        }
        cur_index+=1;
	}
    return NULL;
}

struct shadowtask *find_ccaext_shadowtask(uint64_t stub_app_id){
    uint32_t cur_index=0;
    while(cur_index<CCAEXT_SHADOWTASK_MAX){
        if (shadowtask_total[cur_index].stub_app_id==stub_app_id){
            return &shadowtask_total[cur_index];
        }
        cur_index+=1;
    }
    return NULL;
}

void destroy_ccaext_shadowtask(struct shadowtask *cur_shadowtask){

    uint64_t app_id=cur_shadowtask->stub_app_id;

    uint64_t tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATASET){
        if(cur_shadowtask->total_dataset[tmp_idx].isused==true){

            uint64_t cur_start_addr=cur_shadowtask->total_dataset[tmp_idx].real_addr;
            uint64_t cur_size=cur_shadowtask->total_dataset[tmp_idx].set_size;
            bool is_shadowtask_satisfied=check_and_tag_bitmap_realtask(cur_start_addr,cur_size,0x1);
            if (!is_shadowtask_satisfied){
                ERROR("In app %llx, cannot clear region %llx with size %llx\n",app_id,cur_start_addr,cur_size);
                panic();
            }
            datafiller(cur_shadowtask->total_dataset[tmp_idx].real_addr,cur_shadowtask->total_dataset[tmp_idx].set_size,0);

            cur_shadowtask->total_dataset[tmp_idx].isused=false;
            cur_shadowtask->total_dataset[tmp_idx].stub_addr=0;
            cur_shadowtask->total_dataset[tmp_idx].real_addr=0;
            cur_shadowtask->total_dataset[tmp_idx].set_size=0;
            gptconfig_sublevel(0xa00a0000,cur_start_addr,cur_size,CCAEXT_ALL);
        }

        if(cur_shadowtask->total_databuffer[tmp_idx].isused==true){

            uint64_t cur_start_addr=cur_shadowtask->total_databuffer[tmp_idx].real_addr;
            uint64_t cur_size=cur_shadowtask->total_databuffer[tmp_idx].set_size;
            bool is_shadowtask_satisfied=check_and_tag_bitmap_realtask(cur_start_addr,cur_size,0x1);
            if (!is_shadowtask_satisfied){
                ERROR("In app %llx, cannot clear region %llx with size %llx\n",app_id,cur_start_addr,cur_size);
                panic();
            }
            datafiller(cur_shadowtask->total_databuffer[tmp_idx].real_addr,cur_shadowtask->total_databuffer[tmp_idx].set_size,0);

            gptconfig_sublevel(0xa00a0000,cur_start_addr,cur_size,CCAEXT_ALL);
            gptconfig_sublevel(cur_shadowtask->gpu_gpt_mempart_startaddr,cur_start_addr,cur_size,CCAEXT_INVAL);

            cur_shadowtask->total_databuffer[tmp_idx].isused=false;
            cur_shadowtask->total_databuffer[tmp_idx].stub_addr=0;
            cur_shadowtask->total_databuffer[tmp_idx].real_addr=0;
            cur_shadowtask->total_databuffer[tmp_idx].set_size=0;
        }


        tmp_idx+=1;
    }

    cur_shadowtask->is_available=true;
    cur_shadowtask->stub_app_id=0;
    NOTICE("Shadow app id: %llx destroyed\n",app_id);
}

void record_each_dataset(uint64_t stub_app_id,uint64_t stub_addr,uint64_t real_addr,uint64_t set_size){

	bool is_shadowtask_satisfied=check_and_tag_bitmap_realtask(real_addr, set_size, 0x0);
    if (!is_shadowtask_satisfied){
        ERROR("In app %llx, cannot record dataset real_addr %llx, size: %llx\n",stub_app_id,real_addr,set_size);
        panic();
    }
    struct shadowtask *cur_shadowtask=find_ccaext_shadowtask(stub_app_id);
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATASET){
        if(cur_shadowtask->total_dataset[tmp_idx].isused==false){
            cur_shadowtask->total_dataset[tmp_idx].isused=true;
            cur_shadowtask->total_dataset[tmp_idx].stub_addr=stub_addr;
            cur_shadowtask->total_dataset[tmp_idx].real_addr=real_addr;
            cur_shadowtask->total_dataset[tmp_idx].set_size=set_size;
            break;
        }
        tmp_idx+=1;
    }
}

void record_each_databuffer(uint64_t stub_app_id,uint64_t stub_addr,uint64_t real_addr,uint64_t set_size){

	bool is_shadowtask_satisfied=check_and_tag_bitmap_realtask(real_addr, set_size, 0x0);
    if (!is_shadowtask_satisfied){
        ERROR("In app %llx, cannot record databuffer real_addr %llx, size: %llx\n",stub_app_id,real_addr,set_size);
        panic();
    }
    struct shadowtask *cur_shadowtask=find_ccaext_shadowtask(stub_app_id);
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER){
        if(cur_shadowtask->total_databuffer[tmp_idx].isused==false){
            cur_shadowtask->total_databuffer[tmp_idx].isused=true;
            cur_shadowtask->total_databuffer[tmp_idx].stub_addr=stub_addr;
            cur_shadowtask->total_databuffer[tmp_idx].real_addr=real_addr;
            cur_shadowtask->total_databuffer[tmp_idx].set_size=set_size;
            break;
        }
        tmp_idx+=1;
    }
}

void protect_total_dataset(uint64_t stub_app_id){
    struct shadowtask *cur_shadowtask=find_ccaext_shadowtask(stub_app_id);
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATASET){
        if(cur_shadowtask->total_dataset[tmp_idx].isused==true){
            uint64_t cur_start_addr=cur_shadowtask->total_dataset[tmp_idx].real_addr;
            uint64_t cur_size=cur_shadowtask->total_dataset[tmp_idx].set_size;
            gptconfig_sublevel(0xa00a0000,cur_start_addr,cur_size,CCAEXT_ROOT);
        }
        tmp_idx+=1;
    }
}

uint64_t get_dataset_real_addr(struct shadowtask *cur_shadowtask, uint64_t stub_addr){
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATASET){
        if(cur_shadowtask->total_dataset[tmp_idx].isused==true){
            if (cur_shadowtask->total_dataset[tmp_idx].stub_addr==stub_addr){
                return cur_shadowtask->total_dataset[tmp_idx].real_addr;
            }
        }
        tmp_idx+=1;
    }
    return 0;
}


uint64_t get_databuffer_real_addr(struct shadowtask *cur_shadowtask, uint64_t stub_addr){
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER){
        if(cur_shadowtask->total_databuffer[tmp_idx].isused==true){
            if (cur_shadowtask->total_databuffer[tmp_idx].stub_addr==stub_addr){
                return cur_shadowtask->total_databuffer[tmp_idx].real_addr;
            }
        }
        tmp_idx+=1;
    }
    return 0;
}

uint64_t get_databuffer_stub_addr(struct shadowtask *cur_shadowtask, uint64_t real_addr){
    int tmp_idx=0;
    while(tmp_idx<CCAEXT_SHADOWTASK_MAX_TOTALDATABUFFER){
        if(cur_shadowtask->total_databuffer[tmp_idx].isused==true){
            if (cur_shadowtask->total_databuffer[tmp_idx].real_addr==real_addr){
                return cur_shadowtask->total_databuffer[tmp_idx].stub_addr;
            }
        }
        tmp_idx+=1;
    }
    return 0;
}



void sha256_final(uint32_t *ctx, const void *in, size_t remain_size, size_t tot_size) {
	size_t block_num = remain_size / 64;

    sha256(ctx, in, block_num*64);

	size_t remainder = remain_size % 64;
	size_t tot_bits = tot_size * 8;
	char last_block[64];
	fast_memset(last_block, 0, sizeof(last_block));
	fast_memcpy(last_block, (void*)in+block_num*64, remainder);
	last_block[remainder] = 0x80;
	if (remainder < 56) {}
	else {
		sha256_block_data_order(ctx, last_block, 1);
		fast_memset(last_block, 0, sizeof(last_block));
	}
	for (int i = 0 ; i < 8 ; ++ i) last_block[63-i] = tot_bits >> (i * 8);
	sha256_block_data_order(ctx, last_block, 1);
}

void sha256(uint32_t *ctx, const void *in, size_t size) {
	size_t block_num = size / 64;
	if (block_num) sha256_block_data_order(ctx, in, block_num);
}

int parse_mali_instruction(uint64_t *code) {
	// refer: https://gitlab.freedesktop.org/panfrost/mali-isa-docs/-/blob/master/Midgard.md
	// 3 - Texture (4 words)
	// 5 - Load/Store (4 words)
	// 8 - ALU (4 words)
	// 9 - ALU (8 words)
	// A - ALU (12 words)
	// B - ALU (16 words)
	uint64_t code_start = (uint64_t)code;
	// int code_length = 0;
	int current_type, next_type;
	while (1) {
		current_type = ((*code) & 0xf);
		next_type    = ((*code) & 0xf0) >> 4;
		switch (current_type) {
			case 3:
				code += 2;
				break;
			case 4:
				code += 2;
				break;
			case 5:
				code += 2;
				break;
			case 8:
				code += 2;
				break;
			case 9:
				code += 4;
				break;
			case 0xa:
				code += 6;
				break;
			case 0xb:
				code += 8;
				break;
			default:
				ERROR("[Unexcepted Behavior]: Instruction format [%d] error!\n", current_type);
				panic();
		}

		if (next_type == 1) break;
	}
	int code_length = (uint64_t)(code) - code_start;
	return code_length;
}

void hmac_sha256(void *out, const void *in, size_t size) {
	char k[64], k_ipad[64], k_opad[64];
	fast_memset(k, 0, 64);
	// fast_memcpy(k, aes_key, 32);
	for (int i = 0 ; i < 64 ; ++ i) {
		k_ipad[i] ^= k[i];
		k_opad[i] ^= k[i];
	}
	uint32_t ihash[8]; fast_memcpy(ihash, init_H, sizeof(ihash));
	sha256(ihash, k_ipad, 64);
	sha256(ihash, in, size);
	sha256_final(ihash, k_ipad, size%64, size+64);
	uint32_t ohash[8]; fast_memcpy(ohash, init_H, sizeof(ohash));
	sha256(ohash, k_opad, 64);
	sha256(ohash, ihash, 64);
	sha256_final(ohash, k_opad, 0, 128);
	fast_memcpy(out, ohash, 64);
}
