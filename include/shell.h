#pragma once

#include <stdbool.h>

extern char* cmds[];

void prompt();
bool chkcmd(char* cmds[], char* cmd);
void callcmd(const char* cmd);
