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
    const bool *keys;

    Uint64 now_time;
    Uint64 last_time;

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *ball_texture;
} AppState;

AppState app;
bool init();
void cleanup();
void pollEvents();
void clearScreen(SDL_Color *color);
void present();
float frameTime();


/****************************************************************
 *                          BALL                                *
 ****************************************************************/
#define BALL_RADIUS 15
#define BALL_SPEED 450

typedef struct {
    float radius;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    bool in_play;
} Ball;
Ball ball;

void ballInit();
void ballDraw();
void ballUpdate(float dt);
void ballLaunch();
void ballReset();
void ballWallBounce();


/****************************************************************
 *                          PADDLE                              *
 ****************************************************************/
#define PADDLE_WALL_OFFSET 10
#define PADDLE_WIDTH 25
#define PADDLE_HEIGHT 125
#define PADDLE_SPEED 400

typedef enum {
    PADDLE_LEFT,
    PADDLE_RIGHT
} PaddlePos;

SDL_FRect left_paddle;

void paddleInit(PaddlePos pos);
void paddleDraw();
void paddleMoveUp(PaddlePos pos, float dt);
void paddleMoveDown(PaddlePos pos, float dt);
void paddleCheckBounds();
bool paddleBallCollision(PaddlePos pos);
void paddleBallDeflect(PaddlePos pos);

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
    paddleInit(PADDLE_LEFT);

    while (app.running) {
        float delta_time = frameTime();

        // input
        pollEvents();
        if (app.keys[SDL_SCANCODE_ESCAPE]) break;

        if (!ball.in_play)
            if (app.keys[SDL_SCANCODE_SPACE]) ballLaunch();

        if (app.keys[SDL_SCANCODE_BACKSPACE]) ballReset();

        if (app.keys[SDL_SCANCODE_DOWN]) paddleMoveDown(PADDLE_LEFT, delta_time);
        if (app.keys[SDL_SCANCODE_UP])   paddleMoveUp(PADDLE_LEFT, delta_time);

        // update
        ballWallBounce();
        ballUpdate(delta_time);

        if (paddleBallCollision(PADDLE_LEFT)) {
            paddleBallDeflect(PADDLE_LEFT);
        }

        paddleCheckBounds();

        // draw
        clearScreen(&bg_color);

        ballDraw();
        paddleDraw();

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

    app.now_time = SDL_GetPerformanceCounter();
    app.last_time = app.now_time;

    SDL_srand(0);

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

float frameTime() {
    app.now_time = SDL_GetPerformanceCounter();
    float frame_time = (float)(app.now_time - app.last_time) / (float)SDL_GetPerformanceFrequency();
    app.last_time = app.now_time;

    return frame_time;
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

    ball.in_play = false;
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

void ballUpdate(float dt) {
    ball.x += ball.velocity_x * dt;
    ball.y += ball.velocity_y * dt;

    ball.in_play = ball.velocity_x != 0;
}

void ballLaunch() {
    ball.velocity_x = BALL_SPEED;

    // 50/50 chance of launching left or right
    if (SDL_rand(100) > 50) ball.velocity_x *= -1;

    ball.velocity_y = SDL_rand(300);
    if (SDL_rand(100) > 50) ball.velocity_y *= -1;
}

void ballReset() {
    ballInit();
}

void ballWallBounce() {

    if (ball.x <= 0 + ball.radius) {
        ball.x = ball.radius;
        ball.velocity_x *= -1;
    } else if (ball.x >= WIDTH - ball.radius) {
        ball.x = WIDTH - ball.radius;
        ball.velocity_x *= -1;
    }

    if (ball.y <= 0 + ball.radius) {
        ball.y = ball.radius;
        ball.velocity_y *= -1;
    } else if (ball.y >= HEIGHT - ball.radius) {
        ball.y = HEIGHT - ball.radius;
        ball.velocity_y *= -1;
    }
}

/****************************************************************
 *                          PADDLE                              *
 ****************************************************************/


void paddleInit(PaddlePos pos) {

    if (pos == PADDLE_LEFT) {

        left_paddle.w = PADDLE_WIDTH;
        left_paddle.h = PADDLE_HEIGHT;

        left_paddle.x = PADDLE_WALL_OFFSET;
        left_paddle.y = (HEIGHT / 2.0f) - (left_paddle.h / 2.0f);
    }
}

void paddleDraw() {
    SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(app.renderer, &left_paddle);
}

void paddleMoveUp(PaddlePos pos, float dt) {
    if (pos == PADDLE_LEFT)
        left_paddle.y -= PADDLE_SPEED * dt;
}

void paddleMoveDown(PaddlePos pos, float dt) {
    if (pos == PADDLE_LEFT)
        left_paddle.y += PADDLE_SPEED * dt;
}

void paddleCheckBounds() {
    if (left_paddle.y <= 0)
        left_paddle.y = 0;
    else if (left_paddle.y >= HEIGHT - left_paddle.h)
        left_paddle.y = HEIGHT - left_paddle.h;
}

float clamp(float n, float min, float max) {
    if (n < min) return min;

    if (n > max) return max;

    return n;
}

bool paddleBallCollision(PaddlePos pos) {

    if (ball.velocity_x == 0) return false;

    if (pos == PADDLE_LEFT) { // Moving left

        if (ball.velocity_x > 0) return false;

        float closest_x = clamp(ball.x, left_paddle.x, left_paddle.x + left_paddle.w);
        float closest_y = clamp(ball.y, left_paddle.y, left_paddle.y + left_paddle.h);

        float dist_x = ball.x - closest_x;
        float dist_y = ball.y - closest_y;

        float dist_squared = (dist_x * dist_x) + (dist_y * dist_y);

        return dist_squared < (ball.radius * ball.radius);
    }

    return false;
}

void paddleBallDeflect(PaddlePos pos) {

    if (pos == PADDLE_LEFT) {
        ball.x = (left_paddle.x + left_paddle.w) + ball.radius;
        ball.velocity_x *= -1;
    }


    float center = left_paddle.y + left_paddle.h / 2.0f;
    float hit_pos = ball.y - center;

    float hit_pos_normalized = hit_pos / (left_paddle.h / 2.0f);

    if (hit_pos_normalized < -1.0f)
        hit_pos_normalized = -1.0f;
    else if (hit_pos_normalized > 1.0f)
        hit_pos_normalized = 1.0f;

    ball.velocity_y = hit_pos_normalized * 300;
}
