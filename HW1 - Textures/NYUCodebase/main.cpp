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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program;
    ShaderProgram program2;
    program.Load("vertex.glsl", "fragment.glsl");
    program2.Load("vertex_textured.glsl", "fragment_textured.glsl");
    
    GLuint pikachuPng = LoadTexture("Pikachu.png"); //Load Pikachu
    GLuint pokeballPng = LoadTexture("Pokeball.png"); //Load Pokeball
    GLuint ashPng = LoadTexture("Ash.png"); //Load Ash
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glClearColor(0.5f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program.programID);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        glUseProgram(program2.programID);
        glEnableVertexAttribArray(program2.positionAttribute);
        glEnableVertexAttribArray(program2.texCoordAttribute);
        program2.SetProjectionMatrix(projectionMatrix);
        program2.SetViewMatrix(viewMatrix);
        
        //light beam coming from pikachu
        float vertices1[] = {-1.5f, -3.0f, 0.25f, 1.0f, 2.0f, 20.0f};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.05f, 0.0f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 0.85f, 1.0f);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        //Ash Sprite
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.9f, -0.25f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.0f));
        program2.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, ashPng);
        float vertices8[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices8);
        float texCoords3[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //Pokeball Sprite
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.1f, -0.25f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.6f, 1.0f));
        program2.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, pokeballPng);
        float vertices7[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices7);
        float texCoords2[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //Pikachu Sprite
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.85f, -0.65f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
        program2.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, pikachuPng);
        float vertices6[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices6);
        float texCoords1[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program2.positionAttribute);
        glDisableVertexAttribArray(program2.texCoordAttribute);
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}
