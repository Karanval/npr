#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "primitives.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsequenced"
// structure to hold render info
// -----------------------------
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices);
void bindParticleAttributes();
void bindLineAttributes();
void setup();
void setupParticles();
void setupLines();
void drawObjects();
void drawParticles(float delta);
void drawLines(float delta, glm::mat4 viewProjection);

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
const unsigned int sizeOfFloat = 4;
unsigned int particlesVAO, particlesVBO;
unsigned int linesVAO, linesVBO;
const unsigned int particlesAmount = 65536; // # of particles apparently
const unsigned int linesAmount = 3000; // # of lines apparently
const unsigned int particleSize = 3; //TODO upt when there is movement
Shader* shaderProgram;
Shader* shaderProgramParticles;
Shader* shaderProgramLines;
bool particlesSelected = true;

float boxSize = 10;
bool raining = true;
glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 speed = glm::vec3(0.0f, -1.0f, 0.0f);
//RAIN
glm::vec3 gravityRAIN = glm::vec3(0.0f, -3.0f, 0.0f);
glm::vec3 windRAIN = glm::vec3(1.0f, 0.0f, 1.0f);
//SNOW
glm::vec3 gravitySNOW = glm::vec3(0.0f, -.1f, 0.0f);
glm::vec3 windSNOW = glm::vec3(.5f, 0.0f, .1f);
std::vector<glm::vec3> randos= {glm::vec3(((float) rand() / RAND_MAX), 0.0f, ((float) rand() / RAND_MAX)),
                                glm::vec3(((float)rand() / RAND_MAX), 0.0f,((float)rand() / RAND_MAX))};

// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;


