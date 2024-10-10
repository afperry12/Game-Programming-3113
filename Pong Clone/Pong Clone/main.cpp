/**
* Author: Peter Perry
* Assignment: Simple 2D Scene
* Date due: 2023-09-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

// Dependencies
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

// Constant for window scaling
constexpr float WINDOW_SIZE_MULT = 2.0f;

// Window dimensions based on scale factor
constexpr int WINDOW_WIDTH  = 640 * WINDOW_SIZE_MULT,
              WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;

// Background color components
constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

// Viewport parameters
constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Vertex and fragment shader program paths
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// Constant for ms
constexpr float MILLISECONDS_IN_SECOND = 1000.0;

// File paths for character images
constexpr char SPONGEBOB_SPRITE_FILEPATH[] = "textures/Spongebob1.PNG",
               PATRICK_SPRITE_FILEPATH[]  = "textures/Patrick.png";

// Constant distance for collision detection
constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;

// Height for scaling characters
float targetHeight = 1.5f;

// Define pixel ratios
float aspectRatioSpongebob = 308.0f / 382.0f;
float aspectRatioPatrick = 310.0f / 473.0f;

// Scale characters based on pixel ratios
glm::vec3 INIT_SCALE_SPONGEBOB = glm::vec3(targetHeight * aspectRatioSpongebob, targetHeight, 0.0f);
glm::vec3 INIT_SCALE_PATRICK = glm::vec3(targetHeight * aspectRatioPatrick, targetHeight, 0.0f);

// Set starting positions of characters
constexpr glm::vec3 INIT_POS_SPONGEBOB  = glm::vec3(0.0f, 0.0f, 0.0f),
                    INIT_POS_PATRICK   = glm::vec3(-2.0f, 0.0f, 0.0f);

// Variables for window, app status, shader program
SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

// Transformation matrices for view, model, and projection
glm::mat4 g_view_matrix, g_spongebob_matrix, g_projection_matrix, g_patrick_matrix;

// Track ticks
float g_previous_ticks = 0.0f;

// Define texture IDs
GLuint g_background_texture_id;
GLuint g_spongebob_texture_id;
GLuint g_patrick_texture_id;

// Position and movements vectors
glm::vec3 g_spongebob_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_spongebob_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float g_spongebob_speed = 1.0f;

// Main function calls
void initialize();
void process_input();
void update();
void render();
void shutdown();

// Texture loading constants
constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;

// Window border limits
constexpr float LEFT_BORDER = -5.0f;
constexpr float RIGHT_BORDER = 5.0f;
constexpr float BOTTOM_BORDER = -3.75f;
constexpr float TOP_BORDER = 3.75f;


// Vectors to store multiple entities for jellyfish and manrays
std::vector<Entity> jellyfish;
std::vector<Entity> manrays;
GLuint jellyfish_texture_id;
GLuint manray_texture_id;

// Load texture from files
GLuint load_texture(const char* filepath)
{
    // Load the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    // Ensure loaded correctly
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

// Function to initialize jellyfish and manray entities
void initialize_entities() {
    // Load textures for jellyfish and manrays
    jellyfish_texture_id = load_texture("textures/Jellyfish.PNG");
    manray_texture_id = load_texture("textures/ManRay.png");

    // Spawn 5 jellyfish with random positions
    for (int i = 0; i < 5; ++i) {
        glm::vec3 randomPos = glm::vec3(
            -4.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 9.0f)),
            -3.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 7.0f)),
            0.0f
        );
        jellyfish.push_back(Entity(randomPos, jellyfish_texture_id, glm::vec3(1.0f, 1.0f, 0.0f)));
    }

    // Spawn 2 manrays with random positions
    for (int i = 0; i < 2; ++i) {
        glm::vec3 randomPos = glm::vec3(
            -4.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 9.0f)),
            -3.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 7.0f)),
            0.0f
        );
        manrays.push_back(Entity(randomPos, manray_texture_id, glm::vec3(1.0f, 1.0f, 0.0f)));
    }
}



void initialize()
{
    // Initialize SDL and create a window for rendering
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Spongebob!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Set viewport
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    // View and projection matrices setup
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    // Send shader the view and projection matrices
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    // Use shader
    glUseProgram(g_shader_program.get_program_id());

    // Set background
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // Load Spongebob and Patrick textures
    g_spongebob_texture_id = load_texture(SPONGEBOB_SPRITE_FILEPATH);
    g_patrick_texture_id = load_texture(PATRICK_SPRITE_FILEPATH);
    
    // Load background texture
    g_background_texture_id = load_texture("textures/BikiniBottom.png");

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Process user input
void process_input()
{
    // Handle no input
    g_spongebob_movement = glm::vec3(0.0f);

    SDL_Event event;
    
    // Poll for events
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // Window close = end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            
            // Key is pressed
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                                                                             
                    case SDLK_RIGHT:
                        // Move the player right
                        g_spongebob_movement.x = 1.0f;
                        break;
                                                                             
                    case SDLK_q:
                        // Quit the game
                        g_app_status = TERMINATED;
                        break;
                                                                             
                    default:
                        break;
                }
                                                                             
            default:
                break;
        }
    }
                                                                             
                                                                             
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
                          
    // Check for continuous key presses
    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_spongebob_movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_spongebob_movement.x = 1.0f;
    }
                                                                             
    if (key_state[SDL_SCANCODE_UP])
    {
        g_spongebob_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_spongebob_movement.y = -1.0f;
    }
                                                                             
    // Normalize movement vector to prevent faster diagonal movement (cheating)
    if (glm::length(g_spongebob_movement) > 1.0f)
    {
        g_spongebob_movement = glm::normalize(g_spongebob_movement);
    }
}

// Function to clamp position to screen borders
void clamp_to_borders(glm::vec3& position, float width, float height) {
    // Adjust to left border
    if (position.x < LEFT_BORDER + (width / 2.0f)) {
        position.x = LEFT_BORDER + (width / 2.0f);
    }
    // Adjust to right border
    else if (position.x > RIGHT_BORDER - (width / 2.0f)) {
        position.x = RIGHT_BORDER - (width / 2.0f);
    }
    // Adjust to bottom border
    if (position.y < BOTTOM_BORDER + (height / 2.0f)) {
        position.y = BOTTOM_BORDER + (height / 2.0f);
    }
    // Adjust to top border
    else if (position.y > TOP_BORDER - (height / 2.0f)) {
        position.y = TOP_BORDER - (height / 2.0f);
    }
}



void update() {
    // Update ticks
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // Update Spongebob's position
    g_spongebob_position += g_spongebob_movement * g_spongebob_speed * delta_time;

    // SpongeBob dimensions for collision detection
    float spongeBobWidth = INIT_SCALE_SPONGEBOB.x;
    float spongeBobHeight = INIT_SCALE_SPONGEBOB.y;

    // Keep SpongeBob within the window's borders
    clamp_to_borders(g_spongebob_position, spongeBobWidth, spongeBobHeight);

    // Update and check collisions with Jellyfish
    jellyfish.erase(std::remove_if(jellyfish.begin(), jellyfish.end(), [&](Entity& jelly) {
        jelly.update(delta_time);
        if (jelly.checkCollision(g_spongebob_position, glm::vec3(spongeBobWidth, spongeBobHeight, 0.0f))) {
            LOG("Jellyfish collision: +1 point");
            // Respawning doesn't work quite yet
            jelly.respawn(g_spongebob_position, spongeBobWidth, spongeBobHeight);
            return true;
        }
        // Handle no collision
        return false;
    }), jellyfish.end());

    // Update and check collisions with Manrays
    manrays.erase(std::remove_if(manrays.begin(), manrays.end(), [&](Entity& manray) {
        manray.update(delta_time);
        if (manray.checkCollision(g_spongebob_position, glm::vec3(spongeBobWidth, spongeBobHeight, 0.0f))) {
            LOG("Manray collision: -1 life");
            manray.respawn(g_spongebob_position, spongeBobWidth, spongeBobHeight);
            // Respawning doesn't work quite yet
            return true;
        }
        // Handle no collision again
        return false;
    }), manrays.end());

    // Spongebob's transformation matrix
    g_spongebob_matrix = glm::mat4(1.0f);
    g_spongebob_matrix = glm::translate(g_spongebob_matrix, g_spongebob_position);
    g_spongebob_matrix = glm::scale(g_spongebob_matrix, INIT_SCALE_SPONGEBOB);
    
    
    // Update Patrick's position and rotation
    g_patrick_position += g_patrick_movement * 5.0f * delta_time;

    // Randomly change Patrick's direction 1% of the time
    if (rand() % 100 == 0) {
            g_patrick_movement.x = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
            g_patrick_movement.y = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
            g_patrick_movement = glm::normalize(g_patrick_movement);
        }
    // Keep Patrick within borders
    clamp_to_borders(g_patrick_position, INIT_SCALE_PATRICK.x, INIT_SCALE_PATRICK.y);

    // Patrick's FAST rotation
    static float patrickRotationAngle = 0.0f;
    patrickRotationAngle += 360.0f * delta_time;
    
    // Patrick's random scaling
    static glm::vec3 patrickScale = INIT_SCALE_PATRICK;

    // 1% chance to change scale each frame
    if (rand() % 100 == 0) {
        // Randomly pick which size Patrick will be
        int scaleOption = rand() % 3;
        
        switch (scaleOption) {
            case 0: // Really small
                patrickScale = INIT_SCALE_PATRICK * 0.5f;
                break;
            case 1: // Normal size
                patrickScale = INIT_SCALE_PATRICK;
                break;
            case 2: // Really large
                patrickScale = INIT_SCALE_PATRICK * 3.0f;
                break;
        }
    }

    // Update Patrick's matrix with translation, rotation, and scaling
    g_patrick_matrix = glm::mat4(1.0f);
    g_patrick_matrix = glm::translate(g_patrick_matrix, g_patrick_position);
    g_patrick_matrix = glm::rotate(g_patrick_matrix, glm::radians(patrickRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    g_patrick_matrix = glm::scale(g_patrick_matrix, patrickScale);

}



void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    // Draw objects using 2 triangles
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Set character vertices
GLfloat front_objects_vertices[] = {
    -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,          // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
};

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


void render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set shader to identity matrix
    g_shader_program.set_model_matrix(glm::mat4(1.0f));

    // Render background
    glBindTexture(GL_TEXTURE_2D, g_background_texture_id);
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Set character vertex positions
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, front_objects_vertices);
    
    // Render SpongeBob
    draw_object(g_spongebob_matrix, g_spongebob_texture_id);

    // Render Jellyfish
    for (Entity &jelly : jellyfish) {
        jelly.draw(g_shader_program);
    }

    // Render Manrays
    for (Entity &manray : manrays) {
        manray.draw(g_shader_program);
    }
    
    // Render Patrick
    draw_object(g_patrick_matrix, g_patrick_texture_id);

    // Display rendering
    SDL_GL_SwapWindow(g_display_window);
}

// Exit SDL
void shutdown() { SDL_Quit(); }

// Main function
int main(int argc, char* argv[]) {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));
    // Initialize SDL etc
    initialize();
    // Initialize jellyfish and manrays
    initialize_entities();

    // Main loop
    while (g_app_status == RUNNING) {
        // Handle user input
        process_input();
        // Update game logic
        update();
        // Render current state
        render();
    }
    
    // Call shutdown to quit
    shutdown();
    // Exit successfully
    return 0;
}
