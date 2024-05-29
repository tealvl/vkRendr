#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <limits>
#include <glm/glm.hpp>
#include <array>
#include <chrono>
#include <tiny_obj_loader.h>
#include <unordered_map>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "camera.hpp"
#include "transform.hpp"
#include "utility.hpp"
#include "inputManager.hpp"
#include "CameraManipulator.hpp"
#include "timer.hpp"
#include "simpleMaterial.hpp"

const std::string MODEL_PATH = "C:/Dev/cpp-projects/engine/resources/models/vikingRoom.obj";
const std::string TEXTURE_PATH = "C:/Dev/cpp-projects/engine/resources/textures/viking_room.png";
const int FramesInFlight = 2;

class Application {
public:
    void run();
   
    Application()
    : 
    glwfContext_(), 
    window_(), 
    renderer(),

    
    {}

private:
    rendr::GlfwContext glwfContext;
    rendr::Window window;
    rendr::Renderer renderer;

    rendr::Image depthImage_;

    std::vector<vk::raii::CommandBuffer> commandBuffers_;
    std::vector<rendr::PerFrameSync> framesSyncObjs_;

    uint32_t currentFrame = 0;
    bool framebufferResized;

    rendr::Buffer vertexBuffer_;
    rendr::Buffer indexBuffer_;

    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    vk::raii::DescriptorPool descriptorPool_;
    std::vector<std::vector<vk::raii::DescriptorSet>> descriptorSets_;
    
    rendr::Image textureImage;
    vk::raii::Sampler textureSampler;


    rendr::InputManager inputManager_;
    rendr::CameraManipulator camManip_;
    rendr::Camera camera_;
    rendr::Transform model_matrix_;
    Timer timer_;
    
    void init();
    void mainLoop();
    void cleanupSwapChain();
    void recreateSwapChain();
    void recordCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
};