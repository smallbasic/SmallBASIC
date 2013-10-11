/*
 *   SmallBASIC platform driver for Unix|Win32 + SDL
 *
 *   ndc: 2001-03-03
 *
 *   2006-12-20 - 2007-2-3 Attila Haraszti
 *     correct the sound driver
 *     add make_update and speed up 10-20 times of graphic performance depending on usage
 *     correct keyboard handling to honor kernel keyboard remapping and national characters
 *     add wider character handling up to 16x32
 *     add font selection and Latin 1/2/5/7 fonts in four different sizes
 *         system font 0 select Latin 1 - Western Europe
 *         system font 1 select Latin 2 - Eastern Europe but latin based
 *         system font 2 select Latin 5 - Cyrillic
 *         system font 3 select Latin 7 - Greek
 *
 *     todo:   latin-3 Maltese, Esperanto, Afrikans
 *             latin-4 Baltic Rim Esthonian, Latvian, Lithuanian, Lappish, Greenlandic...
 *             latin-6 Arabic
 *             latin-8 Hebrew
 *
 *         any system font selection will reset the size to the default 8x16
 *         SB font 0 select size 6x9
 *         SB font 1 select size 8x16 - default
 *         SB font 2 select size 10x20
 *         SB font 3 select size 16x29 - symulate 40 character wide screen on 640x480!
 *     add language detection for correct default character set selection
 *
 *     add image support (need a new compiler switch -DHAVE_SDL_IMAGE and a 
 *         small modification in device.c!)
 *         with SDL, output graphic file format is BMP because SDL support just 
 *         that one but for write
 *         with support of SDL_image library the input can be JPG, GIF, BMP, PNG, TIFF
 *
 *     image support has a cache parameters:
 *         MAX_IMAGE_IN_CACHE - maximum how many image could be in the cache 
 *         independently from size
 *         MAX_IMAGE_PIXEL_COUNT - maximum how many total pixels are in the cache
 *             the required memory is MAX_IMAGE_PIXEL_COUNT * BPP /8
 *
 *     add new escape sequence esc[N - next character is graphic
 *         means no interpret control codes just draw a glyph to the screen 
 *         for example esc, cr, lf could have graphical form (except the null which
 *         terminate the string)
 *
 *     and correct a lot of small error
 * 
 */

#include "common/device.h"
#include "common/osd.h"
#include "common/smbas.h"
#include "common/str.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_UnixOS)
#include <unistd.h>
#endif

// sdl-config --cflags includes the SDL path
#include <SDL.h>
#include <ctype.h>

#ifdef HAVE_SDL_IMAGE
#include <SDL_image.h>
// bitmap cache functions
#define MAX_IMAGE_IN_CACHE 50
#define MAX_IMAGE_PIXEL_COUNT 2000000

struct image_cache {
  SDL_Surface *image_entry;
  char *file_name;
  long usage_count;
} i_cache[MAX_IMAGE_IN_CACHE];

static SDL_Surface *Look_in_cache(char *file_name);
static void Add_to_cache(SDL_Surface * img, char *file_name);
static void Init_cache(void);
static void Clear_cache(void);
static long maxpixelcount;
#endif

#define GET_FG_COLOR 1
#define GET_BG_COLOR 2
#define DEF_FG_COLOR 15
#define DEF_BG_COLOR 0
static void osd_setbgcolor(long color);
static long get_screen_color(int fgbg);

extern char g_file[];
static int cur_x = 0;
static int cur_y = 0;
static int bytespp = 1;
static int mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x,
  mouse_down_y, mouse_pc_x, mouse_pc_y;
static int tabsize = 32;        // from dev_palm
static int maxline;

static int con_use_bold = 0;
static int con_use_ul = 0;
static int con_use_reverse = 0;
static long fg_screen_color;
static long fg_screen_color, bg_screen_color;
static int fast_exit = 0;
static int exit_app = 0;        // allow CTRL+C or ALT+F4 while displaying exit 
                                // message

int dev_w = 640, dev_h = 480, dev_d = 16;
int sb_console_main(int argc, char *argv[]);

// VGA16 colors in RGB
static unsigned long vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

static long cmap[16];

/* fonts
 * i1 - Iso latin 1 Western Europe
 * i2 - Iso latin 2 Eastern Europe
 * i5 - Iso latin 5 Cyrillic
 * i7 - Iso latin 7 Greek
 *
 *   These fonts are translated from original Digital Equipment VT100 terminal fonts
 *       (c) Digital Equipment Corporation
 */
#include "fonts/BI1.c"
#include "fonts/BI2.c"
#include "fonts/BI5.c"
#include "fonts/BI7.c"

static unsigned char *fonttable[4][4] = {
  {font6x9i1, font8x16i1, font10x20i1, font16x29i1},
  {font6x9i2, font8x16i2, font10x20i2, font16x29i2},
  {font6x9i5, font8x16i5, font10x20i5, font16x29i5},
  {font6x9i7, font8x16i7, font10x20i7, font16x29i7}
};

/*  You could change the default system font character set and size
 */
static int currentsystemfont = 1;
static int savedsystemfont = 1;
static unsigned char *currentfont = font8x16i2;
static int font_w = 8;
static int font_h = 16;
static int font_l = 1;          // this is the length of a row in bytes equal
                                // INT((font_w + 7) 
                                // / 8)

/*static*/ SDL_Surface *screen = NULL;
// extern in blib_sdl_ui.cpp
static int has_audio = 0;
static int mouse_hot_x = -16, mouse_hot_y = -16;
static SDL_AudioSpec audiospec;
static int scr_update = 0;
static SDL_Rect Update_rect;

struct voice_info {
  int period;                   // we don't need float because we could
  // calculate just integer 
  // number of sample
  int toggle;                   // and integer calculation is much quicker!
  int setting;
  int vol;
  int remain_ms;
};

static char *mixbuf;
#define   AUDIO_STACK_MAX      8192*2
static struct voice_info audio_info[AUDIO_STACK_MAX];
static int audio_head = 0;
static int audio_tail = 0;

/*
 */
static int keymap[] = {
  SDLK_CAPSLOCK, -1,
  SDLK_LSHIFT, -1,
  SDLK_RSHIFT, -1,
  SDLK_LALT, -1,
  SDLK_RALT, -1,
  SDLK_LCTRL, -1,
  SDLK_RCTRL, -1,
  SDLK_RETURN, SB_KEY_ENTER,
  SDLK_ESCAPE, 27,
  SDLK_KP_PERIOD, SB_KEY_KP_DEL,
  SDLK_KP_ENTER, SB_KEY_KP_ENTER,
  SDLK_KP_DIVIDE, SB_KEY_KP_DIV,
  SDLK_KP_MULTIPLY, SB_KEY_KP_MUL,
  SDLK_KP_MINUS, SB_KEY_KP_MINUS,
  SDLK_KP_PLUS, SB_KEY_KP_PLUS,
  SDLK_KP_EQUALS, '=',
  SDLK_KP0, SB_KEY_KP_INS,
  SDLK_KP1, SB_KEY_KP_END,
  SDLK_KP2, SB_KEY_KP_DOWN,
  SDLK_KP3, SB_KEY_KP_PGDN,
  SDLK_KP4, SB_KEY_KP_LEFT,
  SDLK_KP5, SB_KEY_KP_CENTER,
  SDLK_KP6, SB_KEY_KP_RIGHT,
  SDLK_KP7, SB_KEY_KP_HOME,
  SDLK_KP8, SB_KEY_KP_UP,
  SDLK_KP9, SB_KEY_KP_PGUP,
  SDLK_BACKSPACE, SB_KEY_BACKSPACE,
  SDLK_DELETE, SB_KEY_DELETE,
  SDLK_UP, SB_KEY_UP,
  SDLK_DOWN, SB_KEY_DN,
  SDLK_LEFT, SB_KEY_LEFT,
  SDLK_RIGHT, SB_KEY_RIGHT,
  SDLK_PAGEUP, SB_KEY_PGUP,
  SDLK_PAGEDOWN, SB_KEY_PGDN,
  SDLK_HOME, SB_KEY_HOME,
  SDLK_END, SB_KEY_END,
  SDLK_INSERT, SB_KEY_INSERT,
  SDLK_BREAK, SB_KEY_BREAK,
  SDLK_F1, SB_KEY_F(1),
  SDLK_F2, SB_KEY_F(2),
  SDLK_F3, SB_KEY_F(3),
  SDLK_F4, SB_KEY_F(4),
  SDLK_F5, SB_KEY_F(5),
  SDLK_F6, SB_KEY_F(6),
  SDLK_F7, SB_KEY_F(7),
  SDLK_F8, SB_KEY_F(8),
  SDLK_F9, SB_KEY_F(9),
  SDLK_F10, SB_KEY_F(10),
  SDLK_F11, SB_KEY_F(11),
  SDLK_F12, SB_KEY_F(12),
  0, 0
};

