#include "dusk/interrupts/isr.h"
#include "dusk/kernel.h"
#include "dusk/memory/vmm.h"

#include <stddef.h>

void isr_handler(struct isr_registers* regs) {
	if (regs->exception >= 32) return;

	switch (regs->exception) {
    	case 0:  kpanic("[Exception] Divide Error.", NULL); 				   break;
		case 1:  kpanic("[Exception] Debug Exception.", NULL); 			   break;
		case 2:  kpanic("[Exception] NMI Interrupt.", NULL); 				   break;
		case 3:  kpanic("[Exception] Breakpoint.", NULL); 				   break;
		case 4:  kpanic("[Exception] Overflow.", NULL); 					   break;
		case 5:  kpanic("[Exception] BOUND Range Exceeded.", NULL); 		   break;
		case 6:  kpanic("[Exception] Invalid Opcode.", NULL); 			   break;
		case 7:  kpanic("[Exception] Device Not Available.", NULL); 		   break;
		case 8:  kpanic("[Exception] Double Fault.", NULL); 				   break;
		case 9:  kpanic("[Exception] Coprocessor Segment Overrun.", NULL);   break;
		case 10: kpanic("[Exception] Invalid TSS.", NULL); 				   break;
		case 11: kpanic("[Exception] Segment Not Present.", NULL); 		   break;
		case 12: kpanic("[Exception] Stack-Segment Fault.", NULL); 		   break;
		case 13: kpanic("[Exception] General Protection.", NULL); 		   break;
		
		case 14: {
			uint32_t fault_addr;
    		asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

			kpanic("[Exception] Page Fault.", &fault_addr);
			break;
		}

		case 15: kpanic("[Exception] Intel Reserved.", NULL);                break;
		case 16: kpanic("[Exception] x87 FPU Floating-Point Error.", NULL);  break;
		case 17: kpanic("[Exception] Alignment Check.", NULL);               break;
		case 18: kpanic("[Exception] Machine Check.", NULL);                 break;
		case 19: kpanic("[Exception] SIMD Floating-Point Exception.", NULL); break;
		case 20: kpanic("[Exception] Virtualization Exception.", NULL);      break;
		case 21: kpanic("[Exception] Control Protection Exception.", NULL);  break;

		default: kpanic("[Exception] Unknown.", NULL); break;
    }
}