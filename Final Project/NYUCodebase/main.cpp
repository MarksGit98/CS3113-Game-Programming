//Platformer Game
//Mark Bekker and Kenny Yip


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
#include "Entity.hpp"

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

//Add sound
//Amimate enemies (and figure out playerID to fix player animation
//Figure out shooting
//Figure out key entity
//Make sure game state changes work
//Make sure enemy and player collision works


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

bool win = false; //when win condition is reached
bool hasKey = false;
bool hitSpikes = false;
int keyID = 86;
int endgateID = 7;
float invulnerableTime = 3.0;

ShaderProgram program;
vector<Entity> players;
GLuint fontTexture;
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;
vector<Entity> enemies;

vector<Entity> levelkeys;


vector<float> vertexData;
vector<float> texCoordData;
vector<int> palpables = {32,33,34,35,32,17,100,7}; //IDs of collidable blocks
vector<int> collectables = {keyID};
int spikeID = 100;
int playerID = 98;
int sprite_count_x = 16;
int sprite_count_y = 8;
float tileSize = 0.15;
float scale = 0.1;
FlareMap map;
float gravity = 0.0075f;
float accumulator = 0.0f;
bool JumpOn = false;
int defaultHealth = 10;
int health = defaultHealth;
int currentGameLevel = 3;
SDL_Window* displayWindow;

float playerAnimationTimer = 0.10;

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
    float text_size = 1.0 / 16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
    for (size_t i = 0; i < text.size(); i++) {
        float text_x = ((text[i]) % 16) / 16.0f;
        float text_y = ((text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            text_x, text_y,
            text_x, text_y + text_size,
            text_x + text_size, text_y,
            text_x + text_size, text_y + text_size,
            text_x + text_size, text_y,
            text_x, text_y + text_size,
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
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6.0f);
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
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2.0f);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void renderEntities(){
    float spriteWidth = 1.0f/(float)sprite_count_x;
    float spriteHeight = 1.0f/(float)sprite_count_y;
  
    //Create enemy sprites
    for(FlareMapEntity enemy: map.entities){
        if(enemy.type == "Key"){
            cout<<"key"<<endl;
        //Create key
        float u = (float)((86) % sprite_count_x) / (float) sprite_count_x;
        float v = (float)((86) / sprite_count_x) / (float) sprite_count_y;
        
        SheetSprite key = SheetSprite(spriteSheetTexture,u, v, spriteWidth, spriteHeight, tileSize);
        levelkeys.push_back(Entity(float(enemy.x)*tileSize,float(-enemy.y)*tileSize,1.0,0, key.width, key.height, 0, 0, 0, key.u , key.v, key.textureID, key.size));
        }
        if(enemy.type == "Minion"){
            //Create Minion
            float u = (float)((80) % sprite_count_x) / (float) sprite_count_x;
            float v = (float)((80) / sprite_count_x) / (float) sprite_count_y;
            SheetSprite minion = SheetSprite(spriteSheetTexture, u, v,spriteWidth , spriteHeight, tileSize);
            enemies.push_back(Entity(float(enemy.x)*tileSize,float(-enemy.y)*tileSize,1.0,0,minion.width,minion.height,0,0,0,minion.u,minion.v,minion.textureID, minion.size));
        }
        if(enemy.type == "Ghost"){
            //Create Ghosts
            float u = (float)((103) % sprite_count_x) / (float) sprite_count_x;
            float v = (float)((103) / sprite_count_x) / (float) sprite_count_y;
            SheetSprite ghost = SheetSprite(spriteSheetTexture, u, v,spriteWidth , spriteHeight, tileSize);
            enemies.push_back(Entity(float(enemy.x)*tileSize,float(-enemy.y)*tileSize,1.0,0,ghost.width,ghost.height,0,0,0,ghost.u,ghost.v,ghost.textureID, ghost.size));
        }
    }
}
//Collision Detection
void worldToTileCoordinates(float worldX, float worldY, int *map_X, int *map_Y) {
    *map_X = (int)(worldX / tileSize);
    *map_Y = (int)(worldY / -tileSize);
}

bool playerCollideTop(){ //handle top collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (players[0].position.x / tileSize);
    map_Y = ((players[0].position.y + (players[0].height / 2)) / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int ID: palpables){
            if(map.mapData[map_Y][map_X] == ID){
                players[0].position.y -= fabs(((-tileSize * map_Y) - tileSize) - (players[0].position.y + players[0].height/2))+0.001;
                return true;
            }
            if (map.mapData[map_Y+1][map_X] == spikeID){ //Spikes kill you if you land on it
                if(invulnerableTime < 0){
                    health-=1;
                    invulnerableTime=2;
                }
            }
        }
    }
    players[0].collidedBottom = false;
    return false;
}

