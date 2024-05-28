#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include "renderer.hpp"

namespace rendr{

class IResourceBinder{
    virtual void bindResources(const vk::raii::CommandBuffer& buffer) = 0;
};

class DrawableObj : IResourceBinder{
    void bindResources(const vk::raii::CommandBuffer& buffer);
    virtual ~DrawableObj();
};

class Material{
private:
    int renderSetupIndex = -1;
    virtual RenderSetup createRenderSetup(){};
public:  
    void init(Renderer& renderer){
        renderSetupIndex = renderer.addRenderSetup(createRenderSetup());
    };
    virtual ~Material() = default;
};
}