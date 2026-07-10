#pragma once

#include <stdint.h>

#define VMM_PAGE_PRESENT  0x00000001
#define VMM_PAGE_WRITE    0x00000002
#define VMM_PAGE_USER     0x00000004

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
void vmm_map_range(uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags, page_directory_t* directory);
void vmm_unmap_range(uint32_t virt_start, uint32_t virt_end, page_directory_t* directory);

uint32_t vmm_get_physical(uint32_t virt_addr, page_directory_t* directory);