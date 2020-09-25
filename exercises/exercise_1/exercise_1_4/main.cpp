#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>


// function declarations
// ---------------------
void createArrayBuffer(const std::vector<float> &array, unsigned int &VBO);
void setupShape(unsigned int shaderProgram, unsigned int &VAO, unsigned int &vertexCount);
void setupShapeE(const unsigned int shaderProgram,unsigned int &VAO, unsigned int &vertexCount);
void draw(unsigned int shaderProgram, unsigned int VAO, unsigned int vertexCount);


// glfw functions
// --------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


// settings
// --------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


// shader programs
// ---------------
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 //"layout (location = 1) in vec3 aInd;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "out vec3 vtxColor; // output a color to the fragment shader\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "   vtxColor = aColor;\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in  vec3 vtxColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(vtxColor, 1.0);\n"
                                   "}\n\0";



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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // setup vertex array object (VAO)
    // -------------------------------
    unsigned int VAO, vertexCount;
    // generate geometry in a vertex array object (VAO), record the number of vertices in the mesh,
    // tells the shader how to read it
    setupShape(shaderProgram, VAO, vertexCount);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(.2f, .2f, .2f, 1.0f); // background
        glClear(GL_COLOR_BUFFER_BIT); // clear the framebuffer

        draw(shaderProgram, VAO, vertexCount);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window); // we normally use 2 frame buffers, a back (to draw on) and a front (to show on the screen)
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// create a vertex buffer object (VBO) from an array of values, return VBO handle (set as reference)
// -------------------------------------------------------------------------------------------------
void createArrayBuffer(const std::vector<float> &array, unsigned int &VBO){
    // create the VBO on OpenGL and get a handle to it
    glGenBuffers(1, &VBO);
    // bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the content of the VBO (type, size, pointer to start, and how it is used)
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);
}


// create the geometry, a vertex array object representing it, and set how a shader program should read it
// -------------------------------------------------------------------------------------------------------
void setupShape(const unsigned int shaderProgram,unsigned int &VAO, unsigned int &vertexCount){
    std::vector<float> vertices,  colors;
    float angleStep = (float)360 / 16; // degres
    angleStep = 2.0f * M_PI / (float)16; // pi radiands = 180 degress
    angleStep = M_2_PI / (float)16; // pi radiands = 180 degress
    int times = 0;
    for(double angle = 0; angle <= 360;){
        vertices.push_back(0.0f); colors.push_back(.5f);
        vertices.push_back(0.0f); colors.push_back(.5f);
        vertices.push_back(0.0f); colors.push_back(.5f);

        vertices.push_back(.5f*cos(angle));colors.push_back(vertices.back() + .5f);
        vertices.push_back(.5f*sin(angle));colors.push_back(vertices.back() + .5f);
        vertices.push_back(0.0f);colors.push_back(vertices.back() + .5f);

        angle+=angleStep;
        vertices.push_back(.5f*cos(angle));colors.push_back(vertices.back() + .5f);
        vertices.push_back(.5f*sin(angle));colors.push_back(vertices.back() + .5f);
        vertices.push_back(0.0f);colors.push_back(vertices.back() + .5f);
        //angle+=angleStep;
    }


    unsigned int posVBO, colorVBO;
    createArrayBuffer(vertices, posVBO);

    createArrayBuffer( colors, colorVBO);

    // tell how many vertices to draw
    vertexCount = vertices.size()/3;

    // create a vertex array object (VAO) on OpenGL and save a handle to it
    glGenVertexArrays(1, &VAO);

    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "aPos"
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);

    int posSize = 3;
    int posAttributeLocation = glGetAttribLocation(shaderProgram, "aPos");

    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, posSize, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "aColor"
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);

    int colorSize = 3;
    int colorAttributeLocation = glGetAttribLocation(shaderProgram, "aColor");

    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, colorSize, GL_FLOAT, GL_FALSE, 0, 0);

}
void setupShapeE(const unsigned int shaderProgram,unsigned int &VAO, unsigned int &vertexCount){

    //unsigned int posVBO, colorVBO;
//    createArrayBuffer(std::vector<float>{
//            // position
//            0.0f,  0.0f, 0.0f,
//            0.5f,  0.0f, 0.0f,
//            0.5f,  0.5f, 0.0f
//    }, posVBO);
//
//    createArrayBuffer( std::vector<float>{
//            // color
//            1.0f,  0.0f, 0.0f,
//            1.0f,  0.0f, 0.0f,
//            1.0f,  0.0f, 0.0f
//    }, colorVBO);
    // change 16 here to have more triangles
//    float vertices[(16+1)*3]={},  colors[(16+1)*3]={};
//    float angle;
//    unsigned int indices[16*3], i, current, lindex = 0, lvalue=2; // total triangles
//    vertices[0] = 0.0f;
//    vertices[1] = 0.0f;
//    vertices[2] = 0.0f;
//    current = 3;
//    unsigned int n = 16;
//    angle = 360 / n;
//
//    for (unsigned int i = 3; i <= n; i++) {
//        //x value
//        vertices[current] = 0.5f * std::cos(angle); //+A.x if not in origin
//        colors[current] = 0.6f;
//        // y value
//        vertices[++current] = 0.5f * std::sin(angle);//+A.y if not in origin
//        colors[current] = 0.1f;
//        //z value
//        vertices[++current] = 0.0f;
//        colors[current] = 0.3f;
//        current ++;
//        for (int j = 1; j< 3; j++) {
//            indices[lindex] = 1;
//            indices[++lindex] = lvalue;
//            std::cout << "lindex:, lvalue:"<<lindex << lvalue << std::endl;
//            indices[++lindex] = ++lvalue;
//            std::cout << "lindex:, lvalue:"<<lindex << lvalue << std::endl;
//            ++lindex;
//        }
//    }
    float vertices[] = {
            0.5f,  0.5f, 0.0f,  // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left
    };
    float colors[] = {
            0.5f,  0.5f, 0.0f,  // top right
            0.5f,  0.5f, 0.0f,  // bottom right
            0.5f,  0.5f, 1.0f,  // bottom left
            0.5f,  0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
    };

    // create a vertex array object (VAO) on OpenGL and save a handle to it
    glGenVertexArrays(1, &VAO);

    // bind vertex array object
    glBindVertexArray(VAO);

    unsigned int posVBO, posEBO, colorVBO;
    // create the VBO on OpenGL and get a handle to it
    glGenBuffers(1, &posEBO);
    glGenBuffers(1, &colorVBO);
    // bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, posEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    // tell how many vertices to draw
    vertexCount = 17;//?


    // set vertex shader attribute "aPos"
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, posEBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    int posSize = 3;
    int posAttributeLocation = glGetAttribLocation(shaderProgram, "aPos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, posSize, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    int indSize = 3;
    int indAttributeLocation = glGetAttribLocation(shaderProgram, "aInd");
    glEnableVertexAttribArray(indAttributeLocation);
    glVertexAttribPointer(indAttributeLocation, indSize, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "aColor"
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);

    int colorSize = 3;
    int colorAttributeLocation = glGetAttribLocation(shaderProgram, "aColor");

    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, colorSize, GL_FLOAT, GL_FALSE, 0, 0);

}


// tell opengl to draw a vertex array object (VAO) using a give shaderProgram
// --------------------------------------------------------------------------
void draw(const unsigned int shaderProgram, const unsigned int VAO, const unsigned int vertexCount){
    // set active shader program
    glUseProgram(shaderProgram);
    // bind vertex array object
    glBindVertexArray(VAO);
    // draw geometry
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    //glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

