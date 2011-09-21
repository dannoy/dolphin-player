#include "broov_config.h"
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include "SDL.h"
#include "SDL_ttf.h"


#define RENDER_MODE             0 //0=solid 1=shaded 2=blended
#define TTF_HINTING_NORMAL      0
#define TTF_STYLE_STRIKETHROUGH 0x08
#define SPACING                 0

static int       start_glyph=0;
static int       style=TTF_STYLE_NORMAL; //int       style=TTF_STYLE_BOLD;
static int       kerning=1;
static int       hinting=TTF_HINTING_NORMAL;
static int       outline=0;

static TTF_Font *font=0;
static int       font_size=14;

int subtitle_clean_msg(SDL_Surface *screen, int x, int y, const char *msg)
{
        SDL_Color black = { 0x00, 0x00, 0x00, 0x00 };

	//SDL_Color fg={255, 165,0,0}; // orange color
	SDL_Color fg={0, 0, 0,0};
	SDL_Color bg={0, 0, 0, 0};

        SDL_Surface *black_text_surface = TTF_RenderText_Shaded(font, msg, fg, bg);

        if (black_text_surface) 
        {

           SDL_TextureID black_text = SDL_CreateTextureFromSurface(0, black_text_surface);
           if (black_text) {
               SDL_Rect black_rect = {x, y, black_text_surface->w, black_text_surface->h };
               SDL_RenderCopy(black_text, NULL, &black_rect);

               SDL_DestroyTexture(black_text);


               { 
                 char log_msg[256];
                 sprintf(log_msg, "CleanedMsg:%s", msg);
                 __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);
               }
           }

           SDL_FreeSurface(black_text_surface);
        }

}

int overlay_transparent(SDL_Surface* TheSurface, int r, int g, int b)
{
    if (TheSurface == NULL)
        return 0;

    else SDL_SetColorKey(TheSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(TheSurface->format, r, g, b));

    return 1;
}

int overlay_subtitle_draw_msg_type1(SDL_Surface *screen, int x, int y, const char *msg)
{
        int blender = 45;
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

        SDL_Surface *mBackground = SDL_CreateRGBSurface
               (SDL_SWSURFACE, sw, sh, 32, screen->format->Rmask,
                screen->format->Gmask,
                screen->format->Bmask,
                screen->format->Amask);
        Uint32 color = SDL_MapRGB(mBackground->format, 255, 255, 255);
        SDL_FillRect(mBackground, 0, color);
        
	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        //Central alignment for subtitle
        x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };

        SDL_Surface *alpha_surf = SDL_DisplayFormat(mBackground);
        overlay_transparent(alpha_surf, 0,0,0);
        SDL_SetAlpha(alpha_surf, SDL_SRCALPHA, blender); //255-opaque,0-invisible
        SDL_TextureID text = SDL_CreateTextureFromSurface(0, alpha_surf);

        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_FreeSurface(alpha_surf);
        SDL_DestroyTexture(text);
}


int overlay_subtitle_draw_msg_type2(SDL_Surface *screen, int x, int y, const char *msg)
{
        int blender = 254;
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        //Central alignment for subtitle
        x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };

        SDL_Surface *alpha_surf = SDL_DisplayFormat(surf);
        overlay_transparent(alpha_surf, 0,0,0);
        //SDL_Surface *alpha_surf = SDL_DisplayFormatAlpha(surf);
        SDL_SetAlpha(alpha_surf, SDL_SRCALPHA, blender); //255-opaque,0-invisible
        SDL_TextureID text = SDL_CreateTextureFromSurface(0, alpha_surf);

        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_FreeSurface(alpha_surf);
        SDL_DestroyTexture(text);
}

SDL_Surface* get_rendered_text_unicode(const char *msg)
{
    SDL_Color bg={0,0,0,0};
    SDL_Color fg={255,255,255,0};
    //SDL_Color fg={255,228,181,0}; //moccasin

    //return TTF_RenderText_Shaded(font, msg, fg, bg);
    //return TTF_RenderText_Solid(font, msg, fg);
    return TTF_RenderUNICODE_Solid(font, (Uint16*)msg, fg);
}

SDL_Surface* get_rendered_text(const char *msg)
{
    SDL_Color bg={0,0,0,0};
    SDL_Color fg={255,255,255,0};
    //SDL_Color fg={255,228,181,0}; //moccasin

    //return TTF_RenderText_Shaded(font, msg, fg, bg);
    return TTF_RenderText_Solid(font, msg, fg);
}


