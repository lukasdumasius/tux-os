#include "paging.h"

// ####### Yan 10/17
/* init_paging
 * 
 * Description: Initialize Paging. The first entry in PDE is mapped to PTE,
 *          that PTE is mapped to 4KB memory, which 0xB8000- 0xB9000 is video(lib.c)
 *          All other entries in PDE are mapped directly to 4MB memory. 
 *          From 4MB -8MB is the kernel.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.c, paging.h, x86_desc.h, x86_desc.c, kernel,c
 */
void init_paging()
{
    uint32_t i;
    // we first do page directory
    page_directory.present = 1;    // mapping for video
    page_directory.read_write = 1; // always r/w
    page_directory.user_supervisor = 0;		// *** CHANGED BACK TO 0 FROM 1
    page_directory.page_write_through = 0;
    page_directory.page_cashed_disabled = 0;
    page_directory.accessed = 0;
    page_directory.reserved_zero = 0;
    page_directory.page_size = 0;  // indicate 4KB pde
    page_directory.global_bit = 0; // not set for video
    page_directory.available = 0;
    page_directory.page_table_base_addr = (int)page_table >> 12;
    // cuz we don't care last 12bit, only first 20 bits determine the table
    // E.g. from [31:0], we only need to know [31:12] to find the table
    //      And [31:12] is what we store in page_table_base_addr

    // The second part is to initailze all other entries(all 4MB) in page directory
    // Only the 1st 4MB page in PD is enable for kernel, all other not present
    for (i = 0; i < NUM_PAGE_DIRECTORY_ENTRY - 1; i++)
    {
        page_directory_remain[i].present = (i == 0) ? 1 : 0;    // explained above
        page_directory_remain[i].read_write = (i == 0) ? 1 : 0; // explained above
        page_directory_remain[i].user_supervisor = 0;           // 0 for both kernel and video, 1 for user
        page_directory_remain[i].page_write_through = 0;
        page_directory_remain[i].page_cashed_disabled = (i == 0) ? 1 : 0; // 1 for kernel, x86_desc.h
        page_directory_remain[i].accessed = 0;
        page_directory_remain[i].dirty = 0;
        page_directory_remain[i].page_size = 1;                 // indicate 4MB pde
        page_directory_remain[i].global_bit = (i == 0) ? 1 : 0; // set for only kernel
        page_directory_remain[i].available = 0;
        page_directory_remain[i].page_attribute_table = 0;
        page_directory_remain[i].reserved = 0;
        page_directory_remain[i].page_base_addr = i + 1; // +1 cuz the first entry above
    }

    // Now PDE are done, we move on to PDT(memory)
    //      We only initialize the page starting at 0xB8000, check lib.c for details
    //      If remove 12 last bits from B8000, we get B8
    for (i = 0; i < NUM_PAGE_TABLE_ENTRY; i++)
    {
        page_table[i].present = (i == 0xB8) ? 1 : 0;    // explained above
        page_table[i].read_write = (i == 0xB8) ? 1 : 0; // explained above
        page_table[i].user_supervisor = 0;              // 0 for both kernel and video, 1 for user
        page_table[i].page_write_through = 0;
        page_table[i].page_cashed_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_attribute_table = 0;
        page_table[i].global_bit = 0;
        page_table[i].available = 0;
        page_table[i].page_base_addr = i;
        // since we only have one table and only mapped to the 0th entry of PD,
        // 12 MSB bit of page_base_addr is 0, we only need the 12 LSB.
    }

    // Finally we need to set CR0, CR3, CR4
    //      Reference: https://courses.grainger.illinois.edu/ece391/fa2020/secure/fa20_lectures/ECE391_Lecture15(F).pdf Page 18
    paging_enable((uint32_t)&page_directory); // Defined in x86_desc.S -- Yan 10/24

    return;
}
