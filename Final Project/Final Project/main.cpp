/**
* Author: Peter Perry
* Assignment: Final Project
* Date due: 2024-12-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "MenuScreen.h"
//#include "LevelB.h"
//#include "Effects.h"
#include "EndScreen.h"
#include "WinScreen.h"

bool player_won = false;
Uint32 win_time = 0;

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.1922f,
BG_BLUE    = 0.549f,
BG_GREEN   = 0.9059f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH  = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

Mix_Music* globalMusic;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
LevelA *g_levelA;
//LevelB *g_levelB;
EndScreen *g_EndScreen;
WinScreen *g_WinScreen;
MenuScreen* g_MenuScreen;

//Effects *g_effects;
Scene   *g_levels[4];
int g_lives = 5;
int g_level = 0;

SDL_Window* g_display_window;


ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;
bool g_is_paused = true;

AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
  
    if (scene == g_MenuScreen || scene == g_EndScreen || scene == g_WinScreen) {
            g_is_paused = true; // Pause all updates
            static_cast<LevelA *>(g_levelA)->set_paused(true); // Pause LevelA
            g_shader_program.set_use_lighting(false);
    } else {
        Mix_HaltChannel(-1);
        g_is_paused = false; // Resume updates
        if (scene == g_levelA) {
            static_cast<LevelA *>(g_levelA)->set_paused(false); // Unpause LevelA
            g_shader_program.set_use_lighting(true);
        }
    }
    
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
    
    if (scene == g_MenuScreen || scene == g_EndScreen || scene == g_WinScreen) {
            g_view_matrix = glm::mat4(1.0f);
            g_shader_program.set_view_matrix(g_view_matrix);
        }
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Maze Game",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_levelA = new LevelA();
    g_MenuScreen = new MenuScreen();
    // g_levelB = new LevelB();
    g_EndScreen = new EndScreen();
    g_WinScreen = new WinScreen();
    
    g_levels[0] = g_levelA;
    // g_levels[1] = g_levelB;
    g_levels[1] = g_MenuScreen;
    g_levels[2] = g_EndScreen;
    g_levels[3] = g_WinScreen;
    
    // Start at level A
    switch_to_scene(g_levels[1]);
    
    // g_effects = new Effects(g_projection_matrix, g_view_matrix);
    // g_effects->start(SHRINK, 2.0f);
    
    
}

void reset_time() {
    g_previous_ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    g_accumulator = 0.0f;
}

void process_input()
{
    
    if (g_current_scene->get_state().player != nullptr) {
        // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    }
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
                // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:
                        if (g_current_scene == g_MenuScreen) {
                            // Start the game when "Enter" is pressed
                            g_is_paused = false;
                            static_cast<LevelA *>(g_levelA)->set_paused(false);
                            reset_time();
                            switch_to_scene(g_levelA);
                        }
                        break;
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_current_scene->get_state().player != nullptr) {
                            if (g_current_scene->get_state().player->get_collided_bottom())
                            {
                                g_current_scene->get_state().player->jump();
                                Mix_PlayChannel(-1,  g_current_scene->get_state().jump_sfx, 0);
                            }
                        }
                        break;
                    case SDLK_p:
                        // Shoot water drop
                        if (g_current_scene != nullptr) {
                            g_current_scene->shoot_water();
                        }
                        break;
                    case SDLK_r:
                        if (g_current_scene == g_levels[2]) {
                            //                            g_current_scene = g_levels[0];
                            //                            g_current_scene->returnTo();
                            static_cast<LevelA *>(g_levelA)->set_paused(false);
                            g_is_paused = false;
                            reset_time();
                            Mix_ResumeMusic();
                            switch_to_scene(g_levelA);
                        }
                        break;
                    case SDLK_c:
                        if (g_current_scene->get_state().player == nullptr) {
                            g_is_paused = false;
                            static_cast<LevelA *>(g_levelA)->set_paused(false);
                            player_won = false;
                            reset_time();
                            Mix_ResumeMusic();
                            switch_to_scene(g_levelA);
                        }
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    // Handle movement in all four directions
    if (key_state[SDL_SCANCODE_LEFT]) {
        if (g_current_scene->get_state().player != nullptr) {
            g_current_scene->get_state().player->move_left();
        }
    }
    if (key_state[SDL_SCANCODE_RIGHT]) {
        if (g_current_scene->get_state().player != nullptr) {
            g_current_scene->get_state().player->move_right();
        }
    }
    if (key_state[SDL_SCANCODE_UP]) {
        if (g_current_scene->get_state().player != nullptr) {
            g_current_scene->get_state().player->move_up();
        }
    }
    if (key_state[SDL_SCANCODE_DOWN]) {
        if (g_current_scene->get_state().player != nullptr) {
            g_current_scene->get_state().player->move_down();
        }
    }
    if (g_current_scene->get_state().player != nullptr) {
        if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
    
}

void update()
{
    if (g_current_scene == g_MenuScreen || g_current_scene == g_EndScreen || g_current_scene == g_WinScreen) {
            return;
        }
    if (g_current_scene->get_state().player != nullptr) {
        
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        delta_time += g_accumulator;
        
        if (delta_time < FIXED_TIMESTEP)
        {
            g_accumulator = delta_time;
            return;
        }
        
        while (delta_time >= FIXED_TIMESTEP) {
            g_current_scene->update(FIXED_TIMESTEP);
            //        g_effects->update(FIXED_TIMESTEP);
            
            //        if (g_is_colliding_bottom == false && g_current_scene->get_state().player->get_collided_bottom()) g_effects->start(SHAKE, 1.0f);
            
//            g_is_colliding_bottom = g_current_scene->get_state().player->get_collided_bottom();
            
            delta_time -= FIXED_TIMESTEP;
        }
        
        g_accumulator = delta_time;
        
        g_view_matrix = glm::mat4(1.0f);
        
        if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
        } else {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
        }
    }
    if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().y < -8.0f && !player_won) {
        player_won = true;
        switch_to_scene(g_WinScreen);
        
    }
    
    if (g_current_scene == g_MenuScreen || g_current_scene == g_EndScreen || g_current_scene == g_WinScreen) {
            return;
        }
    
    
    if (g_current_scene->get_state().player != nullptr) {
        glm::vec3 player_position = g_current_scene->get_state().player->get_position();
        glm::vec4 view_position = g_view_matrix * glm::vec4(player_position, 1.0f);
        g_shader_program.set_light_position(glm::vec2(view_position.x, view_position.y));
    }
    
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
    
    
    //    g_effects->render();
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_levelA;
    //    delete g_levelB;
    delete g_EndScreen;
    delete g_WinScreen;
    delete g_MenuScreen;
    //    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    globalMusic = Mix_LoadMUS("assets/Maze_of_the_Mind.mp3");
    Mix_PlayMusic(globalMusic, -1);
    Mix_VolumeMusic(50);

    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
        
        render();
    }
    
    shutdown();
    return 0;
}
