#include <windows.h>        
#include <stdlib.h>         
#include <stdio.h>
#include <GL/glew.h>        
#include <GL/freeglut.h>    
#include "loadShaders.h"    
#include "glm/glm.hpp"      
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"          
#include <vector>
#include <ctime>             

GLuint VaoId, VboId, EboId, ProgramId, myMatrixLocation;
GLuint textures[3];       
GLfloat winWidth = 800, winHeight = 600;
glm::mat4 myMatrix, resizeMatrix;
float xMin = -80, xMax = 80.f, yMin = -60.f, yMax = 60.f;

struct Bird {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 direction;
    int frameOffset;      
};

std::vector<Bird> flock;

GLuint LoadTexture(const char* texturePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height;
    unsigned char* image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGBA); 

    if (image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image); 
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
    }
    else {
        printf("Failed to load texture: %s\n", texturePath);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void CreateFlock(int numBirds) {
    srand(static_cast<unsigned>(time(0))); // random start

    float startX = -80.0f;
    float xSpacing = 5.0f;

    for (int i = 0; i < numBirds; ++i) {
        Bird bird;

        // random y
        float randomY = -20.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 40.0f));
        float randomXOffset = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f)) - 1.0f;

        bird.position = glm::vec3(startX + i * xSpacing + randomXOffset, randomY, 0.0f);
        bird.color = glm::vec3(1.0f, 1.0f, 1.0f);
        bird.direction = glm::vec3(0.01f, 0.0f, 0.0f);

        // random texture at start
        bird.frameOffset = rand() % 3;

        flock.push_back(bird);
    }
}

void CreateShaders(void) {
    ProgramId = LoadShaders("shader.vert", "shader.frag");
    glUseProgram(ProgramId);
}

void CreateVBO(void) {
    static const GLfloat Vertices[] = {
        // pos                // texture coord
        -3.5f, -3.5f, 0.0f,   0.0f, 0.0f,  // bot-left
         3.5f, -3.5f, 0.0f,   1.0f, 0.0f,  // bot-right
         3.5f,  3.5f, 0.0f,   1.0f, 1.0f,  // top-right
        -3.5f,  3.5f, 0.0f,   0.0f, 1.0f   // top-left
    };

    static const GLuint Indices[] = {
        0, 1, 2,   
        2, 3, 0    
    };

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

void DestroyShaders(void) {
    glDeleteProgram(ProgramId);
}

void DestroyVBO(void) {
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VboId);
    glDeleteBuffers(1, &EboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void Cleanup(void) {
    DestroyShaders();
    DestroyVBO();
    glDeleteTextures(3, textures); // del textures
}

void Initialize(void) {
    glClearColor(0.91f, 0.95f, 0.96f, 1.0f);  // background
    glEnable(GL_BLEND);                        // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // blend function for transparency

    CreateFlock(20);  // create flock
    CreateVBO();

    // load texture
    textures[0] = LoadTexture("bird_1.png");
    textures[1] = LoadTexture("bird_2.png");
    textures[2] = LoadTexture("bird_3.png");

    CreateShaders();
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
}

void UpdateFlock() {
    for (auto& bird : flock) {
        bird.position += bird.direction;

        // repeat from left
        if (bird.position.x > xMax) {
            bird.position.x = xMin;
        }
    }
}

void RenderFunction(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    UpdateFlock();

    static int frameCounter = 0;

    for (const auto& bird : flock) {
        int textureIndex = (frameCounter / 800 + bird.frameOffset) % 3; // bird start with frameOffset for difference

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);  // bind texture loaded
        glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

        myMatrix = glm::translate(resizeMatrix, bird.position);
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // draw bird quad
    }

    glBindTexture(GL_TEXTURE_2D, 0);  
    glFlush();

    frameCounter++;
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Single Flock of Birds - OpenGL");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutIdleFunc(RenderFunction); 
    glutCloseFunc(Cleanup);

    glutMainLoop();
    return 0;
}
