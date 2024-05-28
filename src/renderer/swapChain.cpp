#include "swapChain.hpp"

rendr::SwapChain::SwapChain()
: swapChain_(nullptr){}

void rendr::SwapChain::create(const rendr::Device &renderDevice, const rendr::Window &win){
    rendr::SwapChainData swapChainData = rendr::createSwapChain(renderDevice.physicalDevice_, renderDevice.surface_, renderDevice.device_, win);
    swapChain_ = std::move(swapChainData.swapChain);
    swapChainExtent_ = std::move(swapChainData.swapChainExtent);
    swapChainImageFormat_ = std::move(swapChainData.swapChainImageFormat);
    swapChainImages_ = swapChain_.getImages();
    swapChainImageViews_ = rendr::createImageViews(swapChainImages_, renderDevice.device_, swapChainImageFormat_, vk::ImageAspectFlagBits::eColor);
}

void rendr::SwapChain::clear(){
    for (auto& imageView : swapChainImageViews_) {
        imageView.clear();
    }
    swapChain_.clear();
}
