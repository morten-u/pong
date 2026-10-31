#include <SDL3/SDL.h>
#include <stdlib.h>

/****************************************************************
 *                          APPSTATE                            *
 ****************************************************************/
#define TITLE "Pong"

// 4:3 ratio
#define WIDTH 1080
#define HEIGHT 810

typedef struct {
    bool running;
    SDL_Window *window;
    SDL_Renderer *renderer;
    const bool *keys;
} AppState;

AppState app;
bool init();
void cleanup();
void pollEvents();
void clearScreen(SDL_Color *color);
void present();

/****************************************************************
 *                          MAIN                                *
 ****************************************************************/
int main() {

    if (!init()) return 1;
    atexit(cleanup);

    SDL_Color bg_color = {
        .r = 40,
        .g = 40,
        .b = 50,
        .a = 255
    };

    while (app.running) {
        pollEvents();

        clearScreen(&bg_color);
        present();
    }

    return 0;
}


/****************************************************************
 *                          APPSTATE                            *
 ****************************************************************/

bool init() {
    app.window = NULL;
    app.renderer = NULL;
    app.running = true;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error: Failed to initialize SDL3: %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(TITLE, WIDTH, HEIGHT, 0, &app.window, &app.renderer)) {
        SDL_Log("Error: Failed to create window/renderer: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    app.keys = SDL_GetKeyboardState(NULL);

    if (!SDL_SetRenderVSync(app.renderer, 1)) { // Limit framerate to screens refresh rate
        SDL_Log("Error: failed to set refresh rate: %s\n", SDL_GetError());
        cleanup();
    }

    return true;
}

void cleanup() {

    if (app.renderer)
        SDL_DestroyRenderer(app.renderer);

    if (app.window)
        SDL_DestroyWindow(app.window);

    SDL_Quit();
}

void pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            app.running = false;
        }
    }
}

void clearScreen(SDL_Color *color) {
    SDL_SetRenderDrawColor(app.renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(app.renderer);
}

void present() {
    SDL_RenderPresent(app.renderer);
}
