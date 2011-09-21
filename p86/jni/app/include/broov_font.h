#ifndef BROOV_FONT_H
#define BROOV_FONT_H

int broov_draw_msg(SDL_Surface *screen, int x, int y, const char *msg);
int broov_font_init(char *font_fname, int my_font_size);
void free_font();
int subtitle_draw_msg(SDL_Surface *screen, int x, int y, const char *msg);
int overlay_subtitle_draw_msg(SDL_Surface *screen, int x, int y, const char *msg);
SDL_Surface* get_rendered_text(const char *msg);
SDL_Surface* get_rendered_text_unicode(const char *msg);
int overlay_transparent(SDL_Surface* TheSurface, int r, int g, int b);
int fps_draw_msg(SDL_Surface *screen, int x, int y, const char *msg);
int draw_msg(SDL_Surface *screen, int x, int y, const char *msg);
int overlay_subtitle_draw_msg_type2(SDL_Surface *screen, int x, int y, const char *msg);

#endif /* #ifndef BROOV_FONT_H */
