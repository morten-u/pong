#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
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
    SDL_Texture *ball_texture;
} AppState;

AppState app;
bool init();
void cleanup();
void pollEvents();
void clearScreen(SDL_Color *color);
void present();


/****************************************************************
 *                          BALL                                *
 ****************************************************************/
#define BALL_RADIUS 15

typedef struct {
    float radius;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
} Ball;
Ball ball;

void ballInit();
void ballDraw();


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

    ballInit();

    while (app.running) {
        pollEvents();

        clearScreen(&bg_color);

        ballDraw();

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
    app.ball_texture = NULL;
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
        return false;
    }

    SDL_Surface *ball_surface = SDL_LoadSurface("circle_white_256.png");
    if (!ball_surface) {
        SDL_Log("Error: failed to load ball surface: %s\n", SDL_GetError());
        cleanup();
        return false;
    }
    app.ball_texture = SDL_CreateTextureFromSurface(app.renderer, ball_surface);
    SDL_DestroySurface(ball_surface);
    if (!app.ball_texture) {
        SDL_Log("Error: Failed to create ball texture from surface: %s\n", SDL_GetError());
        cleanup();
        return false;
    }


    return true;
}

void cleanup() {

    if (app.ball_texture)
        SDL_DestroyTexture(app.ball_texture);

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

/****************************************************************
 *                          BALL                                *
 ****************************************************************/

void ballInit() {
    ball.radius = BALL_RADIUS;

    ball.x = (WIDTH / 2.0f) - ball.radius;
    ball.y = (HEIGHT / 2.0f) - ball.radius;

    ball.velocity_x = 0;
    ball.velocity_y = 0;
}

void ballDraw() {
    SDL_FRect src = {
        .x = 0,
        .y = 0,
        .w = app.ball_texture->w,
        .h = app.ball_texture->h
    };

    SDL_FRect dst = {
        .x = ball.x - ball.radius,
        .y = ball.y - ball.radius,
        .w = ball.radius * 2,
        .h = ball.radius * 2,
    };

    SDL_RenderTexture(app.renderer, app.ball_texture, &src, &dst);
}
