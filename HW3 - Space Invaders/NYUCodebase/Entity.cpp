#include "Entity.hpp"
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

void Entity::Draw(ShaderProgram &p, float elapsed){
    if(sprite.u == -4){
        std::cout << "Untextered";
        float vertices[] = {x,y,x+width,y,x+width,y+height, x,y,x+width,y+height,x,y+height};
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
            sprite.Draw(p, x, y);
        }
    }
Entity::Entity(float x, float y, float x_velocity, float y_velocity, float width, float height , float r=1, float g=1, float b=1, float u=-5, float v=-5, int textureID=0, float size=-5): x(x), y(y), x_velocity(x_velocity), y_velocity(y_velocity), width(width), height(height), r(r), g(g), b(b), sprite(SheetSprite(textureID, u, v, width, height, size)){}
void Entity::update(float elapsed){
    x+=x_velocity *elapsed;
    y+=y_velocity *elapsed;
}
//detects collisions between entities
bool Entity::collision(Entity &e){
    float x_distance = abs(e.x-x)-((e.width+width));
    float y_distance = abs(e.y-y)-((e.height+height));
    if(x_distance< 0 && y_distance < 0){
        return true;
    }
    return false;
}
