#include <iostream>
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

  SDL_Surface *icon = IMG_Load("assets/skull.png");
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

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
  
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

  // HUD setup
  SDL_Color color = { 255, 255, 255 };
  string backgroundtext = "Health";
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
  SDL_Surface * image = IMG_Load("assets/black.png");
  SDL_BlitSurface(surface, NULL, image, &dstrect);

  SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, image);

  // 640 x 480 (width and height) 

  // engine 
  Engine* newengine = new Engine(renderer, 640, 480, 8);

  // player entitiy
  Entity player("player", 5, 5, 0.0, 0.0, 0.0, 0.0, {255,0,0,255});

  newengine -> entities.push_back(&player);

  newengine -> loadMap("maps/test.map");

  // splash screen loops

  SDL_Surface * splashimage = IMG_Load("assets/splash.png");
  SDL_Texture * splashtexture = SDL_CreateTextureFromSurface(renderer, splashimage);

  
  SDL_SetTextureAlphaMod(splashtexture, 100);
  Uint32 initTime = SDL_GetTicks();

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
    // TODO: fade in and fade out effect
    SDL_RenderCopy(renderer, splashtexture, NULL, NULL);
    SDL_SetRenderDrawColor(renderer, 225, 0,0, 225);
    SDL_RenderPresent(renderer);

  }

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

              case SDLK_w:  player.sety(player.gety() - 1); break;
              case SDLK_a:  player.setx(player.getx() - 1); break;
              case SDLK_s:  player.sety(player.gety() + 1); break;
              case SDLK_d:  player.setx(player.getx() + 1); break;
              
          }
      }
      if (event.type == SDL_QUIT)
      {
        // Break out of the loop on quit
        break;
      }
    }

    SDL_RenderCopy(renderer, texture, NULL, NULL);

    //newengine -> randomOpacities();
    newengine -> runPhysics(0.001);
    newengine -> drawFrame();

    SDL_SetRenderDrawColor(renderer, 225, 0,0, 225);

    // Show the renderer contents
    SDL_RenderPresent(renderer);
  }

  // Tidy up

  SDL_CloseAudioDevice(deviceId);
  SDL_FreeWAV(wavBuffer);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
