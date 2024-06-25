cmake_minimum_required(VERSION 3.26)

project(engine VERSION 0.1 LANGUAGES C CXX)

add_subdirectory(dependencies/Vulkan-Hpp)
add_subdirectory(dependencies/Vulkan-Hpp/glm)
add_subdirectory(dependencies/Vulkan-Hpp/glfw)

add_executable(engine

    src/renderer/core/window.cpp
    src/renderer/core/rendr.cpp

    src/renderer/utils/transform.cpp
    src/renderer/utils/inputManager.cpp
    src/renderer/utils/cameraManipulator.cpp
    src/renderer/utils/timer.cpp
    dependencies/ufbx/ufbx.c

    src/main.cpp
    src/Application.cpp

)

# Указываем пути к заголовочным файлам 
target_include_directories(engine
    PRIVATE dependencies/Vulkan-Hpp/glm
    PRIVATE dependencies/Vulkan-Hpp/glfw/include
    PRIVATE dependencies/Vulkan-Hpp/vulkan/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/core
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/utils
    PRIVATE dependencies/stb
    PRIVATE dependencies/tinyobjloader
    PRIVATE dependencies/ufbx

)

# Указываем путь к исходникам
target_link_directories(engine 
PRIVATE dependencies/Vulkan-Hpp/glfw/src
)


find_package(Vulkan REQUIRED)
# Линкуем библиотеки 
target_link_libraries(engine 
PRIVATE Vulkan::Vulkan
PRIVATE glfw
)