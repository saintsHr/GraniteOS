#include "dusk/memory/vmm.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/mem_util.h"
#include "dusk/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VMM_RECURSIVE_INDEX  1023
#define VMM_TEMP_DIR_INDEX   1021
#define VMM_TEMP_TAB_INDEX   1022

#define PAGE_DIRECTORY_VIRT \
    ((page_directory_t*)0xFFFFF000)

#define PAGE_TABLE_VIRT(index) \
    ((page_table_t*)(0xFFC00000 + ((index) * PMM_PAGE_SIZE)))

#define TEMP_DIR_VIRT \
    ((page_directory_t*)(0xFFC00000 + (VMM_TEMP_DIR_INDEX * PMM_PAGE_SIZE)))

#define TEMP_TAB_VIRT \
    ((page_table_t*)(0xFFC00000 + (VMM_TEMP_TAB_INDEX * PMM_PAGE_SIZE)))

static inline uint16_t get_directory_index(uint32_t addr);
static inline uint16_t get_table_index(uint32_t addr);
static inline uint16_t get_offset(uint32_t addr);

static inline void enable_paging(void);
static inline void disable_paging(void);

static inline void load_cr3(page_directory_t* directory);

static inline void tlb_flush(void);
static inline void invlpg(uint32_t addr);

static inline uint32_t current_cr3(void);
static inline bool is_active_directory(page_directory_t* dir);

static page_directory_t* map_target_directory(page_directory_t* dir);
static page_table_t* map_target_table(page_directory_t* dir, uint32_t table_phys, uint16_t dir_index);

static void map_boot_page(page_directory_t* dir_phys, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
static void map_boot_range(page_directory_t* dir_phys, uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags);
static void map_boot();

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

page_directory_t* vmm_kernel_directory;

void vmm_init() {
    disable_paging();

    vmm_kernel_directory = pmm_alloc_page();
    if (!vmm_kernel_directory) kpanic("[Memory] Out of physical memory", NULL);
    set_memory(vmm_kernel_directory, 0, PMM_PAGE_SIZE);

    vmm_kernel_directory->entries[VMM_RECURSIVE_INDEX] = (
        (uint32_t)vmm_kernel_directory |
        VMM_PAGE_PRESENT |
        VMM_PAGE_WRITE
    );

    map_boot();

    load_cr3(vmm_kernel_directory);

    enable_paging();
}

page_directory_t* vmm_create_directory(void) {
    page_directory_t* dir_phys = pmm_alloc_page();
    if (!dir_phys) kpanic("[Memory] Out of physical memory", NULL);

    page_directory_t* dir_virt = map_target_directory(dir_phys);

    set_memory(dir_virt, 0, PMM_PAGE_SIZE);

    dir_virt->entries[VMM_RECURSIVE_INDEX] = (
        (uint32_t)dir_phys |
        VMM_PAGE_PRESENT |
        VMM_PAGE_WRITE
    );

    page_directory_t* kernel_dir_virt = map_target_directory(vmm_kernel_directory);
    for (uint32_t i = 768; i < 1023; i++) {
        dir_virt->entries[i] = kernel_dir_virt->entries[i];
    }

    return dir_phys;
}

void vmm_map(page_directory_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);

    page_directory_t* dir_virt = map_target_directory(dir);

    if (!(dir_virt->entries[dir_index] & VMM_PAGE_PRESENT)) {
        uint32_t table_phys = (uint32_t)pmm_alloc_page();

        page_table_t* new_tab_virt = map_target_table(dir, table_phys, dir_index);
        set_memory(new_tab_virt, 0, PMM_PAGE_SIZE);

        dir_virt = map_target_directory(dir);

        dir_virt->entries[dir_index] = (
            table_phys |
            VMM_PAGE_PRESENT |
            VMM_PAGE_WRITE
        );
    }

    uint32_t table_phys = dir_virt->entries[dir_index] & ~0xFFF;
    page_table_t* tab_virt = map_target_table(dir, table_phys, dir_index);

    tab_virt->entries[tab_index] =
        phys_addr |
        flags |
        VMM_PAGE_PRESENT;

    if (is_active_directory(dir)) {
        invlpg(virt_addr);
    }
}

void vmm_unmap(page_directory_t* dir, uint32_t virt_addr) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);

    page_directory_t* dir_virt = map_target_directory(dir);

    if (!(dir_virt->entries[dir_index] & VMM_PAGE_PRESENT)) return;

    uint32_t table_phys = dir_virt->entries[dir_index] & ~0xFFF;
    page_table_t* tab_virt = map_target_table(dir, table_phys, dir_index);

    tab_virt->entries[tab_index] = 0x00000000;

    bool empty = true;
    for (uint32_t i = 0; i < sizeof(tab_virt->entries) / sizeof(page_entry_t); i++) {
        if (tab_virt->entries[i] & VMM_PAGE_PRESENT) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pmm_free_page((void*)table_phys);
        dir_virt = map_target_directory(dir);
        dir_virt->entries[dir_index] = 0x00000000;
    }

    if (is_active_directory(dir)) {
        tlb_flush();
    }
}

void vmm_map_range(page_directory_t* dir, uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags) {
    uint32_t virt_start_aligned = align_down(virt_start, PMM_PAGE_SIZE);
    uint32_t virt_end_aligned = align_up(virt_end, PMM_PAGE_SIZE);

    uint32_t phys_current = align_down(phys_start, PMM_PAGE_SIZE);

    for (
        uint32_t i = virt_start_aligned;
        i < virt_end_aligned;
        i += PMM_PAGE_SIZE
    ) {
        vmm_map(dir, i, phys_current, flags);
        phys_current += PMM_PAGE_SIZE;
    }
}

