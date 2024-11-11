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
    
private:
    // Declare the safe spawn function here
    glm::vec3 find_safe_spawn(Map* map, Entity* enemies, int enemy_count);
    
};
