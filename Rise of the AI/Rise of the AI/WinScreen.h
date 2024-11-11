#ifndef WINSCREEN_H
#define WINSCREEN_H

#include "Scene.h"
#include "Utility.h"

class WinScreen : public Scene {
public:
    ~WinScreen();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

private:
    GLuint win_screen_texture;
};

#endif
