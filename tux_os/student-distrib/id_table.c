#include "id_table.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "id_table.h"
#include "pit.h"

// #include "assembly_linkage.h"  *** not needed i think

#define KERNEL_SEGMENT 0x0010
#define PIT_HANDLE_ENTRY 0x20
#define KEYBOARD_HANDLER_ENTRY 0x21
#define RTC_HANDLER_ENTRY 0x28
#define syscall_HANDLER_ENTRY 0x80

//**** TODO: make header file and put declarations for all these there
// so they can be moved below the prepare_idt function in this file

// Changed all handlers from this format to new format
// to return control to kernel:

// printf("page_fault_handler Handler\n");
// printf("BLUE SCREEN"); //infinite loop
// for (;;)
// {
// };

/* 
 * exception/interrupt_handlers
 *   DESCRIPTION: All of these handlers have the same
 *				  form in the sense that they clear interrupts to freeze 
 *				  our system, then get stuck in an infinite loop which is
 *				  our "blue screen of death".
 *
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: freezes system and prints blue screen of death.
 */
void default_exception_handler()
{
	printf("Default Exception Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void default_reserved_exception_handler()
{ //used for int vector 15 and 20-31
	printf("Default RESERVED Exception Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void divide_error_handler()
{
	printf("divide_error_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void debug_handler()
{
	printf("Default Exception Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void NMI_interrupt_handler()
{
	printf("NMI_interrupt_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void Breakpoint_handler()
{
	printf("Breakpoint_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void Overflow_handler()
{
	printf("Overflow_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void Bound_range_exceeded_handler()
{
	printf("Bound_range_exceeded_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void invalid_opcode_handler()
{
	printf("invalid_opcode_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void device_not_availble_handler()
{
	printf("device_not_availble_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void double_fault_handler()
{
	printf("double_fault_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void coprocessor_segment_overrun_handler()
{
	printf("coprocessor_segment_overrun_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void invalid_TSS_handler()
{
	printf("invalid_TSS_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void segment_not_present_handler()
{
	printf("segment_not_present_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void stack_segment_fault_handler()
{
	printf("stack_segment_fault_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void general_protection_handler()
{
	printf("general_protection_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void page_fault_handler()
{

	printf("EXCEPTION: PAGE FAULT \n");
	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
//15 is reserved
void floating_point_error_handler()
{
	printf("floating_point_error_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void alignment_check_handler()
{
	printf("alignment_check_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void machine_check_handler()
{
	printf("machine_check_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}
void SIMD_floating_point_exception_handler()
{
	printf("SIMD_floating_point_exception_handler Handler\n");

	int32_t process_total = helper_pass_pid();
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
	pcb->exception_flag = YES_EXECEPTION;
	halt(0); // halt the current process
}

/* 
 * prepare_idt
 *   DESCRIPTION: Initializes the idt table to proper values.  As specified
 *				  in intel ISA manual. 0-20 are used, 15 is reserved, 20-31 reserved
 *				  and 32-256 are not marked as 'present' so if theyre called 
 *				  itll throw a segment not present handler.
 *
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the content of idt entries and
 *					inserts all of our exception, interrupt
 *				 handlers.
 */
void prepare_idt()
{

	int index_0; //used to iterate over our IDT table numerous times

	for (index_0 = 0; index_0 < NUM_VEC; index_0++)
	{ //start at 0 because first entry. (NUM_VEC=256)

		idt[index_0].reserved0 = 0; //0110 is set to mark as 32 bit intr/trap gate
		idt[index_0].reserved1 = 1;
		idt[index_0].reserved2 = 1;
		idt[index_0].reserved4 = 0;
		if (index_0 >= 0x20 && index_0 <= 0x2F)
		{								//0x20=32 is where interrupt gates start
			idt[index_0].reserved3 = 0; //intr. gate, so 0
		}
		else
		{
			idt[index_0].reserved3 = 0; //trap. gate, so 1 // Modified by Yan 12/6 All but SYSCALL is set to INTR Gate
		}

		idt[index_0].size = 1; //1 to mark as 32 bit
		idt[index_0].dpl = 0;  //everything except system calls is called from kernel, so 0

		idt[index_0].seg_selector = KERNEL_SEGMENT; //"you should set this field to be the kernel's code segment descriptor"
		if ((index_0 < 0x20) || (index_0 == KEYBOARD_HANDLER_ENTRY) || (index_0 == RTC_HANDLER_ENTRY) || (index_0 == syscall_HANDLER_ENTRY) || (index_0 == PIT_HANDLE_ENTRY))
		{
			// below 0x20=32, we have define exceptions that are present

			idt[index_0].present = 1; //mark this entry as present, so 1
		}
		else
		{
			idt[index_0].present = 0; //mark this entry as not present, so 0
		}
	}

	idt[syscall_HANDLER_ENTRY].dpl = 3; // 0x80 hex = 128 decimal. We want to be able to access
										// system calls from user space.
										// 3= User space privelage!

	idt[syscall_HANDLER_ENTRY].reserved3 = 1; //	Mark system call as a trap gate
											  //  so that its interruptable (unless we do cli)
											  //  1 to mark as trap gate!

	// now we actually insert the handlers:
	// the magic numbers go, in order, from 0 to 19 to set each handler because
	// for these vector numbers we have actually defined handlers.
	SET_IDT_ENTRY(idt[0], &divide_error_handler_linkage);
	SET_IDT_ENTRY(idt[1], &debug_handler_linkage);
	SET_IDT_ENTRY(idt[2], &NMI_interrupt_handler_linkage);
	SET_IDT_ENTRY(idt[3], &Breakpoint_handler_linkage);
	SET_IDT_ENTRY(idt[4], &Overflow_handler_linkage);
	SET_IDT_ENTRY(idt[5], &Bound_range_exceeded_handler_linkage);
	SET_IDT_ENTRY(idt[6], &invalid_opcode_handler_linkage);
	SET_IDT_ENTRY(idt[7], &device_not_availble_handler_linkage);
	SET_IDT_ENTRY(idt[8], &double_fault_handler_linkage);
	SET_IDT_ENTRY(idt[9], &coprocessor_segment_overrun_handler_linkage);
	SET_IDT_ENTRY(idt[10], &invalid_TSS_handler_linkage);
	SET_IDT_ENTRY(idt[11], &segment_not_present_handler_linkage);
	SET_IDT_ENTRY(idt[12], &stack_segment_fault_handler_linkage);
	SET_IDT_ENTRY(idt[13], &general_protection_handler_linkage);
	SET_IDT_ENTRY(idt[14], &page_fault_handler_linkage);

	SET_IDT_ENTRY(idt[15], &default_reserved_exception_handler_linkage); //reserved, set to default handler

	SET_IDT_ENTRY(idt[16], &floating_point_error_handler_linkage);
	SET_IDT_ENTRY(idt[17], &alignment_check_handler_linkage);
	SET_IDT_ENTRY(idt[18], &machine_check_handler_linkage);
	SET_IDT_ENTRY(idt[19], &SIMD_floating_point_exception_handler_linkage);

	for (index_0 = 20; index_0 < 32; index_0++)
	{																			  //vectors 20-> 31 are reserved
		SET_IDT_ENTRY(idt[index_0], &default_reserved_exception_handler_linkage); //mark as reserved
	}

	for (index_0 = 0x20; index_0 < NUM_VEC; index_0++)
	{																	 //0x20=32 to 256 are unused vectors, so we just set to default
		SET_IDT_ENTRY(idt[index_0], &default_exception_handler_linkage); //user defined / maskable interrupts set all to default
		if (index_0 == PIT_HANDLE_ENTRY)
		{
			SET_IDT_ENTRY(idt[index_0], &pit_linkage);
		}
		if (index_0 == KEYBOARD_HANDLER_ENTRY)
		{
			SET_IDT_ENTRY(idt[index_0], &kbd_linkage);
		}
		if (index_0 == RTC_HANDLER_ENTRY)
		{
			SET_IDT_ENTRY(idt[index_0], &rtc_linkage);
		}
		if (index_0 == syscall_HANDLER_ENTRY)
		{
			SET_IDT_ENTRY(idt[index_0], system_call_linkage); // check system_call_linkage.S -- Yan 11/4
		}
	}
}
