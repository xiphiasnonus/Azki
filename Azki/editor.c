//
//  editor.c
//  Azki
//
//  Created by Thomas Foster on 4/2/20.
//  Copyright © 2020 Thomas Foster. All rights reserved.
//

#include <SDL2/SDL.h>
#include <string.h>

#include "editor.h"
#include "azki.h"
#include "video.h"
#include "obj.h"

enum
{
    VIEW_EDIT,      // map visiable and editing
    VIEW_CHARS      // table of ascii chars displayed
} view;

// object selection grid
typedef struct
{
    bool shown;
    int rows;
    int cols;
    SDL_Rect rect;
} grid_t;

static grid_t grid;
static objtype_t cursor = TYPE_PLAYER;
static char lowermsg[100];
static layer_t layer;

layer_t showlayer;

static char *layer_msg[] = {
    "(Editing Foreground)",
    "(Editing Background)"
};
static char *show_msg[] = {
    "Showing: FG   ",
    "Showing:    BG",
    "Showing: FG BG"
};




//
//  MakeSelectionGrid
//  Initialize the selection grid
//
void MakeSelectionGrid ()
{
    grid.cols = game_res.w / TILE_SIZE;
    grid.rows = NUMLAYERTYPES / grid.cols + 1;
    grid.rect.x = 0;
    grid.rect.y = game_res.h - grid.rows * TILE_SIZE;
    grid.rect.w = game_res.w;
    grid.rect.h = grid.rows * TILE_SIZE;
}



objtype_t TypeAtGridTile (SDL_Point * mousept)
{
    int type;
    int x, y;
    
    x = mousept->x / TILE_SIZE;
    y = (mousept->y / TILE_SIZE) - (game_res.h / TILE_SIZE - grid.rows);

    type = x + y * grid.cols;
    
    if (type >= NUMLAYERTYPES)   // if clicked on an empty tile,
        return cursor;      // don't change the current selection
    return type;
}


SDL_Point GridTileForType (objtype_t type)
{
    SDL_Point tile = { type % grid.cols, type / grid.cols};
    return tile;
}




//
//  DrawSelectionGrid
//  The grid is shown, draw it and the HUD info.
//
void DrawSelectionGrid (SDL_Point * mousept)
{
    int type;
    int x, y;
    SDL_Point mousetile;
    SDL_Rect selbox;
    extern objdef_t objdefs[];
    
    // background
    SDL_SetRenderDrawColor(renderer, 14, 14, 14, 255);
    SDL_RenderFillRect(renderer, &grid.rect);
    
    // draw all editor object types in the selection grid
    type = 0;
    x = 0;
    y = grid.rect.y;
    for (type=0 ; type<NUMLAYERTYPES ; type++, x+=TILE_SIZE )
    {
        //type = objtypes[i];
        
        if ( x >= game_res.w )
        {
            x = 0;
            y += TILE_SIZE;
        }
        if ( type == TYPE_NONE )
            DrawGlyph(&(glyph_t){'E',RED,TRANSP}, x, y, TRANSP);
        else
            DrawGlyph(&objdefs[type].glyph, x, y, TRANSP);
        // TODO: handle when a glyph color is the same as the bkg
    }
    
    // draw selection box
    if ( SDL_PointInRect(mousept, &grid.rect) )
    {
        mousetile.x = mousept->x / TILE_SIZE;
        mousetile.y = mousept->y / TILE_SIZE;
        selbox = (SDL_Rect){
            mousetile.x * TILE_SIZE,
            mousetile.y * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE
        };
        
        // HUD
        objtype_t hover = TypeAtGridTile(mousept);
        TextColor(BRIGHTGREEN);
        PrintString(objdefs[hover].name, TopHUD.x, TopHUD.y);
    }
    else // show current selection
    {
#if 0
        objtype_t *ot;
     
        // can't use cursor type - look it up:
        ot = objtypes;
        while ( *ot != cursor)
            ot++;
#endif
        
        SDL_Point selected_tile = GridTileForType(cursor);
        selbox = (SDL_Rect){
            selected_tile.x * TILE_SIZE,
            (selected_tile.y * TILE_SIZE) + grid.rect.y,
            TILE_SIZE,
            TILE_SIZE,
        };

        // HUD
        TextColor(BRIGHTGREEN);
        PrintString(objdefs[cursor].name, TopHUD.x, TopHUD.y);
    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &selbox);
}