bool playerCollideBottom(){ //handle bottom collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (players[0].position.x / tileSize);
    map_Y = ((players[0].position.y - (tileSize/ 2)) / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                players[0].collidedBottom = true;
                players[0].position.y += fabs((-tileSize * map_Y) - (players[0].position.y - tileSize/2))+0.001;
                JumpOn = true;
                return true;
            }
            if (map.mapData[map_Y+1][map_X] == spikeID){ //Spikes kill you if you land on it
                if(invulnerableTime < 0){
                    health-=1;
                    invulnerableTime=2;
                }
            }
        }
    }
    players[0].collidedBottom = false;
    return false;
}

bool playerCollideLeft(){ //handle left collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = ((players[0].position.x - (players[0].width / 2))/ tileSize);
    map_Y = (players[0].position.y / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                players[0].position.x += fabs(((tileSize * map_X) + tileSize) - (players[0].position.x - tileSize/2))+0.001;
                return true;
            }
            if (map.mapData[map_Y][map_X+1] == endgateID){ //Allow player to advance to the next level if he has the key
                if (hasKey==true){
                    if (currentGameLevel==3){
                        win=true;
                    }
                    health=defaultHealth;
                    hasKey=false;
                    currentGameLevel+=1;
                    enemies.clear();
                    levelkeys.clear();
                    //players.clear();
                    map.entities.clear();
                    if(currentGameLevel == 2){
                        map.Load("Level2.txt");
                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    }
                    else if(currentGameLevel==1){
                        map.Load("Level3.txt");
                        glClearColor(0.72f, 0.61f, 1.0f, 1.0f);
                    }
                    vertexData.clear();
                    texCoordData.clear();
                    for(int y=0; y < map.mapHeight; y++) {
                        for(int x=0; x < map.mapWidth; x++) {
                            if(map.mapData[y][x] != 0){
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
                        players[0].position.x = 3*tileSize;
                        players[0].position.y = 22*(-tileSize);
                    }
                    renderEntities();
                }
            }
        }
    }
    return false;
}

bool playerCollideRight(){ //handle right collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = ((players[0].position.x + (players[0].width / 2))/ tileSize);
    map_Y = (players[0].position.y / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                players[0].position.x -= fabs(((tileSize * map_X) + tileSize) - (players[0].position.x - tileSize/2))+0.001;
                return true;
            }
            if (map.mapData[map_Y][map_X+1] == endgateID){ //Allow player to advance to the next level if he has the key
                if (hasKey==true){
                    if (currentGameLevel==3){
                        win=true;
                    }
                    health=defaultHealth;
                    hasKey=false;
                    currentGameLevel+=1;
                    enemies.clear();
                    levelkeys.clear();
                    //players.clear();
                    map.entities.clear();
                    if(currentGameLevel == 2){
                        map.Load("Level2.txt");
                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    }
                    else if (currentGameLevel==1){
                        map.Load("Level3.txt");
                        glClearColor(0.72f, 0.61f, 1.0f, 1.0f);
                    }
                    vertexData.clear();
                    texCoordData.clear();
                    for(int y=0; y < map.mapHeight; y++) {
                        for(int x=0; x < map.mapWidth; x++) {
                            if(map.mapData[y][x] != 0){
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
                        players[0].position.x = 3*tileSize;
                        players[0].position.y = 22*(-tileSize);
                    }
                    renderEntities();
                }
                }
            }
        }
     return false;
    }

