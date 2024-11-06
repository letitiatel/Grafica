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
#include <cmath>

GLuint VaoId, VboId, EboId, ProgramId, myMatrixLocation;
GLuint textures[3];
GLuint treeTextures[6];
GLuint cloudTextures[3];
GLfloat winWidth = 800, winHeight = 600;
glm::mat4 myMatrix, resizeMatrix;
float xMin = -80, xMax = 80.f, yMin = -60.f, yMax = 60.f;
GLfloat M_PI = 3.141592653;

// pasari
struct Bird {
    glm::vec3 position;
    glm::vec3 direction;
    int frameOffset;
};
std::vector<Bird> flock;

// copaci
struct Tree {
    glm::vec3 position;
    int textureIndex;
};
std::vector<Tree> trees;

// nori
struct Cloud {
    glm::vec3 position;
    int textureIndex;
};
std::vector<Cloud> clouds;

// parametrii cercului format de pasari
const glm::vec3 circleCenter(0.0f, 0.0f, 0.0f);
const float circleRadius = 30.0f;
const float separationDistance = 10.0f; // distanta dintre pasari

bool formingCircle = false;
float timeToStartCircle = 500.0f;


GLuint LoadTexture(const char* texturePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height;
    unsigned char* image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGBA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void CreateFlock(int numBirds) {
    srand(static_cast<unsigned>(time(0))); // random start

    float startx = -80.0f;
    float space = 5.0f;

    for (int i = 0; i < numBirds; ++i) {
        Bird bird;

        // random y si x
        float randomY = -20.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 40.0f));
        float randomoffsetX = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f)) - 1.0f;

        bird.position = glm::vec3(startx + i * space + randomoffsetX, randomY, 0.0f);
        bird.direction = glm::vec3(0.01f, 0.0f, 0.0f);
        // fiecare pasare incepe cu o textura random la inceput ca sa dea impresia de diferenta in zbor
        bird.frameOffset = rand() % 3;

        flock.push_back(bird);
    }
}

void CreateTrees() {
    float startx = -70.0f;
    float space = 10.0f; // distanta dintre copaci

    for (int i = 0; i < 6; ++i) {
        Tree tree;
        tree.position = glm::vec3(startx + i * space, yMin + 10.0f, 0.0f);
        tree.textureIndex = i;
        trees.push_back(tree);
    }
}


void CreateClouds() {
    float startX = -50.0f;
    float space = 50.0f; // distanta dintre nori

    for (int i = 0; i < 3; ++i) {
        Cloud cloud;
        cloud.position = glm::vec3(startX + i * space, 40.0f, 0.0f);  // pozitie deasupra copacilor
        cloud.textureIndex = 0;
        clouds.push_back(cloud);
    }
}


void CreateShaders(void) {
    ProgramId = LoadShaders("shader.vert", "shader.frag");
    glUseProgram(ProgramId);
}