void DrawCursor (SDL_Point *mousetile)
{

    {
        int sh;
        
        // display a helpful box so we know we're editing the bg
        SDL_RenderSetViewport(renderer, &maprect);
        if (layer == LAYER_BG)
        {
            SDL_Rect helpful = {
                mousetile->x * TILE_SIZE - 2,
                mousetile->y * TILE_SIZE - 2,
                TILE_SIZE + 4,
                TILE_SIZE + 4
            };
            SetPaletteColor(BROWN);
            SDL_RenderDrawRect(renderer, &helpful);
        }
        
        // draw cursor
        if (cursor == TYPE_NONE)
        {
            SDL_Rect erasebox = {
                mousetile->x * TILE_SIZE,
                mousetile->y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &erasebox);
        }
        else
        {
            sh = (SDL_GetTicks() % 600) < 300 ? RED : BLACK; // flip shadow
            DrawGlyph(&objdefs[cursor].glyph,
                      mousetile->x*TILE_SIZE,
                      mousetile->y*TILE_SIZE,
                      sh);
        }
        SDL_RenderSetViewport(renderer, NULL);
    }
}


void DrawEditorHUD (SDL_Point *mousept, SDL_Point *mousetile)
{
    int msg_x;
    char mouseinfo[100];
    
    PrintMapName();
    
    // layer
    msg_x = TopHUD.x + maprect.w - (int)strlen(layer_msg[layer]) * GLYPH_SIZE;
    TextColor(layer == LAYER_FG ? YELLOW : BROWN);
    PrintString(layer_msg[layer], msg_x, TopHUD.y);
                
    // lower HUD
    TextColor(MAGENTA);
    PrintString(lowermsg, BottomHUD.x, BottomHUD.y);
    
    // print mouse map coordinates
    if (SDL_PointInRect(mousept, &maprect))
        sprintf(mouseinfo, "%s (%2d, %2d)", show_msg[showlayer], mousetile->x, mousetile->y);
    else
        sprintf(mouseinfo, "%s (--, --)", show_msg[showlayer]);
    LOG(mouseinfo, BRIGHTWHITE);
}


void DrawChars (SDL_Point *mousept)
{
    int x, y, c;
    SDL_Rect r;
    char buf[4];
    
    r.x = (mousept->x / TILE_SIZE) * TILE_SIZE;
    r.y = (mousept->y / TILE_SIZE) * TILE_SIZE;
    r.w = TILE_SIZE;
    r.h = TILE_SIZE;
    c = 0;
    for ( y=0 ; y<16 ; y++ )
    {
        for ( x=0 ; x<16 ; x++)
        {
            TextColor(BRIGHTWHITE);
            PrintChar(c++, x * TILE_SIZE, y * TILE_SIZE);
            if (r.x < 128 && r.y < 128)
            {
                SetPaletteColor(RED);
                SDL_RenderDrawRect(renderer, &r);
            }
        }
    }
    
    TextColor(RED);
    snprintf(buf, 4, "%d", r.y/TILE_SIZE * 16 + r.x/TILE_SIZE);
    PrintString(buf, 17*TILE_SIZE, 0);
}








