/*
 * Hypervisor stub
 *
 * Copyright (C) 2012 ARM Ltd.
 * Author:	Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/irqchip/arm-gic-v3.h>

#include <asm/assembler.h>
#include <asm/kvm_arm.h>
#include <asm/kvm_asm.h>
#include <asm/ptrace.h>
#include <asm/virt.h>

	.text
	.align 11

ENTRY(__hyp_stub_vectors)
	ventry	el2_sync_invalid		// Synchronous EL2t
	ventry	el2_irq_invalid			// IRQ EL2t
	ventry	el2_fiq_invalid			// FIQ EL2t
	ventry	el2_error_invalid		// Error EL2t

	ventry	el2_sync_invalid		// Synchronous EL2h
	ventry	el2_irq_invalid			// IRQ EL2h
	ventry	el2_fiq_invalid			// FIQ EL2h
	ventry	el2_error_invalid		// Error EL2h

	ventry	el1_sync			// Synchronous 64-bit EL1
	ventry	el1_irq_invalid			// IRQ 64-bit EL1
	ventry	el1_fiq_invalid			// FIQ 64-bit EL1
	ventry	el1_error_invalid		// Error 64-bit EL1

	ventry	el1_sync_invalid		// Synchronous 32-bit EL1
	ventry	el1_irq_invalid			// IRQ 32-bit EL1
	ventry	el1_fiq_invalid			// FIQ 32-bit EL1
	ventry	el1_error_invalid		// Error 32-bit EL1
ENDPROC(__hyp_stub_vectors)

	.align 11

el1_sync:

	
	// mrs x30,esr_el2
	// lsr x30,x30, #ESR_ELx_EC_SHIFT
	// cmp x30,ESR_ELx_EC_HVC64
	// b.ne 20f //20: used for data abort

	cmp	x0, #HVC_SET_VECTORS
	b.ne	2f
	msr	vbar_el2, x1
	b	9f

2:	cmp	x0, #HVC_SOFT_RESTART
	b.ne	4f
	mov	x0, x2
	mov	x2, x4
	mov	x4, x1
	mov	x1, x3
	br	x4				// no return

4:	cmp	x0, #HVC_RESET_VECTORS
	beq	9f				// Nothing to reset!

/*
20:
	//S2 write Fault
	
	ldr	x30,=0x89ffff000
	stp	x0, x1, [x30, #16 * 0]
	stp	x2, x3, [x30, #16 * 1]
	stp	x4, x5, [x30, #16 * 2]
	stp	x6, x7, [x30, #16 * 3]
	stp	x8, x9, [x30, #16 * 4]
	stp	x10, x11, [x30, #16 * 5]
	stp	x12, x13, [x30, #16 * 6]
	stp	x14, x15, [x30, #16 * 7]
	stp	x16, x17, [x30, #16 * 8]
	stp	x18, x19, [x30, #16 * 9]
	stp	x20, x21, [x30, #16 * 10]
	stp	x22, x23, [x30, #16 * 11]
	stp	x24, x25, [x30, #16 * 12]
	stp	x26, x27, [x30, #16 * 13]
	stp	x28, x29, [x30, #16 * 14]


	//Get the addr and val of write operation
	
	mrs x30,esr_el2
	ldr x29,=0x1f0000
	and x30,x30,x29
	lsr x30,x30,#13 //x30 now store t value * 8 of Xt
	ldr	x29,=0x89ffff000
	add	x30,x30,x29
	ldr	x1,[x30] //put value into x1
	
	
	mrs x30,FAR_EL2
	at s12e1r,x30
	mrs x29,par_el1
	ldr x28,=0xfffffff0
	lsl x28,x28,#8
	and x29,x29,x28
	ldr x28,=0xff8
	mrs x30,FAR_EL2
	and x28,x30,x28
	add x0,x28,x29 //put addr into x0
	

	//Then help to write
	str x1,[x0]
	

	//Get the prev counter and update the record, currently the addr is in x0 and val is in x1
	
	ldr x30,=0x89ffffff0
	ldr x29,[x30]
	lsl x29,x29,#3
	ldr x30,=0x89f000000
	add x30,x30,x29
	str x0,[x30]
	add x30,x30,#8
	str x1,[x30]
	lsr x29,x29,#3
	add x29,x29,#1
	ldr x30,=0x89ffffff0
	str x29,[x30]
	

	//PC+4
	mrs x30,ELR_EL2
	add x30,x30,#4
	msr ELR_EL2,x30	

	ldr	x30,=0x89ffff000
	ldp	x0, x1, [x30, #16 * 0]
	ldp	x2, x3, [x30, #16 * 1]
	ldp	x4, x5, [x30, #16 * 2]
	ldp	x6, x7, [x30, #16 * 3]
	ldp	x8, x9, [x30, #16 * 4]
	ldp	x10, x11, [x30, #16 * 5]
	ldp	x12, x13, [x30, #16 * 6]
	ldp	x14, x15, [x30, #16 * 7]
	ldp	x16, x17, [x30, #16 * 8]
	ldp	x18, x19, [x30, #16 * 9]
	ldp	x20, x21, [x30, #16 * 10]
	ldp	x22, x23, [x30, #16 * 11]
	ldp	x24, x25, [x30, #16 * 12]
	ldp	x26, x27, [x30, #16 * 13]
	ldp	x28, x29, [x30, #16 * 14]
	eret

	isb
	eret
*/


9:	mov	x0, xzr
	eret
ENDPROC(el1_sync)

.macro invalid_vector	label
\label:
	b \label
ENDPROC(\label)
.endm

	invalid_vector	el2_sync_invalid
	invalid_vector	el2_irq_invalid
	invalid_vector	el2_fiq_invalid
	invalid_vector	el2_error_invalid
	invalid_vector	el1_sync_invalid
	invalid_vector	el1_irq_invalid
	invalid_vector	el1_fiq_invalid
	invalid_vector	el1_error_invalid

/*
 * __hyp_set_vectors: Call this after boot to set the initial hypervisor
 * vectors as part of hypervisor installation.  On an SMP system, this should
 * be called on each CPU.
 *
 * x0 must be the physical address of the new vector table, and must be
 * 2KB aligned.
 *
 * Before calling this, you must check that the stub hypervisor is installed
 * everywhere, by waiting for any secondary CPUs to be brought up and then
 * checking that is_hyp_mode_available() is true.
 *
 * If not, there is a pre-existing hypervisor, some CPUs failed to boot, or
 * something else went wrong... in such cases, trying to install a new
 * hypervisor is unlikely to work as desired.
 *
 * When you call into your shiny new hypervisor, sp_el2 will contain junk,
 * so you will need to set that to something sensible at the new hypervisor's
 * initialisation entry point.
 */

ENTRY(__hyp_set_vectors)
	mov	x1, x0
	mov	x0, #HVC_SET_VECTORS
	hvc	#0
	ret
ENDPROC(__hyp_set_vectors)

ENTRY(__hyp_reset_vectors)
	mov	x0, #HVC_RESET_VECTORS
	hvc	#0
	ret
ENDPROC(__hyp_reset_vectors)
