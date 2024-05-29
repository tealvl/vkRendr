#pragma once
#include <vector>
#include <map>
#include "utility.hpp"

namespace rendr{
 
class DrawableObj;
class Material;

class Renderer{
private:
    const int framesInFlight;  
    int currentFrame;
    rendr::Device device_;
    rendr::SwapChain swapChain_;
    rendr::Image depthImage_;
    std::vector<rendr::RendererSetup> rendrSetups_;
    std::vector<vk::raii::CommandBuffer> commandBuffers_;
    std::vector<rendr::PerFrameSync> framesSyncObjs_;
    std::vector<rendr::Buffer> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    std::map<int, std::vector<rendr::DrawableObj*>> setupIndexToDrawableObjs;

public:
    void drawFrame();
    void setDrawableObjects(const std::vector<rendr::DrawableObj>& objs);
    void initMaterial(Material& material);
    void init(RendererConfig);
   
private:
    int addRenderSetup(rendr::RendererSetup setup);
    void recordCommandBuffer();    
};

struct Material{
    int renderSetupIndex = -1;
    virtual RendererSetup createRendererSetup(const rendr::Device& device, const rendr::SwapChain& swapChain){};
    virtual ~Material() = default;
};

struct DrawableObj {
    Material* renderMaterial;
    virtual void bindResources(const vk::raii::CommandBuffer& buffer) = 0;
    virtual ~DrawableObj() = default;
};


}