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

using namespace glm;

// function declarations
// ---------------------
void loadFloorTexture();
void drawCar();
void drawFloor();
void drawGui();


// Screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Helper structs
// --------------
struct LightOut {
    vec3 specular;
    vec3 lightColor;
    vec3 dilute;
    float shade;
};

struct LightConfig {
    bool enabled = false;
    int type = 2;
    glm::vec3 position = {100.0f, 100.0f, 100.0f};
    glm::vec3 color = {1, 1, 1};
    float intensity = 1;
    glm::vec3 direction = {100.0f, 100.0f, 100.0f};
    float coneAngle = 0.46f;
    float fallOff = 0.7f;
    float attenuationScale = 0;
    bool shadowOn = true;
    bool specular = true;
    glm::mat4 matrix;
};
LightConfig light1;
LightConfig light2;
LightConfig light3;

struct WatercolorConfig {
    // wet in wet and hand tremor deformations
    float tremor = 4.0f;
    float tremorFront = 0.4f;
    float tremorSpeed = 10.0f;
    float tremorFrequency = 10.0f;
    // painterly-like
    float diffuseFactor = 0.2f;
    glm::vec3 shadeColor = {0,0,0};
    float shadeWrap = 0;
    bool useOverrideShade = true;
    float dilute = 0.8f;
    float cangiante = 0.2f;
    float diluteArea = 1.0f;
    float highArea = 0;
    float highTransparency = 0;
    // Dark edged
    float darkEdges = 0;
    // Paper color
    glm::vec3 paperColor = {1,1,1};
    float bleedOffset = 0.5;

} watercolorConfig;

struct ShadingConfig {
    // basic group
    bool useColorTexture = false;
    glm::vec3 colorTint = {1,1,1};
    // normal group
    bool useNormalTexture = false;
    bool flipU = false;
    bool flipV = false;
    float bumpDepth = 1.0f; // -2 to 2 by 0.1
    // specular group
    bool useSpecularTexture = false;
    float specular = 0;// 0 to 1 by 0.05
    float specularDiffusion = 0;//0 to 0.99 by 0.05
    float specularTransparency = 0; // 0 to 1 by 0.05
    //shade group
    bool useShadows = false;
    float shadowDepthBias =(float) 0.001;
}shadingConfig;

struct GnralConfig {
    bool useControl = true;
    vec3 atmosphereColor = {1.0f, 1.0f, 1.0f};
    float atmRangeStart = 50.0f;
    float atmRangeEnd = 100.0f;
    vec2 texel = {1.0f / SCR_HEIGHT, 1.0f /SCR_WIDTH};

}gnralConfig;

// glfw and input functions
// ------------------------
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// function declarations
// ---------------------
void setUniforms();
void drawObjects();
void drawCar();
void drawGui();
float getLightConeAngle(float coneAngle, float coneFallOff, glm::vec3 lightVec, glm::vec3 lightDir);
LightOut calculateLight(int lightNo, vec3 worldVectorPosition, vec3 normalWorld, vec3 viewDir);
vec2 calcVelocity(vec2 currentPos, vec2 prevPos, float zOverW);


// global variables used for rendering
// -----------------------------------
Shader* watercolorShader;
Model* carPaint;
Model* carBody;
Model* carInterior;
Model* carLight;
Model* carWindow;
Model* carWheel;
Model* floorModel;
Model* robotModel;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open