#define   LOCK()      SDL_LockSurface(screen)
#define   UNLOCK()    SDL_UnlockSurface(screen)

/*
 *   I know nothing about sound.
 *   someone to fix it
 */
/*  O.K. I try to fix :)
 *       audio_callback will be called to fill the WHOLE next chunk of sound
 *       this will happend in every audiospec.samples*1000)/audiospec.freq 
 *       which is probably 4096 * 1000 / 44100 = 92 msec
 *       if the sound is quicker we have to fill the buffer with the next sound
 *       because without that, sound stream will have a silent (unwanted staccato!)
 */
void audio_callback(void *user, unsigned char *stream, int length) {
  int volume;
  int mix = 0;
  int sound_len_us;             // this chunk length in microsec
  int one_sample_len_us;        // one sample length in microsec
  struct voice_info *info;

  memset(mixbuf, audiospec.silence, length);    // fill whole buffer with silent 
  if (audio_head != audio_tail || audio_info[audio_head].remain_ms) {
    int left = length, j = 0;
    int count;

    info = &audio_info[audio_head];
    // volume = (info->vol * SDL_MIX_MAXVOLUME) / 100;
    volume = SDL_MIX_MAXVOLUME; // we implemented the volume handling in
    // software
    // because
    // don't know hardware mixer device is working or not!
    sound_len_us = info->remain_ms * 1000;      // get the length of the sound /
    // silent in
    // microsec
    one_sample_len_us = 1000 * 1000 / audiospec.freq;   // one sample length in
    // microsec (cc. 23 microsec)
    // in case of freq 44100
    mix = 1;
    do {
      if (info->period >= 1) {  // we have some sound
        count = (info->toggle < left) ? (int)info->toggle : left;
        left -= count;
        info->toggle -= count;
        sound_len_us -= count * one_sample_len_us;      // yes theoretically the
        // sound 
        // length can be 
        // quicker than half period but it is unrealistic!
        // while (count--) mixbuf[j++] = audiospec.silence + info->setting; 
        memset(&mixbuf[j], audiospec.silence + info->setting, count);
        j += count;
        count = 0;
        // here we generating square wave
        // can we do sinus?? - much more CPU intensive!
        if (info->toggle < 1) {
          info->toggle += info->period;
          info->setting = -info->setting;
        }
        if (sound_len_us <= 0) {
          info->remain_ms = 0;  // ok we finished this sound
          if (audio_head != audio_tail) {       // we have more sound
            audio_head++;       // get the next sound
            if (audio_head >= AUDIO_STACK_MAX)
              audio_head = 0;
            info = &audio_info[audio_head];
            sound_len_us = info->remain_ms * 1000;
          }
        }
      } else {                  // we have no sound but calculate length of
        // silence
        count = sound_len_us / one_sample_len_us;
        if (count < left) {
          sound_len_us = 0;
          info->remain_ms = 0;  // ok we finished the this sound/silent
          if (audio_head != audio_tail) {       // we have more sound
            audio_head++;       // get the next sound
            if (audio_head >= AUDIO_STACK_MAX)
              audio_head = 0;
            info = &audio_info[audio_head];
            sound_len_us = info->remain_ms * 1000;
          }
        } else {
          sound_len_us -= left * one_sample_len_us;     // the remaining length in
          // microsec
          count = left;
        }
        j += count;             // this is the required length of silent
        left -= count;
      }
      // do until buffer has space and there is playable sound
    } while (left > 0 && (info->remain_ms || (audio_head != audio_tail)));

    if (left <= 0 && sound_len_us > 0) {        // we have remain some portion of the 
      // sound /
      // silent!
      info->remain_ms = sound_len_us / 1000;    // take back to que
    }

    if (mix) {
      SDL_MixAudio(stream, (unsigned char *)mixbuf, length, volume);
      // and do the task - play 
    }
  }
}

void make_update(int x, int y, int x1, int y1) {
  // calculate the rectangle which should be updated on next refresh
  int tx, ty, tx1, ty1;
  // first reset clipping zone

  if (x < x1) {
    tx = x;
    tx1 = x1;
  } else {
    tx = x1;
    tx1 = x;
  }
  if (y < y1) {
    ty = y;
    ty1 = y1;
  } else {
    ty = y1;
    ty1 = y;
  }
  if (tx < 0) {
    tx = 0;
  }
  if (tx1 > os_graf_mx - 1) {
    tx1 = os_graf_mx - 1;
  }
  if (ty < 0) {
    ty = 0;
  }
  if (ty1 > os_graf_my - 1) {
    ty1 = os_graf_my - 1;
  }
  // the rectangle in correct order is tx,ty,tx1,ty1
  if (Update_rect.x < 0) {
    Update_rect.x = tx;
    Update_rect.w = tx1 - tx;
  }
  if (Update_rect.y < 0) {
    Update_rect.y = ty;
    Update_rect.h = ty1 - ty;
  }
  if (Update_rect.x > tx) {
    Update_rect.w = Update_rect.x + Update_rect.w - tx;
    Update_rect.x = tx;
  }
  if (Update_rect.x + Update_rect.w < tx1) {
    Update_rect.w = tx1 - Update_rect.x;
  }
  if (Update_rect.y > ty) {
    Update_rect.h = Update_rect.y + Update_rect.h - ty;
    Update_rect.y = ty;
  }
  if (Update_rect.y + Update_rect.h < ty1) {
    Update_rect.h = ty1 - Update_rect.y;
  }
  scr_update = 1;
}

unsigned char *zstr(char *a, char *b) {
  return (unsigned char *)strstr((char *)a, (char *)b);
}

/*
 * initialise system fonts and colors
 */
int init_font() {
  SDL_Color colors[256];
  int i;

  mouse_mode = mouse_x = mouse_y = mouse_b =
    mouse_upd = mouse_down_x = mouse_down_y = mouse_pc_x = mouse_pc_y = 0;
  tabsize = 32;                 // from dev_palm

  mouse_hot_x = -16;
  mouse_hot_y = -16;

  os_graphics = 1;
  os_graf_mx = screen->w;
  os_graf_my = screen->h;
  os_color_depth = screen->format->BitsPerPixel;
  os_color = 1;
  bytespp = screen->format->BytesPerPixel;

  if (os_color_depth == 8) {
    for (i = 0; i < 256; i++) {
      colors[i].r = i;
      colors[i].g = i;
      colors[i].b = i;
    }
    for (i = 0; i < 16; i++) {
#if defined(CPU_BIGENDIAN)
      colors[i].r = (vga16[i] & 0xFF0000) >> 16;
      colors[i].g = (vga16[i] & 0xFF00) >> 8;
      colors[i].b = (vga16[i] & 0xFF);
      cmap[i] = i;
#else
      colors[i].b = (vga16[i] & 0xFF0000) >> 16;
      colors[i].g = (vga16[i] & 0xFF00) >> 8;
      colors[i].r = (vga16[i] & 0xFF);
      cmap[i] = i;
#endif
    }
    SDL_SetColors(screen, colors, 0, 256);
  } else {
    for (i = 0; i < 16; i++) {
#if defined(CPU_BIGENDIAN)
      cmap[i] = SDL_MapRGB(screen->format,
                           (vga16[i] & 0xFF0000) >> 16, (vga16[i] & 0xFF00) >> 8, (vga16[i] & 0xFF));
#else
      cmap[i] = SDL_MapRGB(screen->format,
                           (vga16[i] & 0xFF), (vga16[i] & 0xFF00) >> 8, (vga16[i] & 0xFF0000) >> 16);
#endif
    }
  }
}

/**
 * set the font to initial startup values
 */
