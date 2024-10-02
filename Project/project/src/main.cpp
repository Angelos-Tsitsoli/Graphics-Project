#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../Linking/lib/camera.h"
#include "../../Linking/lib/shader.h"
#include "../../Linking/lib/model.h"
#include <random>

// Random number generator
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(-50.0f, 50.0f);

bool animation = true;

// timing
float lastFrame = 0.0f;
float frameToggled = 0.0f;
float timeSinceLastToggle = 1.0f;

// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

float earthOrbitRadius = 100.0f;
float moonOrbitRadius = 20.0f;

glm::vec3 sunPos = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 earthPos = sunPos + glm::vec3(sin(frameToggled) * earthOrbitRadius, 0.0f, cos(frameToggled) * earthOrbitRadius);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float cameraOrbitRadius = 30.0f;
float rotateAngle = 1.0f;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void keyboardInput(GLFWwindow* window);



int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    Shader planetShader("./src/planet.vs", "./src/planet.fs");
    Shader sunShader("./src/sun.vs", "./src/sun.fs");

    // load the models
    ///
    ///
    Model Sun("./models/sun/sun.obj");
    Model Earth("./models/earth/Earth.obj");
    Model Moon("./models/moon/Moon.obj");
    Model Star("./models/planet/planet.obj");
    ///
    ///
  

    std::vector<glm::vec3> starPositions;
    for (int i = 0; i < 800; i++) {
        glm::vec3 position;
        do {
            position = glm::vec3(distribution(generator), distribution(generator), distribution(generator)) + sunPos;
        } while (glm::length(position - sunPos) < 10.0f || glm::length(position - earthPos) < 10.0f);
        starPositions.push_back(position);
    }

    // render Loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        timeSinceLastToggle += deltaTime;
        if (animation)
            frameToggled += deltaTime;

        keyboardInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

       
        // render the sun object
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sunPos);
        model = glm::scale(model, glm::vec3(2.0f)); 
        sunShader.setMat4("model", model);

        Sun.Draw(sunShader);

       
     

        // be sure to activate shader when setting uniforms/drawing objects
        planetShader.use();
        planetShader.setVec3("viewPos", camera.Position);
        planetShader.setFloat("material.shininess", 32.0f);

        // directional light
        planetShader.setVec3("light.position", sunPos);
        planetShader.setVec3("light.ambient", 0.25f, 0.25f, 0.25f);
        planetShader.setVec3("light.diffuse", 1.8f, 1.8f, 1.8f);
        planetShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // point light 1
        planetShader.setFloat("pointLights[0].constant", 1.0f);
        planetShader.setFloat("pointLights[0].linear", 0.045);
        planetShader.setFloat("pointLights[0].quadratic", 0.0075);

        // render earth
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));


        earthPos = sunPos + glm::vec3(sin(frameToggled) * earthOrbitRadius, 0.0f, cos(frameToggled) * earthOrbitRadius);
        model = glm::translate(model, earthPos);
        model = glm::rotate(model, frameToggled * 1.5f * glm::radians(-50.0f), glm::vec3(0.1f, 1.0f, 0.0f));
        planetShader.setMat4("model", model);

        Earth.Draw(planetShader);

        // render moon
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));


        glm::vec3 moonPos = earthPos + glm::vec3(0.0f, sin(frameToggled) * moonOrbitRadius, cos(frameToggled) * moonOrbitRadius);

        model = glm::translate(model, moonPos);

        planetShader.use();
        planetShader.setMat4("model", model);

        Moon.Draw(planetShader);

        // render star
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);

        // Define the positions of the stars
        for (unsigned int i = 0; i < starPositions.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, starPositions[i]);
            model = glm::scale(model, glm::vec3(0.09f)); // Scale down the model
            planetShader.setMat4("model", model);
            Star.Draw(planetShader);
        }
       

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Handles user keyboard input. Supposed to be used every frame
void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Keyboard_process(FORWARD, cameraOrbitRadius, rotateAngle);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Keyboard_process(BACKWARD, cameraOrbitRadius, rotateAngle);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Keyboard_process(RIGHT, cameraOrbitRadius, rotateAngle);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Keyboard_process(LEFT, cameraOrbitRadius, rotateAngle);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (timeSinceLastToggle > 0.2)
        {
            animation = !animation;
            timeSinceLastToggle = 0.0f;
        }
    }
}

