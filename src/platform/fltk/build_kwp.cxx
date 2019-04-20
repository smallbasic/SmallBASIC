/**
 *   builds the kwp.cxx (keyword tables)
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

char line[256];
char keyword[256];

typedef struct {
  char key[64];
} kw_t;

kw_t ktable[2048];
int kcount;

#define code_t int
#define bid_t int
#define fcode_t int
#define pcode_t int

struct keyword_s {
  char name[16];
  code_t code;
};

struct opr_keyword_s {
  char name[16];
  code_t code;
  code_t opr;
};

struct func_keyword_s {
  char name[16];
  fcode_t fcode;
};

struct proc_keyword_s {
  char name[16];
  pcode_t pcode;
};

struct spopr_keyword_s {
  char name[16];
  code_t code;
};

#include "../../common/kw.h"
#include "../../languages/keywords.en.c"

void add_key(const char *key) {
  strcpy(ktable[kcount].key, key);
  kcount++;
}

int key_cmp(const void *a, const void *b) {
  return strcasecmp(((kw_t *) a)->key, ((kw_t *) b)->key);
}

void sort_ktable() {
  qsort(ktable, kcount, sizeof(kw_t), key_cmp);
}

void print_ktable(FILE *fp) {
  int i;
  for (i = 0; i < kcount; i++) {
    fprintf(fp, "\"%s\"", ktable[i].key);
    if (i != kcount - 1) {
      fprintf(fp, ", ");
    }
    if (((i + 1) % 8) == 0) {
      fprintf(fp, "\n");
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *fo = fopen("kwp.h", "wt");

  kcount = 0;

  int code_keywords_len = 0;
  int code_procedures_len = 0;
  int code_functions_len = 0;

  fprintf(fo, "const char *code_keywords[] = {\n");
  for (int i = 0; keyword_table[i].name[0] != '\0'; i++) {
    if (keyword_table[i].name[0] != '$') {
      add_key(keyword_table[i].name);
      code_keywords_len++;
    }
  }
  sort_ktable();
  print_ktable(fo);

  kcount = 0;
  fprintf(fo, "};\n\nconst char *code_procedures[] = {\n");
  for (int i = 0; proc_table[i].name[0] != '\0'; i++) {
    add_key(proc_table[i].name);
    code_procedures_len++;
  }
  sort_ktable();
  print_ktable(fo);

  kcount = 0;
  fprintf(fo, "};\n\nconst char *code_functions[] = {\n");
  for (int i = 0; func_table[i].name[0] != '\0'; i++) {
    add_key(func_table[i].name);
    code_functions_len++;
  }
  sort_ktable();
  print_ktable(fo);

  fprintf(fo, "};\n\n");

  fprintf(fo, "const int code_keywords_len = %d;\n", code_keywords_len);
  fprintf(fo, "const int code_procedures_len = %d;\n", code_procedures_len);
  fprintf(fo, "const int code_functions_len = %d;\n", code_functions_len);

  fclose(fo);
  return 0;
}