void CreateVBO(void) {

    // vertices pasari
    static const GLfloat Vertices[] = {
        // pos                // texture coord
        -3.5f, -3.5f, 0.0f,   0.0f, 0.0f,  // stanga jos
         3.5f, -3.5f, 0.0f,   1.0f, 0.0f,  // dreapta jos
         3.5f,  3.5f, 0.0f,   1.0f, 1.0f,  // dreapta sus
        -3.5f,  3.5f, 0.0f,   0.0f, 1.0f   // stanga sus
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
    glClearColor(0.91f, 0.95f, 0.96f, 1.0f);  // culoare background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // pentru transparenta

    // creare pasari, copaci, nori
    CreateFlock(20);
    CreateTrees();
    CreateClouds();
    CreateVBO();

    // texturi pasari, copaci, nori
    textures[0] = LoadTexture("bird_1.png");
    textures[1] = LoadTexture("bird_2.png");
    textures[2] = LoadTexture("bird_3.png");

    treeTextures[0] = LoadTexture("tree_1.png");
    treeTextures[1] = LoadTexture("tree_2.png");
    treeTextures[2] = LoadTexture("tree_3.png");
    treeTextures[3] = LoadTexture("tree_4.png");
    treeTextures[4] = LoadTexture("trees_5.png");
    treeTextures[5] = LoadTexture("rock_1.png");

    cloudTextures[0] = LoadTexture("cloud.png");

    CreateShaders();
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
}

void UpdateFlock(float deltaTime) {
    if (formingCircle) {
        float space = 2.0f * M_PI / flock.size(); // ca sa spatiem pasarile in mod egal

        for (size_t i = 0; i < flock.size(); ++i) {
            float targetAngle = space * i + glutGet(GLUT_ELAPSED_TIME) * 0.001f;
            glm::vec3 targetPosition = circleCenter + glm::vec3(cos(targetAngle) * circleRadius, sin(targetAngle) * circleRadius, 0.0f);

            glm::vec3 direction = glm::normalize(targetPosition - flock[i].position);
            flock[i].position += direction * 0.1f * deltaTime;

            // daca pasarea ajunge la o distanta prea mica, ii modificam pozitia
            for (size_t j = 0; j < flock.size(); ++j) {
                if (i != j) {
                    float distance = glm::distance(flock[i].position, flock[j].position);
                    if (distance < separationDistance) {
                        glm::vec3 separationDirection = glm::normalize(flock[i].position - flock[j].position);
                        flock[i].position += separationDirection * (separationDistance - distance);
                    }
                }
            }
        }
    }
    else {
        for (auto& bird : flock) {
            bird.position += bird.direction;

            if (bird.position.x > xMax) {
                bird.position.x = xMin;
            }
        }
    }
}



void RenderTrees() {
    float space = 45.0f;
    float startx = -space * (6 - 1) / 2.0f;

    for (int i = 0; i < 6; ++i) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, treeTextures[i]);
        glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

        glm::vec3 treePosition(startx + i * space, 5.0f, 0.0f);

        // translatare apoi scalare
        glm::mat4 translationMatrix = glm::translate(resizeMatrix, treePosition);
        glm::mat4 scaleMatrix = glm::scale(glm::vec3(5.0f, -5.0f, 1.0f));  // marime
        myMatrix = translationMatrix * scaleMatrix;

        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderClouds() {
    for (const auto& cloud : clouds) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cloudTextures[cloud.textureIndex]);
        glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

        glm::vec3 cloudPosition = cloud.position;

        // translatarea apoi scalarea
        glm::mat4 translationMatrix = glm::translate(resizeMatrix, cloudPosition);
        glm::mat4 scaleMatrix = glm::scale(glm::vec3(5.0f, 5.0f, 1.0f));
        myMatrix = translationMatrix * scaleMatrix;

        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void RenderFunction(void) {
    static float startTime = 0.0f;
    startTime += 0.016f; // aprox. 60 frame-uri pe secunda

    if (startTime > timeToStartCircle && !formingCircle) {
        formingCircle = true;
    }

    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);  // fundalul albastru
    glClear(GL_COLOR_BUFFER_BIT);


    UpdateFlock(0.016f);
    RenderClouds();
    RenderTrees();

    // incepem schimbarea texturilor pasarilor (pentru efectul de zbor)

    static int frameCounter = 0;

    for (const auto& bird : flock) {
        int textureIndex = (frameCounter / 800 + bird.frameOffset) % 3; // folosim frameCounter si frameOffset ca sa alegem texturi diferite ale pasarilor

        // bind la textura apoi desenam pasarea

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);
        glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);


        glm::mat4 translationMatrix = glm::translate(resizeMatrix, bird.position);
        glm::mat4 scaleMatrix = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f)); 

        myMatrix = translationMatrix * scaleMatrix;
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
    glutCreateWindow("FLock Of Birds");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutIdleFunc(RenderFunction);
    glutCloseFunc(Cleanup);

    glutMainLoop();
    return 0;
}
