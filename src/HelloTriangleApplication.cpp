#include "HelloTriangleApplication.hpp"


void Application::run(){
    mainLoop();
}

void Application::mainLoop(){
    while (!window_.shouldClose()) {
        window_.pollEvents();
        timer_.update();
        
        //TODO вынести 
        if(inputManager_.isKeyPressed('Q')){
            camManip_.enableMouseCameraControl();
            window_.disableCursor();  
        }
        if(inputManager_.isKeyPressed('E')){
            camManip_.disableMouseCameraControl();
            window_.enableCursor();
        }

        camManip_.update(timer_.getDeltaTime());
        inputManager_.resetInputOffsets();

        drawFrame();
    }
    device_.waitIdle();
}

void Application::cleanupSwapChain() {
    // Освобождаем framebuffers
    for (auto& framebuffer : swapChainFramebuffers_) {
        framebuffer.clear();
    }

    // Освобождаем depth image
    depthImage_.imageView.clear();
    depthImage_.image.clear();
    depthImage_.imageMemory.clear();

    // Освобождаем image views
    for (auto& imageView : swapChainImageViews_) {
        imageView.clear();
    }

    // Освобождаем swap chain
    swapChain_.clear();
}

void Application::recreateSwapChain() {
    auto size = window_.getFramebufferSize();
    int width = size.first;
    int height = size.second;
    while (width == 0 || height == 0) {
        size = window_.getFramebufferSize();
        width = size.first;
        height = size.second;
        window_.waitEvents();
    }

    device_.waitIdle();

    cleanupSwapChain();

    rendr::SwapChainData swapChainData = rendr::createSwapChain(physicalDevice_, surface_, device_, window_);
    swapChain_ = std::move(swapChainData.swapChain);
    swapChainExtent_ = std::move(swapChainData.swapChainExtent);
    swapChainImageFormat_ = std::move(swapChainData.swapChainImageFormat);
    swapChainImages_ = swapChain_.getImages();
    swapChainImageViews_ = rendr::createImageViews(swapChainImages_, device_, swapChainImageFormat_, vk::ImageAspectFlagBits::eColor);

    depthImage_ = rendr::createDepthImage(physicalDevice_, device_, swapChainExtent_.width, swapChainExtent_.height);
    swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device_, renderPass_, swapChainImageViews_, depthImage_.imageView, swapChainExtent_.width, swapChainExtent_.height);
}    

void Application::recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo(
        {}, // flags
        nullptr // pInheritanceInfo
    );

    commandBuffer.begin(beginInfo);

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassInfo(
        *renderPass_, // renderPass
        *swapChainFramebuffers_[imageIndex], // framebuffer
        vk::Rect2D({0, 0}, swapChainExtent_), // renderArea
        clearValues // clearValues
    );

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline_);

    vk::Viewport viewport(
        0.0f, 0.0f,
        static_cast<float>(swapChainExtent_.width),
        static_cast<float>(swapChainExtent_.height),
        0.0f, 1.0f
    );
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor({0, 0}, swapChainExtent_);
    commandBuffer.setScissor(0, scissor);

    for (uint32_t batchIndex = 0; batchIndex < batches_.size(); ++batchIndex) {

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
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Application::drawFrame() {
    
    vk::Result waitFanceRes = device_.waitForFences({*framesSyncObjs_[currentFrame].inFlightFence}, VK_TRUE, UINT64_MAX);

    std::pair<vk::Result, uint32_t> imageAcqRes = swapChain_.acquireNextImage(UINT64_MAX, 
        *framesSyncObjs_[currentFrame].imageAvailableSemaphore, nullptr);

    uint32_t imageIndex = imageAcqRes.second;

    updateUniformBuffer(currentFrame);

    device_.resetFences({*framesSyncObjs_[currentFrame].inFlightFence});

    commandBuffers_[currentFrame].reset();
    recordCommandBuffer(commandBuffers_[currentFrame], imageIndex);

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submitInfo(
        1, // waitSemaphoreCount
        &(*framesSyncObjs_[currentFrame].imageAvailableSemaphore), // pWaitSemaphores
        &waitStages, // pWaitDstStageMask
        1, // commandBufferCount
        &(*commandBuffers_[currentFrame]), // pCommandBuffers
        1, // signalSemaphoreCount
        &(*framesSyncObjs_[currentFrame].renderFinishedSemaphore) // pSignalSemaphores
    );

    graphicsQueue_.submit({submitInfo}, *framesSyncObjs_[currentFrame].inFlightFence);

    vk::PresentInfoKHR presentInfo(
        1, // waitSemaphoreCount
        &(*framesSyncObjs_[currentFrame].renderFinishedSemaphore), // pWaitSemaphores
        1, // swapchainCount
        &(*swapChain_), // pSwapchains
        &imageIndex // pImageIndices
    );
    
    vk::Result presenrRes = presentQueue_.presentKHR(presentInfo);

    currentFrame = (currentFrame + 1) % FramesInFlight;
}

void Application::updateUniformBuffer(uint32_t currentFrame){
    camera_.aspect = swapChainExtent_.width / (float) swapChainExtent_.height;
    
    rendr::MVPUniformBufferObject ubo;
    //ubo.model = glm::rotate(model_matrix_.model_, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.model = glm::scale(model_matrix_.model_, glm::vec3(0.01f, 0.01f, 0.01f));
    ubo.view = glm::lookAt(camera_.pos, camera_.look_at_pos, camera_.worldUp);

    ubo.proj = glm::perspective(camera_.fov, camera_.aspect, camera_.near_plane, camera_.far_plane);

    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped_[currentFrame], &ubo, sizeof(ubo));
}
