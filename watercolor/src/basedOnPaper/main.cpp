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

// function declarations
// ---------------------
void setCommonUniforms();
void drawCar();
void drawCrate();
void drawRobot();
void drawFloor();
void drawGui();

// glfw and input functions
// ------------------------
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const glm::vec2 texelSize = {1.0f / SCR_HEIGHT, 1.0f /SCR_WIDTH};

// global variables used for rendering
// -----------------------------------
Shader* celShader;
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

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open

// parameters that can be set in our GUI
// -------------------------------------
struct Config {

    // ambient light
    bool useLightModel = false;
    glm::vec3 ambientLightColor = {1.0f, 1.0f, 1.0f};
    float ambientLightIntensity = 0.25f;

    // light 1
    glm::vec3 lightPosition = {1.2f, 2.5f, 0.8f};
    glm::vec3 lightColor = {1.0f, 1.0f, 1.0f};
    float lightIntensity = 0.75f;

    // material
    float specularExponent = 80.0f;
    float ambientOcclusionMix = 1.0f;

    // attenuation (c0, c1 and c2 on the slides)
    float attenuationC0 = 0.25f;
    float attenuationC1 = 0.1f;
    float attenuationC2 = 0.1f;

    // TODO exercise 9.2 scale config variable
    float uvScale = 20.0f;


    // floor texture mode
    unsigned int wrapSetting = GL_REPEAT;
    unsigned int minFilterSetting = GL_LINEAR_MIPMAP_LINEAR;
    unsigned int magFilterSetting = GL_LINEAR;

} config;

struct WatercolorConfig {
    glm::vec3 paperColor = {1,1,1};
    // deformations
    bool applyDeformations = true;
    float tremorAmount = 4.0f;
    float tremorFront = 0.4f;
    float tremorSpeed = 10.0f;
    float tremorFrequency = 10.0f;
    // reflectance
    bool applyReflectance = true;
    float dilute = 0.8f;
    float cangiante = 0.2f;
    float diluteArea = 1.0f;
    // turbulence
    bool applyTurbulence = true;
    float turbulenceControl = 0.6;

} watercolorConfig;


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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Watercolor test", NULL, NULL);
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    celShader = new Shader("shaders/shader.vert", "shaders/shader.frag");
	carPaint = new Model("car/Paint_LOD0.obj");
	carBody = new Model("car/Body_LOD0.obj");
	carLight = new Model("car/Light_LOD0.obj");
	carInterior = new Model("car/Interior_LOD0.obj");
	carWindow = new Model("car/Windows_LOD0.obj");
	carWheel = new Model("car/Wheel_LOD0.obj");
	floorModel = new Model("floor/floor.obj");
	crate = new Model("box/crate.obj");
	robot  = new Model("robot/RIGING_MODEL_04.obj");

    // set up the z-buffer
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // IMGUI init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawBuffer( GL_COLOR_ATTACHMENT1 );
    glm::vec3 clearVec( 0.0, 0.0, -1.0f );
