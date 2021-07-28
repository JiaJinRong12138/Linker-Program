#ifndef LINKER_GUARD
#define LINKER_GUARD

#include <stdint.h>
#include <stdlib.h>

// 定义section header name的字符最大为32
#define MAX_CHAR_SECTION_NAME (32)

// section header table   sh_name, sh_addr, sh_offset, sh_size
typedef struct 
{
    /* data */
    char sh_name[MAX_CHAR_SECTION_NAME];
    uint64_t sh_addr;
    uint64_t sh_offset;     
    uint64_t sh_size;
}sht_entry_t;

#define MAX_CHAR_SYMBOL_NAME (64)

// 因为需要bind的说明Local Global Weak(弱符号)等等
typedef enum 
{
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK
}st_bind_t;
// 同上
typedef enum
{
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC
}st_type_t;
// symbol table  st_name, bind, type, st_shndex, st_value, st_size
typedef struct 
{
    /* data */
    char st_name[MAX_CHAR_SYMBOL_NAME];
    st_bind_t bind;
    st_type_t type;
    char st_shndex[MAX_CHAR_SYMBOL_NAME];
    uint64_t st_value;  // in section offset
    uint64_t st_size;   // count of lines of symbol
}st_entry_t;

#define MAX_ELF_FILE_LENGTH (64) // max 64 effective lines  行
#define MAX_ELF_FILE_WIDTH (128) // max 128 chars per lines 列

typedef struct
{
    /* data */
    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;

    sht_entry_t *sht;
    uint64_t symtab_count;
    st_entry_t* symt;
}elf_t;

void parser_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

#endif