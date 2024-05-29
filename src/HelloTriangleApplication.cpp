#include "HelloTriangleApplication.hpp"


void Application::run(){
    init();
    mainLoop();
}

void Application::init(){
    rendr::DeviceConfig deviceConfig;
    deviceConfig.deviceEnableFeatures.setSamplerAnisotropy(true);
    
    device_.create(deviceConfig, window_);
    swapChain_.create(device_, window_);

    renderPass_ = rendr::createRenderPassWithColorAndDepthAttOneSubpass(device_.device_, swapChain_.swapChainImageFormat_, rendr::findDepthFormat(device_.physicalDevice_));
    descriptorSetLayout_ = rendr::createUboAndSamplerDescriptorSetLayout(device_.device_);
    pipelineLayout_ = rendr::createPipelineLayout(device_.device_, {*descriptorSetLayout_}, {});

    std::vector<char> vertShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/fvertex.spv");
    std::vector<char> fragShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/ffragment.spv");
    vk::raii::ShaderModule vertShaderModule = rendr::createShaderModule(device_.device_, vertShaderCode);
    vk::raii::ShaderModule fragShaderModule = rendr::createShaderModule(device_.device_, fragShaderCode);

    graphicsPipeline_ = rendr::createGraphicsPipelineWithDefaults(device_.device_, renderPass_, pipelineLayout_, swapChain_.swapChainExtent_, rendr::VertexPTN{},
        vertShaderModule, fragShaderModule
    );

    depthImage_ = rendr::createDepthImage(device_.physicalDevice_, device_.device_, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
    swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device_.device_, renderPass_, swapChain_.swapChainImageViews_, depthImage_.imageView, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
    commandPool_ =  rendr::createGraphicsCommandPool(device_.device_, rendr::findQueueFamilies(*device_.physicalDevice_, *device_.surface_));
    
    textureSampler_ = rendr::createTextureSampler(device_.device_, device_.physicalDevice_);


    rendr::UfbxSceneRaii fbxScene("C:/Dev/cpp-projects/engine/resources/zen-studio/source/room.fbx");
    auto meshesAndMatInd = rendr::ufbxLoadMeshesPartsSepByMaterial(fbxScene.get());  
    auto matToMesh = rendr::mergeMeshesByMaterial(meshesAndMatInd);
    
    meshesAndMatInd.clear();
    rendr::STBImageRaii walls("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_walls_Base_color.png");  
    rendr::STBImageRaii details("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_details_Base_color.png");  
    rendr::Image wallsTextureImage = rendr::create2DTextureImage(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, std::move(walls));
    rendr::Image detailsTextureImage = rendr::create2DTextureImage(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, std::move(details));
    rendr::SimpleMaterial wallsMat (0, std::move(wallsTextureImage));
    rendr::SimpleMaterial detailsMat (2, std::move(detailsTextureImage));

    simpleMaterials_.push_back(std::move(wallsMat));
    simpleMaterials_.push_back(std::move(detailsMat));
    singleSimpleMatMeshes_.push_back(std::move(matToMesh[0]));
    singleSimpleMatMeshes_.push_back(std::move(matToMesh[2]));


    batches_.push_back(
    rendr::Batch(
        rendr::createVertexBuffer(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, singleSimpleMatMeshes_[0].vertices),
        rendr::createIndexBuffer(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, singleSimpleMatMeshes_[0].indices),
        &simpleMaterials_[0]
    ));
    batches_.push_back(
    rendr::Batch(
        rendr::createVertexBuffer(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, singleSimpleMatMeshes_[1].vertices),
        rendr::createIndexBuffer(device_.physicalDevice_, device_.device_, device_.commandPool_, device_.graphicsQueue_, singleSimpleMatMeshes_[1].indices),
        &simpleMaterials_[1]
    ));
    
    uniformBuffers_ = rendr::createAndMapUniformBuffers(device_.physicalDevice_, device_.device_, uniformBuffersMapped_, FramesInFlight, rendr::MVPUniformBufferObject());
    descriptorPool_ = rendr::createDescriptorPool(device_.device_, FramesInFlight, batches_.size());
    descriptorSets_ = rendr::createUboAndSamplerDescriptorSets(device_.device_, descriptorPool_, descriptorSetLayout_, uniformBuffers_,
        textureSampler_, FramesInFlight, batches_, rendr::MVPUniformBufferObject()
    );
    commandBuffers_ = rendr::createCommandBuffers(device_.device_, device_.commandPool_, FramesInFlight);
    framesSyncObjs_ = rendr::createSyncObjects(device_.device_, FramesInFlight);


    window_.callbacks.winResized = [this](int,int){
        this->framebufferResized = true;
        this->recreateSwapChain();
    };

    inputManager_.setUpWindowCallbacks(window_);
    camManip_.setCamera(camera_);
    camManip_.setInputManager(inputManager_);
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
    device_.device_.waitIdle();
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

    device_.device_.waitIdle();

    cleanupSwapChain();

    
    swapChain_.create(device_, window_);

    depthImage_ = rendr::createDepthImage(device_.physicalDevice_, device_.device_, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
    swapChainFramebuffers_ = rendr::createSwapChainFramebuffersWithDepthAtt(device_.device_, renderPass_, swapChain_.swapChainImageViews_, depthImage_.imageView, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
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
        vk::Rect2D({0, 0}, swapChain_.swapChainExtent_), // renderArea
        clearValues // clearValues
    );

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline_);

    vk::Viewport viewport(
        0.0f, 0.0f,
        static_cast<float>(swapChain_.swapChainExtent_.width),
        static_cast<float>(swapChain_.swapChainExtent_.height),
        0.0f, 1.0f
    );
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor({0, 0}, swapChain_.swapChainExtent_);
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
    
    vk::Result waitFanceRes = device_.device_.waitForFences({*framesSyncObjs_[currentFrame].inFlightFence}, VK_TRUE, UINT64_MAX);

    std::pair<vk::Result, uint32_t> imageAcqRes = swapChain_.swapChain_.acquireNextImage(UINT64_MAX, 
        *framesSyncObjs_[currentFrame].imageAvailableSemaphore, nullptr);

    uint32_t imageIndex = imageAcqRes.second;

    updateUniformBuffer(currentFrame);

    device_.device_.resetFences({*framesSyncObjs_[currentFrame].inFlightFence});

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

    device_.graphicsQueue_.submit({submitInfo}, *framesSyncObjs_[currentFrame].inFlightFence);

    vk::PresentInfoKHR presentInfo(
        1, // waitSemaphoreCount
        &(*framesSyncObjs_[currentFrame].renderFinishedSemaphore), // pWaitSemaphores
        1, // swapchainCount
        &(*swapChain_.swapChain_), // pSwapchains
        &imageIndex // pImageIndices
    );
    
    vk::Result presenrRes = device_.presentQueue_.presentKHR(presentInfo);

    currentFrame = (currentFrame + 1) % FramesInFlight;
}

void Application::updateUniformBuffer(uint32_t currentFrame){
    camera_.aspect = swapChain_.swapChainExtent_.width / (float) swapChain_.swapChainExtent_.height;
    
    rendr::MVPUniformBufferObject ubo;
    //ubo.model = glm::rotate(model_matrix_.model_, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.model = glm::scale(model_matrix_.model_, glm::vec3(0.01f, 0.01f, 0.01f));
    ubo.view = glm::lookAt(camera_.pos, camera_.look_at_pos, camera_.worldUp);

    ubo.proj = glm::perspective(camera_.fov, camera_.aspect, camera_.near_plane, camera_.far_plane);

    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped_[currentFrame], &ubo, sizeof(ubo));
}
