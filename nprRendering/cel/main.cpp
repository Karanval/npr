#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// function declarations //
// --------------------- //
void setCommonUniforms();
void setCelFramebuffer();
void setEdgeFramebuffer();
unsigned int createVAO();
unsigned int createTexture(char const * path);
void drawCar();
void drawCrate();
void drawRobot();
void drawFloor();
void drawGui();

// glfw and input functions //
// ------------------------ //
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings //
// --------------- //
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const glm::vec2 texelSize = {1.0f / SCR_WIDTH, 1.0f /SCR_HEIGHT};
//vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
const float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
};


// global variables used for rendering //
// ----------------------------------- //
Shader* celShader;
Shader* edgeShader;
Shader* screenShader;
Model* carPaint;
Model* carBody;
Model* carInterior;
Model* carLight;
Model* carWindow;
Model* carWheel;
Model* floorModel;
Model* crate;
Model* robot;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

// global variables used for control //
// --------------------------------- //
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false;
// gl object ids //
// ------------- //
unsigned int celFramebuffer, celTexture, celRbo;
unsigned int edgeFramebuffer, edgeTexture, edgeRbo;
unsigned int edgeVAO, screenVAO;

// Helper structs //
// -------------- //
struct Config {

    // ambient light
    glm::vec3 ambientLightColor = {1.0f, 1.0f, 1.0f};
    float ambientLightIntensity = 0.25f;

    // light 1
    glm::vec3 lightPosition = {1.2f, 2.5f, 0.8f};
    glm::vec3 lightColor = {1.0f, 1.0f, 1.0f};
    float lightIntensity = 0.75f;

    // material
    glm::vec3 reflectionColor = {1.0f, 1.0f, 0.0f};
    float ambientReflectance = 0.5f;
    float diffuseReflectance = 0.5f;
    float specularReflectance = 0.7f;
    float specularExponent = 20.0f;

    // attenuation (c0, c1 and c2 on the slides)
    float attenuationC0 = 0.25f;
    float attenuationC1 = 0.1f;
    float attenuationC2 = 0.1f;

    // Image distortion TODO: use
    bool doCelShading = true;
    bool doEdgeDetection = false;
    bool doLineTremor = false;
    float tremorAmount = 0.01; // noiseAmp
    int celAmount = 4;
    bool justLines = false;

} config;


int main()
{
    // glfw: initialize and configure //
    // ------------------------------ //
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation //
    // -------------------- //
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NPR rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
	glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    // glad: load all OpenGL function pointers //
    // --------------------------------------- //
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Initialize scene objects (models and gl) //
    // ---------------------------------------- //
    celShader = new Shader("shaders/celShader.vert", "shaders/celShader.frag");

    edgeShader = new Shader("shaders/edgeShader.vert", "shaders/edgeShader.frag");
	edgeShader->use();
	edgeShader->setInt("celTexture", 0);

	screenShader = new Shader("shaders/screenShader.vert", "shaders/screenShader.frag");
    screenShader->use();
    screenShader->setInt("edgeTexture", 0);
    unsigned int noiseTexture;
    noiseTexture = createTexture("perlinNoise.png");
    screenShader->setInt("noiseTexture", 1);

    carPaint = new Model("car/Paint_LOD0.obj");
	carBody = new Model("car/Body_LOD0.obj");
	carLight = new Model("car/Light_LOD0.obj");
	carInterior = new Model("car/Interior_LOD0.obj");
	carWindow = new Model("car/Windows_LOD0.obj");
	carWheel = new Model("car/Wheel_LOD0.obj");
	floorModel = new Model("floor/floor.obj");
	crate = new Model("box/crate.obj");
	robot  = new Model("robot/RIGING_MODEL_04.obj");

    edgeVAO = createVAO();
    screenVAO = createVAO();
    setCelFramebuffer();
    setEdgeFramebuffer();

    // IMGUI init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // RENDER LOOP //
    // ----------- //
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        setCommonUniforms();
        /// first pass, normal render with cel framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, celFramebuffer);
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);// if active TODO change back
        glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
        glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
        glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        celShader->use();
        drawFloor();
        drawCar();
        drawCrate();
        drawRobot();
        /// second pass, render to texture with edge framebuffer
