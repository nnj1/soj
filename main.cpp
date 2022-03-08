#include <iostream>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "engine.h"
#include "entity.h"

using namespace std;

int main( int argc, char *argv[] ) {

  SDL_Window *window = SDL_CreateWindow("DANK",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    640,
    480,
    SDL_WINDOW_OPENGL);
  if (window == nullptr)
  {
    SDL_Log("Could not create a window: %s", SDL_GetError());
    return -1;
  }

  SDL_Surface *icon = IMG_Load("assets/icon.png");
  SDL_SetWindowIcon(window, icon);

  if ( TTF_Init() < 0 ) {
    cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
  }

  TTF_Font* font;

  font = TTF_OpenFont("assets/font.ttf", 24);
  if ( !font ) {
    cout << "Failed to load font: " << TTF_GetError() << endl;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr)
  {
    SDL_Log("Could not create a renderer: %s", SDL_GetError());
    return -1;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  
  SDL_Init(SDL_INIT_AUDIO);
  
  // load WAV file
  SDL_AudioSpec wavSpec;
  Uint32 wavLength;
  Uint8 *wavBuffer;

  SDL_LoadWAV("assets/music.wav", &wavSpec, &wavBuffer, &wavLength);

  // open audio device
  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

  // play audio
  //int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
  //SDL_PauseAudioDevice(deviceId, 0);

  // text HUD 
  SDL_Color color = { 255, 255, 255 };
  string backgroundtext = "FPS";
  SDL_Surface * surface = TTF_RenderText_Blended_Wrapped(font, backgroundtext.c_str(), color, 200);
  int texW = 0;
  int texH = 0;
  SDL_Texture * texttexture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_QueryTexture(texttexture, NULL, NULL, &texW, &texH);

  SDL_Rect dstrect;
  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = texW;
  dstrect.h = texH;


  // image background HUD
  SDL_Surface * image = IMG_Load("assets/hud.png");
  SDL_BlitSurface(surface, NULL, image, &dstrect);

  SDL_Texture * hudtexture = SDL_CreateTextureFromSurface(renderer, image);

  // 640 x 480 (width and height) 

  // create custom mouse cursor
  SDL_Surface* cursorimage = IMG_Load("assets/cursor.png");
  SDL_Cursor* cursor = SDL_CreateColorCursor(cursorimage, 1, 1);
  SDL_SetCursor(cursor);

  // engine 
  Engine* newengine = new Engine(renderer, 640, 480, 8);

  // player entitiy
  Entity player("player", 50.0, 50.0, 0.0, 0.0, 0.0, 0.0, 3.0, 3.0, {255,0,0,255}, 10.0);

  newengine -> entities.push_back(&player);

  newengine -> loadMap("maps/test.map");

  // splash screen loops

  SDL_Surface * splashimage = IMG_Load("assets/splash.png");
  SDL_Texture * splashtexture = SDL_CreateTextureFromSurface(renderer, splashimage);

  Uint32 initTime = SDL_GetTicks();

  Uint32 alpha = 0;

  while (SDL_GetTicks() - initTime < 5000)
  { 
    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_KEYDOWN){
        break;
      }
      if (event.type == SDL_QUIT)
      {
        // Break out of the loop on quit
        break;
      }
    }

    SDL_SetTextureAlphaMod(splashtexture, alpha);
    // fade in effect
    if (alpha <= 255){
      SDL_SetTextureAlphaMod(splashtexture, alpha);
      alpha += 1;
      //printf("%d\n", alpha);
    }

    // CLEAR THE SCREEN WITH A WHITE COLOR:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, splashtexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    //SDL_Delay(10);
    
  }

  //SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  while (true)
  {

    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_KEYDOWN){
        switch (event.key.keysym.sym)
        {
          case SDLK_LEFT:  newengine -> offset_viewport_rect(-1,0); break;
          case SDLK_RIGHT: newengine -> offset_viewport_rect(+1,0); break;
          case SDLK_UP:    newengine -> offset_viewport_rect(0,-1); break;
          case SDLK_DOWN:  newengine -> offset_viewport_rect(0,+1); break;

          case SDLK_w:  player.shove(0.0, 1.0); break;
          case SDLK_a:  player.shove(-1.0, 0.0); break;
          case SDLK_s:  player.shove(0.0, -1.0); break;
          case SDLK_d:  player.shove(1.0, 0.0); break;

        }
      }

      // kill force on player once key is lifted
      if (event.type == SDL_KEYUP){
        switch (event.key.keysym.sym)
        {
          case SDLK_w:  player.setay(0.0); player.setvy(0.0); break;
          case SDLK_a:  player.setax(0.0); player.setvx(0.0); break;
          case SDLK_s:  player.setay(0.0); player.setvy(0.0); break;
          case SDLK_d:  player.setax(0.0); player.setvx(0.0); break;
        }
      }

      if (event.type == SDL_MOUSEBUTTONDOWN){
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        switch (event.button.button)
        {
          case SDL_BUTTON_LEFT:
          printf("Left mouse button pressed at %d, %d.\n", mouseX, mouseY);
          break;
          case SDL_BUTTON_RIGHT:
          printf("Right mouse button pressed.\n");;
          break;
          default:
          printf("Some other mouse button pressed.\n");
          break;
        }
      }

      if (event.type == SDL_MOUSEMOTION)
      {

      }
      
      if (event.type == SDL_QUIT)
      {
        // Break out of the loop on quit
        break;
      }
    }

    // CLEAR THE SCREEN
    SDL_RenderClear(renderer);

    //newengine -> randomOpacities();
    newengine -> runPhysics(1);
    newengine -> drawFrame();

    // make default background white
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 225);

    // draw base hud
    SDL_RenderCopy(renderer, hudtexture, NULL, NULL);

    // Show the renderer contents
    SDL_RenderPresent(renderer);
  }

  // Tidy up

  SDL_CloseAudioDevice(deviceId);
  SDL_FreeWAV(wavBuffer);
  SDL_DestroyTexture(hudtexture);
  SDL_FreeSurface(surface);
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
