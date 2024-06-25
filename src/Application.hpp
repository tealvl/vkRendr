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
#include "rendr.hpp"
#include "inputManager.hpp"
#include "CameraManipulator.hpp"
#include "timer.hpp"
#include "simpleMaterial.hpp"
#include "simpleDrawableObj.hpp"

class Application {
public:
    void run();
   
    Application()
    : 
    glwfContext(), 
    window(), 
    renderer()
    {}

private:
    rendr::GlfwContext glwfContext;
    rendr::Window window;
    rendr::Renderer renderer;

    SimpleSetupBinder setupBinder;
    std::vector<MeshWithTextureObj> objsToDraw;

    rendr::InputManager inputManager;
    rendr::CameraManipulator camManip;
    rendr::Camera camera;
    Timer timer;
    
    void init();
    void mainLoop();
};