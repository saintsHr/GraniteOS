#include "dusk/memory/pmm.h"
#include "dusk/multiboot.h"
#include <stdbool.h>
#include <stdint.h>

extern uint32_t _kernel_end;
extern uint32_t _kernel_start;

static uint32_t* pmm_bitmap;

static uint32_t pmm_total_pages;
static uint32_t pmm_used_pages;
static uint32_t pmm_bitmap_end;

static inline void pmm_set_bit(uint32_t page) {
    pmm_bitmap[page / 32] |= (1 << (page % 32));
}

static inline void pmm_clear_bit(uint32_t page) {
    pmm_bitmap[page / 32] &= ~(1 << (page % 32));
}

static inline void pmm_reserve_region(uint32_t addr, uint32_t len) {
	uint32_t page_start = addr / PMM_PAGE_SIZE;
	uint32_t page_end = (addr + len + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE;

	for (uint32_t i = page_start; i < page_end; i++) {
		pmm_set_bit(i);
	}
}

static inline bool pmm_test_bit(uint32_t page) {
    return pmm_bitmap[page / 32] & (1 << (page % 32));
}

void pmm_init(const struct multiboot_info* mbi) {
	pmm_bitmap = &_kernel_end;
	
	struct multiboot_mmap_entry* entry;
	entry = (struct multiboot_mmap_entry*) mbi->mmap_addr;

	uint64_t highest_addr = 0;

	while ((uint32_t)entry < mbi->mmap_addr + mbi->mmap_length) {
		uint64_t top = entry->addr + entry->len;
		if (top > highest_addr) highest_addr = top;

    	entry = (struct multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
	}

	pmm_total_pages = highest_addr / PMM_PAGE_SIZE;

	pmm_bitmap_end = (uint32_t)&_kernel_end + (pmm_total_pages / 8);

	uint32_t bitmap_size = pmm_total_pages / 32;
	for (uint32_t i = 0; i < bitmap_size; i++) {
	    pmm_bitmap[i] = 0xFFFFFFFF;
	}

	entry = (struct multiboot_mmap_entry*) mbi->mmap_addr;
	while ((uint32_t)entry < mbi->mmap_addr + mbi->mmap_length) {
		if (entry->type == 1) {
			uint32_t page_start = entry->addr / PMM_PAGE_SIZE;
			uint32_t page_end = (entry->addr + entry->len) / PMM_PAGE_SIZE;

			for (uint32_t i = 0; i < page_end - page_start; i++) {
				pmm_clear_bit(page_start + i);
			}
		}

		entry = (struct multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
	}

	// bitmap region
	pmm_reserve_region(0, pmm_bitmap_end);

	// kernel code region
	pmm_reserve_region(
    	(uint32_t)&_kernel_start,
    	(uint32_t)&_kernel_end - (uint32_t)&_kernel_start
	);

	// sentinel value (do not remove!)
	pmm_reserve_region(0x00000000, 1);
}

void pmm_free_page(void* addr) {
	uint32_t page = (uint32_t)addr / PMM_PAGE_SIZE;
	pmm_clear_bit(page);
	pmm_used_pages--;
}

void* pmm_alloc_page(void) {
	for (uint32_t i = 0; i < pmm_total_pages; i++) {
		if (!pmm_test_bit(i)) {
			pmm_set_bit(i);
			pmm_used_pages++;
			return (uint32_t*)(i * PMM_PAGE_SIZE);
		}
	}

	return 0;
}