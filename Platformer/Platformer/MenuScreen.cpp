/**
* Author: Peter Perry
* Assignment: Platformer
* Date due: 2024-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "MenuScreen.h"
#include "Utility.h"

void MenuScreen::initialise() {
    // Initialize game state (menu doesn't need map, player, or enemies)
    m_game_state.map = nullptr;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
    m_game_state.bgm = nullptr;
    m_game_state.jump_sfx = nullptr;
    m_game_state.next_scene_id = -1;

    // Load background music for the menu (optional)
//    m_game_state.bgm = Mix_LoadMUS("menu_bgm.mp3");
//    if (m_game_state.bgm) {
//        Mix_PlayMusic(m_game_state.bgm, -1);
//    }
}

void MenuScreen::update(float delta_time) {
    // No updates are needed for the menu unless you add animations or effects
}

void MenuScreen::render(ShaderProgram *program) {
    // Render menu options
    Utility::draw_text(program, Utility::load_texture("./assets/font1.png"), "PRESS ENTER TO START",
                       0.5f, -0.25f, glm::vec3(-3.0f, 1.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("./assets/font1.png"), "PRESS Q TO QUIT",
                       0.5f, -0.25f, glm::vec3(-3.0f, 0.0f, 0.0f));
}
