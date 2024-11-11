/**
* Author: Peter Perry
* Assignment: Rise of the AI
* Date due: 2024-11-09, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelA.h"
#include "Utility.h"
#include "MazeGenerator.h"
// For maze generation
#define LEVEL_WIDTH 51
#define LEVEL_HEIGHT 51

constexpr char SPRITESHEET_FILEPATH[] = "assets/player.png";
constexpr char ENEMY_FILEPATH[]       = "assets/fireball.png";
constexpr char VILLAIN_FILEPATH[]       = "assets/villain.png";

// Track regenerate maze & villian jump data
bool regenerate_maze = true;
int villain_jump_count = 0;
float villain_jump_timer = 3.0f;

LevelA::~LevelA() {
    delete[] m_game_state.enemies;
    delete m_game_state.player;
    delete m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

glm::vec3 LevelA::find_safe_spawn(Map* map, Entity* enemies, int enemy_count) {
    unsigned int* level_data = map->get_level_data();
    int width = map->get_width();
    int height = map->get_height();

    // Find a unconvered block (tile == 1), spawn the enemy above it
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned int tile = level_data[y * width + x];
            if (tile == 1) {
                // Return the position directly above this tile
                return glm::vec3(x, -(y - 1), 0.0f);
            }
        }
    }

    // Fallback if no uncovered block found
    return glm::vec3(0.9f, -1.0f, 0.0f);
}

void LevelA::shoot_water() {
    // Load the water drop texture
    GLuint water_drop_texture = Utility::load_texture("./assets/water_drop.png");
    
    float water_drop_width = 0.2f;
    float water_drop_height = 0.2f;
    
    // Create a new water drop entity
    Entity* water_drop = new Entity(water_drop_texture, 5.0f, water_drop_width, water_drop_height, AMMO);
    
    water_drop->set_scale(glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Set water drop's position to the player's current position
    water_drop->set_position(m_game_state.player->get_position());
    
    // Set water drop to move to the right
    water_drop->set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Add water drop to the list of active water drops
    water_drops.push_back(water_drop);
}

void LevelA::initialise() {
    m_game_state.next_scene_id = -1;

    unsigned int* generated_level_data;
    // Generate maze dynamically
    if (regenerate_maze) {
        MazeGenerator generator(LEVEL_WIDTH, LEVEL_HEIGHT);
        generator.generate_maze();
        generated_level_data = generator.convert_to_level_data();

        // Load maze into the map
        GLuint map_texture_id = Utility::load_texture("assets/tileset2.png");
        m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, generated_level_data, map_texture_id, 1.0f, 4, 1);

        regenerate_maze = false;  // Reset flag after generating the new maze
    }

    // Initialize player walking animation
    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },
        { 3, 7, 11, 15 },
        { 2, 6, 10, 14 },
        { 0, 4, 8, 12 }
    };
    
    // Initialize the player
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    float scale_factor = 0.2f;
    
    float scaled_speed = 5.0f * scale_factor;
    float scaled_jump_power = 5.0f * scale_factor;
    
    m_game_state.player = new Entity(
        player_texture_id,        // Texture
        scaled_speed,             // Speed
        glm::vec3(0.0f, 0.0f, 0.0f), // Acceleration
        scaled_jump_power,        // Jumping power
        player_walking_animation, // Animation
        0.0f,                     // Animation time
        1,                        // Animation frames
        0,                        // Current animation index
        1,                        // Animation columns
        1,                        // Animation rows
        scale_factor,             // Width
        scale_factor,             // Height
        PLAYER                    // Entity type
    );

    // Spawn player safely
    glm::vec3 safe_spawn = glm::vec3(0.9f, -1.0f, 0.0f);
    m_game_state.player->set_position(safe_spawn);
    m_game_state.enemies = new Entity[ENEMY_COUNT];

    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    GLuint villain_texture_id = Utility::load_texture(VILLAIN_FILEPATH);
    GLuint jumping_enemy_texture_id = Utility::load_texture("./assets/evil_fishy.png");
    
//    glm::vec3 jumping_enemy_spawn = find_safe_spawn(m_game_state.map, m_game_state.enemies, ENEMY_COUNT);
    
    // Handle creating enemies and adding their relevant data
    
    for (int i = 0; i < ENEMY_COUNT-1; i++)
    {
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, WALKER, WALKING);
    }
    
    m_game_state.enemies[ENEMY_COUNT-1] =  Entity(villain_texture_id, 0.0f, 1.0f, 1.0f, ENEMY, VILLAIN, JUMPING);
    m_game_state.enemies[ENEMY_COUNT - 2] =  Entity(jumping_enemy_texture_id, 0.0f, 1.0f, 1.0f, ENEMY, GUARD, JUMPING);

    m_game_state.enemies[0].set_position(glm::vec3(4.0f, -1.0f, 0.0f));
    m_game_state.enemies[0].set_original_position(glm::vec3(4.0f, -1.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(-0.5f, 0.0f ,0.0f));
    
    m_game_state.enemies[1].set_position(glm::vec3(4.0f, -2.5f, 0.0f));
    m_game_state.enemies[1].set_original_position(glm::vec3(4.0f, -2.5f, 0.0f));
    m_game_state.enemies[1].set_movement(glm::vec3(-2.0f, 0.0f ,0.0f));
    
    m_game_state.enemies[ENEMY_COUNT-1].set_position(glm::vec3(5.0f, -3.5f, 0.0f));
    m_game_state.enemies[ENEMY_COUNT-1].set_movement(glm::vec3(0.0f, -0.5f, 0.0f));
    m_game_state.enemies[ENEMY_COUNT-1].set_jumping_power(5.0f);
    
    m_game_state.enemies[ENEMY_COUNT - 2].set_position(glm::vec3(3.0f, -3.0f, 0.0f));
    m_game_state.enemies[ENEMY_COUNT - 2].set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.enemies[ENEMY_COUNT - 2].set_jumping_power(55.0f);

    // Load background music and sound effects
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/Maze_of_the_Mind.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(50.0f);
}

void LevelA::update(float delta_time) {
    if(m_game_state.next_scene_id >= 1 || m_game_state.player == nullptr) {
        return;
    }
    
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    // Check if the player has won by falling below y = -7.0f (exiting maze)
    if (m_game_state.player->get_position().y < -7.0f) {
        // Player won, transition to WinScreen
        regenerate_maze = true;
        m_game_state.next_scene_id = 3;
        return;
    }
    
    for (int i = 0; i < ENEMY_COUNT - 1; i++) {
        if (m_game_state.enemies[i].is_active()) {
            m_game_state.enemies[i].update(delta_time, &m_game_state.enemies[i], m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
            // Check if the enemy is off the map
            if (m_game_state.enemies[i].get_position().x < m_game_state.map->get_left_bound() ||
                m_game_state.enemies[i].get_position().x > m_game_state.map->get_right_bound() ||
                m_game_state.enemies[i].get_position().y < m_game_state.map->get_bottom_bound() ||
                m_game_state.enemies[i].get_position().y > m_game_state.map->get_top_bound())
            {
                // Respawn enemy at its original position
                m_game_state.enemies[i].reset_position();
                m_game_state.enemies[i].set_movement(glm::vec3(-0.5f, 0.0f, 0.0f));
            }
            
            // Check for collision between player and enemy
            if (m_game_state.player->check_collision(&m_game_state.enemies[i])) {
                // Collision, transition to lose screen
                regenerate_maze = false;
                m_game_state.next_scene_id = 2;
                return;
            }
        }
    }
    
    m_game_state.enemies[ENEMY_COUNT - 1].update(delta_time, &m_game_state.enemies[ENEMY_COUNT-1], m_game_state.enemies, 1, m_game_state.map);
    // Check for villain jump count
    if (m_game_state.enemies[ENEMY_COUNT - 1].get_jump_count() >= 3 && m_game_state.enemies[ENEMY_COUNT - 1].is_active()) {
            // Villain has jumped 3 times, the player loses
            m_game_state.next_scene_id = 2;
            return;
        }
    
    m_game_state.enemies[ENEMY_COUNT - 2].update(delta_time, &m_game_state.enemies[ENEMY_COUNT-2], m_game_state.enemies, 1, m_game_state.map);
    
    // Update and check water drops
    for (int i = 0; i < water_drops.size(); ) {
        water_drops[i]->update(delta_time, water_drops[i], m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
        water_drops[i]->set_scale(glm::vec3(0.2f, 0.2f, 0.2f));
        // Check for collisions between the water drop and enemies
        bool remove_drop = false;
        for (int j = 0; j < ENEMY_COUNT; j++) {
            if (water_drops[i]->check_collision(&m_game_state.enemies[j])) {
                std::cout << "Water drop " << i << " collided with enemy " << j << std::endl;

                // Deactivate enemy
                m_game_state.enemies[j].deactivate();
                
                // Terminate water drop
                delete water_drops[i];
                water_drops.erase(water_drops.begin() + i);
                remove_drop = true;
                break;
            }
        }

        // If the water drop goes off-screen, remove
        if (!remove_drop && water_drops[i]->get_position().x > 10.0f) {
            std::cout << "Water drop " << i << " went off-screen" << std::endl;
            delete water_drops[i];
            water_drops.erase(water_drops.begin() + i);
            remove_drop = true;
        }

        if (!remove_drop) {
            // Increment if drop not removed
            i++;
        }
    }
    
    
    
}

void LevelA::render(ShaderProgram *program) {
    if(m_game_state.next_scene_id >= 1) {
        return;
    }
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (m_game_state.enemies[i].is_active()) {
            m_game_state.enemies[i].render(program);
        }
    }
    for (Entity* water_drop : water_drops) {
        if(water_drop->is_active()) {
            water_drop->render(program);
        }
    }
}

