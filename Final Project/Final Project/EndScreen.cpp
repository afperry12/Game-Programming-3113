/**
* Author: Peter Perry
* Assignment: Final Project
* Date due: 2024-12-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "EndScreen.h"
#include "Utility.h"

constexpr char ENDSCREEN_FILEPATH[]       = "assets/Mission_Failed.png";

// Destructor to clean up resources, if needed
EndScreen::~EndScreen() {

}

// Initialize end screen
void EndScreen::initialise() {
    // Prevent scene transition
    m_game_state.next_scene_id = -1;
    
    // Texture for losing screen
    end_screen_texture = Utility::load_texture(ENDSCREEN_FILEPATH);
    
    // Play background music
    Mix_Chunk* booing_sfx = Mix_LoadWAV("./assets/boos.wav");
    Mix_PlayChannel(-1, booing_sfx, 0);
}

void EndScreen::update(float delta_time) {
    // No updates needed
}

void EndScreen::render(ShaderProgram* program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    // Use correct dimensions to render screen
    float vertices[] = {
        -5.0f, -3.75f,
         5.0f, -3.75f,
         5.0f,  3.75f,
        -5.0f, -3.75f,
         5.0f,  3.75f,
        -5.0f,  3.75f
    };
    float tex_coords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    glBindTexture(GL_TEXTURE_2D, end_screen_texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