/////////////////////////////////
//      Init & Render loop     //
/////////////////////////////////
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 9", NULL, NULL);
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

    watercolorShader = new Shader("shaders/shader.vert", "shaders/shader.frag");
	carPaint = new Model("car/Paint_LOD0.obj");
	carBody = new Model("car/Body_LOD0.obj");
	carLight = new Model("car/Light_LOD0.obj");
	carInterior = new Model("car/Interior_LOD0.obj");
	carWindow = new Model("car/Windows_LOD0.obj");
	carWheel = new Model("car/Wheel_LOD0.obj");
	floorModel = new Model("floor/floor_no_material.obj");

    // Set light 2 and 3 variables
    // ---------------------------
    light2.position = {-100.0f, 100.0f, 100.0f};
    light3.position = {100.0f, 100.0f, -100.0f};
    light2.coneAngle = 45.0f;
    light3.coneAngle = 45.0f;
    light2.fallOff = 0;
    light3.fallOff = 0;

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

        watercolorShader->use();
        setUniforms();
        drawFloor();
        drawCar();
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

	delete floorModel;
	delete carWindow;
	delete carPaint;
	delete carInterior;
	delete carLight;
	delete carBody;
    delete carWheel;
    delete robotModel;
    delete watercolorShader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void setUniforms(){
    //USED ON VERTEX
    // Watercolor in vertex
    watercolorShader->setFloat("bleedOffset", watercolorConfig.bleedOffset);
    watercolorShader->setFloat("tremorFront", watercolorConfig.tremorFront);
    watercolorShader->setFloat("tremorSpeed", watercolorConfig.tremorSpeed);
    watercolorShader->setFloat("tremorFreq", watercolorConfig.tremorFrequency);
    watercolorShader->setFloat("tremor", watercolorConfig.tremor);
    watercolorShader->setFloat("diffuseFactor", watercolorConfig.diffuseFactor);
    watercolorShader->setFloat("diluteArea", watercolorConfig.diluteArea);
    watercolorShader->setFloat("shaderWrap", watercolorConfig.shadeWrap);
    watercolorShader->setFloat("darkEdges", watercolorConfig.darkEdges);
    watercolorShader->setFloat("timer", deltaTime);
    watercolorShader->setVec2("texel", gnralConfig.texel);
    // LIGHTS
    watercolorShader->setBool("l1enabled",light1.enabled);
    watercolorShader->setInt("l1type", light1.type);
    watercolorShader->setVec3("l1pos", light1.position);
    watercolorShader->setVec3("l1color", light1.color);
    watercolorShader->setFloat("l1intensity", light1.intensity);
    watercolorShader->setVec3("l1direction", light1.direction);
    watercolorShader->setFloat("l1coneAngle", light1.coneAngle);
    watercolorShader->setFloat("l1fallOff", light1.fallOff);
    watercolorShader->setFloat("l1attenuationScale", light1.attenuationScale);
    watercolorShader->setMat4("l1matrix", light1.matrix);
    watercolorShader->setBool("l1UseSpecular", light1.specular);
    watercolorShader->setBool("l2enabled",light2.enabled);
    watercolorShader->setInt("l2type", light2.type);
    watercolorShader->setVec3("l2pos", light2.position);
    watercolorShader->setVec3("l2color", light2.color);
    watercolorShader->setFloat("l2intensity", light2.intensity);
    watercolorShader->setVec3("l2direction", light2.direction);
    watercolorShader->setFloat("l2coneAngle", light2.coneAngle);
    watercolorShader->setFloat("l2fallOff", light2.fallOff);
    watercolorShader->setFloat("l2attenuationScale", light2.attenuationScale);
    watercolorShader->setMat4("l2matrix", light2.matrix);
    watercolorShader->setBool("l2UseSpecular", light2.specular);
    watercolorShader->setBool("l3enabled",light3.enabled);
    watercolorShader->setInt("l3type", light3.type);
    watercolorShader->setVec3("l3pos", light3.position);
    watercolorShader->setVec3("l3color", light3.color);
    watercolorShader->setFloat("l3intensity", light3.intensity);
    watercolorShader->setVec3("l3direction", light3.direction);
    watercolorShader->setFloat("l3coneAngle", light3.coneAngle);
    watercolorShader->setFloat("l3fallOff", light3.fallOff);
    watercolorShader->setFloat("l3attenuationScale", light3.attenuationScale);
    watercolorShader->setMat4("l3matrix", light3.matrix);
    watercolorShader->setBool("l3UseSpecular", light3.specular);
    // SHADING
    watercolorShader->setFloat("specular", shadingConfig.specular);
    watercolorShader->setFloat("specDiffusion", shadingConfig.specularDiffusion);
    watercolorShader->setFloat("specTransparency", shadingConfig.specularTransparency);

    // USED ON FRAGMENT SHADER
    //WATERCOLOR
    watercolorShader->setFloat("dilute", watercolorConfig.dilute);
    watercolorShader->setFloat("cangiante", watercolorConfig.cangiante);
    watercolorShader->setVec3("paperColor", watercolorConfig.paperColor);
    watercolorShader->setFloat("highArea", watercolorConfig.highArea);
    watercolorShader->setFloat("highTransparency", watercolorConfig.highTransparency);
    //watercolorShader->setFloat("darkEdges", watercolorConfig.darkEdges);
    watercolorShader->setBool("useOverrideShade", watercolorConfig.useOverrideShade);
    watercolorShader->setVec3("shadeColor", watercolorConfig.shadeColor);
    //watercolorShader->setFloat("diffuseFactor", watercolorConfig.diffuseFactor);
    // gnral
    watercolorShader->setVec3("atmosphereColor", gnralConfig.atmosphereColor);
    watercolorShader->setFloat("rangeStart", gnralConfig.atmRangeStart);
    watercolorShader->setFloat("rangeEnd", gnralConfig.atmRangeEnd);
    // SHADE
    watercolorShader->setBool("useNormalMapping", shadingConfig.useNormalTexture);
    watercolorShader->setBool("useSpecularMapping", shadingConfig.useSpecularTexture);
    watercolorShader->setBool("useColorMapping", shadingConfig.useColorTexture);
    watercolorShader->setBool("flipU", shadingConfig.flipU);
    watercolorShader->setBool("flipV", shadingConfig.flipV);
    watercolorShader->setFloat("bumpDepth", shadingConfig.bumpDepth);
    watercolorShader->setVec3("colorTint", shadingConfig.colorTint);

    // CONTROL
    watercolorShader->setVec3("inColor0", vec3{1});
    watercolorShader->setVec3("inColor1", vec3{1});
    watercolorShader->setVec3("inColor2", vec3{1});
    watercolorShader->setVec3("inColor3", vec3{1});

}

