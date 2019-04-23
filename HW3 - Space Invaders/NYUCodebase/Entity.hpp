#ifndef Entity_hpp
#define Entity_hpp
#include <stdio.h>
#include "ShaderProgram.h"
#include "SheetSprite.hpp"
#endif /* Entity_hpp */

class Entity {
public:
    Entity(float x, float y, float x_velocity, float y_velocity, float width, float height , float r, float g, float b, float u, float v , int textureID, float size);
    void Draw(ShaderProgram &p, float elapsed);
    void update(float elapsed);
    bool collision(Entity &e);
    float x, y;
    float rotation;
    int textureID;
    SheetSprite sprite;
    float width, height;
    float x_velocity, y_velocity;
    float r,g,b;
};