int overlay_subtitle_draw_msg(SDL_Surface *screen, int x, int y, const char *msg)
{
        int blender = 255;
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

        SDL_Surface *mBackground = SDL_CreateRGBSurface
               (SDL_SWSURFACE, sw, sh, 32, screen->format->Rmask,
                screen->format->Gmask,
                screen->format->Bmask,
                screen->format->Amask);
        Uint32 color = SDL_MapRGB(mBackground->format, 5, 5, 5);
        SDL_FillRect(mBackground, 0, color);
        
	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        //Central alignment for subtitle
        x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };

        //SDL_Rect blit_src_rect = {};
        SDL_Rect blit_dst_rect = { x, 0, sw, sh};
        SDL_BlitSurface(surf, NULL, mBackground, &blit_dst_rect);

        SDL_Surface *alpha_surf = SDL_DisplayFormat(mBackground);
        overlay_transparent(alpha_surf, 0,0,0);
        SDL_SetAlpha(alpha_surf, SDL_SRCALPHA, blender); //255-opaque,0-invisible
        SDL_TextureID text = SDL_CreateTextureFromSurface(0, alpha_surf);

        //SDL_RenderCopy(text, NULL, &rect);
        y = fs_screen_height - sh;
        SDL_Rect bg_rect = {0, y, sw, sh};
        SDL_RenderCopy(text, NULL, &bg_rect);

        SDL_FreeSurface(surf);
        SDL_FreeSurface(mBackground);
        SDL_FreeSurface(alpha_surf);
        SDL_DestroyTexture(text);
}



int subtitle_draw_msg(SDL_Surface *screen, int x, int y, const char *msg)
{
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        SDL_TextureID text = SDL_CreateTextureFromSurface(0, surf);

        //Central alignment for subtitle
        x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };
        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(text);
}
int fps_draw_msg_type1(SDL_Surface *screen, int x, int y, const char *msg)
{
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        SDL_TextureID text = SDL_CreateTextureFromSurface(0, surf);

        //Central alignment for subtitle
        //x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };
        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(text);
}

int fps_draw_msg(SDL_Surface *screen, int x, int y, const char *msg)
{
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={255,255,255,255};

	SDL_Surface *surf= TTF_RenderText_Solid(font, msg, fg);

        SDL_TextureID text = SDL_CreateTextureFromSurface(0, surf);

        //Central alignment for subtitle
        //x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };
        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(text);
}

int draw_msg(SDL_Surface *screen, int x, int y, const char *msg)
{
	//SDL_Color bg={0x19,0x19,0x19,0x40};
	SDL_Color bg={0,0,0,0};
	SDL_Color fg={238,238,230,0x40};

        //defaultTextColor.setAllColors(238, 238, 230, 0x40);
        //colorDefaultWidgetBackground.setAllColors(0x19, 0x19, 0x19, 0x40);
	//SDL_Color bg={0,0,0,0};
	//SDL_Color fg={255,255,255,255};

	SDL_Surface *surf= TTF_RenderText_Shaded(font,msg,fg, bg);

        SDL_TextureID text = SDL_CreateTextureFromSurface(0, surf);

        //Central alignment for subtitle
        //x=(int) ((320.0*fs_screen_width/640.0) -(surf->w/2.0));

        SDL_Rect rect = {x, y, surf->w, surf->h };
        SDL_RenderCopy(text, NULL, &rect);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(text);
}



void free_font()
{
	if (font) 
        {
		TTF_CloseFont(font);
	        font=0;
        }
}

void cache_glyphs()
{
	SDL_Color fg={0,0,0,255};

#if RENDER_MODE==1
	SDL_Color bg={255,255,255,255};
#endif

	if (!font) return;
	
	if (style!=TTF_GetFontStyle(font)) TTF_SetFontStyle(font,style);

	if (kerning != !!TTF_GetFontKerning(font)) TTF_SetFontKerning(font,kerning);

	if (hinting != TTF_GetFontHinting(font)) TTF_SetFontHinting(font,hinting);
	if (outline != TTF_GetFontOutline(font)) TTF_SetFontOutline(font,outline);
}

int load_font(char *fname, int size)
{
	char *p;

	free_font();
	font = TTF_OpenFont(fname, size);

	if (!font)
	{
                //Try to open the in memory font
                SDL_RWops *s;
                s = SDL_RWFromMem(dejavu_sans_ttf, dejavu_sans_ttf_len);
                font = TTF_OpenFontRW(s, 0, size);
                if (font) 
                {
                   __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Open font from memory successful");
                }
                else 
                {
                   __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Open font from memory failed:%s", TTF_GetError());
		   return 3;
                }

	}

	/* print some metrics and attributes */
	//printf("size                    : %d\n",size);
	//printf("TTF_FontHeight          : %d\n",TTF_FontHeight(font));
	//printf("TTF_FontAscent          : %d\n",TTF_FontAscent(font));
	//printf("TTF_FontDescent         : %d\n",TTF_FontDescent(font));
	//printf("TTF_FontLineSkip        : %d\n",TTF_FontLineSkip(font));
	//printf("TTF_FontFaceIsFixedWidth: %d\n",TTF_FontFaceIsFixedWidth(font));
	
	/* cache new glyphs */
	cache_glyphs();
 
        return 0;
}

int broov_font_init(char *font_fname, int my_font_size)
{
	/* start SDL_ttf */
	if (TTF_Init()==-1)
	{
                __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "TTF Init Failed: %s", TTF_GetError());
		return 2;
	}

        if (my_font_size >=8 && my_font_size <= 24) {
            font_size = my_font_size+4;
        }

	return load_font(font_fname, font_size);
}

