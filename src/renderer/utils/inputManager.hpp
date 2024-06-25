#pragma once
#include <unordered_map>
#include <utility>
#include <optional>
#include <memory>
#include "keyCodes.h"
#include "window.hpp"
#include "rendr.hpp"

namespace rendr{

class InputManager {
public:

    InputManager(){}

    void setUpWindowCallbacks(Window& window) {        
        window.callbacks.keyDown = [this](int key) { onKeyDown(key); };
        window.callbacks.keyPressed = [this](int key) { onKeyPressed(key); };
        window.callbacks.keyUp = [this](int key) { onKeyUp(key); };
        window.callbacks.mouseMoved = [this](double xpos, double ypos) { onMouseMoved(xpos, ypos); };
        window.callbacks.mouseBtnDown = [this](int button) { onMouseButtonDown(button); };
        window.callbacks.mouseBtnUp = [this](int button) { onMouseButtonUp(button); };
        window.callbacks.mouseScroll = [this](double xoffset, double yoffset) { onMouseScroll(xoffset, yoffset); };
        window.callbacks.charInput = [this](uint32_t codepoint) { onCharInput(codepoint); };
    }
    
    bool isKeyPressed(int key) const {
        auto it = keyStates.find(key);
        return it != keyStates.end() && it->second;
    }

    bool isMouseButtonPressed(int button) const {
        auto it = mouseButtonStates.find(button);
        return it != mouseButtonStates.end() && it->second;
    }

    std::pair<double, double> getMousePosition() const {
        return mousePosition;
    }

    std::pair<double, double> getMouseOffset() const {
        return mouseOffset;
    }

    std::pair<double, double> getScrollOffset() const {
        return scrollOffset;
    }
   
    void resetInputOffsets(){
        mouseOffset = { 0.0, 0.0 };
        scrollOffset = { 0.0, 0.0 };
    }

    void resetMousePosition(){
        mousePosition = { 0.0, 0.0 };
    }

private:
    std::unordered_map<int, bool> keyStates;
    std::unordered_map<int, bool> mouseButtonStates;
    std::pair<double, double> mousePosition = { 0.0, 0.0 };
    std::pair<double, double> mouseOffset = { 0.0, 0.0 };

    std::pair<double, double> scrollOffset = { 0.0, 0.0 };

    void onKeyDown(int key) {
        int pressedKey = glfwKeyTranslate(key);
        keyStates[pressedKey] = true;
    }

    void onKeyPressed(int key) {
        int pressedKey = glfwKeyTranslate(key);
        //TODO
    }

    void onKeyUp(int key) {
        int releasedKey = glfwKeyTranslate(key);
        keyStates[releasedKey] = false;
    }

    void onMouseMoved(double xpos, double ypos) {
        mouseOffset = { mousePosition.first - xpos, mousePosition.second - ypos};
        mousePosition = { xpos, ypos };  
    }

    void onMouseButtonDown(int button) {
        mouseButtonStates[button] = true;
    }

    void onMouseButtonUp(int button) {
        mouseButtonStates[button] = false;
    }

    void onMouseScroll(double xoffset, double yoffset) {
        scrollOffset = { xoffset, yoffset };
    }

    void onCharInput(uint32_t codepoint) {}

};

}