bool enemyCollideTop(Entity& enemy){ //handle top collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (enemy.position.x / tileSize);
    map_Y = ((enemy.position.y + (players[0].height / 2)) / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int ID: palpables){
            if(map.mapData[map_Y][map_X] == ID){
                enemy.position.y -= fabs(((-tileSize * map_Y) - tileSize) - (enemy.position.y + enemy.height/2))+0.001;
                return true;
            }
        }
    }
    enemy.collidedBottom = false;
    return false;
}

bool enemyCollideBottom(Entity& enemy){ //handle bottom collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = (enemy.position.x / tileSize);
    map_Y = ((enemy.position.y - (tileSize/ 2)) / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                enemy.collidedBottom = true;
                enemy.position.y += fabs((-tileSize * map_Y) - (enemy.position.y - tileSize/2))+0.001;
                return true;
            }
        }
    }
    enemy.collidedBottom = false;
    return false;
}
void PlayerAnimation(){
    if(playerID == 98){
        playerID = 99;
    }
    else if (playerID==99){
        playerID = 98;
    }
    players[0].sprite.u = (playerID % sprite_count_x) / (float) sprite_count_x;
    players[0].sprite.v = (playerID / sprite_count_x) / (float) sprite_count_y;
}
void EnemyAnimation(Entity& enemy){
    if(playerID == 80){ //if Minion
        playerID = 81;
    }
    else if (playerID==81){
        playerID = 80;
    }
    if(playerID == 103){ //if Ghost
        playerID = 104;
    }
    else if (playerID==104){
        playerID = 103;
    }
    enemy.sprite.u = (playerID % sprite_count_x) / (float) sprite_count_x;
    enemy.sprite.v = (playerID / sprite_count_x) / (float) sprite_count_y;
}

bool enemyCollideLeft(Entity &enemy){ //handle left collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = ((enemy.position.x - (enemies[0].width / 2))/ tileSize);
    map_Y = (enemy.position.y / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                enemy.position.x += fabs(((tileSize * map_X) + tileSize) - (enemy.position.x - tileSize/2))+0.001;
                return true;
            }
        }
    }
    enemy.collidedBottom = false;
    return false;
}

