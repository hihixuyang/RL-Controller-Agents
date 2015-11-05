#include "event.h"

void pollEvent(int * delayLen, int * controllerDelay, int * quit, int * agentStats, int * controllerStats, int * actionStats, int * graphics, int * view, int * teamShow)
{
    SDL_Event event, event2;
    int pause = 0;

    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT){
            *quit = 1;
        }
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE){
            *quit = 1;
        }
        if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.sym == SDLK_LEFT){
                *delayLen -= DELAY_STEP;
                if (*delayLen < 0)
                    *delayLen = 0;
            }
            else if (event.key.keysym.sym == SDLK_RIGHT)
                *delayLen += DELAY_STEP;
            else if (event.key.keysym.sym == SDLK_s)
                *agentStats = *agentStats == 0 ? 1 : 0;
            else if (event.key.keysym.sym == SDLK_UP)
                *controllerDelay += 1000;
            else if (event.key.keysym.sym == SDLK_DOWN){
                *controllerDelay -= 1000;
                if (*controllerDelay < 0)
                    *controllerDelay = 0;
            }
            else if (event.key.keysym.sym == SDLK_c)
                *controllerStats = *controllerStats == 0 ? 1 : 0;
            else if (event.key.keysym.sym == SDLK_a)
                *actionStats = *actionStats == 0 ? 1 : 0;
            else if (event.key.keysym.sym == SDLK_g)
                *graphics = *graphics == 0 ? 1 : 0;
            else if (event.key.keysym.sym == SDLK_f)
                *view = *view == 2 ? 0 : *view + 1;
            else if (event.key.keysym.sym == SDLK_t)
                *teamShow = *teamShow == GRN ? RED : GRN;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE)
            pause = 1;

        while (pause){
            SDL_PollEvent(&event2);
            if (event2.type == SDL_KEYUP && (event2.key.keysym.sym == SDLK_SPACE))
                pause = 0;
        }
    }
}
