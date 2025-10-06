#include "shell.h"
#include "terminal.h"
#include "colors.h"
#include "helpers.h"

#define PROMPT "Granite> "

char* cmds[] = {
    "clear"
};

void prompt(){
    print(PROMPT , VGA_LGRAY);
}

bool chkcmd(char* cmds[], char* cmd){
    for (uint16_t i = 0; i < 1000; i++){
        if (cmd == cmds[i]) return true;
    }
    return false;
}

void callcmd(const char* cmd){
    if (strcmp(cmd, "clear")){
        clear(VGA_BLACK);
    }
}