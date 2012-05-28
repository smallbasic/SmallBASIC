#include <stdio.h>
#include <string.h>

void change_ext(char *name, char *ext) {
  char *e = ext;
  char *p;

  p = strrchr(name, '.');
  if (*e == '.')
    e++;

  if (p) {
    *(p + 1) = '\0';
    strcat(name, e);
  } else {
    strcat(name, ".");
    strcat(name, e);
  }
}

void font2c(const char *file_name) {
  FILE *fin, *fout;
  char new_name[1024];
  char buff[4096];
  int i;

  fin = fopen(file_name, "rb");
  if (fin) {
    strcpy(new_name, file_name);
    change_ext(new_name, ".c");
    fout = fopen(new_name, "wb");

    fread(buff, 4096, 1, fin);

    fprintf(fout, "unsigned char font8x16[] = {\n");
    for (i = 0; i < 4096; i++) {
      fprintf(fout, "0x%02X, ", (unsigned char)buff[i]);
      if ((i % 16) == 0)
        fprintf(fout, "\n");
    }
    fprintf(fout, "0 };\n");

    fclose(fin);
    fclose(fout);
  } else
    perror("font2c");
}

int main(int argc, char *argv[]) {
  int i;

  for (i = 1; i < argc; i++)
    font2c(argv[i]);
  return 0;
}
