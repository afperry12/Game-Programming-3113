#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"
#include "GameCommon.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* background;
};

GameStatus game_status = PLAYING;

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 *2,
          WINDOW_HEIGHT = 480 *2;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char SPRITESHEET_FILEPATH[] = "assets/george_0.png";
constexpr char RED_PLATFORM_FILEPATH[]    = "assets/neon_platform_red.png";
constexpr char GREEN_PLATFORM_FILEPATH[]    = "assets/neon_platform_green.png";
constexpr char LAUNCHPAD_FILEPATH[]    = "assets/launchpad.png";
constexpr char BACKGROUND_FILEPATH[] = "assets/Lunar Lander Background.jpeg";
constexpr char WIN_BACKGROUND_FILEPATH[] = "assets/Mission_Accomplished.png";
constexpr char FAIL_BACKGROUND_FILEPATH[] = "assets/Mission_Failed.png";

GLuint win_background_texture_id;
GLuint fail_background_texture_id;
GLuint background_texture_id;



constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

constexpr int CD_QUAL_FREQ    = 44100,
          AUDIO_CHAN_AMT  = 2,
          AUDIO_BUFF_SIZE = 4096;

constexpr char BGM_FILEPATH[] = "assets/crypto.mp3",
           SFX_FILEPATH[] = "assets/bounce.wav";

constexpr int PLAY_ONCE = 0,
          NEXT_CHNL = -1,
          ALL_SFX_CHNL = -1;


Mix_Music *g_music;
Mix_Chunk *g_jump_sfx;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

