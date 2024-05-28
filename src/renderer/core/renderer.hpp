#pragma once
#include <vector>
#include "utility.hpp"
#include "resourceDataTypes.hpp"
#include "swapChain.hpp"
#include "device.hpp"
#include "renderSetup.hpp"



namespace rendr{

    class Renderer{

private:
    const int framesInFlight = 2;  
    int currentFrame;
    rendr::Device device_;
    rendr::SwapChain swapChain_;
    std::vector<rendr::RenderSetup> rendrSetups_;
    std::vector<vk::raii::CommandBuffer> commandBuffers_;
    std::vector<rendr::PerFrameSync> framesSyncObjs_;
    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    std::vector<rendr::DrawableObj> objsToDraw;

public:
    void drawFrame();
    void setDrawableObjects(std::vector<rendr::DrawableObj> objs);
private:
    void recordCommandBuffer();    
};
}