// my_class.h
#ifndef ENGINE_H // include guard
#define ENGINE_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <fstream>

#include <cstdlib>   // rand and srand
#include <ctime>     // For the time function

#include "entity.h"

using namespace std;

static const char *MAINTHEME = "assets/main.mp3";

Uint32 changeOpacities(Uint32 interval, void *param);

class Engine
{
private:

    // dimensions of window
    int mwidth;
    int mheight;

    // dimensiosn of a single pixel
    int mscale;

    // contains x and y offsets
    SDL_Rect viewport_rect;
    
    // contains data loaded from .map file
    vector<vector<char> > mmap;

    // recurrent counter for environment animation updates
    SDL_TimerID my_timer_id;

    // mix music object pointer
    Mix_Music *music;

public:

    // constructor and deconstructor
    Engine(SDL_Renderer *renderer, int width, int height, int scale);
    ~Engine();

    // tick
    Uint32 firsttick;

    void SetEngine(SDL_Renderer *renderer, int width, int height, int scale);
    void randomColors();
    void solidColors();
    void drawFrame();
    void runPhysics(float deltat);
    void flushCells();
    void randomOpacities();
    vector<vector<char> > readMap(string filename);
    void loadMap(string filename);

    int getWidth() { return mwidth; }
    int getHeight() { return mheight; }
    int getScale() { return mscale; }
    SDL_Rect getViewport_rect() { return viewport_rect; }
    void setViewport_rect(int x, int y, int w, int h) { viewport_rect = {x, y, w, h}; }
    void centerViewport_rect();
    SDL_Renderer* mrenderer;
    vector<vector<SDL_Color> > cells;

    void offset_viewport_rect(int x, int y);

    vector<Entity*> entities;
};

// Engine constructor
Engine::Engine(SDL_Renderer *renderer, int width, int height, int scale)
{
    SetEngine(renderer, width, height, scale);
    // sets x,y offset for top left hand corner of viewing window, w and h are useless
    viewport_rect = {5, 5, 0, 0};
    firsttick = SDL_GetTicks();

    // get music set up

    int result = 0;
    int flags = MIX_INIT_MP3;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }

    if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }

    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    music = Mix_LoadMUS(MAINTHEME);
    Mix_PlayMusic(music, 1);
}

// Define the destructor.
Engine::~Engine() {
   // Deallocate the memory that was previously reserved
   SDL_RemoveTimer(my_timer_id);
   Mix_FreeMusic(music);
}

// Engine member function
void Engine::SetEngine(SDL_Renderer *renderer, int width, int height, int scale)
{
    mwidth = width;
    mheight = height;
    mrenderer = renderer;
    mscale = scale;
    vector<vector<SDL_Color> > cells;

    // Get the system time.
    unsigned seed = time(0);
   
    // Seed the random number generator.
    srand(seed);

    // set random colors initially
    randomColors();

}

void Engine::flushCells()
{
    cells.clear();
}

void Engine::randomOpacities()
{
    for(auto y = 0; y < cells.size(); ++y)
    {

        for(auto x = 0; x < cells.at(y).size(); ++x) {
            if(mmap.at(y).at(x) == 'W'){
                cells.at(y).at(x).a = rand() % 50 + 205;
            }
        }
    }
}

void Engine::randomColors()
{

    flushCells();

    for(int y = 0; y < (int)(mheight/mscale); ++y)
    {
        vector<SDL_Color> row;
        for(int x = 0; x < (int)(mwidth/mscale); ++x) {
            // Randomly change the colour
            Uint8 red = rand() % 255;
            Uint8 green = rand() % 255;
            Uint8 blue = rand() % 255;
            struct SDL_Color color;
            color.r = red;
            color.g = green;
            color.b = blue;
            color.a = 255;
            row.push_back(color);
        }
        cells.push_back(row);
    }
}

void Engine::solidColors()
{
    //currently only turns all cells black

    flushCells();

    for(int y = 0; y < mmap.size(); ++y)
    {
        
        vector<SDL_Color> row;
        for(int x = 0; x < mmap.at(y).size(); ++x) {
            struct SDL_Color color;
            color.r = 0;
            color.g = 0;
            color.b = 0;
            color.a = 255;
            row.push_back(color);
        }

        cells.push_back(row);
    }

}