//        glBindFramebuffer(GL_FRAMEBUFFER, edgeFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);// if active TODO change back
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        edgeShader->use();
        glBindVertexArray(edgeVAO);
        glBindTexture(GL_TEXTURE_2D, celTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /// third pass, render to quad with default framebuffer
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glDisable(GL_DEPTH_TEST);
//        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//        screenShader->use();
//        glBindVertexArray(screenVAO);
//        glBindTexture(GL_TEXTURE_2D, edgeTexture); // also bind noise
//        glBindTexture(GL_TEXTURE_2D, noiseTexture); // also bind noise
//        glDrawArrays(GL_TRIANGLES, 0, 6);

		if (isPaused) {
			drawGui();
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // CLEANUP //
    // ------- //
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	//delete carModel;
	delete floorModel;
	delete carWindow;
	delete carPaint;
	delete carInterior;
	delete carLight;
	delete carBody;
    delete carWheel;
    delete crate;
    delete robot;
    delete celShader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

///////////////////////////
//    SETUP FUNCTIONS    //
///////////////////////////
void setCommonUniforms() {
    celShader->use();
    // light uniforms
//    celShader->setVec3("ambientLightColor", config.ambientLightColor * config.ambientLightIntensity);
    celShader->setVec3("lightPosition", config.lightPosition);
//    celShader->setVec3("lightColor", config.lightColor * config.lightIntensity);

    // material uniforms
//    celShader->setVec3("reflectionColor", config.reflectionColor);
//    celShader->setFloat("ambientReflectance", config.ambientReflectance);
//    celShader->setFloat("diffuseReflectance", config.diffuseReflectance);
//    celShader->setFloat("specularReflectance", config.specularReflectance);
//    celShader->setFloat("specularExponent", config.specularExponent);

    // attenuation uniforms
//    celShader->setFloat("attenuationC0", config.attenuationC0);
//    celShader->setFloat("attenuationC1", config.attenuationC1);
//    celShader->setFloat("attenuationC2", config.attenuationC2);

    // NPR
    celShader->setBool("doCelShading", config.doCelShading);
    celShader->setInt("celAmount", config.celAmount);

    edgeShader->use();
    edgeShader->setBool("doEdgeDetection", config.doEdgeDetection);
    edgeShader->setVec2("texelSize", texelSize);

    screenShader->use();
    screenShader->setBool("doLineTremor", config.doLineTremor);
    screenShader->setFloat("tremorAmount", config.tremorAmount);
}

void setCelFramebuffer() {
    glGenFramebuffers(1, &celFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, celFramebuffer);

    // generate texture to attach
    glGenTextures(1, &celTexture);
    glBindTexture(GL_TEXTURE_2D, celTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // needed?

    // generate renderbuffer for depth and stencil testing
    glGenRenderbuffers(1, &celRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, celRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach texture and rbo to current bound framebuffer obj
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, celTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, celRbo);

    // check that framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::CEL FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
    // unbind, and bind back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setEdgeFramebuffer() {
    glGenFramebuffers(1, &edgeFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, edgeFramebuffer);

    // generate texture to attach
    glGenTextures(1, &edgeTexture);
    glBindTexture(GL_TEXTURE_2D, edgeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // needed?

    // generate renderbuffer for depth and stencil testing
    glGenRenderbuffers(1, &edgeRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, edgeRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach texture and rbo to current bound framebuffer obj
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, edgeTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, edgeRbo);

    // check that framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::EDGE FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
    // unbind, and bind back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int createVAO() {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // setting attributes position and texCoord
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    return VAO;
}

unsigned int createTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "ERROR::LOAD TEXTURE DATA:: failed to load texture data at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

///////////////////////////
//    DRAW FUNCTIONS     //
///////////////////////////
void drawGui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings");

        ImGui::Text("Ambient light");
        ImGui::ColorEdit3("Ambient light color", (float*)&config.ambientLightColor);
        ImGui::SliderFloat("Ambient light intensity", &config.ambientLightIntensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Light");
        ImGui::DragFloat3("Light position", (float*)&config.lightPosition, 0.1f, -20.0f, 20.0f);
        ImGui::ColorEdit3("Light color", (float*)&config.lightColor);
        ImGui::SliderFloat("Light intensity", &config.lightIntensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Material");
        ImGui::ColorEdit3("Reflection color", (float*)&config.reflectionColor);
        ImGui::SliderFloat("Ambient reflectance", &config.ambientReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse reflectance", &config.diffuseReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular reflectance", &config.specularReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("specular exponent", &config.specularExponent, 0.1f, 300.0f);
        ImGui::Separator();

        ImGui::Text("Attenuation");
        ImGui::SliderFloat("Attenuation c0", &config.attenuationC0, 0.0f, 1.0f);
        ImGui::SliderFloat("Attenuation c1", &config.attenuationC1, 0.0f, 1.0f);
        ImGui::SliderFloat("Attenuation c2", &config.attenuationC2, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("NPR");
        ImGui::Checkbox("Do cel shading", &config.doCelShading);
        ImGui::Checkbox("Do edge detection", &config.doEdgeDetection);
        ImGui::Checkbox("Do line tremor", &config.doLineTremor);
        ImGui::SliderInt("Cel shading divisions", &config.celAmount, 3, 20);
        ImGui::SliderFloat("Line tremor amount", &config.tremorAmount, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawFloor(){
    celShader->use();
    celShader->setBool("applyDeformations", false);
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

    // set projection matrix uniform
    celShader->setMat4("projection", projection);
    celShader->setMat4("view", view);

    // draw floor,
    // notice that we overwrite the value of one of the uniform variables to set a different floor color
    celShader->setVec3("reflectionColor", .2, .5, .2);
    glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(5.f, 5.f, 5.f));
    celShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    floorModel->Draw(*celShader);
}

void drawCar(){
    celShader->use();
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewInv = glm::inverse(view);
    glm::mat4 viewProjection = projection * view;
    // set projection matrix uniform
    celShader->setMat4("projection", projection);
    celShader->setMat4("view", view);

    // draw wheel
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432, .328, 1.39));
    celShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    carWheel->Draw(*celShader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432, .328, -1.296));
    celShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    carWheel->Draw(*celShader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432, .328, 1.296));
    celShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    carWheel->Draw(*celShader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432, .328, -1.39));
    celShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    carWheel->Draw(*celShader);

    // draw the rest of the car
    model = glm::mat4(1.0f);
    celShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    carBody->Draw(*celShader);
    carInterior->Draw(*celShader);
    carPaint->Draw(*celShader);
    carLight->Draw(*celShader);
    glEnable(GL_BLEND);
    carWindow->Draw(*celShader);
    glDisable(GL_BLEND);

}

void drawCrate() {
    celShader->use();
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewInv = glm::inverse(view);
    glm::mat4 viewProjection = projection * view;
    // set projection matrix uniform
    celShader->setMat4("projection", projection);
    celShader->setMat4("view", view);
    celShader->setMat4("viewInv", viewInv);


    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(3, 1, 1.39));
    celShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    crate->Draw(*celShader);
}

void drawRobot() {
    celShader->use();
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewInv = glm::inverse(view);
    glm::mat4 viewProjection = projection * view;
    // set projection matrix uniform
    celShader->setMat4("projection", projection);
    celShader->setMat4("view", view);
    celShader->setMat4("viewInv", viewInv);


    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0.28, 1.39));
    celShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    celShader->setMat4("invTranspMV", invTranspose);
    robot->Draw(*celShader);
}

// ---------------
// INPUT FUNCTIONS
// ---------------

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

	if (isPaused)
		return;

	// movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}


void cursor_input_callback(GLFWwindow* window, double posX, double posY){

	// camera rotation
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;

	if (isPaused)
		return;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods){

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}