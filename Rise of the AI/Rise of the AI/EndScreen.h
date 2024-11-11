#ifndef ENDSCREEN_H
#define ENDSCREEN_H

#include "Scene.h"
#include "Utility.h"

// Losing Screen
class EndScreen : public Scene {
public:
    // Clean up resources
    ~EndScreen();
    
    // Functions included in endscreen
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

private:
    // Texture for end screen graphic
    GLuint end_screen_texture;
};

#endif
