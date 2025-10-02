#include "terminal/terminal.h"

void kernel_main(void){
    terminal_initialize();
    terminal_writestring("Hello from GraniteOS!");
}