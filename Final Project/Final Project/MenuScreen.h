/**
* Author: Peter Perry
* Assignment: Final Project
* Date due: 2024-12-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#pragma once
#include "Scene.h"

class MenuScreen : public Scene {
public:
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
