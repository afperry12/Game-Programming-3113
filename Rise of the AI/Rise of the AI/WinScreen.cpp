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
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/crowd-cheering.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(50.0f);
}

void WinScreen::update(float delta_time) {
    // No updates needed...
}

void WinScreen::render(ShaderProgram* program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    // Render end screen
    float vertices[] = { 0.0f, -8.0f, 10.0f, -8.0f, 10.0f, 3.0f, 0.0f, -8.0f, 10.0f, 3.0f, 0.0f, 3.0f };
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
