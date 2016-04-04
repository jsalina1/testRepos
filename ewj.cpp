#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_net.h"

#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

SDL_Renderer *gRenderer = NULL;
SDL_Window *gWindow = NULL;

#include "environmentVars.cpp"

#include "objectClasses/timerObject.h"
#include "objectClasses/textureObject.h"

#include "objectClasses/tilesObject.h"
#include "objectClasses/gameObject.h"

#include "objectClasses/cloudsObject.h"
#include "objectClasses/lightMapObject.h"
#include "objectClasses/fontObject.h"
#include "objectClasses/introObject.h"

#include "objectClasses/gameStruct.cpp"
gameStruct game;

int TARGET_FRAME_RATE = 60;

#include "functions/loadObjects.h"
#include "functions/collisionDetect.h"
#include "functions/enemyAI.h"
#include "functions/misc.h"
#include "functions/eventHandler.h"
#include "functions/updateObjects.h"
#include "functions/messaging.h"
#include "functions/loadLevels.h"
#include "functions/gameStatusSupport.h"
#include "functions/gameStatuses.h"
#include "functions/renderObjects.h"

int main(int argc, char *args[])
{
    SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	initSDL();

	/// -- CREATE & LOAD GAME OBJECTS --------------------------------------------------------------------------------------------------------------------------------
    tilesObject *tiles = new tilesObject;

    gameObject *players = new gameObject;
    gameObject *enemies = new gameObject;
    gameObject *effects = new gameObject;
    gameObject *items = new gameObject;
    gameObject *projectiles = new gameObject;
    gameObject *hud = new gameObject;

    cloudClass *clouds = new cloudClass;
    fontObject *font = new fontObject;
    lightMapObject *lightMap = new lightMapObject;

    introObject *intro = new introObject;

    loadAllObjects(tiles, players, enemies, effects, items, projectiles, clouds, lightMap, font, hud);
	/// -- CREATE & LOAD GAME OBJECTS --------------------------------------------------------------------------------------------------------------------------------


    SDL_Event e;
    SDL_Texture *message = IMG_LoadTexture(gRenderer, "img\\messageBackground.png");


    game.frameHistory.resize(300);
    game.fpsTimer.start();

	while (game.GAME_LOOP)
	{
        game.clock.restart();
        game.capTimer.start();

        levelLoader(e, players, enemies, items, effects, projectiles, tiles, clouds, lightMap, font, hud, intro, message);
        gameStatus(e, players, enemies, items, effects, projectiles, tiles, clouds, lightMap, font, hud, intro, message);
        performanceMonitor(font);

        SDL_RenderPresent(gRenderer);

        backgroundMusic();
        frameGovenor();
    }

    game.bgMusic.stop();
    deleteObjects(players, projectiles, tiles, items, enemies, hud, intro, font, lightMap, clouds);
	close();

	return 0;
}