bool enemyCollideRight(Entity& enemy){ //handle right collision with block
    int map_X = 0;
    int map_Y = 0;
    map_X = ((enemy.position.x + (enemies[0].width / 2))/ tileSize);
    map_Y = (enemy.position.y / -tileSize);
    
    if(map_X < map.mapWidth && map_Y < map.mapHeight){
        for(int tileID: palpables){
            if(map.mapData[map_Y][map_X] == tileID){
                enemy.position.x -= fabs(((tileSize * map_X) + tileSize) - (enemy.position.x + tileSize/2))+0.001;
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
    }
    
    void Update() {
        if(currentGameLevel == 1){
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        }
        else if(currentGameLevel == 2){
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else if(currentGameLevel == 3){
            glClearColor(0.72f, 0.61f, 1.0f, 1.0f);
        }
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

class Game{
public:
    void Setup(){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
        glViewport(0, 0, 1280, 720);
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
        fontTexture = LoadTexture("font.png");
        
        //Load map
        if (currentGameLevel == 1){
            map.Load("Level1.txt");
        }
        else if (currentGameLevel == 2){
            map.Load("Level2.txt");
        }
        else if (currentGameLevel == 3){
            map.Load("Level3.txt");
        }
        
        glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
        for(int y=0; y < map.mapHeight; y++) {
            for(int x=0; x < map.mapWidth; x++) {
                if(map.mapData[y][x] != 0 && map.mapData[y][x] != 12){
                    float u = (float)((map.mapData[y][x]) % sprite_count_x) / (float)(sprite_count_x);
                    float v = (float)((map.mapData[y][x]) / sprite_count_x) / (float)(sprite_count_y);
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
    }
    void Events(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && JumpOn) {
                    players[0].velocity.y=1.0;
                    JumpOn = false;
                }
            }
        }
    }
    
    void Update(float elapsedUpdate = elapsed){
        if (health == 0){
            mode = STATE_GAME_OVER;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        //Handle player and enemy movement
        if(keys[SDL_SCANCODE_LEFT]) {
            players[0].velocity.x = -2.5;
            playerAnimationTimer -= elapsedUpdate;
            if(playerAnimationTimer < 0){
                PlayerAnimation();
                playerAnimationTimer = 0.1;
            }
        } else if(keys[SDL_SCANCODE_RIGHT]) {
            players[0].velocity.x = 2.5;
            playerAnimationTimer -= elapsedUpdate;
            if(playerAnimationTimer < 0){
                PlayerAnimation();
                playerAnimationTimer = 0.1;
            }
        }
        else{
            players[0].velocity.x = 0;
        }
        players[0].velocity.y -= gravity;
        
        if(playerCollideBottom()||playerCollideTop()){
            players[0].velocity.y = 0;
        }
        
        if(playerCollideLeft()||playerCollideRight()){
            players[0].velocity.x = 0;
        }
        for (Entity& enemy: enemies){
            enemy.velocity.y -= gravity;
            enemy.velocity.x = 0.25;
            if(enemyCollideBottom(enemy)||enemyCollideTop(enemy)){
                enemy.velocity.y = 0;
            }
            
            if(enemyCollideLeft(enemy)||enemyCollideRight(enemy)){
                enemy.velocity.x = 0;
            }
        }
        for(Entity& enemy: enemies){ //Collision with enemy
                if(players[0].collision(enemy)){
                    if(invulnerableTime <= 0){
                        health-=1;
                        invulnerableTime=1;
                    }
                }
            }
        for(Entity& levelkey: levelkeys){ //Collision with keys
            if(players[0].collision(levelkey)){
                hasKey=true;
                levelkeys.pop_back();
            }
        }
        invulnerableTime-=elapsedUpdate;
        players[0].update(elapsedUpdate);
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-players[0].position.x,-players[0].position.y,0.0f));
        program.SetViewMatrix(viewMatrix);
    }
    
    void Render(){
        if (!win){
            drawMap();
            DrawText(program, fontTexture, "Health:"+ to_string(health),players[0].position.x-1.65,players[0].position.y+0.90,0.07,0.00);
            if (hasKey==false){
             DrawText(program, fontTexture, "Key: Not Found",players[0].position.x-1.65,players[0].position.y+0.77,0.07,0.00);
            }
            else{
                 DrawText(program, fontTexture, "Key: Acquired",players[0].position.x-1.65,players[0].position.y+0.77,0.07,0.00);
            }
            for(Entity& e: players){
                e.Draw(program, elapsed);
            }
            if(levelkeys.empty() == false){
                for(Entity& key: levelkeys){
                    key.Draw(program, elapsed);
                }
            }
            if (enemies.empty() == false){
                for(Entity& enemy: enemies){
                    enemy.Draw(program, elapsed);
                }
            }
        }
        else {
            DrawText(program, fontTexture, "YOU WIN!", players[0].position.x-0.80,players[0].position.y, 0.20, 0.01); //Display YOU WIN to player
            glClearColor(0.0, 0.0, 0.0, 0.0);
        }
    }
    void Clean(){}
};

class GameOver {
public:
    void Render() {
        DrawText(program, fontTexture, "GAME OVER", players[0].position.x-0.82,players[0].position.y, 0.20, 0.01); //Display GAME OVER to player
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0);
    }
    void ProcessEvents(){
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
    void CleanUp(){
        
    }
};
mainMenu menu;
Game game;
GameOver gameOver;

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
        case STATE_GAME_OVER:
            gameOver.ProcessEvents();
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
        case STATE_GAME_OVER:
            gameOver.Update();
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
        case STATE_GAME_OVER:
            gameOver.Render();
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
        case STATE_GAME_OVER:
            gameOver.CleanUp();
            break;
    }
}

int main(int argc, char *argv[]){
    
    Setup();
    
    //Create player sprite
    float spriteWidth = 1.0f/(float)sprite_count_x;
    float spriteHeight = 1.0f/(float)sprite_count_y;
    float u = (float)((98) % sprite_count_x) / (float) sprite_count_x;
    float v = (float)((98) / sprite_count_x) / (float) sprite_count_y;
    SheetSprite mySprite = SheetSprite(spriteSheetTexture, u, v,spriteWidth , spriteHeight, tileSize);
    players.push_back(Entity(0,-4,0,0,mySprite.width,mySprite.height,0,0,0,mySprite.u,mySprite.v,mySprite.textureID, mySprite.size));
    
    renderEntities();
    
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        
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
