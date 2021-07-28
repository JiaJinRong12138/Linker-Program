#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "../headers/linker.h"

#define NULL ((void *)0)

static int parser_table_entry(char *str, char ***ent)
{
// 定义三级指针为ptr -> stack -> heap -> mem
    int count_col = 1;
    int len = strlen(str);
//  计算table的项有多少个，因为最后一个元素后面没有逗号所以要从1开始计算
    for(int i = 0; i < len; ++i)
    {
        if(str[i] == ',')
        {
            count_col++;
        }
    }
    // 申请count_col个数的*4字节指针大小的 空间
    char **arr = malloc(count_col * sizeof(char *)); 
    *ent = arr;

    int col_index = 0;
    int col_width = 0;
    // 将表项存入我们的col_buf中
    #define Col_Number (32)
    char col_buf[Col_Number];

    for(int i = 0; i < len + 1; ++i)
    {
        if(str[i] == ',' || str[i] == '\0')
        {
            // 定义一个如果col_index大于了cout_col程序终止的函数
            assert(col_index < count_col);
            // 申请heap的空间，+1是因为数组的情况下是从0开始的我们需要把最后一位变成\0
            char *col = malloc((col_width + 1) * sizeof(char));
            // 将表项写入col数组中
            for(int j = 0; j < col_width; ++j)
            {
                col[j] = col_buf[j];
            }
            col[col_width] = '\0';
            // 将col(heap)的地址给arr(stack)，对索引进行++，width置0
            arr[col_index] = col;
            col_index ++;
            col_width = 0; 
        }
        else
        {
            assert(col_width < Col_Number);
            col_buf[col_width] = str[i];
            col_width ++;
        }
    }
    return count_col;
}

static void free_table_entry(char **ent, int n)
{
    for(int i = 0; i < n; ++i)
    {
        // 因为二级指针上面已经知道了是heap上的地址 进行free内层
        free(ent[i]);
    }
        // free外层
    free(ent);
}

static void parse_sh(char *str, sht_entry_t *sh)
{
    char **cols;
    int num_cols = parser_table_entry(str, &cols);
    assert(num_cols == 4);
    assert(sh != NULL);
    strcpy(sh->sh_name, cols[0]);
    sh->sh_addr = atoi(cols[1]);
    sh->sh_offset = atoi(cols[2]);
    sh->sh_size = atoi(cols[3]);

    free_table_entry(cols, num_cols);
}

static void print_sh_entry(sht_entry_t *sh)
{
    printf("%s\t%lx\t%lu\t%lu\n", sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);
}

static void parse_symtable(char *str, st_entry_t *ste)
{   
    //sum,STB_GLOBAL,STT_FUNCTION,.text,0,22
    char **cols;
    int num_cols = parser_table_entry(str, &cols);
    assert(num_cols == 6);
    assert(ste != NULL);
    strcpy(ste->st_name, cols[0]);
    // symbol table bind
    if (strcmp(cols[1], "STB_LOCAL") == 0)
    {
        ste->bind = STB_LOCAL;
    }
    else if (strcmp(cols[1], "STB_GLOBAL") == 0)
    {
        ste->bind = STB_GLOBAL;
    }
    else if (strcmp(cols[1], "STB_WEAK") == 0)
    {
        ste->bind = STB_WEAK;
    }
    else
    {
        printf("symbol bind is neither LOCAL, GLBOL nor WEAK\n");
        exit(0);
    }
    // symbol table type
    if (strcmp(cols[2], "STT_NOTYPE") == 0)
    {
        ste->type = STT_NOTYPE;
    }
    else if (strcmp(cols[2], "STT_OBJECT") == 0)
    {
        ste->type = STT_OBJECT;
    }
    else if (strcmp(cols[2], "STT_FUNC") == 0)
    {
        ste->type = STT_FUNC;
    }
    else
    {
        printf("symbol type is neither NOTYPE, OBJECT nor FUNC\n");
        exit(0);
    }
    strcpy(ste->st_shndex, cols[3]);
    ste->st_value = atoi(cols[4]);
    ste->st_size = atoi(cols[5]);

    free_table_entry(cols, num_cols);

}

