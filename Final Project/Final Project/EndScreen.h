/**
* Author: Peter Perry
* Assignment: Final Project
* Date due: 2024-12-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

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
