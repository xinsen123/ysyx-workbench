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

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
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
#include <monitor/ftrace.h>

static char *elf_file = NULL;

fun_sym_t *fun_table = NULL;
int fun_cnt = 0;

static int sym_cmp(const void *a, const void *b) {
  const fun_sym_t *x = (const fun_sym_t *)a;
  const fun_sym_t *y = (const fun_sym_t *)b;
  return (x->begin > y->begin) - (x->begin < y->begin);
}

/* 函数表已按 begin 升序排好, 二分找到最后一个 begin <= addr 的表项再校验区间 */
const fun_sym_t *ftrace_find(vaddr_t addr) {
  int l = 0, r = fun_cnt - 1, ans = -1;
  while (l <= r) {
    int mid = (l + r) / 2;
    if (fun_table[mid].begin <= addr) { ans = mid; l = mid + 1; }
    else r = mid - 1;
  }
  if (ans >= 0 && addr < fun_table[ans].end) return &fun_table[ans];
  return NULL;
}

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
  Elf32_Shdr *shdr = malloc(ehdr.e_shentsize * ehdr.e_shnum);
  if (shdr == NULL) { fclose(fp); return; }
  if (fread(shdr, ehdr.e_shentsize, ehdr.e_shnum, fp) != ehdr.e_shnum) {
    Log("Failed to read section headers.");
    free(shdr); fclose(fp); return;
  }

  /* ---- 3. 找到 .symtab 节及其关联的 .strtab 节 ---- */
  Elf32_Shdr *symtab_hdr = NULL, *strtab_hdr = NULL;
  for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB) {
      symtab_hdr = &shdr[i];
      strtab_hdr = &shdr[symtab_hdr->sh_link];
      break;
    }
  }
  if (symtab_hdr == NULL) {
    Log("No symbol table found in '%s'.", elf_file);
    free(shdr); fclose(fp); return;
  }

  /* ---- 4. 读字符串表; 符号名直接指向这块内存, 所以不能 free ---- */
  char *strtab = malloc(strtab_hdr->sh_size);
  if (strtab == NULL) { free(shdr); fclose(fp); return; }
  fseek(fp, strtab_hdr->sh_offset, SEEK_SET);
  if (fread(strtab, strtab_hdr->sh_size, 1, fp) != 1) {
    Log("Failed to read string table.");
    free(strtab); free(shdr); fclose(fp); return;
  }

  /* ---- 5. 遍历符号表，只保留有函数体的 FUNC 符号 ---- */
  int num_syms = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
  fun_table = malloc(sizeof(fun_sym_t) * num_syms);
  assert(fun_table);

  fun_cnt = 0;
  fseek(fp, symtab_hdr->sh_offset, SEEK_SET);
  for (int i = 0; i < num_syms; i++) {
    Elf32_Sym sym;
    if (fread(&sym, sizeof(sym), 1, fp) != 1) break;
    if (ELF32_ST_TYPE(sym.st_info) != STT_FUNC) continue;
    if (sym.st_size == 0 || sym.st_value == 0) continue;

    fun_table[fun_cnt].begin = sym.st_value;
    fun_table[fun_cnt].end = sym.st_value + sym.st_size;
    fun_table[fun_cnt].name = strtab + sym.st_name;
    fun_cnt++;
  }

  qsort(fun_table, fun_cnt, sizeof(fun_sym_t), sym_cmp);
  Log("Loaded %d functions from ELF symbol table.", fun_cnt);

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

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* 加载并初始化elf文件，把函数表塞进elf_table中以备使用 */
  #ifdef CONFIG_FTRACE
  load_elf();
  #endif

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
