

.globl pit_linkage, rtc_linkage, kbd_linkage, divide_error_handler_linkage, debug_handler_linkage, NMI_interrupt_handler_linkage, Breakpoint_handler_linkage
.globl Overflow_handler_linkage, Bound_range_exceeded_handler_linkage, invalid_opcode_handler_linkage, device_not_availble_handler_linkage, double_fault_handler_linkage
.globl coprocessor_segment_overrun_handler_linkage, invalid_TSS_handler_linkage, segment_not_present_handler_linkage, stack_segment_fault_handler_linkage
.globl general_protection_handler_linkage, page_fault_handler_linkage, default_reserved_exception_handler_linkage, floating_point_error_handler_linkage
.globl alignment_check_handler_linkage, machine_check_handler_linkage, SIMD_floating_point_exception_handler_linkage, default_exception_handler_linkage

	
.globl default_exception_handler;
.globl default_reserved_exception_handler;
.globl divide_error_handler;
.globl debug_handler;
.globl NMI_interrupt_handler;
.globl Breakpoint_handler;
.globl Overflow_handler;
.globl Bound_range_exceeded_handler;
.globl invalid_opcode_handler;
.globl device_not_availble_handler;
.globl double_fault_handler;
.globl coprocessor_segment_overrun_handler;
.globl invalid_TSS_handler;
.globl segment_not_present_handler;
.globl stack_segment_fault_handler;
.globl general_protection_handler;
.globl page_fault_handler;
.globl floating_point_error_handler;
.globl alignment_check_handler;
.globl machine_check_handler;
.globl SIMD_floating_point_exception_handler;
.globl rtc_irq_handler;
.globl keyboard_handler;
.globl pit_irq_handler;


 # rtc_linkage assembly code
 # DESCRIPTION: assembly wrapper for the rtc interrupt handler
 # INPUTS: none
 # OUPTUTS: preserves the state of the registers and returns 
 #			properly from the interrupt
 # RETURN VALUE: none
 # SIDE EFFECTS: none
 #

	rtc_linkage:
		pushal;
		cld;		
		call rtc_irq_handler;		
		popal;
		iret;

 #
 # keyboard_linkage assembly code
 # DESCRIPTION: assembly wrapper for the keyboard interrupt handler
 # INPUTS: none
 # OUPTUTS: preserves the state of the registers and returns 
 #			properly from the interrupt
 # RETURN VALUE: none
 # SIDE EFFECTS: none
 #
	kbd_linkage:
		pushal;
		cld;		
		call keyboard_handler;		
		popal;
		iret;
	####

	pit_linkage:
		pushal;
		cld;
		call pit_irq_handler;
		popal;
		iret;

	####

	divide_error_handler_linkage:
		pushal;
		cld;		
		call divide_error_handler;		
		popal;
		iret;
	debug_handler_linkage:
	pushal;
		cld;		
		call debug_handler;		
		popal;
		iret;
	
	NMI_interrupt_handler_linkage:
	pushal;
		cld;		
		call NMI_interrupt_handler;		
		popal;
		iret;
	
	Breakpoint_handler_linkage:
	pushal;
		cld;		
		call Breakpoint_handler;		
		popal;
		iret;
	Overflow_handler_linkage:
	pushal;
		cld;		
		call Overflow_handler;		
		popal;
		iret;
	
	Bound_range_exceeded_handler_linkage:
	pushal;
		cld;		
		call Bound_range_exceeded_handler;		
		popal;
		iret;
	
	
	invalid_opcode_handler_linkage:
	pushal;
		cld;		
		call invalid_opcode_handler;		
		popal;
		iret;
	
	device_not_availble_handler_linkage:
	pushal;
		cld;		
		call device_not_availble_handler;		
		popal;
		iret;
	
	
	double_fault_handler_linkage:
	pushal;
		cld;		
		call double_fault_handler;		
		popal;
		iret;
	
	
	coprocessor_segment_overrun_handler_linkage:
	pushal;
		cld;		
		call coprocessor_segment_overrun_handler;		
		popal;
		iret;
	
	invalid_TSS_handler_linkage:
	pushal;
		cld;		
		call invalid_TSS_handler;		
		popal;
		iret;
	
	
	segment_not_present_handler_linkage:
	pushal;
		cld;		
		call segment_not_present_handler;		
		popal;
		iret;
	
	stack_segment_fault_handler_linkage:
	pushal;
		cld;		
		call stack_segment_fault_handler;		
		popal;
		iret;
	
	general_protection_handler_linkage:
	pushal;
		cld;		
		call general_protection_handler;		
		popal;
		iret;
	
	page_fault_handler_linkage:
	pushal;
		cld;		
		call page_fault_handler;		
		popal;
		iret;
	
	default_reserved_exception_handler_linkage:
	pushal;
		cld;		
		call default_reserved_exception_handler;		
		popal;
		iret;
	
	floating_point_error_handler_linkage:
	pushal;
		cld;		
		call floating_point_error_handler;		
		popal;
		iret;
	
	alignment_check_handler_linkage:
	pushal;
		cld;		
		call alignment_check_handler;		
		popal;
		iret;
	
	machine_check_handler_linkage:
	pushal;
		cld;		
		call machine_check_handler;		
		popal;
		iret;
	
	SIMD_floating_point_exception_handler_linkage:
	pushal;
		cld;		
		call SIMD_floating_point_exception_handler;		
		popal;
		iret;
	
	default_exception_handler_linkage:
	pushal;
		cld;		
		call default_exception_handler;		
		popal;
		iret;