static void print_symtab_entry(st_entry_t *ste)
{
    printf("%s\t%d\t%d\t%s\t%lu\t%lu\n", ste->st_name, ste->bind, ste->type, ste->st_shndex, ste->st_value, ste->st_size);
}


static int read_elf(const char *filename, uint64_t bufaddr)
{
    //open file and read
    FILE *fp;
    fp = fopen(filename,"r");
    if (fp == NULL)
    { 
        printf ("this file %s can't successful open\n", filename);
        return -1;
    }

    //read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    //gcc ../text/test_elf.c ../Linker\ program/parserELF.c -o ./test_elf
    while(fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
    {
        int len = strlen(line);
        // 进行注释的一个过滤
        // 第一重的过滤: 
        // 1.如果这个len的大小等于0的时候，代表这个是个空行直接continue
        // 2.如果这个len的大小大于等于1的时候，如果第一个位置是个换行符，或者\r等等的情况下，进行continue
        // 3.如果这个len的带下大于等于2的时候，代表有可能前两个元素的情况是注释的情况所以判断该前两个的位置都是注释的情况下进行continue
        if((len == 0) ||
           ((len >= 1) && (line[0] == '\n' || line[0] == '\r')) ||
           ((len >= 2) && (line[0] == '/' && line[1] == '/'))
        )
        {
            continue;
        }
        // 第二重的过滤: 
        // 当前的line是不是全是空格或者是多个\t那种的元素，我们就直接不要了
        int iswhite = 1;
        for (int i = 0; i < len; ++i)
        {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t');
        }
        if (iswhite == 1)
        {
            continue;
        }

        if(line_counter < MAX_ELF_FILE_LENGTH)
        {
            
            uint64_t addr = bufaddr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char); //addr 去表示我们的每一行的长度的问题
            char *linebuf = (char *)addr;

            int i = 0;
            while(i < len && i < MAX_ELF_FILE_WIDTH)
            {
                if (line[i] == '\n' || line[i] == '\t' || 
                   (((i + 1) < len) && ((i + 1) < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/')) // 为了验证下一项是不是'/'
                {
                    break;
                }
                linebuf[i] = line[i]; // 将当前的line[i]元素给linebuf[i]
                i++;
            }
            linebuf[i] = '\0';
            line_counter++;
        }
        else
        {
            printf("ELF file %s is too long > %d \n", filename, MAX_ELF_FILE_LENGTH);
            return -1;
        }
    }
    fclose(fp);
    return line_counter;
}

void parse_elf(char* filename, elf_t *elf)
{
    assert(elf != NULL);
    int line_count = read_elf(filename, (uint64_t)(&(elf->buffer)));
    for(int i = 0; i < line_count; ++i)
    {
        printf("[%d]\t%s\n",i, elf->buffer[i]);
    }
    // 获取section header的行数
    int sh_count = atoi(elf->buffer[1]);
    // 分配heap大小，将空间给予section header table
    elf->sht = malloc(sh_count * sizeof(sht_entry_t));
 
    sht_entry_t* symt_sh = NULL;
    // 获取这两行section header进行分析，输出
    for(int i = 0; i < sh_count; ++i)
    {
        parse_sh(elf->buffer[i+2], &(elf->sht[i]));
        print_sh_entry(&(elf->sht[i]));

        if (strcmp((elf->sht[i].sh_name), ".symtab") == 0)
        {
            symt_sh = &(elf->sht[i]);
        }
    }
    assert(symt_sh != NULL);

    elf->symtab_count = symt_sh->sh_size;
    elf->symt = malloc(elf->symtab_count * sizeof(st_entry_t));

    for (int i = 0; i < symt_sh->sh_size; ++i)
    {
        parse_symtable(elf->buffer[i + symt_sh->sh_offset], &(elf->symt[i]));
        print_symtab_entry(&(elf->symt[i]));
    }

}

void free_elf(elf_t *elf)
{
    assert(elf != NULL);

    free(elf->sht);
}
