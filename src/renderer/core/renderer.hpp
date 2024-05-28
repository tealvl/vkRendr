#pragma once
#include <vector>
#include <map>
#include "utility.hpp"
#include "resourceDataTypes.hpp"

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

    std::map<int, std::vector<rendr::DrawableObj*>> setupIndexToDrawableObjs;

public:
    void drawFrame();
    void setDrawableObjects(const std::vector<rendr::DrawableObj>& objs);
    
    //return setupIndex
    int addRenderSetup(rendr::RenderSetup setup);
private:
    void recordCommandBuffer();    
};
}