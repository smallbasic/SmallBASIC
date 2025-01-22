// This file is part of SmallBASIC
//
// Copyright(C) 2001-2018 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>

#include "config.h"
#include "include/osd.h"
#include "common/device.h"
#include "common/smbas.h"

byte os_graphics = 0;
int os_graf_mx = 80;
int os_graf_my = 25;

// cmd_ *functions
void cmd_arc(int argc, char *argv[]) {}
void cmd_beep() {}
void cmd_bload(char *filename) {}
void cmd_bputc(int ch) {}
void cmd_bsave(char *filename) {}
void cmd_chart() {}
void cmd_chdir(char *path) {}
void cmd_chmod(char *filename, int mode) {}
void cmd_circle(int x, int y, int radius) {}
void cmd_dirwalk(void) {}
void cmd_draw(int x1, int y1, int x2, int y2) {}
void cmd_drawpoly(int *coords, int num_points) {}
void cmd_fclose(FILE *file) {}
void cmd_filecp(char *src, char *dest) {}
void cmd_fkill(char *filename) {}
void cmd_flineinput(char *prompt) {}
void cmd_floadln(char *filename) {}
void cmd_flock(FILE *file) {}
void cmd_fopen(char *filename, const char *mode) {}
void cmd_fread(FILE *file, void *buffer, size_t size) {}
void cmd_fsaveln(char *filename) {}
void cmd_fseek(FILE *file, long offset, int whence) {}
void cmd_fwrite(FILE *file, const void *buffer, size_t size) {}
void cmd_intersect(int x1, int y1, int x2, int y2) {}
void cmd_line(int x1, int y1, int x2, int y2) {}
void cmd_m3apply(float *matrix) {}
void cmd_m3ident() {}
void cmd_m3rotate(float angle) {}
void cmd_m3scale(float scaleX, float scaleY, float scaleZ) {}
void cmd_m3translate(float x, float y, float z) {}
void cmd_mkdir(char *dir) {}
void cmd_nosound() {}
void cmd_paint(int x, int y, int color) {}
void cmd_play(char *sound_file) {}
void cmd_play_reset() {}
void cmd_plot(int x, int y) {}
void cmd_polyext(int *coords, int num_points) {}
void cmd_pset(int x, int y, int color) {}
void cmd_rect(int x1, int y1, int x2, int y2) {}
void cmd_rmdir(char *dir) {}
void cmd_sound(int frequency) {}
void cmd_view(int x1, int y1, int x2, int y2) {}
void cmd_window(int width, int height) {}

// dev_ *functions - handle device operations, file operations, and environment functions
void dev_cls() {}
char_p_t *dev_create_file_list(const char *wc, int *count) {return 0;}
void dev_destroy_file_list(char_p_t *list, int count) {}
int dev_env_count() { return 0; }
int dev_faccess(const char *filename) { return 0; }
int dev_fattr(const char *filename) { return 0; }
int dev_feof(int handle) { return 0; }
int dev_fexists(const char *filename) { return 0; }
int dev_filemtime(var_t *v, char **buffer) { return 0; }
uint32_t dev_flength(int handle) { return 0; }
int dev_fread(int handle, byte *buff, uint32_t size) { return 0; }
int dev_freefilehandle() {return 0; }
int dev_fstatus(int handle) { return 0; }
uint32_t dev_ftell(int handle) { return 0; }
int dev_fwrite(int handle, byte *buffer, uint32_t size) { return 0; }
const char *dev_getenv(const char *name) { return NULL; }
const char *dev_getenv_n(int n) { return NULL; }
int dev_getpen(int code) { return 0; }
long dev_getpixel(int x, int y) { return 0; }
char *dev_gets(char *buffer, int size) { return buffer; }
int dev_getx() { return 0; }
int dev_gety() { return 0; }
int dev_run(const char *src, var_t *v, int wait) {return 0;}
int dev_setenv(const char *name, const char *value) { return 0; }
void dev_setpenmode(int mode) {}
void dev_settextcolor(long fg, long bg) {}
void dev_setxy(int x, int y, int transform) {}
int dev_textheight(const char *text) { return 0; }
int dev_textwidth(const char *text) { return 0; }
void dev_log_stack(const char *keyword, int type, int line) {}
void dev_show_page() {}
int dev_restore() {return 0;}

// geo_ *functions - geometric calculations
float geo_distfromline(int x1, int y1, int x2, int y2, int x, int y) { return 0.0f; }
float geo_distfromseg(int x1, int y1, int x2, int y2, int x, int y) { return 0.0f; }
void geo_polycentroid(int *coords, int num_points, float *centroid) {}
float geo_segangle(int x1, int y1, int x2, int y2) { return 0.0f; }

// system-related functions
char *getcwd(char *buffer, size_t size) { return buffer; }
void graph_reset() {}
int gra_x() { return 0; }
int gra_y() { return 0; }
void net_print(const char *data) {}
void net_send(const char *data) {}
time_t sys_filetime(const char *filename) { return 0; }
int sys_search_path(const char *path, const char *file, char *retbuf) { return 0; }
void v_create_image(var_p_t var) {}
void v_create_form(var_p_t var) {}
void v_create_window(var_p_t var) {}
int wc_match(const char *mask, char *name) { return 0; }

// system calls not available
clock_t _times(struct tms *buf) { return 0;}
int stat(const char *path, struct stat *buf) { return -1; }
int open(const char *path, int flags, ...) { return -1; }
