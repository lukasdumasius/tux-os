#ifndef ID_TABLE
#define ID_TABLE

#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "system_call.h" // 10 SC in linkage defined in SC.h
#include "system_call.h" // in order to use halt()

#ifndef ASM
void prepare_idt();
extern void system_call_linkage(); // ACTUAL SC linkage see system_call_linkage.S for details -- Yan 11/4

extern void default_exception_handler_linkage();
extern void default_reserved_exception_handler_linkage();
extern void divide_error_handler_linkage();
extern void debug_handler_linkage();
extern void NMI_interrupt_handler_linkage();
extern void Breakpoint_handler_linkage();
extern void Overflow_handler_linkage();
extern void Bound_range_exceeded_handler_linkage();
extern void invalid_opcode_handler_linkage();
extern void device_not_availble_handler_linkage();
extern void double_fault_handler_linkage();
extern void coprocessor_segment_overrun_handler_linkage();
extern void invalid_TSS_handler_linkage();
extern void segment_not_present_handler_linkage();
extern void stack_segment_fault_handler_linkage();
extern void general_protection_handler_linkage();
extern void page_fault_handler_linkage();
extern void floating_point_error_handler_linkage();
extern void alignment_check_handler_linkage();
extern void machine_check_handler_linkage();
extern void SIMD_floating_point_exception_handler_linkage();
extern void kbd_linkage();
extern void rtc_linkage();
extern void pit_linkage();

#endif

#endif
