#include "renderer.hpp"

namespace rendr{


void Renderer::drawFrame(){


}

void Renderer::recordCommandBuffer(){
    for (int i = 0; i < setupIndexToDrawableObjs.size(); ++i ) {
        //for(DrawableObj& obj : setupIndexToDrawableObjs[i]){
            // commandBuffer.setViewport(0, viewport);
            // commandBuffer.setScissor(0, scissor);
            // commandBuffer.beginRenderPass(rendrSetups_[i].renderPass_, vk::SubpassContents::eInline);
            // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *rendrSetups_[i].graphicsPipeline);
            
            //obj.bindResources(commandBuffer);
            
            //нужно знать кол-во индексов
            // commandBuffer.drawIndexed(static_cast<uint32_t>(singleSimpleMatMeshes_[batchIndex].indices.size()), 1, 0, 0, 0);
            // commandBuffer.endRenderPass();
        //}        
    }
    // commandBuffer.end();
}

}