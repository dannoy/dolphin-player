#ifndef BROOV_GUI_H
#define BROOV_GUI_H 

#include "SDL.h"
#include "broov_types.h"

extern int sx, sy, sw, sh;
extern int sxleft, sstep;

void broov_gui_init();
void broov_gui_clean();

void broov_gui_show_image(SDL_Surface *screen);
void broov_gui_show_video_image(SDL_Surface *screen);

void broov_gui_init_ds();
void broov_gui_init_audio_image(int audio_image_idx);
void broov_gui_clean_audio_image();
void broov_gui_update_positions();

#endif /* #ifndef BROOV_GUI_H */