void reset_font() {
  con_use_bold = 0;
  con_use_ul = 0;
  con_use_reverse = 0;
  osd_setcolor(DEF_FG_COLOR);
  osd_setbgcolor(DEF_BG_COLOR);
  currentsystemfont = savedsystemfont;
  currentfont = fonttable[currentsystemfont][1];
  font_w = 8;
  font_h = 16;
  font_l = 1;                   // this is the length of a row in bytes equal
  // INT((font_w+7)/8)
  maxline = os_graf_my / font_h;
}

/**
 * Application entry point - prepare SDL surfaces before passing to 
 * smallbasic common code to ensure any load time messages are displayable
 */
 /*
  * Next ifdef main - undef main is a hack to go around a tricky SDL "feature"
  *   If in a file where #include "SDL.h" was somewhere and has main() function
  *   at least on windows systems the main will translate to winmain which means
  *   might be a problem compiling under MINGW (you need -lmingw32 -lSDLmain in that order)
  *   and we loose the stdout/stderr functionality because this will redirect to 
  *   a file called stdout/stderr.txt in the current directory.
  *   Means an SDL program will be a windows application instead of a console application!
  *   (SDL.h defining a macro called main :)
  */
#ifdef main
#undef main
#endif
int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL: Couldn't initialize SDL: %s\n", SDL_GetError());
    return 0;
  }

  /*
   * default video mode 
   */
  if (getenv("SB_SDLMODE") || getenv("SBGRAF")) {
    char *buf, *p, *ps;

    if (getenv("SBGRAF")) {
      ps = p = buf = strdup(getenv("SBGRAF"));
    } else {
      ps = p = buf = strdup(getenv("SB_SDLMODE"));
    }
    p = strchr(buf, 'x');
    if (p) {
      *p = '\0';
      dev_w = atoi(ps);
      ps = ++p;
      p = strchr(ps, 'x');
      if (p) {
        *p = '\0';
        dev_h = atoi(ps);
        ps = ++p;
        if (*p) {
          dev_d = atoi(ps);
        }
      } else if (*ps) {
        dev_h = atoi(ps);
      }
    }
    free(buf);
  }

  if (getenv("LC_ALL") || getenv("LC_CTYPE") || getenv("LANG")) {
    char *l;

    l = (char *)getenv("LC_ALL");
    if (!l) {
      l = (char *)getenv("LC_CTYPE");
    }
    if (!l) {
      l = (char *)getenv("LANG");
    }
    if (zstr(l, "en") || zstr(l, "fr") ||
        zstr(l, "de") || zstr(l, "dk") ||
        zstr(l, "it") || zstr(l, "es") || zstr(l, "nl") || zstr(l, "ISO-8859-1")) {
      savedsystemfont = currentsystemfont = 0;
      currentfont = font8x16i1;
    }
    if (zstr(l, "pl") || zstr(l, "hr") ||
        zstr(l, "hu") || zstr(l, "cs") ||
        zstr(l, "ro") || zstr(l, "sl") || zstr(l, "sk") || zstr(l, "ISO-8859-2")) {
      savedsystemfont = currentsystemfont = 1;
      currentfont = font8x16i2;
    }
    if (zstr(l, "ru") || zstr(l, "uk") || zstr(l, "bg") || zstr(l, "ISO-8859-5")) {
      savedsystemfont = currentsystemfont = 2;
      currentfont = font8x16i5;
    }
    if (zstr(l, "el") || zstr(l, "gr") || zstr(l, "ISO-8859-7")) {
      savedsystemfont = currentsystemfont = 3;
      currentfont = font8x16i7;
    }
  }

  sb_console_main(argc, argv);

  if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {   // if syntax error
    // happened no
    // osd_devrestore was
    // performed
    fast_exit = 1;
    osd_devrestore();
  }
  return 0;
}

/*
 * initialise the system prior to the next program execution
 */
int osd_devinit() {
  char cbuf[256];

  /*
   * setup video mode 
   */
  if (opt_pref_width || opt_pref_height) {
    dev_w = opt_pref_width;
    dev_h = opt_pref_height;
  }

  if (opt_pref_bpp) {
    dev_d = opt_pref_bpp;
  }

  screen = SDL_SetVideoMode(dev_w, dev_h, dev_d, SDL_ANYFORMAT | SDL_HWACCEL);
  if (screen == NULL) {
    fprintf(stderr,
            "SDL: Couldn't set %dx%dx%d video mode: %s\n"
            "Use SB_SDLMODE environment variable (set SB_SDLMODE=800x600x16)\n",
            dev_w, dev_h, dev_d, SDL_GetError());
    exit(1);
  }

  if (screen->format->BitsPerPixel != 8 &&
      screen->format->BitsPerPixel != 16 &&
      screen->format->BitsPerPixel != 24 && screen->format->BitsPerPixel != 32) {
    fprintf(stderr, "SDL: Couldn't set video mode\n");
    exit(1);
  }

  has_audio = (SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0);

  if (has_audio) {
    SDL_AudioSpec wanted;

    memset(&wanted, 0, sizeof(SDL_AudioSpec));

    /*
     * Set the audio format For calculation of parameters: freq/2 = gives the highest
     * frequency of generated tone 44100 means 22050 Hz is the highest format = the
     * quality of the tone 8 bit or 16 bit channels = 1 mono, 2 stereo samples = number of 
     * samples in a chunk will limit the lowest tone frequency for example 4096 means
     * 44100/4096 * 2 = 21 Hz is the lowest tone on the other hand that means on chunk is
     * 4096/44100 *1000 = 92 msec long the required buffer size is samples * format&0xff/8 
     * * channel - calculated by SDL 
     */
    wanted.freq = 44100;
    wanted.format = AUDIO_S8;
    wanted.channels = 1;
    wanted.samples = 4096;
    wanted.callback = audio_callback;
    wanted.userdata = NULL;

    // Open the audio device, forcing the desired format
    if (SDL_OpenAudio(&wanted, &audiospec) < 0) {
      has_audio = 0;
    } else {
      mixbuf = (char *)malloc(audiospec.size + 10);     // +10 just for to be safe
      SDL_PauseAudio(0);        // set state play
    }
  }
#ifdef HAVE_SDL_IMAGE
  Init_cache();
#endif

  // enable keyboard repeat
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  // Enable Unicode translation 
  SDL_EnableUNICODE(1);

  init_font();
  reset_font();

#if defined(_UnixOS)
  setsysvar_str(SYSVAR_OSNAME, "Unix/SDL");
#else
  setsysvar_str(SYSVAR_OSNAME, "Win32/SDL");
#endif
  setsysvar_int(SYSVAR_VIDADR, (int32) screen->pixels);

  snprintf(cbuf, 256, "SmallBASIC - %s", g_file);
  SDL_WM_SetCaption(cbuf, NULL);

  os_graf_mx = screen->w;       // need to reinitialize again because in brun.c 
  // sbasic_exec calling dev_init which calling
  // term_init 
  os_graf_my = screen->h;       // which overwrite our original value of
  // screen->w, screen->h

  // clear the keyboard queue
  dev_clrkb();
  return 1;
}

/*
 *   close
 */
int osd_devrestore() {
  cur_x = 0;
  cur_y = os_graf_my - font_h;

  if (!fast_exit) {             // if the user would like to see anything wait
    osd_write("Press any key to exit...");
    exit_app = 0;
    while (!exit_app && !dev_kbhit()) {
      SDL_Delay(50);            // wait for key hit but not eat the cpu!
    }
  }

  SDL_PauseAudio(1);            // pause after possible beep in dev_kbhit() to prevent lockup

  // if the user would like to quit w/o wait just set the same zero for fg and
  // bg like COLOR 
  // 0,0
  // and print at list one line (has new line character) 

#ifdef HAVE_SDL_IMAGE
  Clear_cache();
#endif
  SDL_Quit();
  if (mixbuf) {
    free(mixbuf);
  }
  return 1;
}

static long get_screen_color(int fgbg) {
  long color = 0;
  switch (fgbg) {
  case GET_FG_COLOR:
    if (dev_fgcolor < 0) {
      color = fg_screen_color;
    } else if (dev_fgcolor < 16) {
      color = cmap[dev_fgcolor];
    } else {
      color = -1;
    }
    break;
  case GET_BG_COLOR:
    if (dev_bgcolor < 0) {
      color = bg_screen_color;
    } else if (dev_bgcolor < 16) {
      color = cmap[dev_bgcolor];
    } else {
      color = -1;
    }
    break;
  default:
    break;
  }
  return color;
}

/*
 */
