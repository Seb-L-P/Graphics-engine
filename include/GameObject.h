#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "Model.h"
#include "Shader.h"

class GameObject {
public:
    std::string name;
    glm::vec3 position;
    glm::vec3 rotation; 
    glm::vec3 scale;
    Model* model; 

    GameObject(std::string n, Model* m) 
        : name(n), model(m), position(0.0f), rotation(0.0f), scale(1.0f) {}

    void Draw(Shader &shader) {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::translate(mat, position);
        mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        mat = glm::scale(mat, scale);
        shader.setMat4("model", mat);
        if (model) model->Draw(shader);
    }

    // NEW: Simple Ray-Sphere Intersection Logic
    // Returns true if the ray hits this object's "bounding sphere"
    bool IntersectRay(glm::vec3 rayOrigin, glm::vec3 rayDir) {
        // 1. Vector from ray origin to sphere center
        glm::vec3 oc = position - rayOrigin;
        
        // 2. Project that vector onto the ray direction
        float t = glm::dot(oc, rayDir);
        
        // 3. Find the closest point on the ray to the sphere center
        glm::vec3 closestPoint = rayOrigin + rayDir * t;
        
        // 4. Calculate distance from sphere center to that point
        float distance = glm::length(position - closestPoint);
        
        // 5. Check if distance is less than radius (approx radius = max scale)
        float radius = std::max(std::max(scale.x, scale.y), scale.z) * 1.0f; // 1.0 is base radius of cube
        
        // Check if 't' is positive (in front of camera) and distance is within radius
        return (t > 0 && distance < radius);
    }
};
#endif