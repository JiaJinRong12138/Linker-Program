#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../headers/linker.h"

extern void parse_elf(char* filename, elf_t *elf);
extern void free_elf(elf_t *elf);

int main()
{
    elf_t elf;
    parse_elf("../text/sum.elf.txt", &elf);
    free_elf(&elf);

    return 0;
}