void direct_setpixel(int x, int y, long c) {
  int offset;

  if (c == -1) {
    return;                     // this case of transparent color
  }
  if (x >= 0 && x < os_graf_mx) {
    if (y >= 0 && y < os_graf_my) {
      offset = y * screen->pitch;
      switch (bytespp) {
      case 1:
        offset += x;
        *((byte *) (screen->pixels + offset)) = c;
        break;
      case 2:
        offset += (x << 1);
        *((word *) (screen->pixels + offset)) = c;
        break;
      case 3:
        offset += x * 3;
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *((byte *) (screen->pixels + offset)) = (c & 0xFF0000) >> 16;
        *((byte *) (screen->pixels + offset + 1)) = (c & 0xFF00) >> 8;
        *((byte *) (screen->pixels + offset + 2)) = (c & 0xFF);
#else
        *((byte *) (screen->pixels + offset)) = (c & 0xFF);
        *((byte *) (screen->pixels + offset + 1)) = (c & 0xFF00) >> 8;
        *((byte *) (screen->pixels + offset + 2)) = (c & 0xFF0000) >> 16;
#endif
        break;
      case 4:
        offset += (x << 2);
        *((long *)(screen->pixels + offset)) = c;
        break;
      }
    }
  }
}

/*
 *   return's the value of the pixel
 */
long osd_getpixel(int x, int y) {
  int offset, i;
  long color = 0;
  unsigned char r, g, b;

  if (x >= 0 && x < os_graf_mx) {
    if (y >= 0 && y < os_graf_my) {
      offset = y * screen->pitch;
      switch (bytespp) {
      case 1:
        offset += x;
        color = *((byte *) (screen->pixels + offset));
        break;
      case 2:
        offset += (x << 1);
        color = *((word *) (screen->pixels + offset));
        break;
      case 3:
        {
          byte *p;

          p = screen->pixels + offset + x * 3;
#if defined(CPU_BIGENDIAN)
          color = p[0] << 16 | p[1] << 8 | p[2];
#else
          color = p[0] | p[1] << 8 | p[2] << 16;
#endif
        }
        // case 4:
      default:
        offset += (x << 2);
        color = *((dword *) (screen->pixels + offset));
        break;
      }

      for (i = 0; i < 16; i++) {
        if (cmap[i] == color) {
          return i;
        }
      }
    }
  }
  SDL_GetRGB(color, screen->format, &r, &g, &b);
#if defined(CPU_IGENDIAN)
  color = (r << 16) + (g << 8) + b;
#else
  color = (b << 16) + (g << 8) + r;
#endif
  color = -color;
  return color;
}

/*
 */
void direct_hline(int x, int x2, int y) {
  long offset, i, len;

  if (fg_screen_color == -1) {
    return;                     // this case of transparent color
  }
  if (x > x2) {
    i = x;
    x = x2;
    x2 = i;
  }

  if (x < 0) {
    x = 0;
  }
  if (x2 < 0) {
    x2 = 0;
  }
  if (x2 >= os_graf_mx) {
    x2 = os_graf_mx - 1;
  }
  if (y < 0) {
    y = 0;
  }
  if (y >= os_graf_my) {
    y = os_graf_my - 1;
  }

  len = (x2 - x) + 1;
  switch (os_color_depth) {
  case 8:
    offset = y * os_graf_mx + x;
    memset(screen->pixels + offset, fg_screen_color, len);
    return;
  case 15:
  case 16:
    offset = y * (os_graf_mx << 1) + (x << 1);
    for (i = 0; i < len; i++, offset += 2)
      *((unsigned short int *)(screen->pixels + offset)) = fg_screen_color;
    return;
  case 24:
    offset = y * (os_graf_mx * 3) + (x * 3);
    for (i = 0; i < len; i++) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF0000) >> 16;
      offset++;
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF00) >> 8;
      offset++;
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF);
      offset++;
#else
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF);
      offset++;
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF00) >> 8;
      offset++;
      *((byte *) (screen->pixels + offset)) = (fg_screen_color & 0xFF0000) >> 16;
      offset++;
#endif
    }
    return;
  case 32:
    offset = y * (os_graf_mx << 2) + (x << 2);
    for (i = 0; i < len; i++, offset += 4) {
      *((long *)(screen->pixels + offset)) = fg_screen_color;
    }
    return;
  }
}

//
//#define SDL_FASTPIX(x,y)   *(((byte *)screen->pixels)+(y)*screen->w+(x)) = dev_fgcolor
void g_setpixel(int x, int y) {
  direct_setpixel(x, y, fg_screen_color);
}

/* Bresenham's algorithm for drawing line */
void direct_line(int x1, int y1, int x2, int y2) {
  if (y1 == y2) {
    direct_hline(x1, x2, y1);
    return;
  }
  g_line(x1, y1, x2, y2, g_setpixel);
}

void direct_fillrect(int x1, int y1, int x2, int y2, long c) {
  int i;
  long co = fg_screen_color;

  if (c == -1) {
    return;                     // this case of transparent color
  }
  fg_screen_color = c;
  for (i = y1; i <= y2; i++) {
    direct_hline(x1, x2, i);
  }
  fg_screen_color = co;
}

void osd_settextcolor(long fg, long bg) {
  osd_setcolor(fg);
  if (bg != -1) {
    osd_setbgcolor(bg);
  }
}

//
void osd_drawchar(int x, int y, byte ch, int overwrite, long fg_rgb, long bg_rgb) {
  int offset;
  int bit, i;
  unsigned char data, data1;

  offset = ch * font_l * font_h;

  if (font_w > 8) {
    for (i = 0; i < font_h; i++, offset += font_l) {
      data = *(currentfont + offset);
      data1 = *(currentfont + offset + 1);
      for (bit = 0; bit < 8; bit++) {
        if (data & (1 << (7 - bit))) {
          direct_setpixel(x + bit, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + bit, y + i, bg_rgb);
        }
      }
      switch (font_w) {
      case 16:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        if (data1 & (1 << 4)) {
          direct_setpixel(x + 11, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 11, y + i, bg_rgb);
        }
        if (data1 & (1 << 3)) {
          direct_setpixel(x + 12, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 12, y + i, bg_rgb);
        }
        if (data1 & (1 << 2)) {
          direct_setpixel(x + 13, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 13, y + i, bg_rgb);
        }
        if (data1 & (1 << 1)) {
          direct_setpixel(x + 14, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 14, y + i, bg_rgb);
        }
        if (data1 & (1 << 0)) {
          direct_setpixel(x + 15, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 15, y + i, bg_rgb);
        }
        break;
      case 15:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        if (data1 & (1 << 4)) {
          direct_setpixel(x + 11, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 11, y + i, bg_rgb);
        }
        if (data1 & (1 << 3)) {
          direct_setpixel(x + 12, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 12, y + i, bg_rgb);
        }
        if (data1 & (1 << 2)) {
          direct_setpixel(x + 13, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 13, y + i, bg_rgb);
        }
        if (data1 & (1 << 1)) {
          direct_setpixel(x + 14, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 14, y + i, bg_rgb);
        }
        break;
      case 14:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        if (data1 & (1 << 4)) {
          direct_setpixel(x + 11, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 11, y + i, bg_rgb);
        }
        if (data1 & (1 << 3)) {
          direct_setpixel(x + 12, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 12, y + i, bg_rgb);
        }
        if (data1 & (1 << 2)) {
          direct_setpixel(x + 13, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 13, y + i, bg_rgb);
        }
        break;
      case 13:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        if (data1 & (1 << 4)) {
          direct_setpixel(x + 11, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 11, y + i, bg_rgb);
        }
        if (data1 & (1 << 3)) {
          direct_setpixel(x + 12, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 12, y + i, bg_rgb);
        }
        break;
      case 12:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        if (data1 & (1 << 4)) {
          direct_setpixel(x + 11, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 11, y + i, bg_rgb);
        }
        break;
      case 11:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        if (data1 & (1 << 5)) {
          direct_setpixel(x + 10, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 10, y + i, bg_rgb);
        }
        break;
      case 10:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        if (data1 & (1 << 6)) {
          direct_setpixel(x + 9, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 9, y + i, bg_rgb);
        }
        break;
      case 9:
        if (data1 & (1 << 7)) {
          direct_setpixel(x + 8, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + 8, y + i, bg_rgb);
        }
        break;
      default:
        break;
      }
    }
  } else {
    for (i = 0; i < font_h; i++, offset++) {
      data = *(currentfont + offset);
      for (bit = 0; bit <= font_w; bit++) {
        if (data & (1 << (font_w - bit))) {
          direct_setpixel(x + bit, y + i, fg_rgb);
        } else if (overwrite) {
          direct_setpixel(x + bit, y + i, bg_rgb);
        }
      }
    }
  }
  make_update(x, y, x + font_w, y + font_h);
}

