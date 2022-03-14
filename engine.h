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
#include <map>

#include <cstdlib>   // rand and srand
#include <ctime>     // For the time function
#include <cmath>     // abs

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
        struct SOJ_Rect
        {
            float x;
            float y;
            float w;
            float h;
        };
        SOJ_Rect viewport_rect;
                
        // contains data loaded from .map file
        vector<vector<char> > mmap;

        // recurrent counter for environment animation updates
        SDL_TimerID my_timer_id;

        // mix music object pointer
        Mix_Music *music;

        // contains all entities
        vector<Entity*> entities;

        // physics constants
        const float GRAVITY = -0.002;
        const float elasticity = 0.995;

        // setting for enabling virtual pixel by pixel drawing, as opposed to smooth
        const bool pixelDraw = true;

        // will contain textures
        map<char, SDL_Surface*> textures; 

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
        void addEntity(Entity * i) { entities.push_back(i); } 

        vector<Entity*> getEntities() {return entities; }
        int getWidth() { return mwidth; }
        int getHeight() { return mheight; }
        int getScale() { return mscale; }
        SOJ_Rect getViewport_rect() { return viewport_rect; }
        void setViewport_rect(int x, int y, int w, int h) { viewport_rect = {(float) x, (float) y, (float) w, (float) h}; }
        void centerViewport_rect(Entity* i);
        SDL_Renderer* mrenderer;
        vector<vector<SDL_Color> > cells;

        void offset_viewport_rect(int x, int y);

        void drawText(string text,int text_size,int x,int y, Uint8 r,Uint8 g,Uint8 b);
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
    Mix_VolumeMusic(MIX_MAX_VOLUME/8);
    Mix_PlayMusic(music, 1);

  if ( TTF_Init() < 0 ) {
    cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
  }
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


    // apply universal forces to certain entities

    for(int j = 0; j < entities.size(); j++) 
    {
        Entity *i = entities[j];

        if (i -> getname() == "bullet")
            i -> setay(i -> getay() + GRAVITY); 
    }

    // check if there's any collisions with map structural components

    for(auto y = 0; y < cells.size(); ++y)
    {
        for(auto x = 0; x < cells.at(y).size(); ++x) {
            
            auto pixely = cells.size() - y; // flip the y axis, of course
            auto pixelx = x;

            // ground collisions only invert y velocity
            if (mmap.at(y).at(x) == 'G'){
                // see if any entities exist in the area denotes by the pixel
                for(int j = 0; j < entities.size(); j++) 
                {
                    Entity *i = entities[j];
                    //if ((int)(i -> getx() + 0.5f) == pixelx && (int)(i -> gety() + 0.5f) == pixely){
                    if ((int)(i -> getx() + 0.5f) == pixelx && i -> gety() < pixely + 1){
                        
                        // TODO: push object out of bounding box (1 pixel by 1 pixel)
                        //i -> setx(i -> getlastx());
                        //i -> sety(i -> getlasty());
                        //i -> sety(pixely + 1);

                        // reflect velocities
                        i -> setvy(i -> getvy() * -1);
                        //i -> setvy(0);

                        // TODO: drain energies depending on degree of inelasticity
                        i -> setvx(i -> getvx() * elasticity);
                        i -> setvy(i -> getvy() * elasticity);


                    }
                }
            }

            // brickwall collisions invert x velocity
            if (mmap.at(y).at(x) == 'B'){
                // see if any entities exist in the area denotes by the pixel
                for(int j = 0; j < entities.size(); j++) 
                {
                    Entity *i = entities[j];
                    if ((int)(i -> getx() + 0.5f) == pixelx && (int)(i -> gety() + 0.5f) == pixely){
                        // reflect velocities
                        i -> setvx(i -> getvx() * -1);
                        // drain energies depending on degree of inelasticity
                        i -> setvx(i -> getvx() * elasticity);
                        i -> setvy(i -> getvy() * elasticity);
                    }
                }
            }

            // water collisions dampen velocity
            if (mmap.at(y).at(x) == 'W'){
                // see if any entities exist in the area denotes by the pixel
                for(int j = 0; j < entities.size(); j++) 
                {
                    Entity *i = entities[j];
                    if ((int)(i -> getx() + 0.5f) == pixelx && (int)(i -> gety() + 0.5f) == pixely){
                        // TODO: push object out of bounding box (1 pixel by 1 pixel)

                        // reflect velocities
                        i -> setvx(i -> getvx() / 1.5);
                        i -> setvy(i -> getvy() / 1.5);
                        
                        // TODO: drain energies depending on degree of inelasticity
                        i -> setvx(i -> getvx() * elasticity);
                        i -> setvy(i -> getvy() * elasticity);
                    }
                }
            }
        }
    }

    // check entity-entity collisions

    for(int j = 0; j < entities.size(); j++) 
    {
        for(int k = j; k < entities.size(); k++) 
        {
            Entity *a = entities[j];
            Entity *b = entities[k];

            if (a -> getname() == "bullet" && a -> getname() == "bullet"
                && (int)(a -> getx() + 0.5f) == (int)(b -> getx() + 0.5f)
                && (int)(a -> gety() + 0.5f) == (int)(b -> gety() + 0.5f)){
                /* determine the type of collision
                 _ _ _ 
                |_|_|_|   
                |_|A|_|
                |_|_|_| 

                */ 

                float elasticity = 0.995;// custom elasticity here
                int lastx = (int) (b -> getlastx());
                int lasty = (int) (b -> getlasty());
                int ax = (int) (a -> getx());
                int ay = (int) (a -> gety());
                int bx = (int) (b -> getx());
                int by = (int) (b -> gety());

                // head on horizantal collision

                if(ax == lastx){
                   
                    /* TODO: push object out of bounding box (1 pixel by 1 pixel)
                    a -> setx(a -> getlastx());
                    a -> sety(a -> getlasty());
                    b -> setx(b -> getlastx());
                    b -> sety(b -> getlasty());*/

                    a -> setvx(a -> getvx() * -1 * elasticity);
                    b -> setvx(b -> getvx() * -1 * elasticity);
                }
                // head on vertical collision
                if(ay == lasty){

                    /* TODO: push object out of bounding box (1 pixel by 1 pixel)
                    a -> setx(a -> getlastx());
                    a -> sety(a -> getlasty());
                    b -> setx(b -> getlastx());
                    b -> sety(b -> getlasty());*/

                    a -> setvy(a -> getvy() * -1 * elasticity);
                    b -> setvy(b -> getvy() * -1 * elasticity);
                }
                // oblique collision
                if(ay != lasty && ax != lastx){

                    /* TODO: push object out of bounding box (1 pixel by 1 pixel)
                    a -> setx(a -> getlastx());
                    a -> sety(a -> getlasty());
                    b -> setx(b -> getlastx());
                    b -> sety(b -> getlasty());*/

                    a -> setvx(a -> getvx() * -1 * elasticity);
                    a -> setvy(a -> getvy() * -1 * elasticity);
                    b -> setvx(b -> getvx() * -1 * elasticity);
                    b -> setvy(b -> getvy() * -1 * elasticity);
                }


            }
        }
    }

    // update positions and velocities now

    for(int j = 0; j < entities.size(); j++) 
    {
        Entity *i = entities[j];

        // update objects velocities
        if (abs(i -> getvx() + (i -> getax() * deltat)) <= i -> gettermvx())
            i -> setvx(i -> getvx() + (i -> getax() * deltat));
        else 
            i -> setvx(i -> getvx());
        if (abs(i -> getvy() + (i -> getay() * deltat)) <= i -> gettermvy())
            i -> setvy(i -> getvy() + (i -> getay() * deltat));
        else 
            i -> setvy(i -> getvy());
        //printf("%f\n", i -> getvx());
        // update object positions
        i -> setx(i -> getx() + (i -> getvx() * deltat));
        i -> sety(i -> gety() + (i -> getvy() * deltat));

    }

    // check to see if any particulate entities are standstill and just zero them out

    for(int j = 0; j < entities.size(); j++) 
    {
        Entity *i = entities[j];

        if (i -> getname() == "particle")
            if (abs(i -> getx() - i -> getlastx()) < 0.00001 || abs(i -> gety() - i -> getlasty()) < 0.00001 ){
                // kill velocities and forces
                i -> setvx(0);
                i -> setvy(0);
                i -> setax(0);
                i -> setay(0);
                i -> standstill++;
            }

    }

    // if anything particulate has been standstill for too long free the memory

    if(entities.empty() == false) {
        for(int i = entities.size() - 1; i >= 0; i--) {
            if(entities.at(i) -> getname() == "particle" && entities.at(i) -> standstill > 100) {
                Entity * bad = entities.at(i);
                entities.erase( entities.begin() + i ); 
                delete bad;
                //printf(" was deleted\n");
            }
        }
    }

    // destroy bullet projectile if out of bounds OF THE MAP!

    if(entities.empty() == false) {
        for(int i = entities.size() - 1; i >= 0; i--) {
            if(entities.at(i) -> getname() == "bullet" && (entities.at(i) -> getx() < 0 || entities.at(i) -> getx() > cells[0].size() || entities.at(i) -> gety() < 0 || entities.at(i) -> gety() > cells.size())) {
                //printf("bullet at %f, %f", entities.at(i) -> getx(), entities.at(i) -> gety());
                Entity * bad = entities.at(i);
                entities.erase( entities.begin() + i ); 
                delete bad;
                //printf(" was deleted\n");
            }
        }
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
            char key;

            try {
                cex = cells.at(y + viewport_rect.y).at(x + viewport_rect.x);
                key = mmap.at(y + viewport_rect.y).at(x + viewport_rect.x);
            }
            catch (const std::out_of_range& oor) {
                //std::cerr << "Out of Range error: " << oor.what() << '\n';
                // color out of rangle pixel red
                cex = SDL_Color{225, 0,0, 225};
                key = '\n';
            }

            // see if the key corresponds to a texture
            if(textures.count(key))
            {
                printf("HAS KEY\n");
            }
            else
            {
                SDL_SetRenderDrawColor(mrenderer, cex.r, cex.g, cex.b, cex.a); 
                SDL_RenderFillRect(mrenderer, &pixel_rect);
            }

        }
    }

    // draw entities (to nearest pixel (or not))

    for(auto & i : entities) 
    {
        // generic rect object for drawing
        SDL_Rect entity_rect;

        if(pixelDraw and i -> getname() != "player") // if entity is player, scale it up
        {
            entity_rect.x = ((int)(i -> getx() + 0.5f) - viewport_rect.x)*mscale;
            //(entities are on flipped y axis!)
            entity_rect.y = (cells.size() - (int)(i -> gety() + 0.5f) - viewport_rect.y)*mscale;
        }
        else{
            entity_rect.x = (i -> getx() - viewport_rect.x)*mscale;
            //(entities are on flipped y axis!)
            entity_rect.y = (cells.size() - i -> gety() - viewport_rect.y)*mscale;
        }
        
        entity_rect.w = mscale;
        entity_rect.h = mscale;

        if(i -> getname() == "player"){
            entity_rect.w = mscale*2;
            entity_rect.h = mscale*2;
            entity_rect.x -= mscale;
            entity_rect.y -= mscale;
        }
        //printf("%d\n", entity_rect.x);
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

    for(auto y = 0; y < mmap.size(); ++y)
    {
        for(auto x = 0; x < mmap.at(y).size(); ++x) {
            
            char key = mmap.at(y).at(x);

            if(key == 'G'){
                cells.at(y).at(x).r = 0;
                cells.at(y).at(x).g = 255;
                cells.at(y).at(x).b = 0;
                cells.at(y).at(x).a = 255;
            }
            else if(key == 'W'){
                cells.at(y).at(x).r = 0;
                cells.at(y).at(x).g = 0;
                cells.at(y).at(x).b = 255;
                cells.at(y).at(x).a = 255;
            }
            // color brick walls brown
            else if (key == 'B'){
                cells.at(y).at(x).r = 255;
                cells.at(y).at(x).g = 255;
                cells.at(y).at(x).b = 0;
                cells.at(y).at(x).a = 255;
            }
            // color all else black
            else if (key != ' '){
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

            if (static_cast<bool>(ifstream("./textures/" + to_string(key) + ".png"))){
                printf("Found texture for %c!\n", key);
                textures[key] = IMG_Load(("./textures/" + to_string(key) + ".png").c_str());
            }
        }
    }



    printf("Loaded map that's %lu x %lu cells\n", cells[0].size(), cells.size());

    //set timer for water animation
    my_timer_id = SDL_AddTimer(250, changeOpacities, this);

}


// only offsets viewport from current position by int x and int y if it's possible!
void Engine::offset_viewport_rect(int x, int y){
    if (viewport_rect.x + x  >= 0 && viewport_rect.x + x <= (int) mwidth/mscale)
        viewport_rect.x += x; 
    else
        printf("hit x bounds!\n");
    if(viewport_rect.y + y >= 0 && viewport_rect.y + y <= (int) mheight/mscale)
        viewport_rect.y += y;
    else
        printf("hit y bounds!\n");
}

// center a viewport rectangle on an entity only if it's possible!
void Engine::centerViewport_rect(Entity* i){
    int x =  i -> getx() - mwidth/(2*mscale);
    int y = (cells.size() -  i -> gety()) - mheight/(2*mscale);

    if (x >= 0 && x <= (cells[0].size() - mwidth/mscale))
        viewport_rect.x = x;
    if (y >= 0 && y <= (cells.size() - mheight/mscale))
        viewport_rect.y = y;
}


void Engine::drawText(string text,int text_size,int x,int y, Uint8 r,Uint8 g,Uint8 b)
{
    TTF_Font* arial = TTF_OpenFont("assets/font.ttf",text_size);
    if(arial == NULL)
    {
        printf("TTF_OpenFont: %s\n",TTF_GetError());
    }
    SDL_Color textColor = {r,g,b};
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(arial,text.c_str(),textColor);
    if(surfaceMessage == NULL)
    {
        printf("Unable to render text surface: %s\n",TTF_GetError());
    }
    SDL_Texture* message = SDL_CreateTextureFromSurface(mrenderer,surfaceMessage);
    SDL_FreeSurface(surfaceMessage);
    int text_width = surfaceMessage->w;
    int text_height = surfaceMessage->h;
    SDL_Rect textRect{x,y,text_width,text_height};

    SDL_RenderCopy(mrenderer,message,NULL,&textRect);
    TTF_CloseFont(arial);
}

// not a part of the class, just a callback for recurrent animations
Uint32 changeOpacities(Uint32 interval, void *param)
{
    static_cast<Engine*> (param) -> randomOpacities();
    return(interval);
}


#endif /* MY_CLASS_H */