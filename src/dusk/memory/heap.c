#include "dusk/memory/heap.h"
#include "dusk/memory/mem_util.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/vmm.h"

static uint32_t heap_start;
static uint32_t heap_current;
static uint32_t heap_end;

static void expand(void);

void heap_init(void) {
    heap_start = align_up(pmm_get_bitmap_end(), PMM_PAGE_SIZE);
    heap_current = heap_start;
    heap_end = heap_start;
}

void* kmalloc(uint32_t size) {
    size = align_up(size, 8);

    while (heap_current + size > heap_end) expand();

    uint32_t addr = heap_current;
    heap_current += size;
    
    return (void*)addr;
}

static void expand(void) {
    uint32_t phys = (uint32_t)pmm_alloc_page();

    vmm_map(
        vmm_kernel_directory,
        heap_end,
        phys,
        VMM_PAGE_PRESENT | VMM_PAGE_WRITE
    );

    set_memory((void*)heap_end, 0, PMM_PAGE_SIZE);
    heap_end += PMM_PAGE_SIZE;
}