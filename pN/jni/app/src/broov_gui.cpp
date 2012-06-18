#include "broov_gui.h"
#include "broov_player.h"

#include <android/log.h>
#include "SDL_image.h"

#include "broov_config.h"

SDL_RWops     *rw4;             // Background image file control
SDL_Surface   *backgroundImage;
SDL_TextureID  backgroundTexture;
SDL_Rect       background_rect = {0, 0, 640, 480};
SDL_Rect       loading_rect = {280, 200, 80, 80};

//int sx=0, sy=420, sw=640, sh=60;
int sx=0, sy=400, sw=640, sh=60;
int sxleft=40, sstep=20;


static int g_audio_image_idx;

bool create_transparent_surface(SDL_Surface* the_surface, int r, int g, int b)
{
	if (the_surface == NULL)
		return false;

	else SDL_SetColorKey(the_surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(the_surface->format, r, g, b));

	return true;
}

void broov_gui_init()
{
}

void broov_gui_clean()
{
	broov_gui_clean_audio_image();

	IMG_Quit();

}

void broov_gui_show_video_image(SDL_Surface *screen)
{
	if (g_audio_image_idx == BROOV_LOADING_IMAGE) {
		SDL_RenderCopy(backgroundTexture, NULL, &loading_rect);
		SDL_RenderPresent();
	}
	else if (g_audio_image_idx == BROOV_BLACK_SCREEN_IMAGE) {
		SDL_RenderCopy(backgroundTexture, NULL, &background_rect);
		SDL_RenderPresent();
	}

}

void broov_gui_show_image(SDL_Surface *screen)
{
	SDL_RenderCopy(backgroundTexture, NULL, &background_rect);
}


void broov_gui_init_audio_image(int audio_image_idx)
{
	broov_gui_clean_audio_image();
	g_audio_image_idx = audio_image_idx;
	switch (audio_image_idx) {

		case BROOV_LOADING_IMAGE:
		default:
			rw4 = SDL_RWFromMem(bg_loading, bg_loading_len);
			break;

	}

	if (rw4) {
                char image[] = "JPG";
		SDL_Surface *backgroundImage = IMG_LoadTyped_RW(rw4, 1, image);
		if (backgroundImage) {
			backgroundTexture = SDL_CreateTextureFromSurface(0, backgroundImage);
		}
	}

}

void broov_gui_init_ds()
{

}

void broov_gui_clean_audio_image()
{
	if (backgroundTexture) {
		SDL_DestroyTexture(backgroundTexture);
		backgroundTexture = NULL;
	}
	if (backgroundImage) {
		SDL_FreeSurface(backgroundImage);
		backgroundImage= NULL;
	}

}

void broov_gui_update_positions()
{
	loading_rect.x = (int) ((double)280.0 * fs_screen_width/640.0);
	loading_rect.y = (int) ((double)200.0 * fs_screen_height/480.0);

	loading_rect.w = (int) (120.0 * fs_screen_width/640.0);
	loading_rect.h = (int) (120.0 * fs_screen_height/480.0);

	//loading_rect.w = (int) (80.0 * fs_screen_width/640.0);
	//loading_rect.h = (int) (80.0 * fs_screen_height/480.0);

        if (loading_rect.w > 80) loading_rect.w = 80;
        if (loading_rect.h > 80) loading_rect.h = 80;


	background_rect.x = 0;
	background_rect.y = 0;
	background_rect.w = (int) ((double)640.0 * fs_screen_width/640.0);
	background_rect.h = (int) ((double)480.0 * fs_screen_height/480.0);

	sy = (int) ((double)400.0 * fs_screen_height/480.0);
	sw = (int) ((double)640.0 * fs_screen_width/640.0);
	sh = (int) ((double)60.0 * fs_screen_height/480.0);
	sxleft= (int) ((double)40.0 * fs_screen_width/640.0);
	sstep = (int) ((double)20.0 * fs_screen_height/480.0);

}