glm::mat4 prevViewProjection = glm::mat4(1.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 4.6", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects
    // ---------------------------------------
    setup();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    double lastTime = glfwGetTime();
    double deltaTime, nowTime;

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        nowTime = glfwGetTime();
        deltaTime = (nowTime - lastTime);
        lastTime = nowTime;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram->use();
        drawObjects();

        if (particlesSelected) {
            // render particles
            shaderProgramParticles->use();
            drawParticles(deltaTime);
        } else {
            // render lines
            glm::mat4 projection = glm::perspectiveFovRH_NO(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
            glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
            glm::mat4 viewProjection = projection * view;
            shaderProgramLines->use();
            drawLines(deltaTime, viewProjection);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgram;
    delete shaderProgramParticles;
    delete shaderProgramLines;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawObjects(){

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // update the camera pose and projection
    // set the matrix that takes points in the world coordinate system and project them
    // world_to_view -> view_to_perspective_projection
    // or if we want ot match the multiplication order, we could read
    // perspective_projection_from_view <- view_from_world
    glm::mat4 projection = glm::perspectiveFovRH_NO(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    // draw floor (the floor was built so that it does not need to be transformed)
    shaderProgram->setMat4("model", viewProjection);
    floorObj.drawSceneObject();

    // draw 2 cubes and 2 planes in different location and with different orientations
    drawCube(viewProjection * glm::translate(2.0f, 1.f, 2.0f) * glm::rotateY(glm::half_pi<float>()) * scale);
    drawCube(viewProjection * glm::translate(-2.0f, 1.f, -2.0f) * glm::rotateY(glm::quarter_pi<float>()) * scale);

}

void drawCube(glm::mat4 model){
    // draw object
    shaderProgram->setMat4("model", model);
    cube.drawSceneObject();
}

void drawParticles(float delta) {
    glm::mat4 projection = glm::perspectiveFovRH_NO(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;
    shaderProgramParticles->setMat4("cameraMatrixView", viewProjection);
    shaderProgramParticles->setFloat("boxSize", boxSize);
    shaderProgramParticles->setVec3("cameraPos", camPosition);
    shaderProgramParticles->setVec3("cameraFor", camForward);

    if (raining) {
        shaderProgramParticles->setBool("rain", true);
        speed += (gravityRAIN + windRAIN) * delta;
    } else {
        shaderProgramParticles->setBool("rain", false);
        speed += (gravitySNOW + windSNOW) * delta;
    }
    offsets = speed; //+ randOffset;
    offsets -= camPosition + camForward + boxSize/2;
    offsets = glm::mod( offsets, boxSize);
//
    shaderProgramParticles->setVec3("offsets", offsets);

    glBindVertexArray(particlesVAO);
    glDrawArrays(GL_POINTS, 0, particlesAmount);
}

void drawLines(float delta, glm::mat4 viewProjection){

    shaderProgramLines->setMat4("cameraMatrixView", viewProjection);
    shaderProgramLines->setFloat("boxSize", boxSize);
    shaderProgramLines->setVec3("cameraPos", camPosition);
    shaderProgramLines->setVec3("cameraFor", camForward);
    shaderProgramLines->setVec3("g_vVelocity", gravitySNOW);
    shaderProgramLines->setVec3("g_fHeightScale", glm::vec3(0,1,0));
    shaderProgramLines->setMat4("g_mViewProjPrev", prevViewProjection);
    prevViewProjection = viewProjection;

    if (raining) {
        speed += (gravityRAIN + windRAIN) * delta;
    } else {
        speed += (gravitySNOW + windSNOW) * delta;
    }

    offsets = speed;// + randOffset;
    offsets -= camPosition + camForward + boxSize/2;
    offsets = glm::mod( offsets, boxSize);
//
    shaderProgramLines->setVec3("offsets", offsets);

    glBindVertexArray(linesVAO);
   // glDrawElements(GL_LINES, linesAmount*particleSize*2, GL_UNSIGNED_INT, 0);
    //std::cout<< "Attempting draw" << std::endl;
    glDrawArrays(GL_LINES, 0, linesAmount*particleSize*2);
}

void setup(){
    // initialize shaders
    shaderProgram = new Shader("shader.vert", "shader.frag");
    shaderProgramParticles = new Shader("particleShader.vert", "particleShader.frag");
    shaderProgramLines = new Shader("lineShader.vert", "lineShader.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();

    // load particles mesh into openGL
    setupParticles();
    setupLines();
}

void setupParticles() { // create Vertex Buffer Object
    glGenVertexArrays(1, &particlesVAO);
    glGenBuffers(1, &particlesVBO);

    glBindVertexArray(particlesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(particlesAmount * particleSize);
    for(unsigned int i = 0; i < particlesAmount; i++) {
        //((b - a) * ((float)rand() / RAND_MAX)) + a;
        data[i] = boxSize * ((float)rand() / RAND_MAX);
        data[++i] = boxSize * ((float)rand() / RAND_MAX);
        data[++i] = boxSize * ((float)rand() / RAND_MAX);
    }

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, particlesAmount * particleSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);
    bindParticleAttributes();
}

void setupLines() {
    glGenVertexArrays(1, &linesVAO);
    glGenBuffers(1, &linesVBO);
   // unsigned int EBO;
   // glGenBuffers(1, &EBO);

    glBindVertexArray(linesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(linesAmount * particleSize * 2);
    for(unsigned int i = 0; i < (linesAmount * particleSize * 2); i++) {
        //((b - a) * ((float)rand() / RAND_MAX)) + a;
        float a = data[i] = boxSize * ((float)rand() / RAND_MAX);
        float b = data[++i] = boxSize * ((float)rand() / RAND_MAX);
        float c = data[++i] = boxSize * ((float)rand() / RAND_MAX);
        data[++i] = a;
        data[++i] = b;
        data[++i] = c;
    }
//    data = {-.5f, -.50f, .50f,
//            .5f, -.50f, .50f,
//            .5f, .50f, .50f,
//            -.5f, .50f, .50f,
//            -.5f, -.50f, -.50f,
//            .5f, -.50f, -.50f,
//            .5f, .50f, -.5f,
//            -.5f, .5f, -.5f};
//    std::vector<unsigned int> indices;
//    for(unsigned int i = 1; i <= linesAmount*2; i++) {
//        indices.push_back(i);
  //  }
//    indices = {1,2,3,4,5,6,7,8};

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_DYNAMIC_DRAW);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
    bindLineAttributes();
}

void bindParticleAttributes(){
    int posSize = 3; // each position has x,y,z
    GLuint vertexLocation = glGetAttribLocation(shaderProgramParticles->ID, "pos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, 0);
}

void bindLineAttributes() {
    int posSize = 3; // each position has x,y,z
    GLuint posLocation = glGetAttribLocation(shaderProgramLines->ID, "pos");
    glEnableVertexAttribArray(posLocation);
    glVertexAttribPointer(posLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, 0);
}

unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgram->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}

unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}

unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    // rotate the camera position based on mouse movements
    // if you decide to use the lookAt function, make sure that the up vector and the
    // vector from the camera position to the lookAt target are not collinear
    static float rotationAroundVertical = 0;
    static float rotationAroundLateral = 0;

    int screenW, screenH;

    // get cursor position and scale it down to a smaller range
    glfwGetWindowSize(window, &screenW, &screenH);
    glm::vec2 cursorPosition(0.0f);
    cursorInRange(posX, posY, screenW, screenH, -1.0f, 1.0f, cursorPosition.x, cursorPosition.y);

    // initialize with first value so that there is no jump at startup
    static glm::vec2 lastCursorPosition = cursorPosition;

    // compute the cursor position change
    auto positionDiff = cursorPosition - lastCursorPosition;

    // require a minimum threshold to rotate
    if (glm::dot(positionDiff, positionDiff) > 1e-5f){
        // rotate the forward vector around the Y axis, notices that w is set to 0 since it is a vector
        rotationAroundVertical += glm::radians(-positionDiff.x * rotationGain);
        camForward = glm::rotateY(rotationAroundVertical) * glm::vec4(0,0,-1,0);
        // rotate the forward vector around the lateral axis
        rotationAroundLateral +=  glm::radians(positionDiff.y * rotationGain);
        // we need to clamp the range of the rotation, otherwise forward and Y axes get parallel
        rotationAroundLateral = glm::clamp(rotationAroundLateral, -glm::half_pi<float>() * 0.9f, glm::half_pi<float>() * 0.9f);
        glm::vec3 lateralAxis = glm::cross(camForward, glm::vec3(0, 1,0));
        camForward = glm::rotate(rotationAroundLateral, lateralAxis) * glm::rotateY(rotationAroundVertical) * glm::vec4(0,0,-1,0);
        camForward = glm::normalize(camForward);

        // save current cursor position
        lastCursorPosition = cursorPosition;
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // move the camera position based on keys pressed (use either WASD or the arrow keys)
    // camera forward in the XZ plane
    glm::vec3 forwardInXZ = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camPosition += forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camPosition -= forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        // vector perpendicular to camera forward and Y-axis
        camPosition -= glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        // vector perpendicular to camera forward and Y-axis
        camPosition += glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        // use snow or rain
        raining = !raining;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
        particlesSelected = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
        particlesSelected = false;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
#pragma clang diagnostic pop