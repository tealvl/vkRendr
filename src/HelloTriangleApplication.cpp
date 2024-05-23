#include "HelloTriangleApplication.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void Application::run(){
    mainLoop();
    cleanup();
}

void Application::mainLoop(){
    while (!window_.shouldClose()) {
        window_.pollEvents();
        drawFrame();
    }
    device_.waitIdle();
}

void Application::cleanup(){
    vkDestroyBuffer(*device_, indexBuffer, nullptr);
    vkFreeMemory(*device_, indexBufferMemory, nullptr);

    vkDestroyBuffer(*device_, vertexBuffer, nullptr);
    vkFreeMemory(*device_, vertexBufferMemory, nullptr);
    
    cleanupSwapChain();
    vkDestroySampler(*device_, textureSampler, nullptr);
    vkDestroyImageView(*device_, textureImageView, nullptr);
    vkDestroyImage(*device_, textureImage, nullptr);
    vkFreeMemory(*device_, textureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(*device_, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(*device_, descriptorPool, nullptr);

    for (size_t i = 0; i < FramesInFlight; i++) {
        vkDestroyBuffer(*device_, uniformBuffers[i], nullptr);
        vkFreeMemory(*device_, uniformBuffersMemory[i], nullptr);
    }

    for (size_t i = 0; i < FramesInFlight; i++) {
        vkDestroySemaphore(*device_, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(*device_, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(*device_, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(*device_, commandPool, nullptr);
    vkDestroyPipeline(*device_, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(*device_, pipelineLayout, nullptr);
    vkDestroyRenderPass(*device_, renderPass, nullptr);
}

void Application::cleanupSwapChain(){

    vkDestroyImageView(*device_, depthImageView, nullptr);
    vkDestroyImage(*device_, depthImage, nullptr);
    vkFreeMemory(*device_, depthImageMemory, nullptr);

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(*device_, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(*device_, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(*device_, swapChain_, nullptr);
}

void Application::recreateSwapChain()
{
    auto size = window_.getFramebufferSize();
    int width = size.first;
    int height = size.second;
    while (width == 0 || height == 0) {
        size = window_.getFramebufferSize();
        width = size.first;
        height = size.second;
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(*device_);

    cleanupSwapChain();


    //createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; 
    beginInfo.pInheritanceInfo = nullptr; 

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}

void Application::drawFrame()
{
    vkWaitForFences(*device_, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(*device_, swapChain_, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);
    
    vkResetFences(*device_, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(*graphicsQueue_, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }   

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapChain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; 
    
    result = vkQueuePresentKHR(*presentQueue_, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % FramesInFlight;
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
    // static auto startTime = std::chrono::high_resolution_clock::now();
    // auto currentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // UniformBufferObject ubo{};
    // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    // ubo.proj[1][1] *= -1;

    // memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

    UniformBufferObject ubo{};
    ubo.model = model_matrix.model_;
    camera.pos = glm::vec3(2.0f, 2.0f, 2.0f);
    ubo.view = glm::lookAt(camera.pos, camera.look_at_pos, camera.up);
    camera.aspect = swapChainExtent_.width / (float) swapChainExtent_.height;
    ubo.proj = glm::perspective(camera.fov, camera.aspect , camera.near_plane, camera.far_plane);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