/*
 *   enable or disable PEN code
 */
void osd_setpenmode(int enable) {
  mouse_mode = enable;
}

/*
 */
int osd_getpen(int code) {
  int r = 0;

  osd_events(0);
  if (mouse_mode) {
    switch (code) {
    case 0:                    // bool: status changed
      r = mouse_upd;

      // wait for an event to prevent code like below
      // from eating 100% of the cpu:
      // while 1
      // if pen(0)
      // line pen(4),pen(5)
      // fi
      // wend
      SDL_WaitEvent(NULL);
      break;
    case 1:                    // last pen-down x
      r = mouse_down_x;
      break;
    case 2:                    // last pen-down y
      r = mouse_down_y;
      break;
    case 3:                    // vert. 1 = down, 0 = up .... unsupported
      r = 0;
      break;
    case 4:                    // last x
      r = mouse_pc_x;
      break;
    case 5:                    // last y
      r = mouse_pc_y;
      break;
    case 10:
      r = mouse_x;
      break;
    case 11:
      r = mouse_y;
      break;
    case 12:
    case 13:
    case 14:
      r = (mouse_b & (1 << (code - 11))) ? 1 : 0;
      break;
    }

    mouse_upd = 0;
  }
  return r;
}

/*
 *   clear screen
 */
void osd_cls() {
  cur_x = cur_y = 0;

  LOCK();
  direct_fillrect(0, 0, os_graf_mx - 1, os_graf_my - 1, get_screen_color(GET_BG_COLOR));
  make_update(0, 0, os_graf_mx - 1, os_graf_my - 1);
  UNLOCK();
}

//   returns the current x position
int osd_getx() {
  return cur_x;
}

//   returns the current y position
int osd_gety() {
  return cur_y;
}

//
void osd_setxy(int x, int y) {
  cur_x = x;
  cur_y = y;
}

/**
 *   next line
 */
void osd_nextln() {
  cur_x = 0;

  if (cur_y < (os_graf_my - font_h)) {
    cur_y += font_h;
  } else {
    int len, to;

    // scroll
    len = cur_y * bytespp * os_graf_mx;
    to = font_h * bytespp * os_graf_mx;
    if (to + len >= os_graf_my * screen->pitch) {
      len = ((os_graf_my * screen->pitch) - bytespp) - to;
    }
    memcpy((char *)screen->pixels, (char *)screen->pixels + to, len);
    cur_y = os_graf_my - 1 - font_h;
    direct_fillrect(0, cur_y, os_graf_mx - 1, os_graf_my - 1, get_screen_color(GET_BG_COLOR));
    make_update(0, 0, os_graf_mx - 1, os_graf_my - 1);
  }
}

/*
 *   calc next tab position
 */
int osd_calctab(int x) {
  int c = 1;

  while (x > tabsize) {
    x -= tabsize;
    c++;
  }
  return c * tabsize;
}

/**
 *   Basic output
 *
 *   Supported control codes:
 *   \t      tab (20 px)
 *   \a      beep
 *   \n      next line (cr/lf)
 *   \r      return (w/o lf)
 *   \xC      clear screen
 *   \e[K   clear to end of line
 *   \e[?G   jump to th ? character position in the current line
 *   \e[N    next is a graphic character - no interpret
 *   \e[0m   reset all attributes to their defaults
 *   \e[1m   set bold on
 *   \e[4m   set underline on
 *   \e[7m   reverse video
 *   \e[21m   set bold off
 *   \e[24m   set underline off
 *   \e[27m   set reverse off
 *   \e[3?m   set foreground color
 *   \e[4?m   set background color
 *   \e[8?m   set system font - aka character set
 *   \e[9?m   set SB font - aka font size
 */
