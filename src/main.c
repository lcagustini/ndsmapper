#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#define SCREEN_SIZE 600

Uint32 getPixel(SDL_Surface *surface, int x, int y){
    Uint32 *pixels = (Uint32 *)surface->pixels;
    return pixels[(y*surface->w) + x];
}

void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel){
    Uint32 *pixels = (Uint32 *)surface->pixels;
    pixels[(y*surface->w) + x] = pixel;
}

SDL_Window *getWindow(){
    SDL_Window *window = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }
    window = SDL_CreateWindow("ndsmapper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE, SCREEN_SIZE, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return NULL;
    }
    return window;
}

void exitWindow(SDL_Window *window){
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Surface *loadTexture(char *path, SDL_Surface *screen){
    SDL_Surface *load = NULL, *opt = NULL;

    load = IMG_Load(path);
    if (load == NULL) {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        return NULL;
    }
    opt = SDL_ConvertSurface(load, screen->format, 0);
    SDL_FreeSurface(load);
    if (opt == NULL) {
        printf("Unable to optimize image! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }

    return opt;
}

int main(int argc, char* args[]){
    SDL_Window *window = getWindow();
    SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
    SDL_Event e;

    SDL_Surface *front = loadTexture("gfx/teste.png", screenSurface);

    {
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                if (!(i % 16) || !(j % 16)) {
                    putPixel(front, i, j, 0);
                }
                else {
                    Uint32 pixel = getPixel(front, i, j);

                    Uint8 or = (pixel >> 16);
                    Uint8 og = ((pixel << 16) >> 24);
                    Uint8 ob = ((pixel << 24) >> 24);

                    double alpha = 0.2;
                    Uint8 r = ((1 - alpha)*or + alpha*0xFF);
                    Uint8 g = ((1 - alpha)*og + alpha*0x00);
                    Uint8 b = ((1 - alpha)*ob + alpha*0x00);

                    putPixel(front, i, j, (r << 16) | (g << 8) | b);
                }
            }
        }

        SDL_BlitScaled(front, NULL, screenSurface, NULL);
    }

    while (1) {
        usleep(100000);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            }
        }

        SDL_UpdateWindowSurface(window);
    }

    exitWindow(window);
    return 0;
}