// from normalized vector to rgb color; from [-1,1] to [0,1]
    clearVec = (clearVec + glm::vec3(1.0f, 1.0f, 1.0f)) * 0.5f;
    glClearColor( clearVec.x, clearVec.y, clearVec.z, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        celShader->use();
        celShader->setFloat("time", lastFrame);
        celShader->setVec2("texelSize", texelSize);

        setCommonUniforms();
        drawFloor();
        drawCar();
        drawCrate();
        drawRobot();
		if (isPaused) {
			drawGui();
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
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
// TAKE CARE OF UNIFORMS //
///////////////////////////
void setCommonUniforms() {

    // light uniforms
    celShader->setVec3("ambientLightColor", config.ambientLightColor * config.ambientLightIntensity);
    celShader->setVec3("lightPosition", config.lightPosition);
    celShader->setVec3("lightColor", config.lightColor * config.lightIntensity);

    // material uniforms
    celShader->setFloat("ambientOcclusionMix", config.ambientOcclusionMix);
    celShader->setFloat("specularExponent", config.specularExponent);

    // attenuation uniforms
    celShader->setFloat("attenuationC0", config.attenuationC0);
    celShader->setFloat("attenuationC1", config.attenuationC1);
    celShader->setFloat("attenuationC2", config.attenuationC2);

    // Send uvScale uniform
    celShader->setFloat("uvScale", config.uvScale);

    // WATERCOLOR
    // deformations
    celShader->setBool("applyDeformations", watercolorConfig.applyDeformations);
    celShader->setFloat("tremorAmount", watercolorConfig.tremorAmount);
    celShader->setFloat("tremorSpeed", watercolorConfig.tremorSpeed);
    celShader->setFloat("tremorFrequency", watercolorConfig.tremorFrequency);
    celShader->setFloat("tremorFront", watercolorConfig.tremorFront);
    // reflectance
    celShader->setBool("applyReflectance", watercolorConfig.applyReflectance);
    celShader->setFloat("dilution", watercolorConfig.dilute);
    celShader->setFloat("cangiante", watercolorConfig.cangiante);
    celShader->setFloat("diluteArea", watercolorConfig.diluteArea);
    celShader->setVec3("paperColor", watercolorConfig.paperColor);
    // turbulence
    celShader->setBool("applyReflectance", watercolorConfig.applyReflectance);
    celShader->setFloat("turbulenceControl", watercolorConfig.turbulenceControl);
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

        ImGui::Text("Deformations");
        ImGui::Checkbox("Apply deformations", &watercolorConfig.applyDeformations);
        ImGui::SliderFloat("Tremor amount", &watercolorConfig.tremorAmount, 0, 10);
        //ImGui::SliderFloat("Tremor front", &watercolorConfig.tremorFront, 0, 1);
        ImGui::SliderFloat("Tremor speed", &watercolorConfig.tremorSpeed, 0, 100);
        ImGui::SliderFloat("Tremor frequency", &watercolorConfig.tremorFrequency, 0, 100);
        ImGui::SliderFloat("Tremor front", &watercolorConfig.tremorFront, 0, 1);
        ImGui::Separator();

        ImGui::Text("Reflectance");
        ImGui::Checkbox("Apply reflectance", &watercolorConfig.applyReflectance);
        ImGui::SliderFloat("Dilute", &watercolorConfig.dilute, 0, 1);
        ImGui::SliderFloat("Cangiante", &watercolorConfig.cangiante, 0, 1);
        ImGui::SliderFloat("Dilute area", &watercolorConfig.diluteArea, 0, 1);
        ImGui::ColorEdit3("Paper color", (float*)&watercolorConfig.paperColor);
//        ImGui::Separator();
        ImGui::Text("Turbulence");
        ImGui::Checkbox("Apply turbulence", &watercolorConfig.applyTurbulence);
        ImGui::SliderFloat("Turbulence control", &watercolorConfig.turbulenceControl, 0.01, 0.99);
        ImGui::Separator();

        ImGui::Text("Ambient light: ");
        ImGui::Checkbox("Use light model", &config.useLightModel);
        ImGui::ColorEdit3("ambient light color", (float*)&config.ambientLightColor);
        ImGui::SliderFloat("ambient light intensity", &config.ambientLightIntensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Light 1: ");
        ImGui::DragFloat3("light 1 position", (float*)&config.lightPosition, 0.1f, -20.0f, 20.0f);
        ImGui::ColorEdit3("light 1 color", (float*)&config.lightColor);
        ImGui::SliderFloat("light 1 intensity", &config.lightIntensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Material: ");
        ImGui::SliderFloat("ambient occlusion mix", &config.ambientOcclusionMix, 0.0f, 1.0f);
        ImGui::SliderFloat("specular exponent", &config.specularExponent, 0.1f, 300.0f);
        ImGui::Separator();

        ImGui::Text("Attenuation: ");
        ImGui::SliderFloat("attenuation c0", &config.attenuationC0, 0.0f, 1.0f);
        ImGui::SliderFloat("attenuation c1", &config.attenuationC1, 0.0f, 1.0f);
        ImGui::SliderFloat("attenuation c2", &config.attenuationC2, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::SliderFloat("uv scale", &config.uvScale, 1.0f, 100.0f);
        ImGui::Separator();

        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawFloor(){

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
    celShader->setBool("applyDeformations", watercolorConfig.applyDeformations);
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewInv = glm::inverse(view);
    glm::mat4 viewProjection = projection * view;
    // set projection matrix uniform
    celShader->setMat4("projection", projection);
    celShader->setMat4("view", view);
    celShader->setMat4("viewInv", viewInv);

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