/////////////////////////////////
//      Drawing functions      //
/////////////////////////////////
void drawGui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Configs");
        static int switchTabs = 3;

        if (ImGui::Button("Watercolor", ImVec2(100.0f, 0.0f)))
            switchTabs = 0;
        ImGui::SameLine(0.0, 2.0f);
        if (ImGui::Button("Lights", ImVec2(100.0f, 0.0f)))
            switchTabs = 1;
        ImGui::SameLine(0.0, 2.0f);
        if (ImGui::Button("Shading", ImVec2(100.0f, 0.0f)))
            switchTabs = 2;
        ImGui::SameLine(0.0, 2.0f);
        if (ImGui::Button("General", ImVec2(100.0f, 0.0f)))
            switchTabs = 3;

        switch (switchTabs) {
            case 0:
                // WATERCOLOR
                ImGui::BeginGroup();
                ImGui::Text("Deformations");
                ImGui::SliderFloat("Tremor", &watercolorConfig.tremor, 0, 10);
                ImGui::SliderFloat("Tremor front", &watercolorConfig.tremorFront, 0, 1);
                ImGui::SliderFloat("Tremor speed", &watercolorConfig.tremorSpeed, 0, 100);
                ImGui::SliderFloat("Tremor frequency", &watercolorConfig.tremorFrequency, 0, 100);
                ImGui::Separator();
                ImGui::Text("Effects");
                ImGui::SliderFloat("Diffuse factor", &watercolorConfig.diffuseFactor, 0, 1);
                ImGui::ColorEdit3("Shade color", (float*)&watercolorConfig.shadeColor);
                ImGui::SliderFloat("Shade wrap", &watercolorConfig.shadeWrap, 0, 1);
                ImGui::Checkbox("Use override shade", &watercolorConfig.useOverrideShade);
                ImGui::SliderFloat("Dilute", &watercolorConfig.dilute, 0, 1);
                ImGui::SliderFloat("Cangiante", &watercolorConfig.cangiante, 0, 1);
                ImGui::SliderFloat("Dilute area", &watercolorConfig.diluteArea, 0, 1);
                ImGui::SliderFloat("High area", &watercolorConfig.highArea, 0, 1);
                ImGui::SliderFloat("High transparency", &watercolorConfig.highTransparency, 0, 1);
                ImGui::Separator();
                ImGui::Text("Dark Edges");
                ImGui::SliderFloat("Dark Edges", &watercolorConfig.darkEdges, 0, 1);
                ImGui::Separator();
                ImGui::Text("Paper color");
                ImGui::ColorEdit3("Paper color", (float*)&watercolorConfig.paperColor);
                ImGui::SliderFloat("Bleed offset", &watercolorConfig.bleedOffset, 0, 1);
                ImGui::EndGroup();
                break;
            case 1:
                // LIGHTS
                ImGui::BeginGroup();
                ImGui::Text("Light 1");
                ImGui::Checkbox("Enabled", &light1.enabled);
                ImGui::DragFloat3("position", (float*)&light1.position, 1, -100, 100);
                ImGui::ColorEdit3("color", (float*)&light1.color);
                ImGui::SliderFloat("intensity", &light1.intensity, 0, 100);
                ImGui::DragFloat3("direction", (float*)&light1.direction, 1, -100, 100);
                ImGui::SliderFloat("cone angle", &light1.coneAngle, 0, 3.14159f/2);
                ImGui::SliderFloat("fall off", &light1.fallOff, 0, 3.14159f/2);
                ImGui::SliderFloat("attenuation scale", &light1.attenuationScale, 0, 1000);
                ImGui::Checkbox("Shadow", &light1.shadowOn);
                ImGui::Checkbox("Specular", &light1.specular);
                ImGui::EndGroup();
                ImGui::BeginGroup();
                ImGui::Separator();
                ImGui::Text("Light 2");
                ImGui::Checkbox("Enabled", &light2.enabled);
                ImGui::DragFloat3("position", (float*)&light2.position, 1, -100, 100);
                ImGui::ColorEdit3("color", (float*)&light2.color);
                ImGui::SliderFloat("intensity", &light2.intensity, 0, 100);
                ImGui::DragFloat3("direction", (float*)&light2.direction, 1, -100, 100);
                ImGui::SliderFloat("cone angle", &light2.coneAngle, 0, 3.14159f/2);
                ImGui::SliderFloat("fall off", &light2.fallOff, 0, 3.14159f/2);
                ImGui::SliderFloat("attenuation scale", &light2.attenuationScale, 0, 1000);
                ImGui::Checkbox("Shadow", &light2.shadowOn);
                ImGui::EndGroup();
                ImGui::BeginGroup();
                ImGui::Text("Light 3");
                ImGui::Separator();
                ImGui::Checkbox("Enabled", &light3.enabled);
                ImGui::DragFloat3("position", (float*)&light3.position, 1, -100, 100);
                ImGui::ColorEdit3("color", (float*)&light3.color);
                ImGui::SliderFloat("intensity", &light3.intensity, 0, 100);
                ImGui::DragFloat3("direction", (float*)&light3.direction, 1, -100, 100);
                ImGui::SliderFloat("cone angle", &light3.coneAngle, 0, 3.14159f/2);
                ImGui::SliderFloat("fall off", &light3.fallOff, 0, 3.14159f/2);
                ImGui::SliderFloat("attenuation scale", &light3.attenuationScale, 0, 1000);
                ImGui::Checkbox("Shadow", &light3.shadowOn);
                ImGui::EndGroup();
                break;
            case 2:
                // SHADING
                ImGui::BeginGroup();
                ImGui::Text("Shading");
                ImGui::Text("Basic group");
                ImGui::Checkbox("Use color texture", &shadingConfig.useColorTexture);
                ImGui::ColorEdit3("Color tint", (float*)&shadingConfig.colorTint);
                ImGui::Separator();
                ImGui::Text("Normal group");
                ImGui::Checkbox("Use normal texture", &shadingConfig.useNormalTexture);
                ImGui::Checkbox("Flip U", &shadingConfig.flipU);
                ImGui::Checkbox("Flip V", &shadingConfig.flipV);
                ImGui::SliderFloat("Bump depth", &shadingConfig.bumpDepth, -2, 2);
                ImGui::Separator();
                ImGui::Text("Specular group");
                ImGui::Checkbox("Use specular texture", &shadingConfig.useSpecularTexture);
                ImGui::SliderFloat("Specular", &shadingConfig.specular, 0, 1);
                ImGui::SliderFloat("Specular diffusion", &shadingConfig.specularDiffusion, 0, 0.99f);
                ImGui::SliderFloat("Specular transparency", &shadingConfig.specularTransparency, 0, 1);
                ImGui::Separator();
                ImGui::Checkbox("Use shadows", &shadingConfig.useShadows);
                ImGui::SliderFloat("Shadow Depth Bias", &shadingConfig.shadowDepthBias, 0.0f, 10.0f);
                ImGui::EndGroup();
                break;
            case 3:
                ImGui::BeginGroup();
                ImGui::Checkbox("Use control", &gnralConfig.useControl);
                ImGui::ColorEdit3("Atmosphere color", (float*)&gnralConfig.atmosphereColor);
                ImGui::SliderFloat("Atm range start", &gnralConfig.atmRangeStart, 0.0f, 50000.0f);
                ImGui::SliderFloat("Atm range end", &gnralConfig.atmRangeEnd, 0.0f, 50000.0f);
                ImGui::EndGroup();
                break;
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawFloor(){
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

    // set projection matrix uniform
    watercolorShader->setMat4("projection", projection);
//
//    glActiveTexture(GL_TEXTURE0);
//    watercolorShader->setInt("texture_diffuse1", 0);
//    glBindTexture(GL_TEXTURE_2D, floorTextureId);
    // draw floor,
    // notice that we overwrite the value of one of the uniform variables to set a different floor color
    watercolorShader->setVec3("reflectionColor", .2, .5, .2);
    glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(5.f, 5.f, 5.f));
    watercolorShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    floorModel->Draw(*watercolorShader);
}

