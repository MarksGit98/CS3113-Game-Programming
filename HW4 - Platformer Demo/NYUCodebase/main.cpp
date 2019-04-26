//Platformer Demo
//Mark Bekker

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FlareMap.hpp"

#ifdef _WINDOWS
#endif
#include "Entity.hpp"

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

using namespace std;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
SDL_Event event;
bool done = false;
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
float lastFrameTicks = 0.0f;
ShaderProgram program2;
float elapsed;
GLuint spriteSheetTexture;
int score = 0;

ShaderProgram program;
vector<Entity> entities;
GLuint fontTexture;
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
vector<Entity> enemies;
vector<float> vertexData;
vector<float> texCoordData;
int sprite_count_x = 16;
int sprite_count_y = 8;
float tileSize = 0.15;
float scale = 0.1;
FlareMap map;
vector<int> palpables;
float gravity = 0.01f;
float accumulator = 0.0f;
bool JumpOn = false;

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}
void DrawText(ShaderProgram &program, int fontTexture, string text, float x, float y, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    
    glUseProgram(program.programID);
    program.SetModelMatrix(textModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}
void drawMap(){
    glUseProgram(program.programID);
    glm::mat4 mapModelMatrix = glm::mat4(1.0);
    program.SetModelMatrix(mapModelMatrix);
    glEnable(GL_BLEND);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

//Collision Detection
void worldToTileCoordinates(float worldX, float worldY, int *map_X, int *map_Y) {
    *map_X = (int)(worldX / tileSize);
    *map_Y = (int)(worldY / -tileSize);
}
bool playerCollideBottom(){ //handle bottom collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (int)(entities[0].position.x / tileSize);
    map_Y = (int)((entities[0].position.y - (tileSize/ 2)) / -tileSize);
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int palpableID: palpables){
            if(map.mapData[map_Y][map_X] == palpableID){
                entities[0].collidedBottom = true;
                entities[0].position.y += fabs((-tileSize * map_Y) - (entities[0].position.y - tileSize/2))+.001;
                JumpOn = true;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideTop(){ //handle top collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (int)(entities[0].position.x / tileSize);
    map_Y = (int)((entities[0].position.y + (entities[0].height / 2)) / -tileSize);
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int palpableID: palpables){
            if(map.mapData[map_Y][map_X] == palpableID){
                entities[0].position.y -= fabs(((-tileSize * map_Y) -tileSize) - (entities[0].position.y + entities[0].height/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideLeft(){ //handle left collision with block
    int map_X =0;
    int map_Y = 0;
    map_X = (int)((entities[0].position.x - (entities[0].width / 2))/ tileSize);
    map_Y = (int)(entities[0].position.y / -tileSize);
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int palpableID: palpables){
            if(map.mapData[map_Y][map_X] == palpableID){
                entities[0].position.x += fabs(((tileSize * map_X) - tileSize) - (entities[0].position.x - entities[0].width/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideRight(){ //handle right collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (int)((entities[0].position.x + (entities[0].width / 2))/ tileSize);
    map_Y = (int)(entities[0].position.y / -tileSize);

    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int palpableID: palpables){
            if(map.mapData[map_Y][map_X] == palpableID){
                entities[0].position.x -= fabs(((tileSize * map_X) + tileSize) - (entities[0].position.x + entities[0].width/2))+.001;
                return true;
            }
        }
    }
    return false;
}

class mainMenu {
public:
    void Render() {
        DrawText(program, fontTexture, "Space to Start", -0.80,0,0.1,0.01);
        //DrawText(program, fontTexture, "Score: "+to_string(score), -1.70,0.95,0.05,0.01);
    }
    void Update() {
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    }
    void Events(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                    lastFrameTicks = (float)SDL_GetTicks()/1000.0f;
                }
            }
        }
    }
    void Clean(){}
};
void GameOver(){
    glClearColor(1.0f, 1.0f, 0.1f, 1.0f);
}
class Game{
public:
    void Setup(){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Platformer Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
        glewInit();
#endif
        glViewport(0, 0, 640, 360);
        projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        program.Load("vertex_textured.glsl","fragment_textured.glsl");
        program2.Load("vertex.glsl","fragment.glsl");
        program2.SetViewMatrix(viewMatrix);
        program2.SetProjectionMatrix(projectionMatrix);
        program2.SetModelMatrix(modelMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelMatrix(modelMatrix);
        
        //Load textures
        spriteSheetTexture = LoadTexture("sheet.png");
        glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
        fontTexture = LoadTexture("font.png");
        map.Load("MyMap.txt");
        for(int y=0; y < map.mapHeight; y++) {
            for(int x=0; x < map.mapWidth; x++) {
                if(map.mapData[y][x] != 0 && map.mapData[y][x] != 12){
                    float u = (float)(((int)map.mapData[y][x]) % sprite_count_x) / (float) sprite_count_x;
                    float v = (float)(((int)map.mapData[y][x]) / sprite_count_x) / (float) sprite_count_y;
                    float spriteWidth = 1.0f/(float)sprite_count_x;
                    float spriteHeight = 1.0f/(float)sprite_count_y;
                    vertexData.insert(vertexData.end(), {
                        tileSize * x, -tileSize * y,
                        tileSize * x, (-tileSize * y)-tileSize,
                        (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                        tileSize * x, -tileSize * y,
                        (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                        (tileSize * x)+tileSize, -tileSize * y
                    });
                    texCoordData.insert(texCoordData.end(), {
                        u, v,
                        u, v+(spriteHeight),
                        u+spriteWidth, v+(spriteHeight),
                        u, v,
                        u+spriteWidth, v+(spriteHeight),
                        u+spriteWidth, v
                    });
                }
            }
        }
        palpables.push_back(17);
    }
    void Events(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && JumpOn) {
                    entities[0].velocity.y=1.0;
                    JumpOn = false;
                }
            }
        }
    }
    void Update(float elapsedUpdate = elapsed){
        glClear(GL_COLOR_BUFFER_BIT);
        
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT]) {
            entities[0].velocity.x = -0.25;
        } else if(keys[SDL_SCANCODE_RIGHT]) {
            entities[0].velocity.x = 0.25;
        }
        else{
            entities[0].velocity.x = 0;
        }
        entities[0].velocity.y -= gravity;
        if(playerCollideBottom()||playerCollideTop()){
            entities[0].velocity.y = 0;
        }
        playerCollideLeft();
        for(Entity& enemy: enemies){
            if(entities[0].collision(enemy)){
                score++;
                enemies.pop_back();
                delete &enemy;
                GameOver();
            }
        }
        entities[0].update(elapsedUpdate);
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[0].position.x,-entities[0].position.y,0.0f));
        program.SetViewMatrix(viewMatrix);
    }
    void Render(){
        drawMap();
        
        for(Entity& e: entities){
            e.Draw(program, elapsed);
        }
        for(Entity& enemy: enemies){
            enemy.Draw(program, elapsed);
        }
    }
    
    void Clean(){}
};

mainMenu menu;
Game game;
void Setup(){
    game.Setup();
}

void Events(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Events();
            break;
        case STATE_GAME_LEVEL:
            game.Events();
            break;
    }
}

void Update(float elapsedUpdate = elapsed){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Update();
            break;
        case STATE_GAME_LEVEL:
            game.Update(elapsedUpdate);
            break;
    }
}

void Render(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Render();
            break;
        case STATE_GAME_LEVEL:
            game.Render();
            break;
    }
}

void Clean(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Clean();
            break;
        case STATE_GAME_LEVEL:
            game.Clean();
            break;
    }
}

int main(int argc, char *argv[]){
    Setup();
    float u = (float)((98) % sprite_count_x) / (float) sprite_count_x;
    float v = (float)((98) / sprite_count_x) / (float) sprite_count_y;
    float spriteWidth = 1.0f/(float)sprite_count_x;
    float spriteHeight = 1.0f/(float)sprite_count_y;
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize);
    u = (float)((80) % sprite_count_x) / (float) sprite_count_x;
    v = (float)((80) / sprite_count_x) / (float) sprite_count_y;
    SheetSprite enemy = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize);
    entities.push_back(Entity(0,-1.0,-1.0,0,mySprite.width,mySprite.height,0,0,0,mySprite.u,mySprite.v,mySprite.textureID, mySprite.size));
    enemies.push_back(Entity(1.60,-1.0,-0.1,0.0,enemy.width,enemy.height,0,0,0,enemy.u,enemy.v,enemy.textureID, enemy.size));
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        Events();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    Clean();
    SDL_Quit();
    return 0;
}
