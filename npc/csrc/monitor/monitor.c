/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <monitor/monitor.h>

void init_rand();
void init_log(const char *log_file);
void init_sdb();
void init_disasm();

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;   /* difftest 预留 */
static char *img_file = NULL;
static int difftest_port = 1234;    /* difftest 预留 */

static long load_img() {
  if (img_file == NULL) {
    panic("No image is given. Use: make run img=<path-to-bin> [elf=<path-to-elf>]");
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

#ifdef CONFIG_FTRACE
#include <elf.h>
static char *elf_file = NULL;

  /* 函数符号表：从 ELF 中提取的所有函数名及其地址范围 */
  typedef struct {
    char name[32];
    uint32_t begin, end;  // [begin, end) 左闭右开区间
  } elf_obj;

  elf_obj fun_table[128];  // 最多存 128 个函数
  int fun_cnt = 0;         // 实际加载的函数数

  /* 从 ELF 的 .symtab 节中提取所有函数符号 */
  static void load_elf() {
    if (elf_file == NULL) {
      Log("No ELF file provided, FTRACE disabled.");
      return;
    }

    FILE *fp = fopen(elf_file, "rb");
    if (fp == NULL) {
      Log("Cannot open ELF file '%s'.", elf_file);
      return;
    }

    /* ---- 1. 读 ELF 文件头，验证魔数和格式 ---- */
    Elf32_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, fp) != 1) {
      Log("Failed to read ELF header.");
      fclose(fp);
      return;
    }
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
      Log("'%s' is not a valid ELF file.", elf_file);
      fclose(fp);
      return;
    }
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
      Log("Only 32-bit ELF is supported.");
      fclose(fp);
      return;
    }

    /* ---- 2. 读取所有节头表 (section headers) ---- */
    fseek(fp, ehdr.e_shoff, SEEK_SET);
    Elf32_Shdr *shdr = (Elf32_Shdr *)malloc(ehdr.e_shentsize * ehdr.e_shnum);
    if (shdr == NULL) { fclose(fp); return; }
    if (fread(shdr, ehdr.e_shentsize, ehdr.e_shnum, fp) != ehdr.e_shnum) {
      Log("Failed to read section headers.");
      free(shdr); fclose(fp); return;
    }

    /* 顺便读一下节头字符串表（shstrtab），本函数未用到，为后续扩展保留 */
    char *shstrtab = NULL;
    if (ehdr.e_shstrndx != SHN_UNDEF) {
      shstrtab = (char *)malloc(shdr[ehdr.e_shstrndx].sh_size);
      if (shstrtab) {
        fseek(fp, shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET);
        if (fread(shstrtab, shdr[ehdr.e_shstrndx].sh_size, 1, fp) != 1) {
          Log("Failed to read section header string table.");
          free(shstrtab); free(shdr); fclose(fp); return;
        }
      }
    }

    /* ---- 3. 找到 .symtab 节及其关联的 .strtab 节 ---- */
    Elf32_Shdr *symtab_hdr = NULL;
    Elf32_Shdr *strtab_hdr = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++) {
      if (shdr[i].sh_type == SHT_SYMTAB) {
        symtab_hdr = &shdr[i];
        strtab_hdr = &shdr[symtab_hdr->sh_link];  // sh_link 指向对应的字符串表
        break;
      }
    }
    if (symtab_hdr == NULL) {
      Log("No symbol table found in '%s'.", elf_file);
      free(shstrtab); free(shdr); fclose(fp); return;
    }

    /* ---- 4. 读字符串表（存着所有符号名） ---- */
    size_t strtab_sz = strtab_hdr->sh_size;
    char *strtab = (char *)malloc(strtab_sz);
    if (strtab == NULL) { free(shstrtab); free(shdr); fclose(fp); return; }
    fseek(fp, strtab_hdr->sh_offset, SEEK_SET);
    if (fread(strtab, strtab_sz, 1, fp) != 1) {
      Log("Failed to read string table.");
      free(strtab); free(shstrtab); free(shdr); fclose(fp); return;
    }

    /* ---- 5. 遍历符号表，筛选出函数符号 ---- */
    int num_syms = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
    fseek(fp, symtab_hdr->sh_offset, SEEK_SET);
    fun_cnt = 0;

    for (int i = 0; i < num_syms && fun_cnt < 128; i++) {
      Elf32_Sym sym;
      if (fread(&sym, sizeof(sym), 1, fp) != 1) break;

      /* 只取 STT_FUNC 类型、有实际体（st_size > 0）且有地址（st_value > 0）的符号 */
      if ((ELF32_ST_TYPE(sym.st_info) == STT_FUNC || 
           ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE)&&
          sym.st_size > 0 && sym.st_value > 0) {
        fun_table[fun_cnt].begin = sym.st_value;
        fun_table[fun_cnt].end = sym.st_value + sym.st_size;
        const char *sym_name = strtab + sym.st_name;  // st_name 是 .strtab 中的偏移
        snprintf(fun_table[fun_cnt].name, sizeof(fun_table[fun_cnt].name), "%s", sym_name);
        fun_cnt++;
      }
    }

    Log("Loaded %d functions from ELF symbol table.", fun_cnt);

    free(strtab);
    free(shstrtab);
    free(shdr);
    fclose(fp);
  }
#endif

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1:
      //case1时为常规字符串，按照此逻辑可无限拓展加参
      if(img_file == NULL) {img_file = optarg;}
      #ifdef CONFIG_FTRACE
      else if (elf_file == NULL) {elf_file = optarg; return 0;}
      #endif
      break;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* NPC: 物理内存为静态数组 (memory/memory.cpp), 无需 init_mem */

  /* Load the image to memory. */
  long img_size = load_img();

  /* 加载并初始化elf文件，把函数表塞进elf_table中以备使用 */
  #ifdef CONFIG_FTRACE
  load_elf();
  #endif
  /* Initialize differential testing. (difftest 由你自己实现) */
  // init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  /* 初始化 capstone 反汇编 (utils/disasm.c, itrace 使用) */
  IFDEF(CONFIG_ITRACE, init_disasm());

  /* Display welcome message. */
  welcome();
}
