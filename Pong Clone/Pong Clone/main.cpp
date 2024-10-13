/**
 * Author: Peter Perry
 * Assignment: Pong
 * Date due: 2024-10-12, 11:59pm
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
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum AppStatus { START, RUNNING, ENDSCREEN, TERMINATED };


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

// Texture loading constants
constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;

// Window border limits
constexpr float LEFT_BORDER = -5.0f;
constexpr float RIGHT_BORDER = 5.0f;
constexpr float BOTTOM_BORDER = -3.75f;
constexpr float TOP_BORDER = 3.75f;

// Vertex and fragment shader program paths
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// Constant for ms
constexpr float MILLISECONDS_IN_SECOND = 1000.0;

// File paths for character images
constexpr char SPONGEBOB_SPRITE_FILEPATH[] = "textures/Spongebob1.PNG",
PATRICK_SPRITE_FILEPATH[]  = "textures/Patrick.png";

// Constant distance for collision detection
constexpr float MINIMUM_COLLISION_DISTANCE = 0.5f;

// Height for scaling characters
float targetHeight = 1.5f;

// Define pixel ratios
float aspectRatioSpongebob = 308.0f / 382.0f;
float aspectRatioPatrick = 310.0f / 473.0f;

// Variable to store the number of jellyfish to spawn
int numJellyfish = 0;

// Keep track of player scores
int patrick_score = 0;
int spongebob_score = 0;

// Keep track of automated patrick bools
bool isSinglePlayerMode = false;
bool isGoingUp = true;

// Scale characters based on pixel ratios
glm::vec3 INIT_SCALE_SPONGEBOB = glm::vec3(targetHeight * aspectRatioSpongebob, targetHeight, 0.0f);
glm::vec3 INIT_SCALE_PATRICK = glm::vec3(targetHeight * aspectRatioPatrick, targetHeight, 0.0f);

// Set starting positions of characters
glm::vec3 INIT_POS_SPONGEBOB = glm::vec3(LEFT_BORDER + (INIT_SCALE_SPONGEBOB.x / 2.0f), 0.0f, 0.0f);
glm::vec3 INIT_POS_PATRICK = glm::vec3(RIGHT_BORDER - (INIT_SCALE_PATRICK.x / 2.0f), 0.0f, 0.0f);

// Variables for window, app status, shader program
SDL_Window* g_display_window;
SDL_Renderer *renderer;
TTF_Font *font;
AppStatus g_app_status = START;
ShaderProgram g_shader_program = ShaderProgram();

// Transformation matrices for view, model, and projection
glm::mat4 g_view_matrix, g_spongebob_matrix, g_projection_matrix, g_patrick_matrix;

// Track ticks
float g_previous_ticks = 0.0f;

// Define texture IDs
GLuint g_background_texture_id;
GLuint g_spongebob_texture_id;
GLuint g_patrick_texture_id;
GLuint g_start_screen_texture_id;


// Position and movements vectors
glm::vec3 g_spongebob_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_spongebob_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_movement = glm::vec3(0.0f, 0.0f, 0.0f);

// Set player speeds
float g_spongebob_speed = 1.0f;
float g_patrick_speed = 1.0f;

// Speed constants for different difficulty levels (hard is best)
constexpr float EASY_SPEED = 1.0f;
constexpr float MEDIUM_SPEED = 5.0f;
constexpr float HARD_SPEED = 10.0f;

// Main function calls
void initialize();
void process_input();
void update();
void render();
void shutdown();

// Entity class
class Entity {
public:
    glm::vec3 position;
    glm::vec3 movement;
    float speed;
    GLuint textureID;
    glm::vec3 scale;
    glm::mat4 matrix;
    
    // Constructor
    Entity(glm::vec3 pos, GLuint texID, glm::vec3 scl)
    : position(pos), textureID(texID), scale(scl), speed(1.0f), matrix(glm::mat4(1.0f)) {
        movement.x = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
        movement.y = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
        movement = glm::normalize(movement);
    }
    
    // Update entity's position and transformation matrix
    void update(float deltaTime) {
        position += movement * speed * deltaTime;
        bounceOffBorders();
        updateMatrix();
    }
    
    // Update the transformation matrix based on position and scale
    void updateMatrix() {
        matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = glm::scale(matrix, scale);
    }
    
    // Bounce off borders
    void bounceOffBorders() {
        if (position.x <= LEFT_BORDER + (scale.x / 2.0f) || position.x >= RIGHT_BORDER - (scale.x / 2.0f)) {
            movement.x = -movement.x; // Reverse horizontal direction
        }
        if (position.y <= BOTTOM_BORDER + (scale.y / 2.0f) || position.y >= TOP_BORDER - (scale.y / 2.0f)) {
            movement.y = -movement.y; // Reverse vertical direction
        }
    }
    
    // Draw entity using the stored matrix
    void draw(ShaderProgram &program) {
        program.set_model_matrix(matrix); // Use the updated matrix
        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    // Collision check using rectangular bounds (AABB)
    bool checkCollision(const glm::vec3 &otherPos, const glm::vec3 &otherScale) {
        // Collision detection
        float xDist = fabs(position.x - otherPos.x) - ((scale.x + otherScale.x) / 2.0f);
        float yDist = fabs(position.y - otherPos.y) - ((scale.y + otherScale.y) / 2.0f);
        
        // If distances are negative, it means the entities are colliding
        return (xDist < 0 && yDist < 0);
    }
    
};

// Handle jellyfish collisions with pong paddles (Spongebob and Patrick)
void handleJellyfishPaddleCollisions(Entity &jellyfish, const glm::vec3 &paddlePosition, const glm::vec3 &paddleScale) {
    float jellyWidth = jellyfish.scale.x;
    float jellyHeight = jellyfish.scale.y;
    float paddleWidth = paddleScale.x;
    float paddleHeight = paddleScale.y;
    
    // Check for collision
    if (jellyfish.checkCollision(paddlePosition, paddleScale)) {
        
        // Calculate relative impact point on paddle
        float collisionPoint = (jellyfish.position.y - paddlePosition.y) / (paddleHeight / 2.0f);
        
        // Reflect jellyfish horizontally
        jellyfish.movement.x = -jellyfish.movement.x;
        
        // Modify the vertical direction based on where it hit the paddle
        jellyfish.movement.y += collisionPoint * 0.5f;
        
        // Normalize the movement
        jellyfish.movement = glm::normalize(jellyfish.movement);
    }
}

// Vectors to store multiple entities for jellyfish
std::vector<Entity> jellyfish;
GLuint jellyfish_texture_id;

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

// Function to reset back to start screen
void reset_game() {
    // Destroy renderer and window to reinitialize from beginning
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_display_window);
    initialize();
    
    g_app_status = START;
    
    // Erase all jellyfish to prevent scoring on reset
    for (std::vector<Entity>::iterator it = jellyfish.begin(); it != jellyfish.end(); ) {
        it = jellyfish.erase(it);
    }
    
    // Reset player scores
    patrick_score = 0;
    spongebob_score = 0;
    
    // Reset player positions
    g_spongebob_position = INIT_POS_SPONGEBOB;
    g_patrick_position = INIT_POS_PATRICK;
    
    // Reset player movements
    g_spongebob_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    g_patrick_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Reset timers
    g_previous_ticks = 0.0f;
    
    // Reset speeds for difficulty selection
    g_spongebob_speed = EASY_SPEED;
    g_patrick_speed = EASY_SPEED;
}


// Function to initialize jellyfish entities with random speeds and directions
void initialize_entities() {
    // Load textures for jellyfish
    jellyfish_texture_id = load_texture("textures/Jellyfish.PNG");
    
    // Clear the previous jellyfish entities
    jellyfish.clear();
    
    // Center position for all jellyfish
    glm::vec3 centerPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Spawn jellyfish with random directions and speeds
    for (int i = 0; i < numJellyfish; ++i) {
        // Create a new jellyfish at the center
        Entity jelly(centerPosition, jellyfish_texture_id, glm::vec3(1.0f, 1.0f, 0.0f));
        
        if (g_spongebob_speed == EASY_SPEED) {
            jelly.speed = EASY_SPEED;
        } else if (g_spongebob_speed == MEDIUM_SPEED) {
            jelly.speed = MEDIUM_SPEED;
        } else if (g_spongebob_speed == HARD_SPEED) {
            jelly.speed = HARD_SPEED;
        }
        
        // Normalize the movement vector to ensure jellyfish move in a random direction
        jelly.movement = glm::normalize(jelly.movement);
        
        // Add the jellyfish to the jellyfish vector
        jellyfish.push_back(jelly);
    }
}


void initialize() {
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
    
    // Load textures for Spongebob, Patrick, and background
    g_spongebob_texture_id = load_texture(SPONGEBOB_SPRITE_FILEPATH);
    g_patrick_texture_id = load_texture(PATRICK_SPRITE_FILEPATH);
    g_background_texture_id = load_texture("textures/BikiniBottom.png");
    g_start_screen_texture_id = load_texture("textures/StartScreen.PNG");
    
    // Load font for endscreen
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(-1);
    }
    font = TTF_OpenFont("fonts/Chrome-Normal.ttf", 50);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        exit(-1);
    }
    
    // Create renderer for SDL2 rendering of font
    renderer = SDL_CreateRenderer(g_display_window, -1, SDL_RENDERER_ACCELERATED);
    
    // Initialize positions
    g_spongebob_position = INIT_POS_SPONGEBOB;  // Initialize Spongebob's position
    g_patrick_position = INIT_POS_PATRICK;      // Initialize Patrick's position
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize entities like jellyfish
    initialize_entities();
}


// Process user input
void process_input() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                
                g_app_status = TERMINATED;
                break;
                
                // Handle various key presses
            case SDL_KEYDOWN:
                if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9 && g_app_status == RUNNING) {
                                    numJellyfish = event.key.keysym.sym - SDLK_1 + 1;
                                    initialize_entities();
                }
                // Easter egg shhhh
                if (event.key.keysym.sym == SDLK_c && g_app_status == RUNNING) {
                                    numJellyfish = event.key.keysym.sym;
                                    initialize_entities();
                }
                if (event.key.keysym.sym == SDLK_e) {
                    g_spongebob_speed = EASY_SPEED;
                    g_patrick_speed = EASY_SPEED;
                    g_app_status = RUNNING;
                } else if (event.key.keysym.sym == SDLK_m) {
                    g_spongebob_speed = MEDIUM_SPEED;
                    g_patrick_speed = MEDIUM_SPEED;
                    g_app_status = RUNNING;
                }  else if (event.key.keysym.sym == SDLK_h) {
                    g_spongebob_speed = HARD_SPEED;
                    g_patrick_speed = HARD_SPEED;
                    g_app_status = RUNNING;
                } else if (event.key.keysym.sym == SDLK_t) {
                    isSinglePlayerMode = !isSinglePlayerMode;
                }
                if (event.key.keysym.sym == SDLK_q && g_app_status == RUNNING) {
                    g_app_status = ENDSCREEN;
                }
                if (event.key.keysym.sym == SDLK_r) {
                    reset_game();
                }
                break;
            default:
                break;
                
        }
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    g_spongebob_movement.y = 0.0f;
    g_patrick_movement.y = 0.0f;
    
    // SpongeBob controls
    if (key_state[SDL_SCANCODE_W]) {
        g_spongebob_movement.y = 1.0f;
    } else if (key_state[SDL_SCANCODE_S]) {
        g_spongebob_movement.y = -1.0f;
    }
    
    // Patrick controls
    if (key_state[SDL_SCANCODE_UP]) {
        g_patrick_movement.y = 1.0f;
    } else if (key_state[SDL_SCANCODE_DOWN]) {
        g_patrick_movement.y = -1.0f;
    }
    
    // Normalize movement to prevent faster diagonal movement
    if (glm::length(g_spongebob_movement) > 1.0f)
    {
        g_spongebob_movement = glm::normalize(g_spongebob_movement);
    }
    if (glm::length(g_patrick_movement) > 1.0f)
    {
        g_patrick_movement = glm::normalize(g_patrick_movement);
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
    if (g_app_status == RUNNING) {
        
        // Update ticks
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        // Update SpongeBob's position
        g_spongebob_position += g_spongebob_movement * g_spongebob_speed * delta_time;
        
        // SpongeBob dimensions for collision detection
        float spongeBobWidth = INIT_SCALE_SPONGEBOB.x;
        float spongeBobHeight = INIT_SCALE_SPONGEBOB.y;
        
        // Patrick dimensions for collision detection
        float patrickWidth = INIT_SCALE_PATRICK.x;
        float patrickHeight = INIT_SCALE_PATRICK.y;
        
        // Keep SpongeBob within the window's borders
        clamp_to_borders(g_spongebob_position, spongeBobWidth, spongeBobHeight);
        
        // To store jellyfish that should be erased
        std::vector<size_t> jellyfishToErase;
        
        // Loop through all jellyfish
        for (size_t i = 0; i < jellyfish.size(); ++i) {
            Entity &jelly = jellyfish[i];
            
            // Update jellyfish position based on movement
            jelly.update(delta_time);
            
            // Check for collision with SpongeBob
            if (jelly.checkCollision(g_spongebob_position, glm::vec3(spongeBobWidth, spongeBobHeight, 0.0f))) {
                LOG("Jellyfish collision with SpongeBob: +1 point");
                // Handle scoring logic here if needed
            }
            
            // Check for collision with Patrick
            if (jelly.checkCollision(g_patrick_position, glm::vec3(patrickWidth, patrickHeight, 0.0f))) {
                LOG("Jellyfish collision with Patrick: +1 point");
                // Handle scoring logic here if needed
            }
            
            // Check if jellyfish hits the left or right border
            if (jelly.position.x <= LEFT_BORDER + (jelly.scale.x / 2.0f)) {
                // Patrick scores
                patrick_score++;
                LOG("Patrick's score: " << patrick_score);
                jellyfishToErase.push_back(i);  // Mark this jellyfish for removal
                // Just in case for requirement 4
                if (patrick_score >= 100) {
                    g_app_status = ENDSCREEN;
                }
            }
            else if (jelly.position.x >= RIGHT_BORDER - (jelly.scale.x / 2.0f)) {
                // SpongeBob scores
                spongebob_score++;
                LOG("SpongeBob's score: " << spongebob_score);
                jellyfishToErase.push_back(i);  // Mark this jellyfish for removal
                // Just in case for requirement 4
                if (spongebob_score >= 100) {
                    g_app_status = ENDSCREEN;
                }
            }
            
            // Handle jellyfish bouncing off SpongeBob and Patrick
            handleJellyfishPaddleCollisions(jelly, g_spongebob_position, INIT_SCALE_SPONGEBOB);
            handleJellyfishPaddleCollisions(jelly, g_patrick_position, INIT_SCALE_PATRICK);
        }
        
        // Remove jellyfish
        for (size_t i = jellyfishToErase.size(); i > 0; --i) {
            jellyfish.erase(jellyfish.begin() + jellyfishToErase[i - 1]);
        }
        
        // Update SpongeBob's transformation matrix
        g_spongebob_matrix = glm::mat4(1.0f);
        g_spongebob_matrix = glm::translate(g_spongebob_matrix, g_spongebob_position);
        g_spongebob_matrix = glm::scale(g_spongebob_matrix, INIT_SCALE_SPONGEBOB);
        
        // Update Patrick's position and movement
        if (isSinglePlayerMode) {
            
            // Simple up-and-down motion
            if (isGoingUp) {
                g_patrick_position.y += g_patrick_speed * delta_time;
            } else {
                g_patrick_position.y -= g_patrick_speed * delta_time;
            }
            
            // When Patrick hits wall, switch directions
            if (g_patrick_position.y >= TOP_BORDER - (INIT_SCALE_PATRICK.y / 2.0f)) {
                isGoingUp = false;
            } else if (g_patrick_position.y <= BOTTOM_BORDER + (INIT_SCALE_PATRICK.y / 2.0f)) {
                isGoingUp = true;
            }
            
            
        } else {
            // Player controls patrick normally
            g_patrick_position += g_patrick_movement * g_patrick_speed * delta_time;
        }
        
        // Keep Patrick within the window's borders
        clamp_to_borders(g_patrick_position, patrickWidth, patrickHeight);
        
        // Update Patrick's transformation matrix
        g_patrick_matrix = glm::mat4(1.0f);
        g_patrick_matrix = glm::translate(g_patrick_matrix, g_patrick_position);
        g_patrick_matrix = glm::scale(g_patrick_matrix, INIT_SCALE_PATRICK);
    }
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

// Function to handle winner/tie message on endscreen
std::string get_winner_message() {
    if (spongebob_score > patrick_score) {
        return "SpongeBob Won: " + std::to_string(spongebob_score) + " to " + std::to_string(patrick_score);
    } else if (patrick_score > spongebob_score) {
        return "Patrick Won: " + std::to_string(patrick_score) + " to " + std::to_string(spongebob_score);
    } else {
        return "It is a Tie!";
    }
}

// Function to handle creating a texture from text (using SDL2_TTF which I tried for hours to integrate with OpenGL for scoreboard and sadly couldn't I think because shader?)
std::pair<SDL_Texture*, SDL_Rect> createTextureFromText(const char* text, SDL_Color color) {
    
    // Create surface from text
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    
    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    // Define rectangle to hold text's dimensions
    SDL_Rect textRect;
    textRect.w = surface->w;
    textRect.h = surface->h;
    
    // Free surface
    SDL_FreeSurface(surface);
    return {texture, textRect};
}



void render() {
    if (g_app_status == START) {
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Reset the shader program to identity matrix
        g_shader_program.set_model_matrix(glm::mat4(1.0f));
        
        // Render the start screen background
        glBindTexture(GL_TEXTURE_2D, g_start_screen_texture_id);
        glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
        glEnableVertexAttribArray(g_shader_program.get_position_attribute());
        glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
        glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(g_display_window);
        
    } else if (g_app_status == RUNNING) {
        
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        g_shader_program.set_model_matrix(glm::mat4(1.0f));
        
        // Render background first
        glBindTexture(GL_TEXTURE_2D, g_background_texture_id);
        glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
        glEnableVertexAttribArray(g_shader_program.get_position_attribute());
        glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_texture_coordinates);
        glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, front_objects_vertices);
        
        // Render SpongeBob
        draw_object(g_spongebob_matrix, g_spongebob_texture_id);
        
        // Render Jellyfish
        for (Entity &jelly : jellyfish) {
            jelly.draw(g_shader_program);
        }
        
        // Render Patrick
        draw_object(g_patrick_matrix, g_patrick_texture_id);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        SDL_GL_SwapWindow(g_display_window);
        
    } else if (g_app_status == ENDSCREEN) {
        
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        g_shader_program.set_model_matrix(glm::mat4(1.0f));
        
        // Display the winner's message
        std::string winnerMessage = get_winner_message();
        // White text on black background
        SDL_Color textColor = {255, 255, 255, 255};
        
        // Create texture from winner's message
        std::pair<SDL_Texture*, SDL_Rect> textureData = createTextureFromText(winnerMessage.c_str(), textColor);
        SDL_Texture* textTexture = textureData.first;
        SDL_Rect textRect = textureData.second;
        
        if (textTexture) {
            
            // Center text
            textRect.x = (VIEWPORT_WIDTH - textRect.w) / 2;
            textRect.y = (VIEWPORT_HEIGHT - textRect.h) / 2;
            
            // Render text
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            
            // Destroy texture (no longer needed since text is rendered)
            SDL_DestroyTexture(textTexture);
        }
        
        SDL_RenderPresent(renderer);
        SDL_GL_SwapWindow(g_display_window);
        
    }
}

// Exit SDL
void shutdown() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_display_window);
    TTF_Quit();
    SDL_Quit();
}

// Main function
int main(int argc, char* argv[]) {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));
    // Initialize SDL etc
    initialize();
    // Initialize jellyfish
    initialize_entities();
    
    // Main loop
    while (g_app_status != TERMINATED) {
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
