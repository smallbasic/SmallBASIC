/**
 *   builds the kwp.cxx (keyword tables)
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ui/strlib.h"

#define code_t int
#define bid_t int

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
  bid_t fcode;
};

struct proc_keyword_s {
  char name[16];
  bid_t pcode;
};

struct spopr_keyword_s {
  char name[16];
  code_t code;
};


#include "../src/common/kw.h"
#include "../src/languages/keywords.en.c"

struct HelpItem {
  char package[20];
  char keyword[20];
  char type[20];
  char id[20];
  char signature[128];
  char help[1024];
};

int compareHelpItem(const void *p1, const void *p2) {
  HelpItem **i1 = (HelpItem **)p1;
  HelpItem **i2 = (HelpItem **)p2;
  int result = strcasecmp((*i1)->package, (*i2)->package);
  if (result == 0) {
    result = strcasecmp((*i1)->keyword, (*i2)->keyword);
  }
  return result;
}

bool isKeyword(const char *keyword) {
  bool result = false;

  for (int i = 0; !result && keyword_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(keyword_table[i].name, keyword) == 0) {
      result = true;
    }
  }

  for (int i = 0; !result && proc_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(proc_table[i].name, keyword) == 0) {
      result = true;
    }
  }

  for (int i = 0; !result && func_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(func_table[i].name, keyword) == 0) {
      result = true;
    }
  }

  for (int i = 0; !result && opr_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(opr_table[i].name, keyword) == 0) {
      result = true;
    }
  }

  for (int i = 0; !result && spopr_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(spopr_table[i].name, keyword) == 0) {
      result = true;
    }
  }

  const char* constants[] = {
    "SBVER", "PI", "XMAX", "YMAX", "TRUE", "FALSE", "CWD", "HOME", "COMMAND", "INCLUDE", "NIL", "MAXINT", ""
  };
  for (int i = 0; !result && constants[i][0] != '\0'; i++) {
    if (strcasecmp(constants[i], keyword) == 0) {
      result = true;
    }
  }

  return result;
}

void logMissing(strlib::List<HelpItem *> *helpItems) {
  for (int i = 0; keyword_table[i].name[0] != '\0'; i++) {
    bool found = false;
    List_each(HelpItem *, it, *helpItems) {
      HelpItem *next = (*it);
      if (strcasecmp(keyword_table[i].name, next->keyword) == 0) {
        found = true;
        break;
      }
    }
    if (!found && keyword_table[i].name[0] != '$') {
      fprintf(stderr, "Missing KEYWORD doc: %s\n", keyword_table[i].name);
    }
  }
  for (int i = 0; proc_table[i].name[0] != '\0'; i++) {
    bool found = false;
    List_each(HelpItem *, it, *helpItems) {
      HelpItem *next = (*it);
      if (strcasecmp(proc_table[i].name, next->keyword) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      fprintf(stderr, "Missing PROC doc: %s\n", proc_table[i].name);
    }
  }
  for (int i = 0; func_table[i].name[0] != '\0'; i++) {
    bool found = false;
    List_each(HelpItem *, it, *helpItems) {
      HelpItem *next = (*it);
      if (strcasecmp(func_table[i].name, next->keyword) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      fprintf(stderr, "Missing FUNC doc: %s\n", func_table[i].name);
    }
  }
}

bool readHelpReference(strlib::List<HelpItem *> *helpItems) {
  FILE *fp = fopen("sbasic_ref.csv", "r");
  if (!fp) {
    fprintf(stderr, "Failed to open sbasic_ref.csv");
    return false;
  }

  char lineBuffer[2048];
  while (1) {
    if (fgets(lineBuffer, sizeof(lineBuffer), fp) == NULL) {
      break;
    }
    HelpItem *item = new HelpItem();
    int len = strlen(lineBuffer);
    int currentCol = 0;
    int start = 0;
    bool quoted = false;

    for (int i = 0; i < len; i++) {
      if (lineBuffer[i] == '\"' && lineBuffer[i + 1] == '\"') {
        // change CSV escape to C escape "" -> \"
        lineBuffer[i] = '\\';
        i += 1;
      } else if (lineBuffer[i] == '\"') {
        quoted = !quoted;
      } else if (!quoted && (i + 1 == len || lineBuffer[i] == ',')) {
        int fieldLen = i - start;
        if (lineBuffer[start] == '\"') {
          start += 1;
          fieldLen -= 2;
        }
        switch (currentCol) {
        case 0:
          // package
          strncpy(item->package, lineBuffer + start, fieldLen);
          item->package[fieldLen] = '\0';
          break;
        case 1:
          // type
          strncpy(item->type, lineBuffer + start, fieldLen);
          item->type[fieldLen] = '\0';
          break;
        case 2:
          // keyword
          strncpy(item->keyword, lineBuffer + start, fieldLen);
          item->keyword[fieldLen] = '\0';
          break;
        case 3:
          // nodeID
          strncpy(item->id, lineBuffer + start, fieldLen);
          item->id[fieldLen] = '\0';
          break;
        case 4:
          // signature
          strncpy(item->signature, lineBuffer + start, fieldLen);
          item->signature[fieldLen] = '\0';
          break;
        case 5:
          // text
          strncpy(item->help, lineBuffer + start, fieldLen);
          item->help[fieldLen] = '\0';
          break;
        }
        currentCol++;
        start = i + 1;
      }
    }
    if (isKeyword(item->keyword)) {
      helpItems->add(item);
    } else if (strcasecmp(item->keyword, "self") != 0) {
      fprintf(stderr,  "OBSOLETE %s\n", item->keyword);
    }
  }
  fclose(fp);
  helpItems->sort(compareHelpItem);
  logMissing(helpItems);
  return true;
}

uint32_t getHash(const char *key) {
  uint32_t hash, i;
  for (hash = i = 0; key[i] != '\0'; i++) {
    hash += tolower(key[i]);
    hash += (hash << 4);
    hash ^= (hash >> 2);
  }
  return hash;
}

int main(int argc, char *argv[]) {
  strlib::List<HelpItem *> helpItems;
  if (!readHelpReference(&helpItems)) {
    exit(1);
  }

  fprintf(stdout, "/* automagicaly generated file */\n");
  fprintf(stdout, "static struct KEYWORD_HELP {\n");
  fprintf(stdout, "  const char *package;\n");
  fprintf(stdout, "  const char *keyword;\n");
  fprintf(stdout, "  const char *signature;\n");
  fprintf(stdout, "  const char *nodeId;\n");
  fprintf(stdout, "  const char *help;\n");
  fprintf(stdout, "} keyword_help[] = {\n");

  int max_keyword_len = 0;
  List_each(HelpItem *, it, helpItems) {
    HelpItem *item = (*it);
    fprintf(stdout, "{\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"},\n", item->package,
            item->keyword, item->signature, item->id, item->help);
    int len = strlen(item->keyword);
    if (len > max_keyword_len) {
      max_keyword_len = len;
    }
  }
  fprintf(stdout, "};\n");
  fprintf(stdout, "const int keyword_help_len = %d;\n", helpItems.size());
  fprintf(stdout, "const int keyword_max_len = %d;\n", max_keyword_len);

  int count = 0;
  fprintf(stdout, "const uint32_t keyword_hash_statement[] = {\n");
  List_each(HelpItem *, it, helpItems) {
    HelpItem *item = (*it);
    if (strcasecmp(item->package, "Language") == 0) {
      count++;
      fprintf(stdout, " %uu, //%s\n", getHash(item->keyword), item->keyword);
    }
  }
  fprintf(stdout, "};\n");
  fprintf(stdout, "const int keyword_hash_statement_len = %d;\n", count);

  count = 0;
  fprintf(stdout, "const uint32_t keyword_hash_command[] = {\n");
  List_each(HelpItem *, it, helpItems) {
    HelpItem *item = (*it);
    if (strcasecmp(item->package, "Language") != 0) {
      count++;
      fprintf(stdout, " %uu, //%s\n", getHash(item->keyword), item->keyword);
    }
  }
  fprintf(stdout, "};\n");
  fprintf(stdout, "const int keyword_hash_command_len = %d;\n", count);

  return 0;
}
