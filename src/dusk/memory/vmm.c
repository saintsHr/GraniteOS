#include "dusk/memory/vmm.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/util.h"
#include "dusk/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static inline uint16_t get_directory_index(uint32_t addr);
static inline uint16_t get_table_index(uint32_t addr);
static inline uint16_t get_offset(uint32_t addr);

static page_directory_t* kernel_directory = NULL;

void vmm_init(void) {
    kernel_directory = pmm_alloc_page();
    if (!kernel_directory) kpanic("[Memory] Out of physical memory");
    set_memory(kernel_directory, 0, PMM_PAGE_SIZE);
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
        set_memory(tab, 0, sizeof(page_table_t));

        page_entry_t dir_entry = (uint32_t)tab | flags | VMM_PAGE_PRESENT;
        dir->entries[dir_index] = dir_entry;
    }

    page_entry_t tab_entry = phys_addr | flags | VMM_PAGE_PRESENT;
    tab->entries[tab_index] = tab_entry;
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

static inline uint16_t get_directory_index(uint32_t virt_addr) {
    return (virt_addr >> 22);
}

static inline uint16_t get_table_index(uint32_t virt_addr) {
    return (virt_addr >> 12) & 0x3FF;
}

static inline uint16_t get_offset(uint32_t virt_addr) {
    return virt_addr & 0xFFF;
}