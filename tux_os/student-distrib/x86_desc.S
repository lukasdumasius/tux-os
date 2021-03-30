# x86_desc.S - Set up x86 segment descriptors, descriptor tables
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

.text

.globl ldt_size, tss_size
.globl gdt_desc, ldt_desc, tss_desc
.globl tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl gdt_ptr, gdtr
.globl idt_desc_ptr, idt

.globl screen_coord_storage

# Yan's global variable 10/24
.globl page_directory, page_directory_remain, page_table, paging_enable, flush_TLB, page_table_2_vidmap
.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:

    .align  16
gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0

    # NULL entry
    .quad 0

    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:

    .align 16
	

# ####### Yan's Version of gdtr
#  gdtr = 32-bit base addr + 16-bit gdt size
#       Reference: https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-74.html
#       Since little endian, first initialize 16-bit size, then 32-bit addr
gdtr:
    .word gdt_bottom - gdt - 1 # mimic idt_desc_ptr
    .long gdt                  # starting addr of gdt, see above
# #######

    .align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding
idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt


    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr

idt_bottom:

# ####### Yan's Version of paging structure in kernel
# Need to align page directory
.align BOUNDRY_4KB
page_directory:
    # The first page_directory entry is in the format of 4kb
    .long 0
page_directory_remain:
    # All remain 1023 page_directory entries are in the format of 4mb
	.rept NUM_PAGE_DIRECTORY_ENTRY - 1
	.long 0
	.endr

# Need to align page table
.align BOUNDRY_4KB
page_table:
    # All 1024 page_table entries are in the format of 4kb, this used to say 4mb for some reason?
	.rept NUM_PAGE_TABLE_ENTRY
	.long 0
	.endr
page_table_2_vidmap:
    # All 1024 page_table entries are in the format of 4kb, same here
	.rept NUM_PAGE_TABLE_ENTRY
	.long 0
	.endr

# Yan's way to work around in-line assembly and initialize 3 regs here -- 10/24
paging_enable:
    PUSHL %EAX
    MOVL 8(%ESP), %EAX      # EAX has the argument -- start of PD

    # Update CR3 to point at the head of PD
    MOVL %EAX, %CR3    

    # Next PSE (page size extension): bit 4 in CR4 (control register 4)
    MOVL %CR4, %EAX       
    ORL $0x10, %EAX   # make bit4 = 1
    MOVL %EAX, %CR4       

    # Finally do PG (page enable): bit 31 in CR0 (control register 0)
    MOVL %CR0, %EAX       
    ORL $0x80000000, %EAX  # make bit31 = 1
    MOVL %EAX, %CR0 

    POPL %EAX
    RET

# Yan's way to flush TLB -- 10/29
# REFERENCE: https://wiki.osdev.org/TLB
# We only need to reload CR3, PUSHFL, EAX is just a precaution but might be unnecessary
flush_TLB:
    PUSHFL
    PUSHL %EAX
    MOVL %CR3, %EAX
    MOVL %EAX, %CR3
    POPL %EAX
    POPFL
    RET


# Storage for holding Screen coords of not currently
# active (in view) terminals
.align 16
screen_coord_storage:
.rept 3
.quad 0
.endr
