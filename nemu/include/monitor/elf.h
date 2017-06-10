#ifndef _ELF_H_
#define _ELF_H_

#include"common.h"
uint32_t find_str(char *args);
void load_elf_tables(int argc, char *argv[]);
int backtrace();

#endif