void vmm_unmap_range(page_directory_t* dir, uint32_t virt_start, uint32_t virt_end) {
    uint32_t virt_start_aligned = align_down(virt_start, PMM_PAGE_SIZE);
    uint32_t virt_end_aligned = align_up(virt_end, PMM_PAGE_SIZE);

    for (
        uint32_t i = virt_start_aligned;
        i < virt_end_aligned;
        i += PMM_PAGE_SIZE
    ) {
        vmm_unmap(dir, i);
    }
}

uint32_t vmm_get_physical(page_directory_t* dir, uint32_t virt_addr) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);
    uint16_t offset = get_offset(virt_addr);

    page_directory_t* dir_virt = map_target_directory(dir);

    if (!(dir_virt->entries[dir_index] & VMM_PAGE_PRESENT)) return 0x00000000;

    uint32_t table_phys = dir_virt->entries[dir_index] & ~0xFFF;
    page_table_t* tab_virt = map_target_table(dir, table_phys, dir_index);

    if (!(tab_virt->entries[tab_index] & VMM_PAGE_PRESENT)) return 0x00000000;

    uint32_t phys_addr = (tab_virt->entries[tab_index] & ~0xFFF) + offset;
    return phys_addr;
}

static inline uint32_t current_cr3(void) {
    uint32_t cr3 = 0;
    asm volatile ("mov %%cr3, %0" : "=a"(cr3));
    return cr3;
}

static inline bool is_active_directory(page_directory_t* dir) {
    return ((uint32_t)dir) == current_cr3();
}

static page_directory_t* map_target_directory(page_directory_t* dir) {
    if (is_active_directory(dir)) {
        return PAGE_DIRECTORY_VIRT;
    }

    page_directory_t* active_dir = PAGE_DIRECTORY_VIRT;

    active_dir->entries[VMM_TEMP_DIR_INDEX] = (
        (uint32_t)dir |
        VMM_PAGE_PRESENT |
        VMM_PAGE_WRITE
    );

    invlpg((uint32_t)TEMP_DIR_VIRT);

    return TEMP_DIR_VIRT;
}

static page_table_t* map_target_table(page_directory_t* dir, uint32_t table_phys, uint16_t dir_index) {
    if (is_active_directory(dir)) {
        return PAGE_TABLE_VIRT(dir_index);
    }

    page_directory_t* active_dir = PAGE_DIRECTORY_VIRT;

    active_dir->entries[VMM_TEMP_TAB_INDEX] = (
        table_phys |
        VMM_PAGE_PRESENT |
        VMM_PAGE_WRITE
    );

    invlpg((uint32_t)TEMP_TAB_VIRT);

    return TEMP_TAB_VIRT;
}

static void map_boot_page(page_directory_t* dir_phys, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint16_t dir_index = get_directory_index(virt_addr);
    uint16_t tab_index = get_table_index(virt_addr);

    if (!(dir_phys->entries[dir_index] & VMM_PAGE_PRESENT)) {
        page_table_t* table_phys = pmm_alloc_page();
        if (!table_phys) kpanic("[Memory] Out of physical memory", NULL);

        set_memory(table_phys, 0, PMM_PAGE_SIZE);

        dir_phys->entries[dir_index] = (
            (uint32_t)table_phys |
            VMM_PAGE_PRESENT |
            VMM_PAGE_WRITE
        );
    }

    page_table_t* tab_phys = (page_table_t*)(dir_phys->entries[dir_index] & ~0xFFF);

    tab_phys->entries[tab_index] =
        phys_addr |
        flags |
        VMM_PAGE_PRESENT;
}

static void map_boot_range(page_directory_t* dir_phys, uint32_t virt_start, uint32_t virt_end, uint32_t phys_start, uint32_t flags) {
    uint32_t virt_start_aligned = align_down(virt_start, PMM_PAGE_SIZE);
    uint32_t virt_end_aligned = align_up(virt_end, PMM_PAGE_SIZE);

    uint32_t phys_current = align_down(phys_start, PMM_PAGE_SIZE);

    for (
        uint32_t i = virt_start_aligned;
        i < virt_end_aligned;
        i += PMM_PAGE_SIZE
    ) {
        map_boot_page(dir_phys, i, phys_current, flags);
        phys_current += PMM_PAGE_SIZE;
    }
}

static inline void invlpg(uint32_t addr) {
    asm volatile(
        "invlpg (%0)"
        :
        : "r"(addr)
        : "memory"
    );
}

static inline void tlb_flush(void) {
    uint32_t cr3 = 0x00000000;

    asm volatile ("mov %%cr3, %0" : "=a"(cr3));
    asm volatile ("mov %0, %%cr3" : : "a"(cr3));
}

static inline void load_cr3(page_directory_t* directory) {
    directory->entries[VMM_RECURSIVE_INDEX] = (
        (uint32_t)directory |
        VMM_PAGE_PRESENT |
        VMM_PAGE_WRITE
    );

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

static void map_boot() {
    // maps kernel
    map_boot_range(
        vmm_kernel_directory,
        (uint32_t)&_kernel_start,
        (uint32_t)&_kernel_end,
        (uint32_t)&_kernel_start,
        VMM_PAGE_PRESENT | VMM_PAGE_WRITE
    );

    // maps bitmap
    map_boot_range(
        vmm_kernel_directory,
        pmm_get_bitmap_start(),
        pmm_get_bitmap_end(),
        pmm_get_bitmap_start(),
        VMM_PAGE_PRESENT | VMM_PAGE_WRITE
    );

    // maps vga text mode
    map_boot_range(
        vmm_kernel_directory,
        0x000B8000,
        0x000B9000,
        0x000B8000,
        VMM_PAGE_PRESENT | VMM_PAGE_WRITE
    );
}