#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
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

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
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

//global vars
float leftpaddle_ypos = 0.0f;
float rightpaddle_ypos = 0.0f;
float leftpaddle_xpos = -1.3f;
float rightpaddle_xpos = 1.3f;
float orgleftpaddle_ypos = 0.0f;
float orgrightpaddle_ypos = 0.0f;
float orgleftpaddle_xpos = -1.3f;
float orgrightpaddle_xpos = 1.3f;
float ball_ypos = 0.0f;
float ball_xpos = 0.0f;
float orgball_ypos = 0.0f;
float orgball_xpos = 0.0f;
bool upbounce = false;
bool downbounce = false;
bool going_right = true;
bool going_left = false;

void reset(){
    leftpaddle_ypos = orgleftpaddle_ypos;
    rightpaddle_ypos = orgrightpaddle_ypos;
    leftpaddle_xpos = orgleftpaddle_xpos;
    rightpaddle_xpos = orgrightpaddle_xpos;
    ball_ypos = orgball_ypos;
    ball_xpos = orgball_xpos;
    if (going_right){
        going_right = false;
        going_left = true;
    }
    else {
        going_left = false;
        going_right = true;
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    //Screen resolution
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program;
    ShaderProgram program2;
    program.Load("vertex.glsl", "fragment.glsl");
    program2.Load("vertex_textured.glsl", "fragment_textured.glsl");
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    //Background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    SDL_Event event;
    bool done = false;
    float lastFrameTicks = 0.0f;
    float paddle_height = 0.5f;
    float paddle_width = 0.075f;
    float ball_xspeed = 1.2;
    float ball_yspeed = 1.05;
    float floor_ypos = -0.9f;
    float ceiling_ypos = 0.9f;
    bool start_game = false;
    bool first_bounce = false;
    int win_score = 3;
    int player_score = 0;
    int comp_score = 0;
    float comp_speed = 0.5;
    float player_speed = 4.8;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    while (!done) {
        
        if(keys[SDL_SCANCODE_SPACE]) { //start game
            start_game = true;
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        if (start_game == true){
            if (first_bounce == false){
                ball_ypos-=elapsed*ball_yspeed;
                ball_xpos+=elapsed*ball_xspeed;
            }
        }
        //Collision with player paddle
        if (ball_ypos<=rightpaddle_ypos+paddle_height/2 && ball_ypos>=rightpaddle_ypos-paddle_height/2 && ball_xpos >=rightpaddle_xpos-paddle_width/2){
            going_left = true;
            going_right = false;
            
        }
        //Collision with computer paddle
        if (ball_ypos<=leftpaddle_ypos+paddle_height/2 && ball_ypos>=leftpaddle_ypos-paddle_height/2 && ball_xpos<=leftpaddle_xpos+paddle_width/2){
            going_right = true;
            going_left = false;
        }
        //ball movement
        if (downbounce == true){
            if (going_right == true){
                ball_ypos-=elapsed*ball_yspeed;
                ball_xpos+=elapsed*ball_xspeed;
            }
            else if (going_left == true){
                ball_ypos-=elapsed*ball_yspeed;
                ball_xpos-=elapsed*ball_xspeed;            }
        }
        else if (upbounce == true){
            if (going_right == true){
                ball_ypos+=elapsed*ball_yspeed;
                ball_xpos+=elapsed*ball_xspeed;
            }
            else if (going_left == true){
                ball_ypos+=elapsed*ball_yspeed;
                ball_xpos-=elapsed*ball_xspeed;
            }
        }
        //collision with ceiling
        if ((ball_ypos+0.05)>=ceiling_ypos){
            first_bounce = true;
            downbounce = true;
            upbounce = false;
        }
        //collision with floor
        if ((ball_ypos-0.05)<=floor_ypos){
            first_bounce = true;
            upbounce = true;
            downbounce = false;
        }
        //Player score condition
        if (ball_xpos <= leftpaddle_xpos-paddle_width){
            player_score+=1;
            std::cout<<"PLAYER SCORES! Player: "<<player_score<<" Computer: "<<comp_score<<std::endl;
            reset();
        }
        //Computer score condition
        else if (ball_xpos >= rightpaddle_xpos+paddle_width){
            comp_score+=1;
            std::cout<<"COMPUTER SCORES! Player: "<<player_score<<" Computer: "<<comp_score<<std::endl;
            reset();
        }
        //Winner determination
        if(player_score == win_score){
            std::cout<<"Player won the game"<<std::endl;
            player_score=0;
            comp_score=0;
        }
        if(comp_score == win_score){
            std::cout<<"Computer won the game"<<std::endl;
            player_score=0;
            comp_score=0;
        }
        //move computer paddle
        if (ball_ypos > leftpaddle_ypos){
            if (ceiling_ypos >= leftpaddle_ypos+leftpaddle_ypos/2){
                leftpaddle_ypos+=elapsed*comp_speed;
            }
        }
        else if (ball_ypos < leftpaddle_ypos){
            if (floor_ypos <= leftpaddle_ypos+leftpaddle_ypos/2){
                leftpaddle_ypos-=elapsed*comp_speed;
            }
        }
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            
            //move player paddle
            if(keys[SDL_SCANCODE_UP]) { //paddle up
                if (ceiling_ypos >= rightpaddle_ypos+rightpaddle_ypos/2){
                    rightpaddle_ypos+=elapsed*player_speed;
                }
            }
            else if(keys[SDL_SCANCODE_DOWN]){ //paddle down
                if (floor_ypos <= rightpaddle_ypos+rightpaddle_ypos/2){
                    rightpaddle_ypos-=elapsed*player_speed;
                }
            }
            
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        //ball
        float vertices[] = {-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5};
        modelMatrix = glm::mat4(1.0f);
         modelMatrix = glm::translate(modelMatrix, glm::vec3(ball_xpos, ball_ypos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.075f, 0.075f, 1.0f));
        glVertexAttribPointer(program.positionAttribute,2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //right paddle
        float vertices2[] = {-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(rightpaddle_xpos, rightpaddle_ypos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(paddle_width, paddle_height, 1.0f));
        glVertexAttribPointer(program.positionAttribute,2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //left paddle
        float vertices3[] = {-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(leftpaddle_xpos, leftpaddle_ypos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(paddle_width, paddle_height, 1.0f));
        glVertexAttribPointer(program.positionAttribute,2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //floor
        float vertices4[] = {-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, floor_ypos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.8f, 0.075f, 1.0f));
        glVertexAttribPointer(program.positionAttribute,2, GL_FLOAT, false, 0, vertices4);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //ceiling
        float vertices5[] = {-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, ceiling_ypos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.8f, 0.075f, 1.0f));
        glVertexAttribPointer(program.positionAttribute,2, GL_FLOAT, false, 0, vertices5);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}

