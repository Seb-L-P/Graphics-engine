#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <fstream> 
#include <sstream> 

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
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024; // Shadow Map Resolution

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
int postProcessEffect = 0; 

char fileDialogBuffer[128] = "level1.scene"; 
bool showSavePopup = false;
bool showLoadPopup = false;

std::vector<GameObject> sceneObjects;

glm::vec3 sunDirection(-0.5f, -1.0f, -0.5f); // Adjusted for better shadow angle
glm::vec3 sunColor(0.9f, 0.9f, 0.9f);

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f), glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f), glm::vec3( 0.0f,  0.0f, -3.0f)
};
glm::vec3 pointLightColors[] = {
    glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f)
};

// Skybox Data (Same as before)
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
};

float quadVertices[] = { 
    -1.0f,  1.0f,  0.0f, 1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f, 1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f
};

// Forward Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); 
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);
void saveScene(const char* filename);
void loadScene(const char* filename, Model* defaultModel);

// --- MAIN ---
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

    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- SHADERS ---
    Shader standardShader("simple_lighting.vert", "standard.frag"); // Renaming to standard in logic
    Shader lampShader("simple_lighting.vert", "lamp.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader screenShader("screen.vert", "screen.frag");
    Shader shadowDepthShader("shadow_depth.vert", "shadow_depth.frag"); // NEW

    Model cubeModel("cube.obj");
    Model lampModel("cube.obj");

    // --- POST PROCESS FBO (From previous step) ---
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    int fboWidth, fboHeight; glfwGetFramebufferSize(window, &fboWidth, &fboHeight);
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fboWidth, fboHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "PostProcess FBO Incomplete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- SHADOW MAP FBO (NEW) ---
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Clamp to border to prevent shadows appearing outside the map range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // Attach to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // No color needed
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- QUAD & SKYBOX SETUP ---
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenVertexArrays(1, &skyboxVAO); glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO); glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
    cubemapTexture = loadCubemap(faces);
    skyboxShader.use(); skyboxShader.setInt("skybox", 0);
    screenShader.use(); screenShader.setInt("screenTexture", 0);
    
    // Configure standard shader to know where to find the shadow map
    standardShader.use();
    standardShader.setInt("texture_diffuse1", 0);
    standardShader.setInt("shadowMap", 1); // Shadow map will be bound to unit 1

    // Initial Scene
    GameObject floor("Floor", &cubeModel); floor.position = glm::vec3(0.0f, -2.0f, 0.0f); floor.scale = glm::vec3(10.0f, 0.1f, 10.0f); sceneObjects.push_back(floor);
    GameObject crate1("Crate 1", &cubeModel); crate1.position = glm::vec3(0.0f, 0.0f, 0.0f); sceneObjects.push_back(crate1);

    float lastTime = 0.0f; int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
        frameCount++; if (currentFrame - lastTime >= 1.0f) { std::string title = "My Game Engine - " + std::to_string(frameCount) + " FPS"; glfwSetWindowTitle(window, title.c_str()); frameCount = 0; lastTime = currentFrame; }

        processInput(window);

        // --- 1. SHADOW PASS ---
        // Render scene from Sun's perspective to generate Depth Map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Calculate Light Space Matrix (Orthographic because Sun is directional)
        float near_plane = 1.0f, far_plane = 20.0f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(sunDirection * -10.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        
        shadowDepthShader.use();
        shadowDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); // Optimization: Render back faces to fix Peter Panning
        for(int i = 0; i < sceneObjects.size(); i++) {
            sceneObjects[i].Draw(shadowDepthShader);
        }
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // End Shadow Pass

        // --- 2. LIGHTING PASS (Render to Post-Process FBO) ---
        glViewport(0, 0, fboWidth, fboHeight); 
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); 
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        standardShader.use();
        standardShader.setVec3("viewPos", camera.Position);
        standardShader.setVec3("dirLight.direction", sunDirection);
        standardShader.setVec3("dirLight.ambient", sunColor * 0.2f);
        standardShader.setVec3("dirLight.diffuse", sunColor);
        standardShader.setVec3("dirLight.specular", sunColor);
        standardShader.setMat4("lightSpaceMatrix", lightSpaceMatrix); // Send matrix for shadow calculations

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
        
        // Bind Shadow Map to Texture Unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        // Bind Standard Textures (handled in Mesh.Draw usually, but we reset here to be safe)
        glActiveTexture(GL_TEXTURE0);

        for(int i = 0; i < sceneObjects.size(); i++) sceneObjects[i].Draw(standardShader);

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

        // Skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // --- 3. POST PROCESS PASS (Screen Quad) ---
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        int w, h; glfwGetFramebufferSize(window, &w, &h); glViewport(0, 0, w, h);
        glDisable(GL_DEPTH_TEST); 
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenShader.setInt("effectType", postProcessEffect);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer); 
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 4. UI PASS ---
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
        if (uiMode) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save As...")) showSavePopup = true;
                    if (ImGui::MenuItem("Load Scene...")) showLoadPopup = true;
                    if (ImGui::MenuItem("Clear Scene")) { sceneObjects.clear(); selectedObjectID = -1; }
                    if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(window, true);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            if (showSavePopup) {
                ImGui::OpenPopup("Save Scene");
            }
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH/2.0f, SCR_HEIGHT/2.0f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Save Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::InputText("##filename", fileDialogBuffer, sizeof(fileDialogBuffer));
                if (ImGui::Button("Save", ImVec2(120, 0))) { saveScene(fileDialogBuffer); showSavePopup = false; ImGui::CloseCurrentPopup(); }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) { showSavePopup = false; ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
            if (showLoadPopup) {
                ImGui::OpenPopup("Load Scene");
            }
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH/2.0f, SCR_HEIGHT/2.0f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::InputText("##filename", fileDialogBuffer, sizeof(fileDialogBuffer));
                if (ImGui::Button("Load", ImVec2(120, 0))) { loadScene(fileDialogBuffer, &cubeModel); showLoadPopup = false; ImGui::CloseCurrentPopup(); }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) { showLoadPopup = false; ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }

            ImGui::Begin("Scene Hierarchy");
            if (ImGui::Button("Add Cube")) {
                GameObject newCube("New Cube", &cubeModel);
                sceneObjects.push_back(newCube);
                selectedObjectID = (int)sceneObjects.size() - 1;
                strncpy(nameBuffer, newCube.name.c_str(), sizeof(nameBuffer));
                nameBuffer[sizeof(nameBuffer)-1] = '\0'; 
            }
            ImGui::Separator();
            for (int i = 0; i < sceneObjects.size(); i++) {
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
                if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) obj.name = std::string(nameBuffer);
                ImGui::Separator();
                ImGui::InputFloat3("Position", &obj.position.x);
                ImGui::InputFloat3("Rotation", &obj.rotation.x);
                ImGui::InputFloat3("Scale", &obj.scale.x);
            } else ImGui::Text("No object selected.");
            ImGui::Separator();
            ImGui::Text("Sun Settings");
            ImGui::DragFloat3("Sun Dir", &sunDirection.x, 0.05f);
            ImGui::ColorEdit3("Sun Color", &sunColor.x);
            ImGui::Separator();
            ImGui::Text("Camera Effects");
            const char* items[] = { "Normal", "Invert", "Grayscale", "Sharpen", "Blur", "Edge Detect" };
            ImGui::Combo("Filter", &postProcessEffect, items, IM_ARRAYSIZE(items));
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// ... Input/Load functions (omitted for brevity) ...
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); 
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else { std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl; stbi_image_free(data); }
    }
    stbi_set_flip_vertically_on_load(true); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}
