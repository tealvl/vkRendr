#include "renderer.hpp"

namespace rendr{


void Renderer::drawFrame(){


}

void Renderer::recordCommandBuffer(){
    for (DrawableObj obj : objsToDraw) {
        rendr::RenderSetup setup = obj.getRendererSetupRef();


        commandBuffer.beginRenderPass(setup.renderPass_, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline_);
        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);
        commandBuffer.bindVertexBuffers(0, *(batches_[batchIndex].vertices.buffer), {0});
        commandBuffer.bindIndexBuffer(*(batches_[batchIndex].indices.buffer), 0, vk::IndexType::eUint32);
    
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *pipelineLayout_,
            0,
            *descriptorSets_[currentFrame][batchIndex],
            {}
        );
        //нужно знать кол-во индексов
        commandBuffer.drawIndexed(static_cast<uint32_t>(singleSimpleMatMeshes_[batchIndex].indices.size()), 1, 0, 0, 0);
        commandBuffer.endRenderPass();
    }

    commandBuffer.end();

}
}