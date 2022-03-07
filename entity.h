// my_class.h
#ifndef ENTITY_H // include guard
#define ENTITY_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <fstream>

#include <cstdlib>   // rand and srand
#include <ctime>     // For the time function

using namespace std;


class Entity
{

    private:

        string mname;
        int mx;
        int my;
        float mvx;
        float mvy;
        float mfx;
        float mfy;
        

    public:

        // constructor and deconstructor
        Entity(string name, int x, int y, float vx, float vy, float fx, float fy, SDL_Color color);
        ~Entity();

        int getx() { return mx; }
        int gety() { return my; }
        int getvx() { return mvx; }
        int getvy() { return mvy; }
        int getfx() { return mfx; }
        int getfy() { return mfy; }

        void setx(int x) { mx = x; }
        void sety(int y) { my = y; }
        void setvx(float vx) { mvx = vx; }
        void setvy(float vy) { mvy = vy; }
        void setfx(float fx) { mfx = fx; }
        void setfy(float fy) { mfy = fy; }

        SDL_Color mcolor;

};

// Engine constructor
Entity::Entity(string name, int x, int y, float vx, float vy, float fx, float fy, SDL_Color color)
{
    mname = name;
    mx = x;
    my = y;
    mvx = vx;
    mvy = vy;
    mfx = fx;
    mfy = fy;
    mcolor = color;
}

// Define the destructor.
Entity::~Entity() {

}


#endif /* MY_CLASS_H */