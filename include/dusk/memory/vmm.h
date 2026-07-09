#pragma once

#include <stdint.h>

#define VMM_PAGE_PRESENT  0x00000001
#define VMM_PAGE_WRITE    0x00000002
#define VMM_PAGE_USER     0x00000004
#define VMM_PAGE_PWT      0x00000008
#define VMM_PAGE_PCD      0x00000010
#define VMM_PAGE_ACCESSED 0x00000020
#define VMM_PAGE_DIRTY    0x00000040
#define VMM_PAGE_PAT      0x00000080
#define VMM_PAGE_SIZE     0x00000080
#define VMM_PAGE_GLOBAL   0x00000100
#define VMM_PAGE_AVL1     0x00000200
#define VMM_PAGE_AVL2     0x00000400
#define VMM_PAGE_AVL3     0x00000800
#define VMM_PAGE_FRAME    0xFFFFF000

#define VMM_CR0_PG  0x80000000
#define VMM_CR0_WP  0x00010000

#define VMM_CR3_PWT 0x00000008
#define VMM_CR3_PCD 0x00000010

#define VMM_CR4_PSE 0x00000010
#define VMM_CR4_PAE 0x00000020
#define VMM_CR4_PGE 0x00000080

typedef uint32_t page_entry_t;

typedef struct {
    page_entry_t entries[1024];
} page_directory_t;

typedef struct {
    page_entry_t entries[1024];
} page_table_t;

void vmm_init(void);

void vmm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags, page_directory_t* directory);
void vmm_unmap(uint32_t virt_addr, page_directory_t* directory);

uint32_t vmm_get_physical(uint32_t virt_addr, page_directory_t* directory);