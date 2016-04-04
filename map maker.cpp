#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "objectClasses/timerObject.h"
#include "objectClasses/textureObject.h"
#include "objectClasses/tilesObject.h"

#include "environmentVars.cpp"

SDL_Renderer *gRenderer=NULL;

int activeLayer=0;
int activeLayerType=0;
int activeLayerImgOffset=0;
int activeTile=0;
int levelNo=2;

bool init(SDL_Window *gWindow)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC)<0) { printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError()); }
	else
	{
        if(!SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" )) printf("Warning: VSync not enabled!");

        gWindow=SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);  // 1280 / 720
        gRenderer=SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
        IMG_Init(IMG_INIT_PNG);
        TTF_Init();
	}

	return true;
}

void close(SDL_Window *gWindow)
{
	SDL_DestroyRenderer(gRenderer); gRenderer=NULL;
	SDL_DestroyWindow(gWindow); gWindow=NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void outText(TTF_Font *gFont, int x, int y, std::stringstream &text, SDL_Color textColor)
{
    textureObject genericText;

    genericText.loadFromRenderedText(gFont, text.str().c_str(), textColor);
    genericText.render(x, y);
}

void changeFocusItemValue(int amount, int focus, tilesObject *tiles, int &scrollSpeed)
{
        if (focus==0) { activeLayer+=amount; activeTile=0; }
        else if (focus==1) activeTile+=amount;
        else if (focus==2) { tiles->setImgOffsetY(activeLayer, tiles->getImgOffsetY(activeLayer)+amount); }
        else if (focus==3) { tiles->setImgOffsetX(activeLayer, tiles->getImgOffsetX(activeLayer)+amount); }
        else if (focus==4) { tiles->setLayerType(activeLayer, amount); }
        else if (focus==5) scrollSpeed+=amount;
        else if (focus==6) tiles->setMapSizeX( tiles->getMapSizeX()+amount );
        else if (focus==7) tiles->setMapSizeY( tiles->getMapSizeY()+amount );
        else if (focus==8) tiles->setTileSizeX( tiles->getTileSizeX()+amount );
        else if (focus==9) tiles->setTileSizeY( tiles->getTileSizeY()+amount );
        else if (focus==10) tiles->setNumLayers( tiles->getNumLayers()+amount );
        else if (focus==11) levelNo+=amount;
}

int main(int argc, char *args[])
{
    SDL_Window *gWindow=NULL;
	SDL_Event e;
    TTF_Font *gFont=NULL;

    std::stringstream text;
    SDL_Color textColor={255,255,255};

    int i, row, col;

	init(gWindow);

    gFont = TTF_OpenFont( "fonts\\ben_gothic.ttf", 10);

    tilesObject *tiles = new tilesObject;

    tiles->setTileSizeX(64);
    tiles->setTileSizeY(64);
    tiles->setCameraXpos(0);
    tiles->setCameraYpos(0);
    tiles->setMapSizeX(41);
    tiles->setMapSizeY(21);
    tiles->setNumLayers(1);
    for (i=0; i<MAX_TILE_LAYERS; i++) tiles->setImgOffsetY(i,0);
    for (i=0; i<MAX_TILE_LAYERS; i++) tiles->setImgOffsetX(i,0);
    for (i=0; i<MAX_TILE_LAYERS; i++) tiles->setLayerType(0,0);

	tiles->loadTileObject(0, "img\\Tiles5.png", 255, 255, 255);
	tiles->setTileSizeAndClips(0, 6, 16);  // using the default tilesize here

	tiles->loadTileObject(1, "img\\bgItems128 copy.png", 255, 255, 255);
	tiles->setTileSizeAndClips(1, 2, 25, 128, 128);

	tiles->loadTileObject(2, "img\\bgItems256 copy.png", 255, 255, 255);
	tiles->setTileSizeAndClips(2, 1, 12, 256, 256);

	tiles->loadTileObject(3, "img\\ewjItemsForMap copy.png", 255, 255, 255);
	tiles->setTileSizeAndClips(3, 15, 1, 128, 128);

	tiles->loadTileObject(4, "img\\enemies_map.png", 250, 0, 255);
	tiles->setTileSizeAndClips(4, 13, 1, 128, 128);

	tiles->loadTileObject(5, "img\\number_layer.png", 250, 0, 255);
	tiles->setTileSizeAndClips(5, 1, 20, 64, 64);

	tiles->loadTileObject(6, "img\\number_layer.png", 250, 0, 255);
	tiles->setTileSizeAndClips(6, 1, 20, 64, 64);

	tiles->loadTileObject(7, "img\\bgItems128_spikes.png", 255, 255, 255);
	tiles->setTileSizeAndClips(7, 1, 5, 128, 128);

	tiles->loadTileObject(8, "img\\number_layer.png", 255, 255, 255);
	tiles->setTileSizeAndClips(8, 1, 20, 64, 64);

	tiles->movingTileFlag = false;

    int mouseClickX=0, mouseClickY=0;
    int clickTileX=0, clickTileY=0;

	bool GAME_LOOP=true;

    int focus=0;
    SDL_Rect focusRect, tileSelect;
    focusRect.w=150; focusRect.h=18;

    int scrollSpeed=100;
    float scale=1.0;
    int maxItems=0;

    Timer mouseHoldTimer;
    mouseHoldTimer.start();
    bool mouseLeftButton=false;

    char levelNoStr[3], filename[30];

    for (i=0; i<MAX_TILE_LAYERS; i++) tiles->setLayerType(i, 0);

    bool view[5]; for (i=0; i<5; i++) view[i]=true;

    if (levelNo == 1)
    {
        //tiles->setBackgroundTexture("img\\level_1 copy.png", 1971, 824);
        //tiles->setBackgroundTexture("img\\level_1_trees.png", 2662, 928, 255, 255, 250, 2);
    }

	while(GAME_LOOP)
	{
        while (SDL_PollEvent(&e))
        {
            if (e.type==SDL_QUIT) GAME_LOOP=false;
            else if (e.key.type==SDL_KEYDOWN)
            {
                //printf("%d\n", e.key.keysym.sym);
                if (e.key.keysym.sym==SDLK_ESCAPE) { GAME_LOOP=false; }

                if (e.key.keysym.sym==SDLK_F1) { if (view[0]) view[0]=false; else view[0]=true; }
                if (e.key.keysym.sym==SDLK_F2) { if (view[1]) view[1]=false; else view[1]=true; }
                if (e.key.keysym.sym==SDLK_F3) { if (view[2]) view[2]=false; else view[2]=true; }
                if (e.key.keysym.sym==SDLK_F4) { if (view[3]) view[3]=false; else view[3]=true; }
                if (e.key.keysym.sym==SDLK_RIGHT) tiles->moveCameraX(-scrollSpeed);
                if (e.key.keysym.sym==SDLK_LEFT) tiles->moveCameraX(+scrollSpeed);
                if (e.key.keysym.sym==SDLK_DOWN) tiles->moveCameraY(-scrollSpeed);
                if (e.key.keysym.sym==SDLK_UP) tiles->moveCameraY(scrollSpeed);
                //if (e.key.keysym.sym==1073741902) tiles->moveCameraY(-scrollSpeed-200);  //page down
                //if (e.key.keysym.sym==1073741899) tiles->moveCameraY(scrollSpeed+200);  // page up
                if (e.key.keysym.sym==1073741911) changeFocusItemValue(1, focus, tiles, scrollSpeed);  // plus key
                if (e.key.keysym.sym==1073741910) changeFocusItemValue(-1, focus, tiles, scrollSpeed); // minus key

                if (e.key.keysym.sym==1073742049) { focus--; if (focus<0) focus=11; }
                if (e.key.keysym.sym==SDLK_TAB) { focus++; if (focus>=12) focus=0; }

                //if (e.key.keysym.sym==SDLK_9) { focus--; if (focus<0) focus=11; }
                //if (e.key.keysym.sym==SDLK_0) { focus++; if (focus>=12) focus=0; }

                if (e.key.keysym.sym==SDLK_o)
                    {
                        if (levelNo>0)
                        {
                            printf("Opened.\n");
                            itoa(levelNo, levelNoStr, 10); strcpy(filename, "maps\\level_"); strcat(filename, levelNoStr); strcat(filename, ".txt");

                            tiles->loadMap(filename);
                            tiles->setCameraXpos(0);
                            tiles->setCameraYpos(0);
                        }
                        else printf("Not Opened Level=0.\n");
                    }
                if (e.key.keysym.sym==SDLK_s)
                    {
                        if (levelNo>0)
                        {
                            printf("Saved.\n");
                            itoa(levelNo, levelNoStr, 10); strcpy(filename, "maps\\level_"); strcat(filename, levelNoStr); strcat(filename, ".txt");

                            tiles->saveMap(filename);
                        }
                        else printf("Not Saved Level=0.\n");
                    }


            }

        }

        if (e.button.clicks==2 or e.button.button==SDL_BUTTON_RIGHT)
        {
            SDL_GetMouseState(&mouseClickX, &mouseClickY);

            clickTileX=(tiles->getCameraXpos()+mouseClickX)/tiles->getTileSizeX();
            clickTileY=(tiles->getCameraYpos()+mouseClickY)/tiles->getTileSizeY();

            tiles->setTileValue(activeLayer, clickTileY, clickTileX, MAP_EMPTY_SPACE);
        }

        if (mouseLeftButton)
        {
            SDL_GetMouseState(&mouseClickX, &mouseClickY);

            clickTileX=(tiles->getCameraXpos()+mouseClickX)/tiles->getTileSizeX();
            clickTileY=(tiles->getCameraYpos()+mouseClickY)/tiles->getTileSizeY();

            tiles->setTileValue(activeLayer, clickTileY, clickTileX, activeTile);
        }

        if (mouseHoldTimer.get_ticks() > 100)
        {
            if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                mouseLeftButton=true;
            }
            else mouseLeftButton=false;

            mouseHoldTimer.restart();
        }

        if (focus==0) { focusRect.x=25; focusRect.y=640; }
        else if (focus==1) { focusRect.x=175; focusRect.y=640; }
        else if (focus==2) { focusRect.x=325; focusRect.y=640; }
        else if (focus==3) { focusRect.x=475; focusRect.y=640; }
        else if (focus==4) { focusRect.x=635; focusRect.y=640; }
        else if (focus==5) { focusRect.x=785; focusRect.y=640; }
        else if (focus==6) { focusRect.x=25; focusRect.y=685; }
        else if (focus==7) { focusRect.x=175; focusRect.y=685; }
        else if (focus==8) { focusRect.x=325; focusRect.y=685; }
        else if (focus==9) { focusRect.x=475; focusRect.y=685; }
        else if (focus==10) { focusRect.x=635; focusRect.y=685; }
        else if (focus==11) { focusRect.x=785; focusRect.y=685; }

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
        SDL_RenderClear(gRenderer);

        tiles->drawBackground();
        tiles->draw(LAYER_6);
        tiles->draw(LAYER_5);
        tiles->draw(LAYER_7, -170, 0, 15, 24);
        tiles->draw(LAYER_0);
        tiles->draw(LAYER_2, -170, 0, 15, 24);
        tiles->draw(LAYER_1, -170, 0, 15, 24);
        tiles->draw(LAYER_4);
        tiles->draw(LAYER_3);
        tiles->draw(LAYER_8);

        if (view[0])
        {
            SDL_SetRenderDrawColor(gRenderer, 30, 30, 30, 0);
            for (int i=tiles->getXoffset(); i<SCREEN_WIDTH; i+=tiles->getTileSizeX()) SDL_RenderDrawLine(gRenderer, i, 0, i, SCREEN_HEIGHT);
            for (int i=tiles->getYoffset(); i<SCREEN_HEIGHT; i+=tiles->getTileSizeY()) SDL_RenderDrawLine(gRenderer, 0, i, SCREEN_WIDTH, i);
        }

        if (view[1])
        {
            SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0);
            SDL_RenderDrawRect(gRenderer, &focusRect);

            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);

            text.str(""); text << "Layer Attributes"; outText(gFont, 5, 625, text, textColor);
            text.str(""); text << "Active Layer=" << activeLayer; outText(gFont, 30, 645, text, textColor);
            text.str(""); text << "Active Tile=" << activeTile; outText(gFont, 180, 645, text, textColor);
            text.str(""); text << "Active Lyr ImgOffsetY=" << tiles->getImgOffsetY(activeLayer); outText(gFont, 330, 645, text, textColor);
            text.str(""); text << "Active Lyr ImgOffsetX=" << tiles->getImgOffsetX(activeLayer); outText(gFont, 480, 645, text, textColor);
            text.str(""); text << "Active Layer Type=" << tiles->getLayerType(activeLayer); outText(gFont, 640, 645, text, textColor);
            text.str(""); text << "Scroll Speed=" << scrollSpeed; outText(gFont, 790, 645, text, textColor);

            text.str(""); text << "Map Attributes"; outText(gFont, 5, 670, text, textColor);
            text.str(""); text << "Map Size X=" << tiles->getMapSizeX(); outText(gFont, 30, 690, text, textColor);
            text.str(""); text << "Map Size Y=" << tiles->getMapSizeY(); outText(gFont, 180, 690, text, textColor);
            text.str(""); text << "Tile Size X=" << tiles->getTileSizeX(); outText(gFont, 330, 690, text, textColor);
            text.str(""); text << "Tile Size Y=" << tiles->getTileSizeY(); outText(gFont, 480, 690, text, textColor);
            text.str(""); text << "Number of Layers=" << tiles->getNumLayers(); outText(gFont, 640, 690, text, textColor);
            text.str(""); text << "Level No=" << levelNo; outText(gFont, 790, 690, text, textColor);
        }

        if (view[2])
        {
            if (activeLayer==0) { scale=1; maxItems=100; }
            else if (activeLayer==1) { scale=.5; maxItems=50; }
            else if (activeLayer==2) { scale=.25; maxItems=12; }
            else if (activeLayer==3) { scale=.5; maxItems=14; }
            else if (activeLayer==4) { scale=.5; maxItems=15; }
            else if (activeLayer==5) { scale=1; maxItems=20; }
            else if (activeLayer==6) { scale=1; maxItems=20; }
            else if (activeLayer==7) { scale=.5; maxItems=8; }
            else if (activeLayer==8) { scale=1; maxItems=20; }

            tiles->drawTile(activeLayer, 700, 15, activeTile, 255, scale);

            if (activeTile>=1) tiles->drawTile(activeLayer, 620, 15, activeTile-1, 110, scale);
            if (activeTile>=2) tiles->drawTile(activeLayer, 540, 15, activeTile-2, 110, scale);
            if (activeTile>=3) { tiles->drawTile(activeLayer, 460, 15, activeTile-3, 110, scale); }

            if (activeTile<=maxItems) tiles->drawTile(activeLayer, 780, 15, activeTile+1, 110, scale);
            if (activeTile<=maxItems-1) tiles->drawTile(activeLayer, 860, 15, activeTile+2, 110, scale);
            if (activeTile<=maxItems-2) tiles->drawTile(activeLayer, 940, 15, activeTile+3, 110, scale);

            SDL_SetRenderDrawColor(gRenderer, 100, 0, 0, 0);
            SDL_Rect tileBorder;
            tileBorder.x=455; tileBorder.y=10; tileBorder.h=74; tileBorder.w=555;
            SDL_RenderDrawRect(gRenderer, &tileBorder);

            SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0);
            tileSelect.x=695; tileSelect.y=10; tileSelect.h=74; tileSelect.w=74;
            SDL_RenderDrawRect(gRenderer, &tileSelect);

            SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0);
            tileSelect.x=694; tileSelect.y=9; tileSelect.h=76; tileSelect.w=76;
            SDL_RenderDrawRect(gRenderer, &tileSelect);
        }

        if (view[3])
        {
            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
            text.str(""); text << "Camera X Pos: " << tiles->getCameraXpos(); outText(gFont, 10, 45, text, textColor);
            text.str(""); text << "Camera Y Pos: " << tiles->getCameraYpos(); outText(gFont, 150, 45, text, textColor);

            SDL_Rect miniMap, mmLocation;
            miniMap.x=10; miniMap.y=90; miniMap.w=(tiles->getMapSizeX()-1)*4; miniMap.h=(tiles->getMapSizeY()-1)*4;
            SDL_SetRenderDrawColor(gRenderer, 150, 150, 150, 0);
            SDL_RenderFillRect(gRenderer, &miniMap);

            mmLocation.x=10+tiles->getCameraXtile()*4; mmLocation.y=90+tiles->getCameraYtile()*4; mmLocation.h=12*4; mmLocation.w=20*4;
            SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 0);
            SDL_RenderFillRect(gRenderer, &mmLocation);

            for (col=0; col<tiles->getMapSizeX()-1; col++)
            {
                for (row=0; row<tiles->getMapSizeY()-1; row++)
                {
                    if (tiles->getTileValue(0, row, col, 0, 0) != 0)
                    {
                        SDL_Rect miniMapTile;
                        miniMapTile.x=10+(col*4); miniMapTile.y=90+(row*4); miniMapTile.w=4; miniMapTile.h=4;
                        SDL_SetRenderDrawColor(gRenderer, 0, 100, 0, 0);
                        SDL_RenderFillRect(gRenderer, &miniMapTile);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
        text.str(""); text << "F1-F4 toggles view, Tab/left Shift cycle attributes, +/- change attributes"; outText(gFont, 10, 10, text, textColor);

        SDL_GetMouseState(&mouseClickX, &mouseClickY);

        clickTileX=(tiles->getCameraXpos()+mouseClickX)/tiles->getTileSizeX();
        clickTileY=(tiles->getCameraYpos()+mouseClickY)/tiles->getTileSizeY();

        text.str(""); text << clickTileY << ", " << clickTileX; outText(gFont, 1220, 20, text, textColor);

        SDL_RenderPresent(gRenderer);
    }

    delete tiles; tiles=NULL;

	close(gWindow);
	return 0;
}
