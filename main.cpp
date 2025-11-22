#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <string> // Ensure string is included for to_string

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "GameObject.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// --- ENGINE STATE ---
bool uiMode = true; 
int selectedObjectID = -1; 
char nameBuffer[128] = ""; 

std::vector<GameObject> sceneObjects;

// Global Light
glm::vec3 sunDirection(-0.2f, -1.0f, -0.3f);
glm::vec3 sunColor(0.9f, 0.9f, 0.9f);

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f), glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f), glm::vec3( 0.0f,  0.0f, -3.0f)
};
glm::vec3 pointLightColors[] = {
    glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f)
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); 
void processInput(GLFWwindow *window);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "My Game Engine", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader standardShader("simple_lighting.vert", "standard.frag");
    Shader lampShader("simple_lighting.vert", "lamp.frag");

    Model cubeModel("cube.obj");
    Model lampModel("cube.obj");

    // --- CREATE SCENE OBJECTS ---
    GameObject floor("Floor", &cubeModel);
    floor.position = glm::vec3(0.0f, -2.0f, 0.0f);
    floor.scale = glm::vec3(10.0f, 0.1f, 10.0f);
    sceneObjects.push_back(floor);

    GameObject crate1("Crate 1", &cubeModel);
    crate1.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sceneObjects.push_back(crate1);

    GameObject crate2("Crate 2", &cubeModel);
    crate2.position = glm::vec3(2.0f, 1.0f, -2.0f);
    crate2.rotation = glm::vec3(0.0f, 45.0f, 0.0f);
    sceneObjects.push_back(crate2);

    glEnable(GL_DEPTH_TEST);

    float lastTime = 0.0f;
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        frameCount++;
        if (currentFrame - lastTime >= 1.0f) { 
            std::string title = "My Game Engine - " + std::to_string(frameCount) + " FPS";
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastTime = currentFrame;
        }

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (uiMode) {
            ImGui::Begin("Scene Hierarchy");
            if (ImGui::Button("Add Cube")) {
                GameObject newCube("New Cube", &cubeModel);
                sceneObjects.push_back(newCube);
                // Auto-select the new object and prep rename buffer
                selectedObjectID = (int)sceneObjects.size() - 1;
                strncpy(nameBuffer, newCube.name.c_str(), sizeof(nameBuffer));
                nameBuffer[sizeof(nameBuffer)-1] = '\0'; // Safety
            }
            ImGui::Separator();
            for (int i = 0; i < sceneObjects.size(); i++) {
                // --- CRASH FIX: UNIQUE ID SUFFIX ---
                // We append "##index" to the name. 
                // ImGui hides everything after ## but uses it for the ID.
                // Even if name is "", the ID is "##5", which is unique and safe.
                std::string label = sceneObjects[i].name + "##" + std::to_string(i);

                if (ImGui::Selectable(label.c_str(), selectedObjectID == i)) {
                    selectedObjectID = i;
                    strncpy(nameBuffer, sceneObjects[i].name.c_str(), sizeof(nameBuffer));
                    nameBuffer[sizeof(nameBuffer)-1] = '\0';
                }
            }
            ImGui::End();

            ImGui::Begin("Inspector");
            if (selectedObjectID >= 0 && selectedObjectID < sceneObjects.size()) {
                GameObject& obj = sceneObjects[selectedObjectID];
                
                if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
                    obj.name = std::string(nameBuffer);
                }
                
                ImGui::Separator();
                ImGui::InputFloat3("Position", &obj.position.x);
                ImGui::InputFloat3("Rotation", &obj.rotation.x);
                ImGui::InputFloat3("Scale", &obj.scale.x);
                
            } else {
                ImGui::Text("No object selected.");
            }
            ImGui::Separator();
            ImGui::Text("Sun Settings");
            ImGui::DragFloat3("Sun Dir", &sunDirection.x, 0.05f);
            ImGui::ColorEdit3("Sun Color", &sunColor.x);
            ImGui::End();
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        standardShader.use();
        standardShader.setVec3("viewPos", camera.Position);
        standardShader.setVec3("dirLight.direction", sunDirection);
        standardShader.setVec3("dirLight.ambient", sunColor * 0.2f);
        standardShader.setVec3("dirLight.diffuse", sunColor);
        standardShader.setVec3("dirLight.specular", sunColor);

        for(int i = 0; i < 4; i++) {
            std::string num = std::to_string(i);
            standardShader.setVec3("pointLights[" + num + "].position", pointLightPositions[i]);
            standardShader.setVec3("pointLights[" + num + "].ambient", pointLightColors[i] * 0.1f);
            standardShader.setVec3("pointLights[" + num + "].diffuse", pointLightColors[i]);
            standardShader.setVec3("pointLights[" + num + "].specular", pointLightColors[i]);
            standardShader.setFloat("pointLights[" + num + "].constant", 1.0f);
            standardShader.setFloat("pointLights[" + num + "].linear", 0.09f);
            standardShader.setFloat("pointLights[" + num + "].quadratic", 0.032f);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        standardShader.setMat4("projection", projection);
        standardShader.setMat4("view", view);

        for(int i = 0; i < sceneObjects.size(); i++) {
            sceneObjects[i].Draw(standardShader);
        }

        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        for(int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); 
            lampShader.setMat4("model", model);
            lampShader.setVec3("lightColor", pointLightColors[i]);
            lampModel.Draw(lampShader);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && uiMode && !ImGui::GetIO().WantCaptureMouse) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float x = (2.0f * (float)xpos) / SCR_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * (float)ypos) / SCR_HEIGHT;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
        ray_wor = glm::normalize(ray_wor);

        int hitIndex = -1;
        float minDistance = 1000.0f; 

        for(int i = 0; i < sceneObjects.size(); i++) {
            if (sceneObjects[i].IntersectRay(camera.Position, ray_wor)) {
                float dist = glm::distance(camera.Position, sceneObjects[i].position);
                if (dist < minDistance) {
                    minDistance = dist;
                    hitIndex = i;
                }
            }
        }

        selectedObjectID = hitIndex;
        if (selectedObjectID != -1) {
            strncpy(nameBuffer, sceneObjects[selectedObjectID].name.c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer)-1] = '\0';
        }
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) {
            uiMode = !uiMode;
            if (uiMode) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); firstMouse = true; }
            tabPressed = true;
        }
    } else { tabPressed = false; }

    if (!uiMode) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (uiMode) return;
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos; 
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}