void osd_write(const char *str) {
  int len, cx = font_w, esc_val, esc_cmd;
  byte *p, buf[3];
  static int next_is_graphic = 0;

  len = strlen(str);

  if (!screen) {
    fprintf(stderr, "%s", str);
    return;
  }

  if (len <= 0) {
    return;
  }
  // LOCK();

  p = (byte *) str;
  while (*p) {
    if (next_is_graphic) {
      // 
      // PRINT THE CHARACTER
      // 
      buf[0] = *p;
      buf[1] = '\0';

      // new line ?
      if (cur_x + cx >= os_graf_mx) {
        osd_nextln();
      }
      if (cur_y + font_h >= os_graf_my) {
        osd_nextln();
      }
      // draw

      // TODO: ??? SJIS on Linux ???
      if (!con_use_reverse) {
        osd_drawchar(cur_x, cur_y, *p, 1, get_screen_color(GET_FG_COLOR), get_screen_color(GET_BG_COLOR));
        if (con_use_bold) {
          osd_drawchar(cur_x - 1, cur_y, *p, 0, get_screen_color(GET_FG_COLOR),
                       get_screen_color(GET_BG_COLOR));
        }
      } else {
        osd_drawchar(cur_x, cur_y, *p, 1, get_screen_color(GET_BG_COLOR), get_screen_color(GET_FG_COLOR));
        if (con_use_bold) {
          osd_drawchar(cur_x - 1, cur_y, *p, 0, get_screen_color(GET_BG_COLOR),
                       get_screen_color(GET_FG_COLOR));
        }
      }

      if (con_use_ul) {
        osd_setcolor(dev_fgcolor);
        direct_line(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
        make_update(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
      }
      // advance
      cur_x += cx;
      next_is_graphic = 0;
    } else
      switch (*p) {             // evaluate the character in normal way
      case '\a':               // beep
        osd_beep();
        break;
      case '\t':
        cur_x = osd_calctab(cur_x + 1);
        break;
      case '\xC':
        osd_cls();
        break;
      case '\033':             // ESC ctrl chars (man console_codes)
        if (*(p + 1) == '[') {
          p += 2;
          esc_val = esc_cmd = 0;

          if (is_digit(*p)) {
            esc_val = (*p - '0');
            p++;

            if (is_digit(*p)) {
              esc_val = (esc_val * 10) + (*p - '0');
              p++;
            }

            esc_cmd = *p;
          } else {
            esc_cmd = *p;
          }

          // control characters
          switch (esc_cmd) {
          case 'K':            // \e[K - clear to eol
            direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h, get_screen_color(GET_BG_COLOR));
            make_update(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h);
            break;
          case 'G':
            dev_setxy(esc_val * font_w, dev_gety(), 0);    // default font = 9x16
            break;
          case 'N':
            next_is_graphic = 1;
            break;
          case 'm':            // \e[...m - ANSI terminal
            switch (esc_val) {
            case 0:            // reset
              reset_font();
              cx = font_w;
              break;
            case 1:            // set bold on
              con_use_bold = 1;
              break;
            case 4:            // set underline on
              con_use_ul = 1;
              break;
            case 7:            // reverse video on
              con_use_reverse = 1;
              break;
            case 21:           // set bold off
              con_use_bold = 0;
              break;
            case 24:           // set underline off
              con_use_ul = 0;
              break;
            case 27:           // reverse video off
              con_use_reverse = 0;
              break;

              // colors - 30..37 foreground, 40..47 background
            case 30:           // set black fg
              osd_setcolor(0);
              break;
            case 31:           // set red fg
              osd_setcolor(4);
              break;
            case 32:           // set green fg
              osd_setcolor(2);
              break;
            case 33:           // set brown fg
              osd_setcolor(6);
              break;
            case 34:           // set blue fg
              osd_setcolor(1);
              break;
            case 35:           // set magenta fg
              osd_setcolor(5);
              break;
            case 36:           // set cyan fg
              osd_setcolor(3);
              break;
            case 37:           // set white fg
              osd_setcolor(7);
              break;

            case 40:           // set black bg
              osd_settextcolor(dev_fgcolor, 0);
              break;
            case 41:           // set red bg
              osd_settextcolor(dev_fgcolor, 4);
              break;
            case 42:           // set green bg
              osd_settextcolor(dev_fgcolor, 2);
              break;
            case 43:           // set brown bg
              osd_settextcolor(dev_fgcolor, 6);
              break;
            case 44:           // set blue bg
              osd_settextcolor(dev_fgcolor, 1);
              break;
            case 45:           // set magenta bg
              osd_settextcolor(dev_fgcolor, 5);
              break;
            case 46:           // set cyan bg
              osd_settextcolor(dev_fgcolor, 3);
              break;
            case 47:           // set white bg
              osd_settextcolor(dev_fgcolor, 7);
              break;
              // select system fonts aka code set Latin 1/2/5/7
            case 80:
              currentsystemfont = 0;
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
            case 81:
              currentsystemfont = 1;
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
            case 82:
              currentsystemfont = 2;
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
            case 83:
              currentsystemfont = 3;
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
            case 84:
            case 85:
            case 86:
            case 87:           // reset to the defoult system font
              currentsystemfont = savedsystemfont;
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
              // select SB fonts
            case 90:           // font size 6x9
              currentfont = fonttable[currentsystemfont][0];
              font_h = 9;
              maxline = os_graf_my / font_h;
              cx = font_w = 6;
              font_l = 1;
              break;
            case 91:           // font size reset to default
              currentfont = fonttable[currentsystemfont][1];
              font_h = 16;
              maxline = os_graf_my / font_h;
              cx = font_w = 8;
              font_l = 1;
              break;
            case 92:           // font size 10x20
              currentfont = fonttable[currentsystemfont][2];
              font_h = 20;
              maxline = os_graf_my / font_h;
              cx = font_w = 10;
              font_l = 2;
              break;
            case 93:           // font size 16x29
              currentfont = fonttable[currentsystemfont][3];
              font_h = 29;
              maxline = os_graf_my / font_h;
              cx = font_w = 16;
              font_l = 2;
              break;
            };
            break;
          default:
            {
              char buf[128];
              sprintf(buf, "Unknown escape sequence command after [ (%d)\n", esc_cmd);
              osd_write(buf);
              osd_refresh();
            }
            break;
          }
        } else {
          {
            char buf[128];
            sprintf(buf, "Unknown escape sequence command (%d)\n", *(p + 1));
            osd_write(buf);
            osd_refresh();
          }
        }
        break;
      case '\n':               // new line
        // UNLOCK();
        osd_nextln();
        if ((dev_fgcolor == dev_bgcolor) && (dev_fgcolor == 0)) {
          fast_exit = 1;        // the user don't want to see anything at the
          // end
        }
        // LOCK();
        break;
      case '\r':               // return
        cur_x = 0;
        direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h, get_screen_color(GET_BG_COLOR));
        make_update(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h);
        break;
      default:
        // 
        // PRINT THE CHARACTER
        // 
        buf[0] = *p;
        buf[1] = '\0';

        // new line ?
        if (cur_x + cx >= os_graf_mx) {
          osd_nextln();
        }
        if (cur_y + font_h >= os_graf_my) {
          osd_nextln();
        }
        // draw

        // TODO: ??? SJIS on Linux ???
        if (!con_use_reverse) {
          osd_drawchar(cur_x, cur_y, *p, 1, get_screen_color(GET_FG_COLOR), get_screen_color(GET_BG_COLOR));
          if (con_use_bold) {
            osd_drawchar(cur_x - 1, cur_y, *p, 0, get_screen_color(GET_FG_COLOR),
                         get_screen_color(GET_BG_COLOR));
          }
        } else {
          osd_drawchar(cur_x, cur_y, *p, 1, get_screen_color(GET_BG_COLOR), get_screen_color(GET_FG_COLOR));
          if (con_use_bold)
            osd_drawchar(cur_x - 1, cur_y, *p, 0, get_screen_color(GET_BG_COLOR),
                         get_screen_color(GET_FG_COLOR));
        }

        if (con_use_ul) {
          osd_setcolor(dev_fgcolor);
          direct_line(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
          make_update(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
        }
        // advance
        cur_x += cx;
      };

    if (*p == '\0') {
      break;
    }

    p++;
  }

  // UNLOCK();
}

/*
 * check SDL's events
 */
int osd_events(int wait_flag) {
  int ch, button, i;
  int evc = 0;
  SDL_Event ev;

  if (scr_update) {
    // refresh
    SDL_UpdateRect(screen, Update_rect.x, Update_rect.y, Update_rect.w + 1, Update_rect.h + 1);
    scr_update = 0;
    Update_rect.x = -1;
    Update_rect.y = -1;
    Update_rect.w = 0;
    Update_rect.h = 0;
  }

  if (wait_flag) {
    SDL_WaitEvent(NULL);
  }

  if (SDL_PollEvent(&ev)) {
    switch (ev.type) {
    case SDL_QUIT:
      exit_app = 1;
      return -2;

    case SDL_KEYDOWN:
      ch = ev.key.keysym.sym;
      if ((ch == SDLK_c && (ev.key.keysym.mod & KMOD_CTRL)) || ch == SDLK_BREAK) {
        exit_app = 1;
        // break
        return -2;
      } else {
        // dev_printf("--- K=0x%X ---", ch);
        // scan keymap
        for (i = 0; keymap[i] != 0; i += 2) {
          if (keymap[i] == ch) {        // !!!!!!!!!!!!!!!
            if (keymap[i + 1] != -1) {
              dev_pushkey(keymap[i + 1]);
            }
            ch = -1;
            break;
          }
        }

        // not found
        if (ch > 0 && ev.key.keysym.unicode) {
          // using kernel keyboard mapping
          dev_pushkey(ev.key.keysym.unicode & 0xFF);
        }
        // CTRL, ALT(group), SHIFT already interpreted
      }

      evc++;
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
      if (mouse_mode) {
        button = SDL_GetMouseState(&mouse_x, &mouse_y);

        if (mouse_x < mouse_hot_x) {
          mouse_x = mouse_hot_x;
        }
        if (mouse_y < mouse_hot_y) {
          mouse_y = mouse_hot_y;
        }
        if (mouse_x >= os_graf_mx - 1) {
          mouse_x = os_graf_mx - 1;
        }
        if (mouse_y >= os_graf_my - 1) {
          mouse_y = os_graf_my - 1;
        }
        mouse_b = 0;            // / bug
        if (button & SDL_BUTTON(1)) {
          if ((mouse_b & SDL_BUTTON(1)) == 0) { // new press
            mouse_down_x = mouse_x;
            mouse_down_y = mouse_y;
          }
          mouse_upd = 1;
          mouse_pc_x = mouse_x;
          mouse_pc_y = mouse_y;
          mouse_b |= 1;
        }
        if (button & SDL_BUTTON(2)) {
          mouse_b |= 2;
        }
        if (button & SDL_BUTTON(3)) {
          mouse_b |= 4;
        }
        evc++;
      }
    }
  }
  return evc;
}

///////////////////////////////////////////////////////////////

static void osd_setbgcolor(long color) {
  dev_bgcolor = color;
  if (color >= 0 && color <= 15) {
    bg_screen_color = cmap[color];
  } else if (color < 0) {
    bg_screen_color = -color;
#if defined(CPU_BIGENDIAN)
    bg_screen_color = SDL_MapRGB(screen->format,
                                 (bg_screen_color & 0xFF0000) >> 16,
                                 (bg_screen_color & 0xFF00) >> 8, (bg_screen_color & 0xFF));
#else
    bg_screen_color = SDL_MapRGB(screen->format,
                                 (bg_screen_color & 0xFF),
                                 (bg_screen_color & 0xFF00) >> 8, (bg_screen_color & 0xFF0000) >> 16);
#endif
  }
}

void osd_setcolor(long color) {
  dev_fgcolor = color;
  if (color >= 0 && color <= 15) {
    fg_screen_color = cmap[color];
  } else if (color < 0) {
    fg_screen_color = -color;
#if defined(CPU_BIGENDIAN)
    fg_screen_color = SDL_MapRGB(screen->format,
                                 (fg_screen_color & 0xFF0000) >> 16,
                                 (fg_screen_color & 0xFF00) >> 8, (fg_screen_color & 0xFF));
#else
    fg_screen_color = SDL_MapRGB(screen->format,
                                 (fg_screen_color & 0xFF),
                                 (fg_screen_color & 0xFF00) >> 8, (fg_screen_color & 0xFF0000) >> 16);
#endif
  }
}

void osd_line(int x1, int y1, int x2, int y2) {
  LOCK();

  if ((x1 == x2) && (y1 == y2)) {
    direct_setpixel(x1, y1, fg_screen_color);
  } else {
    direct_line(x1, y1, x2, y2);
  }
  make_update(x1, y1, x2, y2);
  UNLOCK();
}

void osd_setpixel(int x, int y) {
  LOCK();

  direct_setpixel(x, y, fg_screen_color);
  make_update(x, y, x + 1, y + 1);
  UNLOCK();
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  int y;

  LOCK();

  if (fill) {
    for (y = y1; y <= y2; y++) {
      direct_hline(x1, x2, y);
    }
  } else {
    direct_line(x1, y1, x1, y2);
    direct_line(x1, y2, x2, y2);
    direct_line(x2, y2, x2, y1);
    direct_line(x2, y1, x1, y1);
  }
  make_update(x1, y1, x2 + 1, y2 + 1);

  UNLOCK();
}

///////////////////////////////////////////////////////////////

void osd_sound(int freq, int ms, int vol, int bgplay) {
  struct voice_info *info;
  int i, loops, last_loop, lchunk;

  if (has_audio) {
    if (!freq) {
      freq = audiospec.freq / 2;
    }
    // wrong!! loops = ((ms * audiospec.freq / freq / 2.0) / audiospec.samples) 
    // / 1000;
    // calculate how many chunk needed for the required ms
    lchunk = audiospec.samples * 1000 / audiospec.freq;
    // one chunk longness in milisec - take care on int rounding!!
    loops = (int)ms / lchunk;   // the longness is rounded to chunk size!! cc.
    // 100
    // msec!
    last_loop = ms - (int)(loops * lchunk);     // length of last chunk in msec

    SDL_LockAudio();

    info = &audio_info[audio_tail];
    if (vol < 0)
      vol = 0;
    if (vol > 100)
      vol = 100;                // volume should be between 0 and 100 according 
    // to spec!
    info->vol = vol;
    // the correct info->setting is according to the volume because we don't
    // know hardware 
    // mixer is working or not!
    info->setting = ((audiospec.format & 0xff) == 0x8) ? 0x7f : 0x7fff;
    // 8 or 16 bit

    // sample
    // size?
    info->setting = (info->setting * info->vol) / 100;  // take care on integer
    // rounding!

    if ((freq < audiospec.freq / 2) && (freq != 0)) {
      info->period = (int)(audiospec.freq / freq / 2.0);
    } else {
      info->period = 0;
      // we generate silent!! for example sound 32767 
      // definitely
      // larger then 22050!
    }
    info->toggle = info->period;

    // 

    if (loops)
      for (i = 1; i <= loops; i++) {    // handling consecutive chunks - long
        // sound
        audio_info[audio_tail] = *info;
        audio_info[audio_tail].remain_ms = lchunk;
        // next line try to compensate the noise if the end of the previous
        // chunk are
        // in a negativ half period
        if (info->period > 1) {
          if (!((((i - 1) * lchunk) / info->period) & 1)) {
            audio_info[audio_tail].setting = -info->setting;
          }
        }
        audio_tail++;
        if (audio_tail >= AUDIO_STACK_MAX) {
          audio_tail = 0;
        }
      }
    if (last_loop)              // handling the last chunk from a long sound or 
      // a quick one
    {
      audio_info[audio_tail] = *info;
      audio_info[audio_tail].remain_ms = last_loop;
      // next line try to compensate the noise if the end of a chunk are in a
      // negativ
      // half period
      // if no any loops (loops is zero) this does nothing might be the wave
      // start with
      // a negative half
      if (info->period > 1) {
        if (!(((loops * lchunk) / info->period) & 1)) {
          audio_info[audio_tail].setting = -info->setting;
        }
      }
      audio_tail++;
      if (audio_tail >= AUDIO_STACK_MAX) {
        audio_tail = 0;
      }
    }

    SDL_UnlockAudio();

    if (!bgplay) {
      while (audio_head != audio_tail) {
        SDL_Delay(10);
      }
    }
  }
}

void osd_beep() {
  if (has_audio) {
    osd_sound(440, 250, 75, 0);
  } else {
    printf("\a");
  }
}

void osd_clear_sound_queue() {
  if (has_audio) {
    SDL_LockAudio();
    audio_head = audio_tail;
    SDL_UnlockAudio();
  }
}

///////////////////////////////////////////////////////////////

int osd_textwidth(const char *str) {
  int l = strlen(str);

  // SJIS ???
  return (l * font_w);
}

int osd_textheight(const char *str) {
  // TODO: count CRLF or just LF or just CR or ????
  return font_h;
}

void osd_refresh() {
  if (scr_update) {
    // refresh
    SDL_UpdateRect(screen, Update_rect.x, Update_rect.y, Update_rect.w + 1, Update_rect.h + 1);
    scr_update = 0;
    Update_rect.x = -1;
    Update_rect.y = -1;
    Update_rect.w = 0;
    Update_rect.h = 0;
  }
}

#ifdef HAVE_SDL_IMAGE
// image part 

static void Init_cache() {
  int i;

  for (i = 0; i < MAX_IMAGE_IN_CACHE; ++i) {
    i_cache[i].image_entry = NULL;
    i_cache[i].file_name = NULL;
    i_cache[i].usage_count = 0;
  }
  maxpixelcount = 0;
}

void Clear_cache() {
  int i;

  for (i = 0; i < MAX_IMAGE_IN_CACHE; ++i) {
    if (i_cache[i].image_entry != NULL) {
      SDL_FreeSurface(i_cache[i].image_entry);
      i_cache[i].image_entry = NULL;
    }
    if (i_cache[i].file_name != NULL) {
      free(i_cache[i].file_name);
      i_cache[i].file_name = NULL;
    }
    i_cache[i].usage_count = 0;
  }
  maxpixelcount = 0;
}

static SDL_Surface *Look_in_cache(char *filename) {
  int i;

  for (i = 0; i < MAX_IMAGE_IN_CACHE; ++i) {
    if (i_cache[i].file_name)
      if (0 == strcmp(filename, i_cache[i].file_name)) {
        i_cache[i].usage_count++;
        return i_cache[i].image_entry;
      }
  }
  return NULL;
}

static void Add_to_cache(SDL_Surface * image, char *filename) {
  int i, save;
  long usage;

  if (image->w * image->h > MAX_IMAGE_PIXEL_COUNT) {
    return;                     // if the image alone larger than the cache
    // size just return.
  }
  for (i = 0; i < MAX_IMAGE_IN_CACHE; ++i) {
    if (i_cache[i].file_name)
      if (0 == strcmp(filename, i_cache[i].file_name))
        return;                 // already in cache
  }
  if ((maxpixelcount + image->w * image->h) < MAX_IMAGE_PIXEL_COUNT)
    for (i = 0; i < MAX_IMAGE_IN_CACHE; ++i) {
      if (!i_cache[i].file_name) {      // we found a not used entry
        i_cache[i].file_name = strdup(filename);
        i_cache[i].image_entry = image;
        i_cache[i].usage_count = 0;
        maxpixelcount += image->w * image->h;
        return;                 // done
      }
    }

  do {
    // not in cache and no free space - look for the lowest usage count
    usage = i_cache[0].usage_count;
    save = 0;
    for (i = 1; i < MAX_IMAGE_IN_CACHE; ++i) {
      if (usage < i_cache[i].usage_count) {
        usage = i_cache[i].usage_count;
        save = i;
      }
    }
    // save contain the lowest usage count cache entry index
    if (i_cache[save].image_entry) {
      maxpixelcount -= i_cache[save].image_entry->w * i_cache[save].image_entry->h;
      SDL_FreeSurface(i_cache[save].image_entry);       // free the memory
      free(i_cache[save].file_name);
      i_cache[save].image_entry = NULL;
      i_cache[save].file_name = NULL;
      i_cache[save].usage_count = 0;
    } else
      i_cache[save].usage_count = 0;    // should not be happen but ....
  } while ((maxpixelcount + image->w * image->h) > MAX_IMAGE_PIXEL_COUNT);

  i_cache[save].file_name = strdup(filename);
  i_cache[save].image_entry = image;
  i_cache[save].usage_count = 0;
  maxpixelcount += image->w * image->h;
  return;
}

void dev_image(int handle, int index, int x, int y, int sx, int sy, int w, int h) {
  SDL_Rect srect = { 0, 0, 0, 0 };
  SDL_Rect drect = { 0, 0, 0, 0 };
  char BMPfilename[256];
  int savehandle;
  int saveflags;
  char buf[256];
  SDL_Surface *img, *img1;
  long transparentcolor;

  if (screen == NULL)
    return;                     // SDL is not initialized
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0) {
    return;
  }

  strcpy((char *)&BMPfilename, filep->name);
  savehandle = handle;
  saveflags = filep->open_flags;
  transparentcolor = 0;
  if (filep->open_flags == DEV_FILE_INPUT) {
    if (!(img = Look_in_cache(filep->name))) {
      // dev_fclose(handle); 
      // We have to close the file because SDL_LoadBMP do not use a shared
      // open!
      // img1 = SDL_LoadBMP((char *) &BMPfilename);
      img1 = IMG_Load_RW(SDL_RWFromFile((const char *)&BMPfilename, "rb"), 1);  // SDL_image 
      // 
      // support!
      if (img1 == NULL) {
        // sprintf(buf, "Couldn't load %s: %s\n", filep->name, SDL_GetError());
        sprintf(buf, "Couldn't load %s: %s\n", filep->name, IMG_GetError());
        osd_write(buf);
        osd_refresh();
        return;
      }
      img = SDL_DisplayFormat(img1);    // convert the image to the same format
      // of
      // screen for fast blitting
      SDL_FreeSurface(img1);
      // dev_fopen(handle,(char *) &BMPfilename,saveflags); // reopen the file
      // - rewind
      if (img)
        Add_to_cache(img, filep->name);
    }
    if (img) {
      // input/read image and display
      srect.w = w == 0 ? img->w : w;
      srect.h = h == 0 ? img->h : h;
      srect.x = sx;
      srect.y = sy;
      drect.x = x;
      drect.y = y;              // drect.w, .h ignored as input by BlitSurface
      // but updated!
      if (index < 0) {
        transparentcolor = -index;
#if defined(CPU_BIGENDIAN)
        transparentcolor = SDL_MapRGB(img->format,
                                      (transparentcolor & 0xFF0000) >> 16,
                                      (transparentcolor & 0xFF00) >> 8, (transparentcolor & 0xFF));
#else
        transparentcolor = SDL_MapRGB(img->format,
                                      (transparentcolor & 0xFF),
                                      (transparentcolor & 0xFF00) >> 8, (transparentcolor & 0xFF0000) >> 16);
#endif
        SDL_SetColorKey(img, SDL_SRCCOLORKEY, transparentcolor);
      } else                    // clear colorkeying even if it was declared in 
        // the image file
        SDL_SetColorKey(img, 0, 0);
      SDL_BlitSurface(img, &srect, screen, &drect);
      make_update(drect.x, drect.y, drect.x + drect.w, drect.y + drect.h);
      osd_refresh();            // show the screen
    } else {
      sprintf(buf, "Couldn't load %s: %s\n", filep->name, SDL_GetError());
      osd_write(buf);
      osd_refresh();
      return;
    }
  } else {
    // output selected area of screen image to BMP
    img1 = IMG_Load_RW(SDL_RWFromFile((const char *)&BMPfilename, "rb"), 1);
    if (img1 != NULL) {
      int ww, hh;
      ww = (img1->w > sx + w) ? img1->w : sx + w;
      hh = (img1->h > sy + h) ? img1->h : sy + h;
      // create enough large surface to hold the image in the file and the
      // saved one
      img = SDL_CreateRGBSurface(SDL_SWSURFACE, ww, hh, screen->format->BitsPerPixel,
                                 screen->format->Rmask, screen->format->Gmask,
                                 screen->format->Bmask, screen->format->Amask);
      SDL_FillRect(img, NULL, get_screen_color(GET_BG_COLOR));
      // fill the surface with the background color
      SDL_BlitSurface(img1, NULL, img, NULL);   // copy the whole original image
      // to
      // 0,0
      SDL_FreeSurface(img1);
    } else
      img = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, screen->format->BitsPerPixel,
                                 screen->format->Rmask, screen->format->Gmask,
                                 screen->format->Bmask, screen->format->Amask);
    srect.x = x;
    drect.x = sx;
    srect.y = y;
    drect.y = sy;
    srect.w = w;
    drect.w = w;
    srect.h = h;
    drect.h = h;
    SDL_BlitSurface(screen, &srect, img, &drect);
    dev_fclose(handle);         // We have to close the file because
    // SDL_SaveBMP do not use a
    // shared open!
    SDL_SaveBMP(img, BMPfilename);
    SDL_FreeSurface(img);
    dev_fopen(handle, (char *)&BMPfilename, DEV_FILE_APPEND);   // reopen the
    // file
    // with APPEND!!!
  }
}

