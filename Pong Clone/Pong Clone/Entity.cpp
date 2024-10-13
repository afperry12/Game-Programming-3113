////
////  Entity.cpp
////  Pong Clone
////
////  Created by Peter Perry on 10/4/24.
////
//#include <SDL.h>
//#include <SDL_opengl.h>
//#include "glm/mat4x4.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//#include "ShaderProgram.h"
//#include "stb_image.h"
//#include "cmath"
//#include <ctime>
//
//// Class representing a character for standardization
//class Entity {
//public:
//    // Entity's attributes
//    glm::vec3 position;
//    glm::vec3 movement;
//    float speed;
//    GLuint textureID;
//    glm::vec3 scale;
//    
//    // Constructor for attributes
//    Entity(glm::vec3 pos, GLuint texID, glm::vec3 scl)
//        : position(pos), textureID(texID), scale(scl), speed(1.0f) {
//        movement.x = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
//        movement.y = ((rand() % 2 == 0) ? -1 : 1) * static_cast<float>(rand()) / RAND_MAX;
//        movement = glm::normalize(movement);
//    }
//    
//    // Update entity's position
//    void update(float deltaTime) {
//        position += movement * speed * deltaTime;
//        bounceOffBorders();
//    }
//
//    // Bounce off borders to prevent going off screen
//    void bounceOffBorders() {
//        if (position.x <= LEFT_BORDER + (scale.x / 2) || position.x >= RIGHT_BORDER - (scale.x / 2)) {
//            movement.x = -movement.x;
//        }
//        if (position.y <= BOTTOM_BORDER + (scale.y / 2) || position.y >= TOP_BORDER - (scale.y / 2)) {
//            movement.y = -movement.y;
//        }
//    }
//
//    // Draw entity on screen
//    void draw(ShaderProgram &program) {
//        glm::mat4 modelMatrix = glm::mat4(1.0f);
//        modelMatrix = glm::translate(modelMatrix, position);
//        modelMatrix = glm::scale(modelMatrix, scale);
//        program.set_model_matrix(modelMatrix);
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//    }
//
//    // Check for collision with another entity based on position and scale
//    bool checkCollision(const glm::vec3 &otherPos, const glm::vec3 &otherScale) {
//        float xDist = fabs(position.x - otherPos.x) - ((scale.x + otherScale.x) / 2.0f);
//        float yDist = fabs(position.y - otherPos.y) - ((scale.y + otherScale.y) / 2.0f);
//        return (xDist < 0 && yDist < 0);
//    }
//    
//    // Not currently functioning but should respawn entities not within a specification location
//    void respawn(const glm::vec3 &spongebobPosition, float spongeBobWidth, float spongeBobHeight) {
//        glm::vec3 newPosition;
//        do {
//            newPosition = glm::vec3(
//                -4.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 9.0f)),
//                -3.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 7.0f)),
//                0.0f
//            );
//        } while (fabs(newPosition.x - spongebobPosition.x) < spongeBobWidth &&
//                 fabs(newPosition.y - spongebobPosition.y) < spongeBobHeight);
//        position = newPosition;
//    }
//
//
//};
