//Space Invaders
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

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include "Entity.hpp"

using namespace std;
//Globals
glm::mat4 projectionMatrix = glm::mat4(1.0f);
SDL_Event event;
bool done = false;
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
float lastFrameTicks = 0.0f;
ShaderProgram untexteredShader;
float elapsed;
GLuint spriteSheetTexture;
GLuint backgroundImage; //didn't end up using this
int num_of_ships = 24; //24 total ships

ShaderProgram texteredShader;
vector<Entity> entities;
vector<Entity> ships;
vector<Entity> bullets;
float shipsVelocity = 0.25;
int bulletInd = 0;
GLuint fontTexture;
int score = 0;
float bulletTimer = 0;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;

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
            text_x, text_y + texture_size,
            text_x + texture_size, text_y,
            text_x + texture_size, text_y + texture_size,
            text_x + texture_size, text_y,
            text_x, text_y + texture_size,
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
class mainMenu {
public:
    void Clean(){}
    void Render() {
        DrawText(texteredShader, fontTexture, "Space to Start", -0.77,0.0,0.1,0.01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void Events(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                    lastFrameTicks = SDL_GetTicks()/1000.0f;
                }
            }
        }
    }
};
class Game{
    public:
    void Setup(){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
        #ifdef _WINDOWS
                glewInit();
        #endif
        glViewport(0, 0, 640, 360);
        projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        texteredShader.Load("vertex_textured.glsl", "fragment_textured.glsl");
        untexteredShader.Load("vertex.glsl", "fragment.glsl");
        untexteredShader.SetViewMatrix(viewMatrix);
        untexteredShader.SetProjectionMatrix(projectionMatrix);
        untexteredShader.SetModelMatrix(modelMatrix);
        texteredShader.SetViewMatrix(viewMatrix);
        texteredShader.SetProjectionMatrix(projectionMatrix);
        texteredShader.SetModelMatrix(modelMatrix);
        
        //Load Background Image
        backgroundImage = LoadTexture("background.png"); //unused
        //Load Sprite and Font textures
        spriteSheetTexture = LoadTexture("ships.png");
        fontTexture = LoadTexture("font.png");

    }
    void Events(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            //Shoot bullets with space bar
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && bulletTimer >=1) {
                    bullets[bulletInd].x = entities[0].x;
                    bullets[bulletInd].y = entities[0].y;
                    bulletInd++;
                    if(bulletInd > 20){
                        bulletInd = 0;
                    }
                    bulletTimer = 0;
                }
            }
        }
    }
    void Update(){
        glClear(GL_COLOR_BUFFER_BIT);
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT] && entities[0].x-entities[0].width>= -1.77) {
            entities[0].x_velocity = -2;
        } else if(keys[SDL_SCANCODE_RIGHT] && entities[0].x+entities[0].width <= 1.77) {
            entities[0].x_velocity = 2;
        }
        else{entities[0].x_velocity = 0;}
        entities[0].update(elapsed);
        for(Entity &a: ships){
            if(!(a.x < -250) && a.x+a.sprite.size/2<= -1.50){
                shipsVelocity = 0.35;
                for(Entity &al: ships){
                    al.y -= 0.025;
                }
            }
            else if(!(a.x < -250) && a.x+(a.sprite.size/2) >= 1.50){
                shipsVelocity = -0.35;
                for(Entity &al: ships){
                    al.y -= 0.025;
                }
            }
        }
        for(Entity &a: ships){
            a.x_velocity = shipsVelocity;
            for(Entity &b: bullets){
                if(b.collision(a)){
                    a.x = -1000;
                    b.x = 200;
                    score++;
                }
            }
            if(a.collision(entities[0])){
                done = true;
            }
            a.update(elapsed);
        }
        for(Entity &b: bullets){
            b.update(elapsed);
        }
        bulletTimer += elapsed;
        if(score == num_of_ships){
            DrawText(texteredShader, fontTexture, "You Win!", -0.95, 0.0, 0.25, 0.01);
        }
        
    }
    void Render(){
        DrawText(texteredShader, fontTexture, "Score: "+to_string(score), -1.70,0.95,0.05,0.01);
        entities[0].Draw(texteredShader, elapsed);
        for(Entity &a: ships){
            a.Draw(texteredShader, elapsed);
        }
        for(Entity &b: bullets){
            b.Draw(texteredShader, elapsed);
        }
    }
    void Clean(){
    }
};

Game game;
mainMenu menu;

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
void Update(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Update();
            break;
        case STATE_GAME_LEVEL:
            game.Update();
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
    SheetSprite playerShip = SheetSprite(spriteSheetTexture, 113.0f/1024.0f, 865.0f/1024.0f, 113.0f/1024.0f, 75.0f/1024.0f, 0.20f);
    SheetSprite ship = SheetSprite(spriteSheetTexture, 424.0f/1024.0f, 730.0f/1024.0f, 93.0f/1024.0f, 85.0f/1024.0f, 0.20f);
    SheetSprite bullet = SheetSprite(spriteSheetTexture, 850.0f/1024.0f, 360.0f/1024.0f, 10.0f/1024.0f, 60.0f/1024.0f, 0.25f);

    entities.push_back(Entity(0.0,-0.825,-0.05,0.0,playerShip.width,playerShip.height,0,0,0,playerShip.u,playerShip.v,playerShip.textureID, playerShip.size));
    float x = -1.25;
    float y = 0.15;
    for(int i = 0; i <3; i++){ //3 ships per column
        x=-1.25;
        for(int j = 0; j < 8; j++){ //8 ships per row
            ships.push_back(Entity(x,y,0.5,0,ship.width,ship.height,0,0,0,ship.u,ship.v,ship.textureID, ship.size));
            x+=0.325;
        }
        y+=0.20;
    }
    for(int i = 0; i <20; i++){
        bullets.push_back(Entity(-100,-100,0,5,bullet.width,bullet.height,0,0,0,bullet.u,bullet.v,bullet.textureID, bullet.size));
    }
    while (!done) {
        Events();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    Clean();
    
    SDL_Quit();
    return 0;
}
