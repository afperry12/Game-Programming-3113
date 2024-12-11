/**
* Author: Peter Perry
* Assignment: Platformer
* Date due: 2024-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 4;
    bool regenerate_maze = true;
    std::vector<Entity*> water_drops;
    ~LevelA();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void shoot_water() override;
    // Setter and getter for pause state
        void set_paused(bool paused);
        bool is_paused() const;
    
private:
    bool m_is_paused = false;
    // Declare the safe spawn function here
    glm::vec3 find_safe_spawn(Map* map, Entity* enemies, int enemy_count);
    Mix_Chunk* shoot_sfx;
};
