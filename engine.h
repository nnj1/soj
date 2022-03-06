// my_class.h
#ifndef ENGINE_H // include guard
#define ENGINE_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <vector>

#include <cstdlib>   // rand and srand
#include <ctime>     // For the time function

using namespace std;

class Engine
{
private:
    int mwidth;
    int mheight;
    int mscale;

public:
    Engine(SDL_Renderer *renderer, int width, int height, int scale);

    void SetEngine(SDL_Renderer *renderer, int width, int height, int scale);
    void randomColors();
    void drawFrame();
    void flushCells();

    int getWidth() { return mwidth; }
    int getHeight() { return mheight; }
    int getScale() { return mscale; }
    SDL_Renderer* mrenderer;
    vector<vector<SDL_Color> > cells;
};

// Engine constructor
Engine::Engine(SDL_Renderer *renderer, int width, int height, int scale)
{
    SetEngine(renderer, width, height, scale);
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

}

void Engine::flushCells()
{
    cells.clear();
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

void Engine::drawFrame()
{
    for(auto y = 0; y < cells.size(); ++y)
    {
        for(auto x = 0; x < cells[y].size(); ++x) {
            SDL_Color ceg = cells[y][x];
            //printf("%p\n", &ceg);
            SDL_Rect pixel_rect;
            pixel_rect.x = x*mscale;
            pixel_rect.y = y*mscale;
            pixel_rect.w = mscale;
            pixel_rect.h = mscale;
            SDL_SetRenderDrawColor(mrenderer, ceg.r, ceg.g, ceg.b, ceg.a); 
            SDL_RenderFillRect(mrenderer, &pixel_rect);
        }
    }
}

#endif /* MY_CLASS_H */