#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#include <sdl_image.h>
#endif

#include "broov_config.h"

bool Transparent(SDL_Surface* TheSurface, int r, int g, int b)
{
    if (TheSurface == NULL)
        return false;

    else SDL_SetColorKey(TheSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(TheSurface->format, r, g, b));

    return true;
}

int test_main( int argc, char** argv )
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    // load an image

    SDL_Surface *bmp = NULL;
    SDL_Surface *bmp2 = NULL;
    SDL_Surface *temp = NULL;
    SDL_Surface *temp2 = NULL;
    SDL_RWops     *rw;             // Play image file control
    SDL_RWops     *rw2;             // Play image file control

    rw = SDL_RWFromMem(play_png, play_png_len);
    rw2 = SDL_RWFromMem(pause_png, pause_png_len);
    if((temp = IMG_Load_RW(rw, 1)) == NULL)
        return 1;
    else
    {
        bmp = SDL_DisplayFormatAlpha(temp);
        SDL_FreeSurface(temp);
    }

    if((temp2 = IMG_Load_RW(rw2, 1)) == NULL)
        return 1;
    else
    {
        bmp2 = SDL_DisplayFormat(temp2);
        SDL_FreeSurface(temp2);
        Transparent(bmp2, 0,0,0);
    }

    // centre the bitmap on screen
    SDL_Rect dstrect;
    dstrect.x = (screen->w - bmp->w) / 2;
    dstrect.y = (screen->h - bmp->h) / 2;

    SDL_Rect dstrect2;
    dstrect2.x = dstrect.x + (bmp->w + 20);
    dstrect2.y = dstrect.y;

    // program main loop
    int blender = 255;
    bool increase = false;
    bool done = false;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // exit if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                break;
            }
            } // end switch
        } // end of message processing

        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 80, 255));

        // draw bitmap
        if(blender <= 0)
            increase = true;
        if(blender >= 255)
            increase = false;

        if(increase == true)
            blender ++;
        else blender--;

        if(SDL_SetAlpha(bmp2, SDL_SRCALPHA, blender) == -1) //255-opaque,0-invisible
            printf("there was an error \n");
        else SDL_BlitSurface(bmp2, 0, screen, &dstrect2);

        if(SDL_SetAlpha(bmp, SDL_SRCALPHA, blender) == -1)
            printf("there was an error \n");
        else SDL_BlitSurface(bmp, 0, screen, &dstrect);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop

    // free loaded bitmap
    SDL_FreeSurface(bmp);
    SDL_FreeSurface(bmp2);

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
