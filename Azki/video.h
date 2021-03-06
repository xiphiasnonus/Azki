//
//  video.h
//  Azki
//
//  Created by Thomas Foster on 3/30/20.
//  Copyright © 2020 Thomas Foster. All rights reserved.
//

#ifndef video_h
#define video_h

#include <SDL2/SDL.h>
#include "azki.h"

enum
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    WHITE,
    GRAY,
    BRIGHTBLUE,
    BRIGHTGREEN,
    BRIGHTCYAN,
    BRIGHTRED,
    BRIGHTMAGENTA,
    YELLOW,
    BRIGHTWHITE, // end of CGA colors
    PITCHBLACK,
    TRANSP,
    NUMCOLORS
};

#define NUM_CGA_COLORS 16
#define BLINK   0x20

extern SDL_Window * window;
extern SDL_Renderer * renderer;
extern SDL_Texture * font_table;
extern SDL_Texture * shadow_table;

extern int windowed_scale;
extern SDL_Rect game_res;

void StartFrame (void);
int  LimitFrameRate (int fps);

void StartVideo (void);
void ShutdownVideo (void);
void ToggleFullscreen (void);
void UpdateDrawLocations (float scl);

void SetPaletteColor (int c);
void SetRGBColor (uint8_t r, uint8_t g, uint8_t b);
void SetScale (int scl);
void TextColor (int c);
void BackgroundColor (int c);
void PrintString (const char *s, pixel x, pixel y);
void PrintCenteredString (const char *s, pixel x, pixel y);
void PrintChar (char c, int winx, int winy);
void LOG (const char *message, int color);

void FillRect (int x, int y, int w, int h);
void Clear (uint8_t r, uint8_t g, uint8_t b);
void Refresh (void);

#endif /* video_h */
