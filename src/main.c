#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 400

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
    window = SDL_CreateWindow("ndsmapper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
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

void rotateSurface(SDL_Surface *s) { 
    SDL_Surface *buffer = SDL_CreateRGBSurface(0, 256, 192, 32, 0, 0, 0, 0);

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 192; j++) {
            putPixel(buffer, i, j, getPixel(s, 255 - i, 191 - j));
        }
    }

    SDL_BlitSurface(buffer, NULL, s, NULL);
    SDL_FreeSurface(buffer);
} 

void updateScreen(SDL_Surface *screenSurface, Uint8 grid[16][24]) {
    SDL_Surface *top = loadTexture("gfx/teste.png", screenSurface);
    rotateSurface(top);

    SDL_Surface *bottom = loadTexture("gfx/teste.png", screenSurface);

    double alpha = 0.4;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 192; j++) {
            if (!(i % 16) || !(j % 16)) {
                putPixel(top, i, j, 0);
            }
            else {
                Uint32 pixel = getPixel(top, i, j);

                Uint8 or = (pixel >> 16);
                Uint8 og = ((pixel << 16) >> 24);
                Uint8 ob = ((pixel << 24) >> 24);

                Uint8 r = ((1 - alpha)*or + alpha * (grid[i/16][j/16] ? 0x00 : 0xFF));
                Uint8 g = ((1 - alpha)*og + alpha * 0x00);
                Uint8 b = ((1 - alpha)*ob + alpha * (grid[i/16][j/16] ? 0xFF : 0x00));

                putPixel(top, i, j, (r << 16) | (g << 8) | b);
            }
        }
    }

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 192; j++) {
            if (!(i % 16) || !(j % 16)) {
                putPixel(bottom, i, j, 0);
            }
            else {
                Uint32 pixel = getPixel(bottom, i, j);

                Uint8 or = (pixel >> 16);
                Uint8 og = ((pixel << 16) >> 24);
                Uint8 ob = ((pixel << 24) >> 24);

                Uint8 r = ((1 - alpha)*or + alpha * (grid[i/16][12 + j/16] ? 0x00 : 0xFF));
                Uint8 g = ((1 - alpha)*og + alpha * 0x00);
                Uint8 b = ((1 - alpha)*ob + alpha * (grid[i/16][12 + j/16] ? 0xFF : 0x00));

                putPixel(bottom, i, j, (r << 16) | (g << 8) | b);
            }
        }
    }

    SDL_Rect screen = {.x = 0, .y = 0, .w = 256, .h = 192};
    SDL_Rect topScreen = {.x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT/2};
    SDL_Rect bottomScreen = {.x = 0, .y = SCREEN_HEIGHT/2, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT/2};

    SDL_BlitScaled(top, &screen, screenSurface, &topScreen);
    SDL_BlitScaled(bottom, &screen, screenSurface, &bottomScreen);

    SDL_FreeSurface(top);
    SDL_FreeSurface(bottom);
}

int main(int argc, char* args[]){
    SDL_Window *window = getWindow();
    SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
    SDL_Event e;

    Uint8 grid[16][24] = {0};

    updateScreen(screenSurface, grid);

    while (1) {
        usleep(100000);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                printf("{");
                for (int i = 0; i < 16; i++) {
                    printf("{");
                    for (int j = 0; j < 24; j++) {
                        printf("%d, ", grid[i][j] ? 255 : -1);
                    }
                    printf("},");
                    printf("\n");
                }
                printf("}\n");

                exit(0);
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                int x = ((16.0*e.button.x)/SCREEN_WIDTH);
                int y = ((24.0*e.button.y)/SCREEN_HEIGHT);

                grid[x][y] = !grid[x][y];

                updateScreen(screenSurface, grid);
            }
        }

        SDL_UpdateWindowSurface(window);
    }

    exitWindow(window);
    return 0;
}
