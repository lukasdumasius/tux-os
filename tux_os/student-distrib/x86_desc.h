/* x86_desc.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _X86_DESC_H
#define _X86_DESC_H

#include "types.h"

/* Segment selector values */
#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define USER_CS 0x0023
#define USER_DS 0x002B
#define KERNEL_TSS 0x0030
#define KERNEL_LDT 0x0038

// Yan's define
#define NUM_PAGE_DIRECTORY_ENTRY 1024 // PDE has most 1024 entries
#define NUM_PAGE_TABLE_ENTRY 1024     // PDE has most 1024 entries
#define BOUNDRY_4KB 4096              // 4KB = 4*1024B = 4096B

/* Size of the task state segment (TSS) */
#define TSS_SIZE 104

/* Number of vectors in the interrupt descriptor table (IDT) */
#define NUM_VEC 256

#ifndef ASM

/* This structure is used to load descriptor base registers
 * like the GDTR and IDTR */
typedef struct x86_desc
{
    uint16_t padding;
    uint16_t size;
    uint32_t addr;
} x86_desc_t;

//*** adding structures for Checkpoint 5 (Multiple terminals)...
typedef struct screen_coord_info
{
    int screen_x;
    int screen_y;
} screen_coord_info_t;
extern screen_coord_info_t screen_coord_storage[3]; //3 bc number of terminals

/* This is a segment descriptor.  It goes in the GDT. */
typedef struct seg_desc
{
    union {
        uint32_t val[2];
        struct
        {
            uint16_t seg_lim_15_00;
            uint16_t base_15_00;
            uint8_t base_23_16;
            uint32_t type : 4;
            uint32_t sys : 1;
            uint32_t dpl : 2;
            uint32_t present : 1;
            uint32_t seg_lim_19_16 : 4;
            uint32_t avail : 1;
            uint32_t reserved : 1;
            uint32_t opsize : 1;
            uint32_t granularity : 1;
            uint8_t base_31_24;
        } __attribute__((packed));
    };
} seg_desc_t;

/* TSS structure */
typedef struct __attribute__((packed)) tss_t
{
    uint16_t prev_task_link;
    uint16_t prev_task_link_pad;

    uint32_t esp0;
    uint16_t ss0;
    uint16_t ss0_pad;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_pad;

    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_pad;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t es_pad;

    uint16_t cs;
    uint16_t cs_pad;

    uint16_t ss;
    uint16_t ss_pad;

    uint16_t ds;
    uint16_t ds_pad;

    uint16_t fs;
    uint16_t fs_pad;

    uint16_t gs;
    uint16_t gs_pad;

    uint16_t ldt_segment_selector;
    uint16_t ldt_pad;

    uint16_t debug_trap : 1;
    uint16_t io_pad : 15;
    uint16_t io_base_addr;
} tss_t;

// ***** Yan 10/16
//////////////////////////////////////////////////////////////////////////////////
// Got inspired by the above seg_desc
// Use union cuz we can either do bit_masking(val) or use bitfield(struct)
// Union reference: https://www.tutorialspoint.com/cprogramming/c_unions.htm
// Bitfield reference: http://www.catb.org/esr/structure-packing/ && https://atadiat.com/en/e-embedded-c-struct-union-part-2/
// PDE_4kb format reference: https://courses.grainger.illinois.edu/ece391/fa2020/secure/references/descriptors.pdf 0.2
typedef struct __attribute__((packed)) pde_4KB
{
    // pde_4kB is mapped to 4kB page table
    // union: we can either do bit_masking(val) or use bitfield(struct)

    // remember littel endian here
    uint32_t present : 1;              // 1 if mapped
    uint32_t read_write : 1;           // always 1 => read/write
    uint32_t user_supervisor : 1;      // video memory => supervisor => 0
    uint32_t page_write_through : 1;   // always 0
    uint32_t page_cashed_disabled : 1; // 1 for program code/ data pages (kernel pages, program pages).
                                       // It should be 0 for video memorypages
    uint32_t accessed : 1;             // don't care
    uint32_t reserved_zero : 1;        // reserved 0
    uint32_t page_size : 1;            // If 0, 4KB pde; if 1, 4MB pde
    uint32_t global_bit : 1;           // set the bit for kernel page(s) only
    uint32_t available : 3;            // don't care
    uint32_t page_table_base_addr : 20;
    // 1*9 + 3 + 20 = 32bit in total
} pde_4KB_t;

typedef struct __attribute__((packed)) pde_4MB
{
    // pde_4MB is directly mapped to 4kB page
    // union: we can either do bit_masking(val) or use bitfield(struct)
    // remember littel endian here
    uint32_t present : 1;              // 1 if mapped
    uint32_t read_write : 1;           // always 1 => read/write
    uint32_t user_supervisor : 1;      // video memory => supervisor => 0
    uint32_t page_write_through : 1;   // always 0
    uint32_t page_cashed_disabled : 1; // 1 for program code/ data pages (kernel pages, program pages).
                                       // It should be 0 for video memorypages
    uint32_t accessed : 1;             // don't care
    uint32_t dirty : 1;                // always 0
    uint32_t page_size : 1;            // If 0, 4KB pde; if 1, 4MB pde
    uint32_t global_bit : 1;           // set the bit for kernel page(s) only
    uint32_t available : 3;            // don't care
    uint32_t page_attribute_table : 1; // always 0
    uint32_t reserved : 9;             // must always 0
    uint32_t page_base_addr : 10;
    // 1*9 + 3 + 1 + 9 + 10 = 32bit in total

} pde_4MB_t;

