#include "renderer.hpp"

namespace rendr{


void Renderer::drawFrame(){


}

void Renderer::setDrawableObjects(const std::vector<DrawableObj> &objs){
    
}

void Renderer::initMaterial(Material& material){
    material.renderSetupIndex = addRenderSetup(material.createRendererSetup(device_, swapChain_));
}

void Renderer::init(RendererConfig){
    depthImage_ = rendr::createDepthImage(device_.physicalDevice_, device_.device_, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
    
}

int Renderer::addRenderSetup(RendererSetup setup)
{
    rendrSetups_.push_back(setup);
    return rendrSetups_.size() - 1;
}

void Renderer::recordCommandBuffer()
{
    for (int i = 0; i < setupIndexToDrawableObjs.size(); ++i ) {

        // commandBuffer.setViewport(0, viewport);
        // commandBuffer.setScissor(0, scissor);
        // commandBuffer.beginRenderPass(rendrSetups_[i].renderPass_, vk::SubpassContents::eInline);
        // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *rendrSetups_[i].graphicsPipeline);
        //for(DrawableObj& obj : setupIndexToDrawableObjs[i]){
            //obj.bindResources(commandBuffer);            
            //нужно знать кол-во индексов
            // commandBuffer.drawIndexed(static_cast<uint32_t>(singleSimpleMatMeshes_[batchIndex].indices.size()), 1, 0, 0, 0);
            // commandBuffer.endRenderPass();
        //}        
    }
    // commandBuffer.end();
}
}