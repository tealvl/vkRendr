#include "renderSetup.hpp"

namespace rendr{

RendererSetup::RendererSetup():
descriptorSetLayout_(nullptr), 
renderPass_(nullptr),
pipelineLayout_(nullptr),
graphicsPipeline_(nullptr),
swapChainFramebuffers_()
{}

void RendererSetup::createDefaultSetup(const rendr::Device &device, const rendr::SwapChain &SwapChain){

}




}

