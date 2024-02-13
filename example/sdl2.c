#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>

#define PREFIX "[SDL2 TestApp] " 
const int w = 640;
const int h = 480;
const int bpp = 32;
static SDL_Rect rt = {0};
static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_GLContext context = {0};

static void flip(void)
{
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void test_color(void)
{
    printf(PREFIX"Test Color\n");
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0x00, 0x00));

    rt.x = 50;
    rt.y = 50;
    rt.w = 30;
    rt.h = 30;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0xff, 0x00));
 
    rt.x = 100;
    rt.y = 100;
    rt.w = 50;
    rt.h = 100;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0x00, 0xff));
    flip();
    SDL_Delay(3000);
}

static void test_gfx(void)
{
    printf(PREFIX"Test GFX\n");
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
    SDL_RenderClear(renderer);
    boxRGBA(renderer, 10, 10, 150, 100, 0xff, 0x00, 0x00, 0xff);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
}

static void test_png(void)
{
    printf(PREFIX"Test PNG\n");
    SDL_Surface *png = IMG_Load("bg.png");
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
    SDL_BlitSurface(png, NULL, screen, NULL);
    SDL_FreeSurface(png);

    flip();
    SDL_Delay(3000);
}

static void test_font(void)
{
    printf(PREFIX"Test Font\n");
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("font.ttf", 24);
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));

    int ww = 0, hh = 0;
    SDL_Color col = {255, 0, 0};
    SDL_Rect rt = {0, 100, 0, 0};
    const char *cc = "SDL2 TestApp by 司徒";

    TTF_SizeUTF8(font, cc, &ww, &hh);
    rt.x = (w - ww) / 2;
    SDL_Surface *msg = TTF_RenderUTF8_Solid(font, cc, col);
    SDL_BlitSurface(msg, NULL, screen, &rt);
    SDL_FreeSurface(msg);

    flip();
    SDL_Delay(3000);

    TTF_CloseFont(font);
    TTF_Quit();
}

static void test_audio(void)
{
    printf(PREFIX"Test Audio\n");
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
    flip();

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music *music = Mix_LoadMUS("nokia.wav");
    Mix_PlayMusic(music, 1);
    SDL_Delay(5000);
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
}

static void test_tear(void)
{
    int cc = 300;
    uint32_t col[]={0xff0000, 0xff00, 0xff};

    printf(PREFIX"Test Tear\n");
    while (cc--) {
        SDL_FillRect(screen, &screen->clip_rect, col[cc % 3]);
        flip();
        SDL_Delay(1000 / 60);
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    window = SDL_CreateWindow("main", 0, 0, w, h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    context = SDL_GL_CreateContext(window);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
    screen = SDL_CreateRGBSurface(0, w, h, bpp, 0, 0, 0, 0);

    test_color();
    //test_gfx(); // not implement
    test_png();
    test_font();
    test_audio();
    test_tear();
 
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