void saveScene(const char* filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return;
    out << sceneObjects.size() << "\n";
    for (const auto& obj : sceneObjects) {
        out << obj.name << "\n";
        out << obj.position.x << " " << obj.position.y << " " << obj.position.z << "\n";
        out << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << "\n";
        out << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << "\n";
    }
    out << "SUN_SETTINGS\n"; out << sunDirection.x << " " << sunDirection.y << " " << sunDirection.z << "\n"; out << sunColor.x << " " << sunColor.y << " " << sunColor.z << "\n";
    out.close();
}
void loadScene(const char* filename, Model* defaultModel) {
    std::ifstream in(filename); if (!in.is_open()) return;
    sceneObjects.clear(); selectedObjectID = -1;
    int count; in >> count; std::string dummy; std::getline(in, dummy); 
    for (int i = 0; i < count; i++) {
        std::string name; std::getline(in, name); if(name.empty()) name = "Unnamed Object";
        GameObject obj(name, defaultModel);
        in >> obj.position.x >> obj.position.y >> obj.position.z;
        in >> obj.rotation.x >> obj.rotation.y >> obj.rotation.z;
        in >> obj.scale.x >> obj.scale.y >> obj.scale.z;
        std::getline(in, dummy); sceneObjects.push_back(obj);
    }
    std::string tag; if (std::getline(in, tag) && tag == "SUN_SETTINGS") { in >> sunDirection.x >> sunDirection.y >> sunDirection.z; in >> sunColor.x >> sunColor.y >> sunColor.z; }
    in.close();
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && uiMode && !ImGui::GetIO().WantCaptureMouse) {
        double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos);
        float x = (2.0f * (float)xpos) / SCR_WIDTH - 1.0f; float y = 1.0f - (2.0f * (float)ypos) / SCR_HEIGHT;
        glm::vec3 ray_nds = glm::vec3(x, y, 1.0f);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip; ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
        glm::mat4 view = camera.GetViewMatrix(); glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye); ray_wor = glm::normalize(ray_wor);
        int hitIndex = -1; float minDistance = 1000.0f; 
        for(int i = 0; i < sceneObjects.size(); i++) {
            if (sceneObjects[i].IntersectRay(camera.Position, ray_wor)) {
                float dist = glm::distance(camera.Position, sceneObjects[i].position);
                if (dist < minDistance) { minDistance = dist; hitIndex = i; }
            }
        }
        selectedObjectID = hitIndex;
        if (selectedObjectID != -1) { strncpy(nameBuffer, sceneObjects[selectedObjectID].name.c_str(), sizeof(nameBuffer)); nameBuffer[sizeof(nameBuffer)-1] = '\0'; }
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