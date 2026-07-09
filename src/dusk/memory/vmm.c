#include "dusk/memory/vmm.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/util.h"
#include "dusk/kernel.h"
#include <stdint.h>
#include <stddef.h>

static inline uint16_t get_directory_index(uint32_t addr);
static inline uint16_t get_table_index(uint32_t addr);
static inline uint16_t get_offset(uint32_t addr);

static page_directory_t* kernel_directory = NULL;

void vmm_init(void) {
    kernel_directory = pmm_alloc_page();
    if (!kernel_directory) kpanic("[Memory] Out of physical memory");
    set_memory(kernel_directory, 0, PMM_PAGE_SIZE);
}

void vmm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);
    
    page_directory_t* dir = kernel_directory;
    page_table_t* tab = NULL;

    if (kernel_directory->entries[dir_index] & PAGE_PRESENT) {
        tab = (page_table_t*)(
            dir->entries[dir_index] &
            ~0xFFF
        );
    } else {
        tab = pmm_alloc_page();
        set_memory(tab, 0, sizeof(page_table_t));

        page_entry_t dir_entry = (uint32_t)tab | flags | PAGE_PRESENT;
        dir->entries[dir_index] = dir_entry;
    }

    page_entry_t tab_entry = phys_addr | flags | PAGE_PRESENT;
    tab->entries[tab_index] = tab_entry;
}

static inline uint16_t get_directory_index(uint32_t addr) {
    return (addr >> 22);
}

static inline uint16_t get_table_index(uint32_t addr) {
    return (addr >> 12) & 0x3FF;
}

static inline uint16_t get_offset(uint32_t addr) {
    return addr & 0xFFF;
}