void drawCar(){
    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    //glm::mat4 viewProjection = projection * view;

    // set projection matrix uniform
    watercolorShader->setMat4("projection", projection);

    // draw wheel
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432, .328, 1.39));
    watercolorShader->setMat4("model", model);
    glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    carWheel->Draw(*watercolorShader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432, .328, -1.296));
    watercolorShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    carWheel->Draw(*watercolorShader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432, .328, 1.296));
    watercolorShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    carWheel->Draw(*watercolorShader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432, .328, -1.39));
    watercolorShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    carWheel->Draw(*watercolorShader);

    // draw the rest of the car
    model = glm::mat4(1.0f);
    watercolorShader->setMat4("model", model);
    invTranspose = glm::inverse(glm::transpose(view * model));
    watercolorShader->setMat4("invTranspose", invTranspose);
    watercolorShader->setMat4("view", view);
    carBody->Draw(*watercolorShader);
    carInterior->Draw(*watercolorShader);
    carPaint->Draw(*watercolorShader);
    carLight->Draw(*watercolorShader);
    glEnable(GL_BLEND);
    carWindow->Draw(*watercolorShader);
    glDisable(GL_BLEND);

}

/////////////////////////////////
// Based on prototypeC of MNPR //
/////////////////////////////////
float getLightConeAngle(float coneAngle, float coneFallOff, glm::vec3 lightVec, glm::vec3 lightDir) {
    if (coneFallOff< coneAngle)
        coneFallOff = coneAngle;
    float lDotDir = glm::dot(lightVec, lightDir);
    float edge0 = glm::cos(coneFallOff);
    float edge1 = glm::cos(coneAngle);
    // Hermite interpolation // smooth step function
    float cone = glm::clamp((lDotDir - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    cone = cone * cone * (3 - 2 * cone);
    return cone;
}

LightOut calculateLight(int lightNo, vec3 worldVectorPosition, vec3 normalWorld, vec3 viewDir) {
    LightConfig lConfig;
    switch (lightNo) {
        case 1:
            lConfig = light1;
            break;
        case 2:
            lConfig = light2;
            break;
        case 3:
            lConfig = light3;
            break;
    }

    LightOut light;
    light.specular = vec3(0.0f);
    light.lightColor = vec3(0.0f);
    light.dilute = vec3(0.0f);

    if(lConfig.enabled){
        // ambient light // no diffuse, specular nor shadow casting
        if (lConfig.type == 5) {
            light.lightColor = lConfig.color * lConfig.intensity;
            return light;
        }

        // directional light // no position
        bool isDirectionalLight = (lConfig.type == 4);
        vec3 lightVector = mix(lConfig.position - worldVectorPosition, lConfig.direction, isDirectionalLight);
        vec3 normalizedLV = normalize(lightVector);

        // diffuse, dot product
        float nDotL = dot(normalWorld, normalizedLV);

        // Wrapped Lambert
        // Derived from half lambert, presents problems w/ shadow mapping
//        Cg code:
//        float3 result = saturate(texCol0.rgb - Density*(texCol1.rgb));
//        GLSL equivalent:
//        vec3 result = clamp(texCol0.rgb - Density*(texCol1.rgb), 0.0, 1.0);
        float dotMask = clamp(nDotL, 0.0f, 1.0f);
        float diffuseFactor = mix(1.0f, dotMask, watercolorConfig.diffuseFactor);
        float shadedWrap = mix(0.0f, clamp(-nDotL, 0.0f, 1.0f), watercolorConfig.shadeWrap);
        float customLambert = clamp(diffuseFactor * (1 - shadedWrap), 0.0f, 1.0f);
        vec3 diffuseColor = lConfig.color * lConfig.intensity * customLambert; // diffuse lambert reflectance

        // Dilute area
        float clampVal = clamp((dotMask + (watercolorConfig.diluteArea - 1)) / watercolorConfig.diluteArea, 0.0f, 1.0f);
        vec3 diluted = vec3(clampVal, clampVal, clampVal);

        // Phong specular
        vec3 specularColor = vec3(0.0f);
        if (lightNo == 1 && lConfig.specular) {
            float rDotV = dot(reflect(normalizedLV, normalWorld), -viewDir);
            float clamped = clamp(((1-shadingConfig.specular)-rDotV)*200/5, 0.0f, 1.0f);// 5=> depth TODO find out where that value comes from
            float specularEdge = watercolorConfig.darkEdges * (clamped -1); // darkened edges mask
            float specularColorFloat = (mix(specularEdge, 0.0f, shadingConfig.specularDiffusion) +
                                        2 * clamp((float)((glm::max(1.0f - shadingConfig.specular, rDotV) - (1 - shadingConfig.specular)) *
                                                          pow((2 - shadingConfig.specularDiffusion), 10)),0.0f, 1.0f)) *
                                       (1 - shadingConfig.specularTransparency);
            specularColor = vec3(specularColorFloat, specularColorFloat, specularColorFloat); // TODO ?
            specularColor *= clamp(dot(normalWorld, lConfig.direction) * 2, 0.0f, 1.0f);
        }

        // Attenuation
        if (!isDirectionalLight) {
            bool enableAttenuation = lConfig.attenuationScale > 0.00001f;
            float attenuation = mix(1.0f, 1 / pow(length(lightVector), lConfig.attenuationScale), enableAttenuation);
            // Need to compensate for diffuse and specular
            diffuseColor *= attenuation;
            specularColor *= attenuation;
        }

        // Spot light and angle of cone
        if(lConfig.type == 2) {
            float angle = getLightConeAngle(lConfig.coneAngle, lConfig.fallOff, normalizedLV, lConfig.direction);
            diffuseColor *= angle;
            specularColor *= angle;
        }

        // shadows

        //L.shade = 0.0;
//        if (_UseShadows && lightShadowOn) {
//            float shadow = lightShadow(lightViewPrjMatrix, lightShadowMap, vertWorldPos, nDotL);
//            if(shadow<1){
//                diffuseColor =  lightColor * lightIntensity * (1-diffuseFactor);
//                L.shade = 1;
//            }
//            specularColor *= floor(shadow); //get rid of specular in the shade
//        }

        light.lightColor = diffuseColor;
        light.specular = specularColor;
        light.dilute = diluted;
    }
    return light;
}

vec2 calcVelocity(vec2 currentPos, vec2 prevPos, float zOverW) {
    return (currentPos-prevPos)/2.0f;
}

////////////////////////////////
//       Input handling       //
////////////////////////////////
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