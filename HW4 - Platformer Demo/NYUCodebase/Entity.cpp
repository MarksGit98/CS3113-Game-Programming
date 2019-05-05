#include "Entity.hpp"
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

void Entity::Draw(ShaderProgram &p, float elapsed){
    if(sprite.u == -4){
        float vertices[] = {position.x,position.y,position.x+width,position.y,position.x+width,position.y+height, position.x,position.y,position.x+width,position.y+height,position.x,position.y+height};
        p.SetColor(r, g, b, 1);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        p.SetModelMatrix(modelMatrix);
        glUseProgram(p.programID);
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    else{
        sprite.Draw(p, position.x, position.y);
    }
}
Entity::Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r=1, float g=1, float b=1, float u=-4, float v=-4, int textureID=-4, float size=-4): position(x,y), velocity(velocity_x, velocity_y), width(width), height(height), r(r), g(g), b(b), sprite(SheetSprite(textureID, u, v, width, height, size)){
}
void Entity::update(float elapsed){
    position.x += velocity.x *elapsed;
    position.y += velocity.y * elapsed;
}
bool Entity::collision(Entity &e){
    float x_distance = abs(e.position.x-position.x)-((e.width+width));
    float y_distance = abs(e.position.y-position.y)-((e.height+height));
    if(x_distance< 0 && y_distance < 0){
        return true;
    }
    return false;
}