int platform_at_risk;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, Physics (again)!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ––––– BGM ––––– //
    Mix_OpenAudio(CD_QUAL_FREQ, MIX_DEFAULT_FORMAT, AUDIO_CHAN_AMT, AUDIO_BUFF_SIZE);

    g_music = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);

    g_jump_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– TEXTURES ––––– //
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    GLuint red_platform_texture_id = load_texture(RED_PLATFORM_FILEPATH);
    GLuint green_platform_texture_id = load_texture(GREEN_PLATFORM_FILEPATH);
    GLuint launchpad_texture_id = load_texture(LAUNCHPAD_FILEPATH);
    win_background_texture_id = load_texture(WIN_BACKGROUND_FILEPATH);
    fail_background_texture_id = load_texture(FAIL_BACKGROUND_FILEPATH);

    // ––––– PLAYER (NOT GEORGE) ––––– //
    int player_walking_animation[4][4] = {
        { 1, 5, 9, 13 },  // Move left
        { 3, 7, 11, 15 }, // Move right
        { 2, 6, 10, 14 }, // Move up
        { 0, 4, 8, 12 }   // Move down
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -0.5f, 0.0f);
    g_state.player = new Entity(
        player_texture_id,
        5.0f,
        acceleration,
        3.0f,
        player_walking_animation,
        0.0f,
        4,
        0,
        4,
        4,
        1.0f,
        1.0f,
        PLAYER
    );

    // Place the player in a fixed position
    g_state.player->set_position(glm::vec3(-2.0f, 2.0f, 0.0f));

    // ––––– LAUNCHPAD ––––– //
    Entity launchpad;
    launchpad.set_texture_id(launchpad_texture_id);
    launchpad.set_position(glm::vec3(-2.0f, 1.0f, 0.0f));
    launchpad.set_width(1.0f);
    launchpad.set_height(0.25f);
    launchpad.set_entity_type(PLATFORM);
    launchpad.update(0.0f, NULL, NULL, 0);

    // ––––– PLATFORMS ––––– //
    g_state.platforms = new Entity[PLATFORM_COUNT + 1];
    g_state.platforms[0] = launchpad;

    // Randomly place the remaining platforms
    for (int i = 1; i < PLATFORM_COUNT + 1; i++) {
        bool valid_position = false;
        while (!valid_position) {
            float random_x = ((rand() % 1000) / 1000.0f) * 10.0f - 5.0f;
            float random_y = ((rand() % 1000) / 1000.0f) * 7.5f - 3.75f;

            bool overlaps = false;

            // Check overlap with player
            if (fabs(random_x - g_state.player->get_position().x) < 1.0f &&
                fabs(random_y - g_state.player->get_position().y) < 1.0f) {
                overlaps = true;
            }

            // Check overlap with other platforms
            for (int j = 0; j < i; j++) {
                if (fabs(random_x - g_state.platforms[j].get_position().x) < 1.0f &&
                    fabs(random_y - g_state.platforms[j].get_position().y) < 0.5f) {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps) {
                PlatformType platform_type;
                GLuint platform_texture_id;

                if (rand() % 2 == 0) {
                    platform_texture_id = green_platform_texture_id;
                    platform_type = GREEN_PLATFORM;
                } else {
                    platform_texture_id = red_platform_texture_id;
                    platform_type = RED_PLATFORM;
                }
                g_state.platforms[i].set_texture_id(platform_texture_id);
                g_state.platforms[i].set_position(glm::vec3(random_x, random_y, 0.0f));
                g_state.platforms[i].set_width(0.8f);
                g_state.platforms[i].set_height(0.25f);
                g_state.platforms[i].set_entity_type(PLATFORM);
                g_state.platforms[i].set_platform_type(platform_type);
                g_state.platforms[i].update(0.0f, NULL, NULL, 0);
                valid_position = true;
            }
        }
    }

    srand(static_cast<unsigned int>(time(0)));
    platform_at_risk = rand() % PLATFORM_COUNT;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    background_texture_id = load_texture(BACKGROUND_FILEPATH);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_q)
                {
                    // Quit the game with a keystroke
                    g_game_is_running = false;
                }
                else if (event.key.keysym.sym == SDLK_r && game_status != PLAYING)
                {
                    // Restart the game if the mission is accomplished or failed
                    game_status = PLAYING;
                    initialise();
                }
                break;

            default:
                break;
        }
    }

    // Only process player input if the game is still playing
    if (game_status == PLAYING) {
        const Uint8 *key_state = SDL_GetKeyboardState(NULL);

        g_state.player->set_movement(glm::vec3(0.0f));

        if (key_state[SDL_SCANCODE_LEFT])
        {
            g_state.player->move_left();
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            g_state.player->move_right();
        }

        // Apply thrust if the up arrow key is pressed
        if (key_state[SDL_SCANCODE_UP])
        {
            g_state.player->apply_thrust();
        }
        else
        {
            // If up arrow is not pressed, stop adding thrust
            g_state.player->stop_thrust();
        }

        // Normalize movement vector if its length exceeds 1 to avoid excessive speeds.
        if (glm::length(g_state.player->get_movement()) > 1.0f)
        {
            g_state.player->normalise_movement();
        }
    }
}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        // Update the player entity (assuming g_state.player exists)
        g_state.player->update(FIXED_TIMESTEP, nullptr, g_state.platforms, PLATFORM_COUNT);

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
}

void render_background() {

    // Set background vertices
    GLfloat background_vertices[] = {
        -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f,          // triangle 1
        -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f   // triangle 2
    };

    // Set background texture coords
    GLfloat background_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,             // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Reset the shader program to identity matrix
    g_program.set_model_matrix(glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, background_texture_id);
    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());
    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_win_background() {
    // Set background vertices
    GLfloat background_vertices[] = {
        -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f,          // triangle 1
        -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f           // triangle 2
    };

    // Set background texture coords
    GLfloat background_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,             // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f              // triangle 2
    };

    g_program.set_model_matrix(glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, win_background_texture_id);
    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());
    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_fail_background() {
    // Set background vertices
    GLfloat background_vertices[] = {
        -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f,          // triangle 1
        -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f           // triangle 2
    };

    // Set background texture coords
    GLfloat background_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,             // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f              // triangle 2
    };

    g_program.set_model_matrix(glm::mat4(1.0f));
    glBindTexture(GL_TEXTURE_2D, fail_background_texture_id);
    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());
    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (game_status == PLAYING) {
        render_background();
        g_state.player->render(&g_program);
        for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    }
    else if (game_status == MISSION_FAILED) {
        render_fail_background();
    }
    else if (game_status == MISSION_ACCOMPLISHED) {
        render_win_background();
    }

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();

    delete [] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        if (game_status == PLAYING) {
            update();
        }
        render();
    }

    shutdown();
    return 0;
}
