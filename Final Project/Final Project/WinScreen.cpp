/**
* Author: Peter Perry
* Assignment: Platformer
* Date due: 2024-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "WinScreen.h"
#include "Utility.h"

constexpr char WINSCREEN_FILEPATH[]       = "assets/Mission_Accomplished.png";


WinScreen::~WinScreen() {
    // Next level
    delete[] m_game_state.enemies;
    delete m_game_state.player;
    delete m_game_state.map;
}

void WinScreen::initialise() {
    // Load the end screen texture
    m_game_state.next_scene_id = -1;
    win_screen_texture = Utility::load_texture(WINSCREEN_FILEPATH);
    Mix_Chunk* cheering_sfx = Mix_LoadWAV("assets/crowd-cheering.mp3");
    Mix_PlayChannel(-1, cheering_sfx, 0);
}

void WinScreen::update(float delta_time) {
    // No updates needed...
}

void WinScreen::render(ShaderProgram* program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    // Render end screen
    float vertices[] = {
        -5.0f, -3.75f,
         5.0f, -3.75f,
         5.0f,  3.75f,
        -5.0f, -3.75f,
         5.0f,  3.75f,
        -5.0f,  3.75f
    };
    float tex_coords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    glBindTexture(GL_TEXTURE_2D, win_screen_texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
