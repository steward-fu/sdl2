#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>

#define PREFIX "[SDL2 TestApp] " 
const int w = 320;
const int h = 240;
const int bpp = 32;
static SDL_Rect rt = {0};
static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_GLContext context = {0};

const char *vShaderSrc =
    "attribute vec4 a_position;   \n"
    "attribute vec2 a_texCoord;   \n"
    "varying vec2 v_texCoord;     \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = a_position; \n"
    "   v_texCoord = a_texCoord;  \n"
    "}                            \n";
 
const char *fShaderSrc =
    "#ifdef GL_ES                                         \n"
    "precision mediump float;                             \n"
    "#endif                                               \n"
    "varying vec2 v_texCoord;                             \n"
    "uniform sampler2D s_texture;                         \n"
    "void main()                                          \n"
    "{                                                    \n"
    "    gl_FragColor = texture2D(s_texture, v_texCoord); \n"
    "}                                                    \n";

static void flip(void)
{
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void fill_color(void)
{
    printf(PREFIX"%s\n", __func__);
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

static void draw_gfx(void)
{
    printf(PREFIX"%s\n", __func__);
    SDL_RenderClear(renderer);
    boxRGBA(renderer, 50, 100, 100, 150, 0xff, 0x00, 0x00, 0xff);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
}

static void load_png(void)
{
    printf(PREFIX"%s\n", __func__);
    SDL_Surface *png = IMG_Load("bg.png");
    SDL_BlitSurface(png, NULL, screen, NULL);
    SDL_FreeSurface(png);

    flip();
    SDL_Delay(3000);
}

static void run_screentear(void)
{
    int cc = 300;
    uint32_t col[]={0xff0000, 0xff00, 0xff};

    printf(PREFIX"%s\n", __func__);
    while (cc--) {
        SDL_FillRect(screen, &screen->clip_rect, col[cc % 3]);
        flip();
        SDL_Delay(1000 / 60);
    }
}

static void play_wav(void)
{
    printf(PREFIX"%s\n", __func__);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music *music = Mix_LoadMUS("nokia.wav");
    Mix_PlayMusic(music, 1);
    SDL_Delay(26000);
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
}

static void draw_text(void)
{
    printf(PREFIX"%s\n", __func__);
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("font.ttf", 24);
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));

    int ww = 0, hh = 0;
    SDL_Color col = {0, 255, 0};
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

static void draw_texture(void)
{
    GLuint vShader = 0;
    GLuint fShader = 0;
    GLuint pObject = 0;

    printf(PREFIX"%s\n", __func__);
    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSrc, NULL);
    glCompileShader(vShader);

    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSrc, NULL);
    glCompileShader(fShader);

    pObject = glCreateProgram();
    glAttachShader(pObject, vShader);
    glAttachShader(pObject, fShader);
    glLinkProgram(pObject);
    glUseProgram(pObject);

    GLint positionLoc = glGetAttribLocation(pObject, "a_position");
    GLint texCoordLoc = glGetAttribLocation(pObject, "a_texCoord");
    GLint samplerLoc = glGetUniformLocation(pObject, "s_texture");

    GLuint textureId = 0;
    GLubyte pixels[320 * 240 * 3] = {0};

    int x = 0, y = 0;
    for (y = 0; y < 240; y++) {
        for (x = 0; x < 320; x++) {
            switch (y / 80) {
            case 0:
                pixels[(y * 320 * 3) + (x * 3) + 0] = 0xff;
                pixels[(y * 320 * 3) + (x * 3) + 1] = 0x00;
                pixels[(y * 320 * 3) + (x * 3) + 2] = 0x00;
                break;
            case 1:
                pixels[(y * 320 * 3) + (x * 3) + 0] = 0x00;
                pixels[(y * 320 * 3) + (x * 3) + 1] = 0xff;
                pixels[(y * 320 * 3) + (x * 3) + 2] = 0x00;
                break;
            case 2:
                pixels[(y * 320 * 3) + (x * 3) + 0] = 0x00;
                pixels[(y * 320 * 3) + (x * 3) + 1] = 0x00;
                pixels[(y * 320 * 3) + (x * 3) + 2] = 0xff;
                break;
            }
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 320, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLfloat vVertices[] = {
       -1.0f,  1.0f, 0.0f, 0.0f,  0.0f,
       -1.0f, -1.0f, 0.0f, 0.0f,  1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,  1.0f,
        1.0f,  1.0f, 0.0f, 1.0f,  0.0f
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(samplerLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    SDL_GL_SwapWindow(window);
    SDL_Delay(3000);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(context);
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    screen = SDL_CreateRGBSurface(0, w, h, bpp, 0, 0, 0, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
    context = SDL_GL_CreateContext(window);

    fill_color();
    draw_gfx();
    load_png();
    draw_text();
    play_wav();
    run_screentear();
    draw_texture();
 
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
