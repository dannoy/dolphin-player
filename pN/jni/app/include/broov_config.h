#ifndef BROOV_CONFIG_H
#define BROOV_CONFIG_H 

/* List of global configurations */

/* List of resource data structures */
extern unsigned char dejavu_sans_ttf[];
extern unsigned int  dejavu_sans_ttf_len;

extern unsigned char bg_loading[];
extern unsigned int  bg_loading_len;

#define BROOV_BLACK_SCREEN_IMAGE 200
#define BROOV_LOADING_IMAGE      201

#define BROOV_PREV_BUTTON_CLICKED 100
#define BROOV_NEXT_BUTTON_CLICKED 101
#define BROOV_FF_QUIT             102
#define BROOV_SDL_QUIT            103

extern  int fs_screen_width;
extern  int fs_screen_height;

extern int sx, sy, sw, sh;

#include "prof.h"
#endif /* #ifdef BROOV_CONFIG_H  */
