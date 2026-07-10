#pragma once

#include <stdint.h>
#include <stddef.h>

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

extern page_directory_t* vmm_kernel_directory;

void vmm_init();

void vmm_map(page_directory_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void vmm_unmap(page_directory_t* dir, uint32_t virt_addr);
void vmm_map_range(page_directory_t* dir, uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags);
void vmm_unmap_range(page_directory_t* dir, uint32_t virt_start, uint32_t virt_end);
uint32_t vmm_get_physical(page_directory_t* dir, uint32_t virt_addr);

page_directory_t* vmm_create_directory(void);