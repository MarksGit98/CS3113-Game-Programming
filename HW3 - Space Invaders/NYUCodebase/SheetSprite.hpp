#ifndef SpriteSheet_hpp
#define SpriteSheet_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#endif /* SpriteSheet_hpp */

class SheetSprite {
public:
    SheetSprite(unsigned int textureID = 0, float u = 5, float v = 5, float width = 5, float height = 5, float size = 5): textureID(textureID), u(u), v(v), width(width), height(height), size(size){}
    void Draw(ShaderProgram &program, float x, float y) const;
    float size;
    unsigned int textureID;
    float u, v;
    float width, height;
};
