/* FNT.H */
/* Custom Font Driver */
/* Earle F. Philhower, III */
/* earle@ziplabel.com */

// Our fonts
#define smallFont       0
#define stdFont	        1
#define smallFixedFont	2
#define stdFixedFont	3
#define userFont        4
#define userFont0       4
#define userFont1       5
#define userFont2       6
#define userFont3       7
#define userFont4       8
#define userFont5       9
#define userFont6      10
#define userFont7      11

#define FontID unsigned int

#define noUnderline 0
#define solidUnderline 1

int FntLineHeight(void);  // Height of character + linespace
int FntCharHeight(void);  // Height of just character
int FntCharsWidth(const unsigned char *string, int len); // Width of string when displayed
int FntStringWidth(const unsigned char *string); // Width of string when displayed
int FntCharWidth(unsigned char chr); // Width of character when displayed
void FntSetFont(int fontID); // Set font to ID
int FntGetFont(void); // Current font
void FntSetUnderlineMode(int underline); // 0=no underline, !0=underline
int FntIsFixedWidth(int fontID); // Is font fixed width?
void FntDrawChars(const unsigned char *string, int len, int xstart, int ystart, int invert); // x,y from upper-left hand corner
void FntDrawChar(unsigned char letter, int xstart, int ystart, int invert); // x,y from upper-left hand corner
void FntDrawString(const unsigned char *string, int xstart, int ystart, int invert); // x,y from upper-left hand corner
void FntBeep(); // Play a beep on speaker
void FntClearScreen(); // Clear entire display
void FntInstallFont(const char *filename);
void FntInstallFontNum(const char *filename, int number);

// Where should this go???
void StippleBackground();
void *SaveBackground();
void RestoreBackground(void *save);
