#include "dusk/interrupts/idt.h"
#include "dusk/interrupts/pic.h"
#include "dusk/memory/vmm.h"
#include "dusk/memory/pmm.h"
#include "dusk/memory/heap.h"
#include "dusk/multiboot.h"
#include "dusk/vga.h"
#include "dusk/serial.h"

static inline void hang(void);

void kpanic(const char* msg, const uint32_t* addr) {
    vga_cursor_disable();
    vga_clear();

    // KERNEL PANIC
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    vga_move(0, 0);
    vga_print("KERNEL PANIC");

    // Reason: [msg]
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_move(0, 2);
    vga_print("Reason: ");
    vga_print(msg);
    vga_print("\n");

    if (addr) {
        // Address: [addr]
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_move(0, 3);
        vga_print("Address: ");
        vga_print_hex(*addr);
        vga_print("\n");
    }

    hang();
}

void kinit(const struct multiboot_info* mbi) {
    __asm__ volatile ("cli");

    // debug
    serial_init();

    // interrupts
    idt_init();
    pic_remap(0x20, 0x28);

    // memory
    pmm_init(mbi);
    vmm_init();
    heap_init();

    __asm__ volatile ("sti");
}

void kmain(struct multiboot_info* mbi, uint32_t magic) {
    if (magic != 0x2BADB002) {
        kpanic("[Boot]: Invalid magic number.", NULL);
    }

    if (mbi == NULL || (uint32_t)mbi < 0x500) {
        kpanic("[Boot]: Invalid MBI provided.", NULL);
    }

    kinit(mbi);

    while (1);
}

static inline void hang(void) {
    __asm__ volatile ("cli" ::: "memory");
    while (1) __asm__ volatile ("hlt");
}