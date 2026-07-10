#include "dusk/memory/vmm.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/mem_util.h"
#include "dusk/kernel.h"
#include "dusk/serial.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static inline uint16_t get_directory_index(uint32_t addr);
static inline uint16_t get_table_index(uint32_t addr);
static inline uint16_t get_offset(uint32_t addr);

static inline void enable_paging(void);
static inline void disable_paging(void);

static inline void load_cr3(page_directory_t* directory);

static inline void tlb_flush(void);

static page_directory_t* kernel_directory = NULL;

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void vmm_init(void) {
    kernel_directory = pmm_alloc_page();
    if (!kernel_directory) kpanic("[Memory] Out of physical memory");
    set_memory(kernel_directory, 0, PMM_PAGE_SIZE);

    // maps 16mib of memory (temporary)
    vmm_map_range(
        0x00000000,
        0x01000000,
        0x00000000,
        VMM_PAGE_PRESENT | VMM_PAGE_WRITE,
        kernel_directory
    );

    load_cr3(kernel_directory);

    enable_paging();
}

void vmm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags, page_directory_t* directory) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);
    
    page_directory_t* dir = directory;
    page_table_t* tab = NULL;

    if (dir->entries[dir_index] & VMM_PAGE_PRESENT) {
        tab = (page_table_t*)(
            dir->entries[dir_index] &
            ~0xFFF
        );
    } else {
        tab = pmm_alloc_page();
        set_memory(tab, 0, PMM_PAGE_SIZE);

        page_entry_t dir_entry = (uint32_t)tab | flags | VMM_PAGE_PRESENT;
        dir->entries[dir_index] = dir_entry;
    }

    page_entry_t tab_entry = phys_addr | flags | VMM_PAGE_PRESENT;
    tab->entries[tab_index] = tab_entry;

    tlb_flush();
}

void vmm_unmap(uint32_t virt_addr, page_directory_t* directory) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);

    page_directory_t* dir = directory;
    page_table_t* tab = NULL;

    if (!(dir->entries[dir_index] & VMM_PAGE_PRESENT)) return;

    tab = (page_table_t*)(
        dir->entries[dir_index] &
        ~0xFFF
    );

    tab->entries[tab_index] = 0x00000000;

    bool empty = true;
    for (uint32_t i = 0; i < sizeof(tab->entries) / sizeof(page_entry_t); i++) {
        if (tab->entries[i] & VMM_PAGE_PRESENT) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pmm_free_page(tab);
        dir->entries[dir_index] = 0x00000000;
    }

    tlb_flush();
}

void vmm_map_range(uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags, page_directory_t* directory) {
    uint32_t virt_start_aligned = align_down(virt_start, PMM_PAGE_SIZE);
    uint32_t virt_end_aligned = align_up(virt_end, PMM_PAGE_SIZE);

    uint32_t phys_current = align_down(phys_start, PMM_PAGE_SIZE);

    for (
        uint32_t i = virt_start_aligned;
        i < virt_end_aligned;
        i += PMM_PAGE_SIZE
    ) {
        vmm_map(i, phys_current, flags, directory);
        phys_current += PMM_PAGE_SIZE;
    }
}

void vmm_unmap_range(uint32_t virt_start, uint32_t virt_end, page_directory_t* directory) {
    uint32_t virt_start_aligned = align_down(virt_start, PMM_PAGE_SIZE);
    uint32_t virt_end_aligned = align_up(virt_end, PMM_PAGE_SIZE);

    for (
        uint32_t i = virt_start_aligned;
        i < virt_end_aligned;
        i += PMM_PAGE_SIZE
    ) {
        vmm_unmap(i, directory);
    }
}

uint32_t vmm_get_physical(uint32_t virt_addr, page_directory_t* directory) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);
    uint16_t offset = get_offset(virt_addr);

    page_directory_t* dir = directory;
    page_table_t* tab = NULL;

    if (!(dir->entries[dir_index] & VMM_PAGE_PRESENT)) return 0x00000000;
    
    tab = (page_table_t*)(
        dir->entries[dir_index] &
        ~0xFFF
    );

    if (!(tab->entries[tab_index] & VMM_PAGE_PRESENT)) return 0x00000000;

    uint32_t phys_addr = (tab->entries[tab_index] & ~0xFFF) + offset;
    return phys_addr;
}

static inline void tlb_flush(void) {
    uint32_t cr3 = 0x00000000;

    asm volatile ("mov %%cr3, %0" : "=a"(cr3));
    asm volatile ("mov %0, %%cr3" : : "a"(cr3));
}

static inline void load_cr3(page_directory_t* directory) {
    uint32_t phys_addr = (uint32_t)directory;

    asm volatile ("mov %0, %%eax" : : "a"(phys_addr));
    asm volatile ("mov %%eax, %%cr3" : : );
}

static inline void enable_paging(void) {
    uint32_t cr0 = 0x00000000;
    uint32_t pg_flag = 0x1 << 31;

    asm volatile ("mov %%cr0, %0" : "=a"(cr0));
    cr0 = cr0 | pg_flag;
    asm volatile ("mov %0, %%cr0" : : "a"(cr0));
}

static inline void disable_paging(void) {
    uint32_t cr0 = 0x00000000;
    uint32_t pg_flag = 0x1 << 31;

    asm volatile ("mov %%cr0, %0" : "=a"(cr0));
    cr0 = cr0 & ~pg_flag;
    asm volatile ("mov %0, %%cr0" : : "a"(cr0));
}

static inline uint16_t get_directory_index(uint32_t virt_addr) {
    return (virt_addr >> 22);
}

static inline uint16_t get_table_index(uint32_t virt_addr) {
    return (virt_addr >> 12) & 0x3FF;
}

static inline uint16_t get_offset(uint32_t virt_addr) {
    return virt_addr & 0xFFF;
}