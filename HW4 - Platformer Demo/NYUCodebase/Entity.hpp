#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#include "SheetSprite.hpp"

#endif /* Entity_hpp */
struct vec2 {
    vec2(float x, float y): x(x), y(y){}
    float x;
    float y;
};
class Entity {
    
public:
    Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r, float g, float b, float u, float v , int textureID, float size);
    void Draw(ShaderProgram &p, float elapsed);
    void update(float elapsed);
    bool collision(Entity &e);
    bool collidedBottom = false;
    vec2 position;
    float rotation;
    int textureID;
    SheetSprite sprite;
    float width, height;
    vec2 velocity;
    float r,g,b;
};

