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
        float mx;
        float my;
        float mvx;
        float mvy;
        float max;
        float may;
        float mmass;
        float mtermvx;
        float mtermvy;
        float mlastx;
        float mlasty;
        

    public:

        // constructor and deconstructor
        Entity(string name, float x, float y, float vx, float vy, float ax, float ay, float termvx, float termvy, SDL_Color color, float mass);
        ~Entity();

        float getx() { return mx; }
        float gety() { return my; }
        float getlastx() { return mlastx; }
        float getlasty() { return mlasty; }
        float getvx() { return mvx; }
        float getvy() { return mvy; }
        float getax() { return max; }
        float getay() { return may; }
        string getname() {return mname; }
        float gettermvx() { return mtermvx; }
        float gettermvy() { return mtermvy; }
        float getmass() { return mmass; }

        void setx(float x) { mlastx = mx; mx = x; }
        void sety(float y) { mlasty = my; my = y; }
        void setvx(float vx) { mvx = vx; }
        void setvy(float vy) { mvy = vy; }
        void setax(float ax) { max = ax; }
        void setay(float ay) { may = ay; }
        void settermvx(float termvx) { mtermvx = termvx; }
        void settermvy(float termvy) { mtermvy = termvy; }
        void setmass(float mass) { mmass = mass; }

        void shove(float fx, float fy) { max += fx / mmass; may += fy / mmass; }

        SDL_Color mcolor;

};

// Engine constructor
Entity::Entity(string name, float x = 0, float y = 0, float vx = 0.0, float vy = 0.0, float ax = 0.0, float ay = 0.0, float termvx = 10, float termvy = 10, SDL_Color color = {255, 0,0, 255}, float mass = 1.0)
{
    mname = name;
    mx = x;
    my = y;
    mlastx = x;
    mlasty = y;
    mvx = vx;
    mvy = vy;
    max = ax;
    may = ay;
    mtermvx = termvx;
    mtermvy = termvy;
    mcolor = color;
    mmass = mass;
}

// Define the destructor.
Entity::~Entity() {

}


#endif /* MY_CLASS_H */