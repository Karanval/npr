#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <vector>
#include <math.h>

// structure to hold the info necessary to render an object
struct SceneObject {
    unsigned int VAO;           // vertex array object handle
    unsigned int vertexCount;   // number of vertices in the object
    float r, g, b;              // for object color
    float x, y;                 // for position offset
};

// declaration of the function you will implement in voronoi 1.1
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY);

// mouse, keyboard and screen reshape glfw callbacks
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables we will use to store our objects, shaders, and active shader
std::vector<SceneObject> sceneObjects;
std::vector<Shader> shaderPrograms;
Shader* activeShader;


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment - Voronoi Diagram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // setup input callbacks
    glfwSetMouseButtonCallback(window, button_input_callback); // NEW!
    glfwSetKeyCallback(window, key_input_callback); // NEW!

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NEW!
    // build and compile the shader programs
    shaderPrograms.push_back(Shader("shader.vert", "color.frag"));
    shaderPrograms.push_back(Shader("shader.vert", "distance.frag"));
    shaderPrograms.push_back(Shader("shader.vert", "distance_color.frag"));
    activeShader = &shaderPrograms[0];

    // NEW!
    // set up the z-buffer
    glDepthRange(1,-1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // notice that now we are clearing two buffers, the color and the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the cones
        glUseProgram(activeShader->ID);

        // TODO voronoi 1.3
        // Iterate through the scene object, for each object:
        // - bind the VAO; set the uniform variables; and draw.
        // CODE HERE
        for (SceneObject so : sceneObjects) {
            glBindVertexArray(so.VAO);
            activeShader->use();
            activeShader->setVec2("aOffset", so.x, so.y);
//            float timeValue = glfwGetTime()+so.r;
//            float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
            activeShader->setVec3("coneColor", so.r, so.r, so.b);
            glDrawArrays(GL_TRIANGLES, 0, so.vertexCount);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// creates a cone triangle mesh, uploads it to openGL and returns the VAO associated to the mesh
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY){
    // TODO voronoi 1.1
    // (exercises 1.7 and 1.8 can help you with implementing this function)

    // Create an instance of a SceneObject,
    SceneObject sceneObject{};

    // you will need to store offsetX, offsetY, r, g and b in the object.
    // CODE HERE
    sceneObject.r =r;
    sceneObject.g = g;
    sceneObject.b = b;
    sceneObject.x = offsetX;
    sceneObject.y = offsetY;
    // Build the geometry into an std::vector<float> or float array.
    // CODE HERE
    std::vector<float> vertices;
    float angleStep = 2.0f * M_PI / (float)16; // pi radians = 180 degrees
    for(float angle = 0; angle <= 360;){
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);

        vertices.push_back(2.9f*cos(angle));
        vertices.push_back(2.9f*sin(angle));
        vertices.push_back(0.0f);

        angle+=angleStep;
        vertices.push_back(2.9f*cos(angle));
        vertices.push_back(2.9f*sin(angle));
        vertices.push_back(0.0f);
        //angle+=angleStep;
    }
    // Store the number of vertices in the mesh in the scene object.
    // CODE HERE
    sceneObject.vertexCount = vertices.size() / 3;
    // Declare and generate a VAO and VBO (and an EBO if you decide the work with indices).
    // CODE HERE
    // Bind and set the VAO and VBO (and optionally a EBO) in the correct order.
    // CODE HERE
    unsigned int VAO, posVBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
    // Set the position attribute pointers in the shader.
    // CODE HERE
    glEnableVertexAttribArray(0); // TODO change to actual index
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Store the VAO handle in the scene object.
    // CODE HERE
    sceneObject.VAO = VAO;

    // 'return' the scene object for the cone instance you just created.
    return sceneObject;
}

// glfw: called whenever a mouse button is pressed
void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    // TODO voronoi 1.2
    // (exercises 1.9 and 2.2 can help you with implementing this function)

    // Test button press, see documentation at:
    //     https://www.glfw.org/docs/latest/input_guide.html#input_mouse_button
    // CODE HERE
    // If a left mouse button press was detected, call instantiateCone:
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // - Push the return value to the back of the global 'vector<SceneObject> sceneObjects'.
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
    // - The click position should be transformed from screen coordinates to normalized device coordinates,
    //   to obtain the offset values that describe the position of the object in the screen plane.
    // 0 to 600 to -1 to 1
        float xDvc = (xpos/SCR_WIDTH) * 2 - 1, yDvc = -(ypos/SCR_HEIGHT * 2 - 1);
    // - A random value in the range [0, 1] should be used for the r, g and b variables.
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        sceneObjects.push_back(instantiateCone(r, g, b, xDvc, yDvc));
    }
}

// glfw: called whenever a keyboard key is pressed
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    // TODO voronoi 1.4

    // Set the activeShader variable by detecting when the keys 1, 2 and 3 were pressed;
    // see documentation at https://www.glfw.org/docs/latest/input_guide.html#input_keyboard
    // Key 1 sets the activeShader to &shaderPrograms[0];
    //   and so on.
    // CODE HERE
    if (action == GLFW_PRESS) {
        switch (button ) {
            case GLFW_KEY_1:
                activeShader = &shaderPrograms[0];
                break;
            case GLFW_KEY_2:
                activeShader = &shaderPrograms[1];
                break;
            case GLFW_KEY_3:
                activeShader = &shaderPrograms[2];
                break;
        }
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}