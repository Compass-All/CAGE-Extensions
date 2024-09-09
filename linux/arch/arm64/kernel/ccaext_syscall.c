#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE3(ccaext_create_app, uint64_t, app_id, uint64_t, address, uint64_t, size){
	// To establish a real task

	printk("Start establishing App %llx, size ADDR %llx\n",app_id,size);
	printk("INIT CURRENT PID:%x\n",current->pid);

	uint64_t required_size;
	required_size=0x1000+((size>>12)<<12);

	if ((size&0xfff)!=0){
		required_size+=0x1000;
	}

	//Call the service to specify the real task area.

	uint64_t required_address=0;
	asm volatile(
	"ldr x0,=0xc7000002\n"
	"mov x1,%1\n"
	"smc #0\n"
	"mov %0,x1\n"
	:"=r"(required_address)
	:"r"(required_size)
	:"x0","x1"
	);
	printk("Get required address %llx\n",required_address);

	//define and write header
	void __iomem *real_task_header = ioremap(required_address, 0x1000);	

	//copy data
	void __iomem *real_task_data = ioremap(required_address+0x1000, required_size-0x1000);
	copy_from_user(real_task_data,(void *)address,size);

	//Allocate it
	asm volatile(
	"ldr x0,=0xc7000001\n"
	"mov x1,%0\n"
	"mov x2,%1\n"
	"mov x3,%2\n"
	"smc #0\n"
	:
	:"r"(app_id),"r"(required_address),"r"(required_size)
	:"x0","x1","x2","x3"
	);
	printk("App %llx established successfully\n",app_id);

	//return the current->pid as the app_id
	return 0;
}

SYSCALL_DEFINE1(ccaext_destory_app, uint64_t, app_id)
{
	// To destory a confidential application
	printk("Start destroying App %llx\n",app_id);
	printk("FIN CURRENT PID:%x\n",current->pid);

	asm volatile(
	"ldr x0,=0xc7000009\n"
	"mov x1,%0\n"
	"smc #0\n"
	:
	:"r"(app_id)
	:"x0","x1"
	);

	printk("App %llx destroyed successfully\n",app_id);
	return 0;
}
