#include "Application.hpp"

void Application::run(){
    init();
    mainLoop();
}

void Application::init(){
    rendr::RendererConfig renderConfig;
    renderConfig.deviceConfig.deviceEnableFeatures.setSamplerAnisotropy(true);
    renderer.init(renderConfig, window);
    
    SimpleMaterial material;
    renderer.initMaterial(material);

    MeshWithTextureObj walls(material);
    MeshWithTextureObj details(material);

    rendr::STBImageRaii wallsTex("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_walls_baked.png");  
    rendr::STBImageRaii detailsTex("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_details_Baked.png");  
    walls.loadTexture(std::move(wallsTex), renderer);    
    details.loadTexture(std::move(detailsTex), renderer);

    rendr::UfbxSceneRaii fbxScene("C:/Dev/cpp-projects/engine/resources/zen-studio/source/room.fbx");
    auto meshesAndMatInd = rendr::ufbxLoadMeshesPartsSepByMaterial(fbxScene.get());  
    auto fbxMatToMesh = rendr::mergeMeshesByMaterial(meshesAndMatInd);
    walls.loadMesh(fbxMatToMesh[0], renderer);
    details.loadMesh(fbxMatToMesh[2], renderer);

    
    textureSampler = rendr::createTextureSampler(device_.device_, device_.physicalDevice_);
    descriptorPool_ = rendr::createDescriptorPool(device_.device_, FramesInFlight, batches_.size());
    descriptorSets_ = rendr::createUboAndSamplerDescriptorSets(device_.device_, descriptorPool_, descriptorSetLayout_, uniformBuffers_,
        textureSampler_, FramesInFlight, batches_, rendr::MVPUniformBufferObject()
    );

    inputManager.setUpWindowCallbacks(window);
    camManip.setCamera(camera);
    camManip.setInputManager(inputManager);
}

void Application::mainLoop(){
    while (!window.shouldClose()) {
        window.pollEvents();
        timer.update();
        
        //TODO вынести 
        if(inputManager.isKeyPressed('Q')){
            camManip.enableMouseCameraControl();
            window.disableCursor();  
        }
        if(inputManager.isKeyPressed('E')){
            camManip.disableMouseCameraControl();
            window.enableCursor();
        }

        camManip.update(timer.getDeltaTime());
        inputManager.resetInputOffsets();

        rendr::MVPUniformBufferObject ubo = camera.getMVPubo(0.1f, renderer.getSwapChainAspect());
        renderer.updateUniformBuffer(ubo);
        //renderer.setDrawableObjs();
        renderer.drawFrame();
    }
    renderer.waitIdle();
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