void Engine::runPhysics(float deltat)
{
    for(Entity *i : entities) 
    {
        // update objects velocities
        // TODO: implement terminal velocity cap
        i -> setvx(i -> getvx() + (i -> getax() * deltat));
        i -> setvy(i -> getvy() + (i -> getay() * deltat));

        // update object positions
        i -> setx(i -> getx() + (i -> getvx() * deltat));
        i -> sety(i -> gety() + (i -> getvy() * deltat));
    }
}

void Engine::drawFrame()
{

    // draw background

    for(auto y = 0; y < (int) mheight/mscale; ++y)
    {
        for(auto x = 0; x < (int) mwidth/mscale; ++x) {
            // generic pixel object
            SDL_Rect pixel_rect;
            pixel_rect.x = x*mscale;
            pixel_rect.y = y*mscale;
            pixel_rect.w = mscale;
            pixel_rect.h = mscale;

            // get the color based on viewport offset (enables camera movement)!
            SDL_Color cex;

            try {
                cex = cells.at(y + viewport_rect.y).at(x + viewport_rect.x);
            }
            catch (const std::out_of_range& oor) {
                std::cerr << "Out of Range error: " << oor.what() << '\n';
                // color out of rangle pixel red
                cex = SDL_Color{225, 0,0, 225};
            }
            
            SDL_SetRenderDrawColor(mrenderer, cex.r, cex.g, cex.b, cex.a); 
            SDL_RenderFillRect(mrenderer, &pixel_rect);

        }
    }

    // draw entities (to nearest pixel)
    for(Entity *i : entities) 
    {
        // generic entity object
        SDL_Rect entity_rect;
        entity_rect.x = (i -> getx() - viewport_rect.x)*mscale;
        //(entities are on flipped y axis!)
        entity_rect.y = mheight - (int(floor(i -> gety())) + viewport_rect.y)*mscale;
        entity_rect.w = mscale;
        entity_rect.h = mscale;
        SDL_SetRenderDrawColor(mrenderer, i -> mcolor.r, i -> mcolor.g, i -> mcolor.b, i -> mcolor.a);
        SDL_RenderFillRect(mrenderer, &entity_rect);
    }
}

vector<vector<char> > Engine::readMap(string filename)
{
    vector<vector<char> > map;
    
    ifstream input(filename);

    for(string line; getline(input,line);)
    {
        vector<char> linevec(line.length());
        copy(line.begin(), line.end(), linevec.begin());
        map.push_back(linevec);
    }

    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, ("map: " + filename).c_str(), (to_string(map.size()) + " by " + to_string(map[1].size())).c_str(), NULL);

    return map;
}

void Engine::loadMap(string filename)
{

    mmap = readMap(filename);

    // intialize background
    solidColors();

    // TODO: MAKE THIS SCALE!

    for(auto y = 0; y < mmap.size(); ++y)
    {
        for(auto x = 0; x < mmap.at(y).size(); ++x) {
            
            if(mmap.at(y).at(x) == 'G'){
                cells.at(y).at(x).r = 0;
                cells.at(y).at(x).g = 255;
                cells.at(y).at(x).b = 0;
                cells.at(y).at(x).a = 255;
            }
            else if(mmap.at(y).at(x) == 'W'){
                cells.at(y).at(x).r = 0;
                cells.at(y).at(x).g = 0;
                cells.at(y).at(x).b = 255;
                cells.at(y).at(x).a = 255;
            }
            // color all else black
            else if (mmap.at(y).at(x) != ' '){
                cells.at(y).at(x).r = 255;
                cells.at(y).at(x).g = 255;
                cells.at(y).at(x).b = 240;
                cells.at(y).at(x).a = 255;
            }
            // color all else black
            else{
                cells.at(y).at(x).r = 0;
                cells.at(y).at(x).g = 0;
                cells.at(y).at(x).b = 0;
                cells.at(y).at(x).a = 255;
            }
        }
    }

    //set timer for water animation
    my_timer_id = SDL_AddTimer(250, changeOpacities, this);

}


// only offsets viewport if it's possible!
void Engine::offset_viewport_rect(int x, int y){
    if (viewport_rect.x + x  >= 0 && viewport_rect.y + y >= 0 
        && viewport_rect.x + x <= (int) mwidth/mscale && viewport_rect.y + y <= (int) mheight/mscale){
        viewport_rect.x += x; 
        viewport_rect.y += y;
    }
    else{
        printf("hit bounds!\n");
    }
}


// not a part of the class, just a callback for recurrent animations
Uint32 changeOpacities(Uint32 interval, void *param)
{
    static_cast<Engine*> (param) -> randomOpacities();
    return(interval);
}


#endif /* MY_CLASS_H */