void EditorKeyDown (SDL_KeyCode key)
{
    memset(lowermsg, 0, sizeof(lowermsg)); // clear the LOG message
    
    switch (key)
    {
        case SDLK_ESCAPE:
            Quit();
            break;
            
        // window scale
        case SDLK_MINUS:
            SetScale(windowed_scale - 1);
            break;
        case SDLK_EQUALS:
            SetScale(windowed_scale + 1);
            break;
            
        // fullscreen
        case SDLK_f:
            if (CTRL) ToggleFullscreen();
            break;
        case SDLK_BACKSLASH:
            ToggleFullscreen();
            break;
            
        // level select
        case SDLK_LEFTBRACKET:
            NextLevel(-1);
            break;
        case SDLK_RIGHTBRACKET:
            NextLevel(+1);
            break;
            
        // switch layer
        case SDLK_SPACE:
            layer ^= 1;
            break;
            
        // switch to erase
        case SDLK_x:
            cursor = TYPE_NONE;
            break;
            
        // save map
        case SDLK_s:
            if (CTRL)
            {
                if ( SaveMap(&map) )
                    strcpy(lowermsg, "Map saved!");
                else
                    strcpy(lowermsg, "Error saving map!");
            }
            break;
            
        // switch to play
        case SDLK_BACKQUOTE:
            SaveMap(&map);
            state = GS_PLAY;
            break;
            
        default:
            break;
    }
}




void EditorMouseDown (SDL_Point * mousept, SDL_Point * mousetile)
{
    obj_t *obj;
    
    // place an object on map if editing
    if (!grid.shown && SDL_PointInRect(mousept, &maprect))
    {
        if (layer == LAYER_FG)
        {
            obj = &map.foreground[mousetile->y][mousetile->x];
            *obj = NewObjectFromDef(cursor, mousetile->x, mousetile->y);
        }
        else if (layer == LAYER_BG)
        {
            obj = &map.background[mousetile->y][mousetile->x];
            *obj = NewObjectFromDef(cursor, mousetile->x, mousetile->y);
        }
        mapdirty = true;
    }
    
    // select an object if grid is open
    if (grid.shown && SDL_PointInRect(mousept, &grid.rect))
    {
        cursor = TypeAtGridTile(mousept);
    }
}




void EditorLoop (void)
{
    SDL_Event   event;
    uint32_t    mousestate;
    SDL_Point   mousept;
    SDL_Point   mousetile;
    
    memset(lowermsg, 0, sizeof(lowermsg));
    MakeSelectionGrid();
    layer = LAYER_FG;
    LoadMap(map.num, &map); // entities were removed in play, reload
        
    while (state == GS_EDITOR)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                Quit();
            }
            else if (event.type == SDL_KEYDOWN) {
                EditorKeyDown(event.key.keysym.sym);
            }
        }
        
        grid.shown = keys[SDL_SCANCODE_TAB];
        view = keys[SDL_SCANCODE_G] ? VIEW_CHARS : VIEW_EDIT;
        
        if (keys[SDL_SCANCODE_F])
            showlayer = LAYER_FG;
        else if (keys[SDL_SCANCODE_B])
            showlayer = LAYER_BG;
        else
            showlayer = LAYER_BOTH;
        
        mousestate = SDL_GetMouseState(&mousept.x, &mousept.y);
        mousept.x /= windowed_scale; // TODO: fix for just current scale
        mousept.y /= windowed_scale;
        mousetile.x = (mousept.x - maprect.x) / TILE_SIZE;
        mousetile.y = (mousept.y - maprect.y) / TILE_SIZE;

        if (mousestate & SDL_BUTTON_LMASK && view == VIEW_EDIT)
            EditorMouseDown(&mousept, &mousetile);

        Clear(0, 0, 0);
        
        if (view == VIEW_EDIT)
        {
            DrawMap(&map);
            DrawEditorHUD(&mousept, &mousetile);
            if (SDL_PointInRect(&mousept, &maprect))
                DrawCursor(&mousetile);
            if (grid.shown)
                DrawSelectionGrid(&mousept);
        }
        else if (view == VIEW_CHARS)
        {
            DrawChars(&mousept);
        }

        Refresh();
        SDL_Delay(10);
    }
}
