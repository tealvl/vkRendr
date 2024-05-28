#pragma once
#include "camera.hpp"
#include "keyCodes.h"
#include "inputManager.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace rendr{

class CameraManipulator {
public:
    CameraManipulator() : camera(nullptr), inputManager(nullptr), yaw(0.0f), pitch(0.0f) {}
    
    

    void enableMouseCameraControl(){
        mouseCameraControl = true;
    }

    void disableMouseCameraControl(){
        mouseCameraControl = false;
    }

    void setCamera(Camera& cameraToManip){
        camera = &cameraToManip;
    }

    void setInputManager(InputManager& inManager){
        inputManager = &inManager;
    }

    void update(float deltaTime) {
        if (!camera || !inputManager) return;

        if(mouseCameraControl){
            auto mouseOffset = inputManager->getMouseOffset();
                    
            float xOffset = mouseOffset.first * mouseSensitivity * deltaTime;
            float yOffset = mouseOffset.second * mouseSensitivity * deltaTime;

            yaw += xOffset;
            pitch += yOffset;        
            pitch = std::clamp(pitch, -89.0f, 89.0f);
                
            glm::quat rotation = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));
            glm::vec3 lookDirection = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            lookDirection = glm::normalize(lookDirection);

            camera->look_at_pos = lookDirection + camera->pos;
        }

        glm::vec3 front = glm::normalize(camera->look_at_pos - camera->pos);
        glm::vec3 right = glm::normalize(glm::cross(front, camera->worldUp));
        
        float adjustedSpeed = cameraSpeed * deltaTime;

        if (inputManager->isKeyPressed(GLFW_KEY_W)) {
            camera->pos += front * adjustedSpeed;
        }
        if (inputManager->isKeyPressed(GLFW_KEY_S)) {
            camera->pos -= front * adjustedSpeed;
        }
        if (inputManager->isKeyPressed(GLFW_KEY_A)) {
            camera->pos -= right * adjustedSpeed;
        }
        if (inputManager->isKeyPressed(GLFW_KEY_D)) {
            camera->pos += right * adjustedSpeed;
        }

        camera->look_at_pos = camera->pos + front;
    }

private:
    Camera* camera;
    InputManager* inputManager;
    
    glm::quat orientation;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    float cameraSpeed = 1.0f;
    float mouseSensitivity = 5.0f;
    
    float yaw;
    float pitch;
    bool mouseCameraControl = false;
    bool ignoreFirstMouseMovement = false;
};
    
}
