#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "engine.h"

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
  int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
  SDL_PauseAudioDevice(deviceId, 0);

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


  // player (soon to be in engine) 
  SDL_Rect player_rect = {320*2 -8 , 240*2-8, 8, 8};

  // engine 
  Engine* newengine = new Engine(renderer, 640, 480, 8);
  
  newengine -> loadMap("maps/test.map");

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
    newengine -> drawFrame();

    SDL_SetRenderDrawColor(renderer, 225, 0,0, 225);
    SDL_RenderFillRect(renderer, &player_rect);

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