int dev_image_width(int handle, int index) {
  int i = -1;
  char BMPfilename[256];
  int savehandle;
  int saveflags;
  dev_file_t *filep = dev_getfileptr(handle);
  SDL_Surface *img, *img1;

  if (screen == NULL)
    return 0;                   // SDL is not initialized
  if (filep) {
    if (filep->open_flags == DEV_FILE_INPUT) {
      strcpy((char *)&BMPfilename, filep->name);
      savehandle = handle;
      saveflags = filep->open_flags;
      if (!(img = Look_in_cache(filep->name))) {
        // dev_fclose(handle); 
        // We have to close the file because SDL_SaveBMP do not use a shared
        // open!
        // img1 = SDL_LoadBMP((char *) &BMPfilename);
        img1 = IMG_Load_RW(SDL_RWFromFile((const char *)&BMPfilename, "rb"), 1);
        if (img1 == NULL) {
          char buf[256];
          // sprintf(buf, "Couldn't load %s: %s\n", filep->name,
          // SDL_GetError());
          sprintf(buf, "Couldn't load %s: %s\n", filep->name, IMG_GetError());
          osd_write(buf);
          osd_refresh();
          return 0;
        }
        img = SDL_DisplayFormat(img1);  // convert the image to the same format 
        // of
        // screen for fast blitting
        SDL_FreeSurface(img1);
        // dev_fopen(handle,(char *) &BMPfilename,saveflags); // reopen the
        // file
      }
      if (img != 0) {
        i = img->w;
        /*
         * Cache the allocated BMP surface 
         */
        Add_to_cache(img, filep->name);
      }
    }
  }
  return i;
}

