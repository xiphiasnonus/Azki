//
//  azki.h
//  Azki
//
//  Created by Thomas Foster on 3/30/20.
//  Copyright © 2020 Thomas Foster. All rights reserved.
//

#ifndef azki_h
#define azki_h

#define CTRL (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
#define sign(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)

typedef int tile;
typedef int pixel;

enum
{
    GS_PLAY,
    GS_EDITOR
};

extern int state;
extern const uint8_t * keys;

extern int tics;

void Error (const char * message);
void Quit (void);
void PlayLoop (void);
void HUDMessage(const char * msg);

#endif /* azki_h */