// https : //gcc.gnu.org/onlinedocs/gcc-4.5.0/gcc/Type-Attributes.html
typedef struct __attribute__((packed)) pte_4KB
{
    // pte_4KB is directly mapped to 4kB page from page table!
    // union: we can either do bit_masking(val) or use bitfield(struct)

    // remember littel endian here
    uint32_t present : 1;              // 1 if mapped
    uint32_t read_write : 1;           // always 1 => read/write
    uint32_t user_supervisor : 1;      // video memory => supervisor => 0
    uint32_t page_write_through : 1;   // always 0
    uint32_t page_cashed_disabled : 1; // 1 for program code/ data pages (kernel pages, program pages).
                                       // It should be 0 for video memorypages
    uint32_t accessed : 1;             // don't care
    uint32_t dirty : 1;                // always 0
    uint32_t page_attribute_table : 1; // always 0
    uint32_t global_bit : 1;           // set the bit for kernel page(s) only
    uint32_t available : 3;            // don't care
    uint32_t page_base_addr : 20;
    // 1*9 + 3 + 20 = 32bit in total
} pte_4KB_t;

// The first page_directory entry is in the format of 4kb
// extern pde_t page_directory;
extern pde_4KB_t page_directory;
// All remain page_directory entries are in the format of 4mb
// extern pde_t page_directory_remain[NUM_PAGE_DIRECTORY_ENTRY - 1];
extern pde_4MB_t page_directory_remain[NUM_PAGE_DIRECTORY_ENTRY - 1];
// All page_table entries are in the format of 4mb
extern pte_4KB_t page_table[NUM_PAGE_TABLE_ENTRY];
extern void paging_enable(uint32_t starting_addr);
extern void flush_TLB();
//*** ADDED: New vidmap PT
extern pte_4KB_t page_table_2_vidmap[NUM_PAGE_TABLE_ENTRY];

//////////////////////////////////////////////////////////////////////////////////

/* Some external descriptors declared in .S files */
extern x86_desc_t gdt_desc;

extern uint16_t ldt_desc;
extern uint32_t ldt_size;
extern seg_desc_t ldt_desc_ptr;

// gdt_ptr points to the 2nd entry, since 0th, 1st unused
// gdtr = 32bit base addr + 16bit gdt size
extern seg_desc_t gdt_ptr, gdtr; //***added gdtr
extern uint32_t ldt;

extern uint32_t tss_size;
extern seg_desc_t tss_desc_ptr;
extern tss_t tss;

/* Sets runtime-settable parameters in the GDT entry for the LDT */
#define SET_LDT_PARAMS(str, addr, lim)                        \
    do                                                        \
    {                                                         \
        str.base_31_24 = ((uint32_t)(addr)&0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr)&0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr)&0x0000FFFF;         \
        str.seg_lim_19_16 = ((lim)&0x000F0000) >> 16;         \
        str.seg_lim_15_00 = (lim)&0x0000FFFF;                 \
    } while (0)

/* Sets runtime parameters for the TSS */
#define SET_TSS_PARAMS(str, addr, lim)                        \
    do                                                        \
    {                                                         \
        str.base_31_24 = ((uint32_t)(addr)&0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr)&0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr)&0x0000FFFF;         \
        str.seg_lim_19_16 = ((lim)&0x000F0000) >> 16;         \
        str.seg_lim_15_00 = (lim)&0x0000FFFF;                 \
    } while (0)

/* An interrupt descriptor entry (goes into the IDT) */
typedef union idt_desc_t {
    uint32_t val[2];
    struct
    {
        uint16_t offset_15_00;
        uint16_t seg_selector;
        uint8_t reserved4;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
        uint32_t size : 1;
        uint32_t reserved0 : 1;
        uint32_t dpl : 2;
        uint32_t present : 1;
        uint16_t offset_31_16;
    } __attribute__((packed));
} idt_desc_t;

/* The IDT itself (declared in x86_desc.S */
extern idt_desc_t idt[NUM_VEC];
/* The descriptor used to load the IDTR */
extern x86_desc_t idt_desc_ptr;

/* Sets runtime parameters for an IDT entry */
#define SET_IDT_ENTRY(str, handler)                                \
    do                                                             \
    {                                                              \
        str.offset_31_16 = ((uint32_t)(handler)&0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler)&0xFFFF);           \
    } while (0)

/* Load task register.  This macro takes a 16-bit index into the GDT,
 * which points to the TSS entry.  x86 then reads the GDT's TSS
 * descriptor and loads the base address specified in that descriptor
 * into the task register */
#define ltr(desc)                       \
    do                                  \
    {                                   \
        asm volatile("ltr %w0"          \
                     :                  \
                     : "r"(desc)        \
                     : "memory", "cc"); \
    } while (0)

/* Load the interrupt descriptor table (IDT).  This macro takes a 32-bit
 * address which points to a 6-byte structure.  The 6-byte structure
 * (defined as "struct x86_desc" above) contains a 2-byte size field
 * specifying the size of the IDT, and a 4-byte address field specifying
 * the base address of the IDT. */
#define lidt(desc)                \
    do                            \
    {                             \
        asm volatile("lidt (%0)"  \
                     :            \
                     : "g"(desc)  \
                     : "memory"); \
    } while (0)

/* Load the local descriptor table (LDT) register.  This macro takes a
 * 16-bit index into the GDT, which points to the LDT entry.  x86 then
 * reads the GDT's LDT descriptor and loads the base address specified
 * in that descriptor into the LDT register */
#define lldt(desc)                \
    do                            \
    {                             \
        asm volatile("lldt %%ax"  \
                     :            \
                     : "a"(desc)  \
                     : "memory"); \
    } while (0)

#endif /* ASM */

#endif /* _x86_DESC_H */