int dev_image_height(int handle, int index) {
  int i = -1;
  char BMPfilename[256];
  int savehandle;
  int saveflags;
  dev_file_t *filep = dev_getfileptr(handle);
  SDL_Surface *img, *img1;

  if (screen == NULL)
    return 0;                   // SDL is not initialized
  if (filep) {
    if (filep->open_flags == DEV_FILE_INPUT) {
      strcpy((char *)&BMPfilename, filep->name);
      savehandle = handle;
      saveflags = filep->open_flags;
      if (!(img = Look_in_cache(filep->name))) {
        // dev_fclose(handle); 
        // We have to close the file because SDL_SaveBMP do not use a shared
        // open!
        // img1 = SDL_LoadBMP((char *) &BMPfilename);
        img1 = IMG_Load_RW(SDL_RWFromFile((const char *)&BMPfilename, "rb"), 1);
        if (img1 == NULL) {
          char buf[256];
          // sprintf(buf, "Couldn't load %s: %s\n", filep->name,
          // SDL_GetError());
          sprintf(buf, "Couldn't load %s: %s\n", filep->name, IMG_GetError());
          osd_write(buf);
          osd_refresh();
          return 0;
        }
        img = SDL_DisplayFormat(img1);  // convert the image to the same format 
        // of
        // screen for fast blitting
        SDL_FreeSurface(img1);
        // dev_fopen(handle,(char *) &BMPfilename,saveflags); // reopen the
        // file
      }
      if (img != 0) {
        i = img->h;
        /*
         * Add the allocated BMP surface to cache
         */
        Add_to_cache(img, filep->name);
      }
    }
  }
  return i;
}

#else

// HAVE_SDL_IMAGE not defined
int dev_image_width(int handle, int index) {
  return -1;
}

int dev_image_height(int handle, int index) {
  return -1;
}

void dev_image(int handle, int index, int x, int y, int sx, int sy, int w, int h) {
}

#endif
