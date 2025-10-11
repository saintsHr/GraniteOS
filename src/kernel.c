#include "terminal.h"
#include "helpers.h"
#include "shell.h"
#include "colors.h"
#include "keyboard.h"

typedef struct multiboot_info {
    uint32_t flags;            // Flags indicating which fields are valid
    uint32_t mem_lower;        // Amount of lower memory (below 1 MB) in KB
    uint32_t mem_upper;        // Amount of upper memory (above 1 MB) in KB
    uint32_t boot_device;      // Boot device (BIOS disk device)
    uint32_t cmdline;          // Pointer to the command line string
    uint32_t mods_count;       // Number of modules loaded
    uint32_t mods_addr;        // Pointer to the first module structure
    uint32_t syms[4];          // Symbol table information (deprecated)
    uint32_t mmap_length;      // Size of the memory map
    uint32_t mmap_addr;        // Pointer to the memory map
    uint32_t drives_length;    // Size of the BIOS drive information
    uint32_t drives_addr;      // Pointer to the BIOS drive information
    uint32_t config_table;     // Pointer to the ROM configuration table
    uint32_t boot_loader_name; // Pointer to the bootloader name string
    uint32_t apm_table;        // Pointer to the APM (Advanced Power Management) table
    uint32_t vbe_control_info; // VBE (VESA BIOS Extensions) control information
    uint32_t vbe_mode_info;    // VBE mode information
    uint16_t vbe_mode;         // VBE mode number
    uint16_t vbe_interface_seg; // VBE interface segment
    uint16_t vbe_interface_off; // VBE interface offset
    uint16_t vbe_interface_len; // VBE interface length
} multiboot_info_t;

void disInfo(multiboot_info_t* mb_info){
    char* info_head = "[ INFO ] ";

    // Print multiboot1 flags
    print(info_head ,VGA_CYAN);
    print("Flags: " ,VGA_LGRAY);
    print(uint_to_str(mb_info->flags), VGA_LGRAY);
    print("\n", VGA_LGRAY);
    wait(10000000);

    // Print total RAM in MB
    char* strmem = uint_to_str((mb_info->mem_lower + mb_info->mem_upper) / 1024);
    print(info_head, VGA_CYAN);
    print("Total memory: ", VGA_LGRAY);
    print(strmem, VGA_LGRAY);
    print(" MB\n", VGA_LGRAY);
    wait(10000000);

    // Print boot device if available
    if (mb_info->flags & 0x2) {
        print(info_head, VGA_CYAN);
        print("Boot device: 0x", VGA_LGRAY);
        print(uint_to_str(mb_info->boot_device), VGA_LGRAY);
        print("\n", VGA_LGRAY);
    }
    wait(10000000);

    // Print bootloader name if available
    if (mb_info->flags & 0x200) {
        print(info_head, VGA_CYAN);
        print("Bootloader: ", VGA_LGRAY);
        print((char*)mb_info->boot_loader_name, VGA_LGRAY);
        print("\n", VGA_LGRAY);
    }
    wait(10000000);

    // Print drives BIOS info if available
    if (mb_info->flags & 0x80) {
        print(info_head, VGA_CYAN);
        print("Drives info at: 0x", VGA_LGRAY);
        print(uint_to_str(mb_info->drives_addr), VGA_LGRAY);
        print(" (length: ", VGA_LGRAY);
        print(uint_to_str(mb_info->drives_length), VGA_LGRAY);
        print(" bytes)\n", VGA_LGRAY);
    }
    wait(10000000);

    // Print VBE info if available
    if (mb_info->flags & 0x800) {
        print(info_head, VGA_CYAN);
        print("VBE mode: ", VGA_LGRAY);
        print(uint_to_str(mb_info->vbe_mode), VGA_LGRAY);
        print("\n", VGA_LGRAY);

        print(info_head, VGA_CYAN);
        print("VBE mode info at: 0x", VGA_LGRAY);
        print(uint_to_str(mb_info->vbe_mode_info), VGA_LGRAY);
        print("\n", VGA_LGRAY);
    }
}
void welcome(){
    print("=============================", 0x07);
    print("\n [Welcome to GraniteOS]\n\n", 0x07);
    print(" Version:        v0.0.0\n", 0x07);
    print("=============================", 0x07);
    move_cursor(0, 6);
    print("\n", VGA_BLACK);
    prompt();
}

void kernel_main(uint32_t magic, multiboot_info_t* mb_info){
    if (magic != 0x2BADB002) return;

    disInfo(mb_info);

    print("\n\nBooting", VGA_LGRAY);
    wait(75000000);
    print(" .", VGA_LGRAY);
    wait(75000000);
    print(" .", VGA_LGRAY);
    wait(75000000);
    print(" .", VGA_LGRAY);
    wait(150000000);

    clear(VGA_LGRAY);
    welcome();

    while (1){
        keyboard_handler();
    }
}