#include "utility.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace rendr{

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
//TODO Добавить настройку для выбора физического устройства
bool isPhysicalDeviceSuitable(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface) {
    vk::PhysicalDeviceFeatures features = device.getFeatures();
    vk::PhysicalDeviceProperties properties = device.getProperties();
    
    QueueFamilyIndices indices = findQueueFamilies(*device, *surface);
    bool extensionsSupported = checkDeviceExtensionSupport(*device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(*device, &supportedFeatures);

    return 
    indices.isComplete() 
    && extensionsSupported 
    && swapChainAdequate  
    && supportedFeatures.samplerAnisotropy 
    &&  properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu 
    && features.geometryShader;
}

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance const & instance, vk::raii::SurfaceKHR const & surface){
    vk::raii::PhysicalDevice physicalDevice(nullptr);
    vk::raii::PhysicalDevices devices( instance );
    
    bool suitableDevicePicked = false;
    for (const auto& device : devices) {
        if (isPhysicalDeviceSuitable(device, surface)) {
            physicalDevice = device;
            suitableDevicePicked = true;
            break;
        }
    }
    if (!suitableDevicePicked) {
        throw std::runtime_error("failed to find a suitable physical device!");
    }
    return physicalDevice;
}
//TODO все типы очередей
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport){
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    details.formats = device.getSurfaceFormatsKHR(*surface);
    details.presentModes = device.getSurfacePresentModesKHR(*surface);
    
    return details;
}

DeviceWithQueues createDeviceWithQueues( vk::raii::PhysicalDevice const & physicalDevice,  vk::raii::SurfaceKHR const & surface){
    QueueFamilyIndices indices = findQueueFamilies(*physicalDevice, *surface);
    
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
   
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo( vk::DeviceQueueCreateFlags(), indices.graphicsFamily.value(), 1, &queuePriority));
    }

    //TODO вынести требования к устройству
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfos.size()); 
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data()); 
    deviceCreateInfo.setEnabledExtensionCount(deviceExtensions.size()); 
    deviceCreateInfo.setPpEnabledExtensionNames(deviceExtensions.data()); 
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures); 
    deviceCreateInfo.setFlags(vk::DeviceCreateFlags());

    vk::raii::Device device(physicalDevice, deviceCreateInfo);
    vk::raii::Queue graphicsQueue(device, indices.graphicsFamily.value(), 0);
    vk::raii::Queue presentQueue (device, indices.presentFamily.value(), 0);
    
    return DeviceWithQueues{std::move(device), std::move(graphicsQueue), std::move(presentQueue)};
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities, std::pair<int,int> const & winFramebufferSize) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(winFramebufferSize.first),
            static_cast<uint32_t>(winFramebufferSize.second)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainData createSwapChain(vk::raii::PhysicalDevice const & physicalDevice, vk::raii::SurfaceKHR const & surface, vk::raii::Device const & device, rendr::Window const & win) 
{
    rendr::SwapChainSupportDetails swapChainSupport = rendr::querySwapChainSupport(physicalDevice, surface);
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, win.getFramebufferSize());
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; 
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = *surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    rendr::QueueFamilyIndices indices = rendr::findQueueFamilies(*physicalDevice, *surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vk::raii::SwapchainKHR swapChain(device, createInfo);

    return {std::move(swapChain), surfaceFormat.format, extent};
}

vk::raii::ImageView createImageView(vk::raii::Device const & device, vk::Image const & image, vk::Format const & format, vk::ImageAspectFlags aspectFlags) {
    vk::ImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.image = image;
    viewCreateInfo.viewType = vk::ImageViewType::e2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    vk::raii::ImageView imageView(device, viewCreateInfo);

    return imageView;
}

std::vector<vk::raii::ImageView> createImageViews(std::vector<vk::Image> const & images, vk::raii::Device const & device, vk::Format const & format, vk::ImageAspectFlags aspectFlags)
{
    std::vector<vk::raii::ImageView> swapChainImageViews;
    swapChainImageViews.reserve(images.size());
    for (auto & image : images) {
        
       swapChainImageViews.push_back(createImageView(device, image, format, aspectFlags));
    }
    return swapChainImageViews;
}

vk::Format findSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice, std::vector<vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

//TODO вынести форматы в конфигурацию
vk::Format findDepthFormat(const vk::raii::PhysicalDevice& physicalDevice) {
    return findSupportedFormat(
        physicalDevice,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::raii::RenderPass createRenderPass(
    const vk::raii::Device& device,
    const std::vector<vk::AttachmentDescription>& attachments,
    const std::vector<vk::SubpassDescription>& subpasses,
    const std::vector<vk::SubpassDependency>& dependencies) {

    vk::RenderPassCreateInfo renderPassInfo(
        {},
        static_cast<uint32_t>(attachments.size()), attachments.data(),
        static_cast<uint32_t>(subpasses.size()), subpasses.data(),
        static_cast<uint32_t>(dependencies.size()), dependencies.data()
    );

    return vk::raii::RenderPass(device, renderPassInfo);
}

vk::raii::RenderPass createRenderPassWithColorAndDepthAttOneSubpass(const vk::raii::Device& device, vk::Format swapChainImageFormat, vk::Format depthFormat) {
    vk::AttachmentDescription colorAttachment(
        {},
        swapChainImageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentDescription depthAttachment(
        {},
        depthFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::AttachmentReference colorAttachmentRef(0,vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    
    vk::SubpassDescription subpass(
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentRef,
        nullptr, &depthAttachmentRef
    );

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        {}, 
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    );

    std::vector<vk::AttachmentDescription> attachments = { colorAttachment, depthAttachment };
    std::vector<vk::SubpassDescription> subpasses = { subpass };
    std::vector<vk::SubpassDependency> dependencies = { dependency };

    return createRenderPass(device, attachments, subpasses, dependencies);
}


vk::raii::DescriptorSetLayout createDescriptorSetLayout(
        const vk::raii::Device& device,
        std::vector<vk::DescriptorSetLayoutBinding> bindings) {
        
        vk::DescriptorSetLayoutCreateInfo layoutInfo(
            {},
            static_cast<uint32_t>(bindings.size()),
            bindings.data()
        );

        return vk::raii::DescriptorSetLayout(device, layoutInfo);
    }


vk::raii::DescriptorSetLayout createUboAndSamplerDescriptorSetLayout(const vk::raii::Device& device) {
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
        0,
        vk::DescriptorType::eUniformBuffer,
        1,
        vk::ShaderStageFlagBits::eVertex,
        nullptr
    );

    vk::DescriptorSetLayoutBinding samplerLayoutBinding(
        1,
        vk::DescriptorType::eCombinedImageSampler,
        1,
        vk::ShaderStageFlagBits::eFragment,
        nullptr
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

    return createDescriptorSetLayout(device, bindings);
}

vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo(
        {},
        code.size(),
        reinterpret_cast<const uint32_t*>(code.data())
    );

    return vk::raii::ShaderModule(device, createInfo);
}

vk::raii::PipelineLayout createPipelineLayout(
    const vk::raii::Device& device,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const std::vector<vk::PushConstantRange>& pushConstantRanges = {}) {

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        static_cast<uint32_t>(pushConstantRanges.size()),
        pushConstantRanges.data()
    );

    return vk::raii::PipelineLayout(device, pipelineLayoutInfo);
}

vk::raii::Pipeline createGraphicsPipeline(
    const vk::raii::Device& device,
    const vk::raii::PipelineLayout& pipelineLayout,
    const vk::raii::RenderPass& renderPass,
    const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
    const vk::PipelineVertexInputStateCreateInfo& vertexInputInfo,
    const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly,
    const vk::PipelineViewportStateCreateInfo& viewportState,
    const vk::PipelineRasterizationStateCreateInfo& rasterizer,
    const vk::PipelineMultisampleStateCreateInfo& multisampling,
    const vk::PipelineColorBlendStateCreateInfo& colorBlending,
    const vk::PipelineDepthStencilStateCreateInfo& depthStencil,
    const vk::PipelineDynamicStateCreateInfo& dynamicState) {

    vk::GraphicsPipelineCreateInfo pipelineInfo(
        {}, // flags
        static_cast<uint32_t>(shaderStages.size()), // stageCount
        shaderStages.data(), // pStages
        &vertexInputInfo, // pVertexInputState
        &inputAssembly, // pInputAssemblyState
        nullptr, // pTessellationState
        &viewportState, // pViewportState
        &rasterizer, // pRasterizationState
        &multisampling, // pMultisampleState
        &depthStencil, // pDepthStencilState
        &colorBlending, // pColorBlendState
        &dynamicState, // pDynamicState
        *pipelineLayout, // layout
        *renderPass, // renderPass
        0, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    );

    return vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

vk::raii::Pipeline createGraphicsPipelineWithDefaults(
    const vk::raii::Device& device,
    const vk::raii::RenderPass& renderPass,
    const vk::raii::PipelineLayout& pipelineLayout,
    vk::Extent2D swapChainExtent) {

    std::vector<char> vertShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/fvertex.spv");
    std::vector<char> fragShaderCode = rendr::readFile("C:/Dev/cpp-projects/engine/src/shaders/ffragment.spv");

    vk::raii::ShaderModule vertShaderModule = rendr::createShaderModule(device, vertShaderCode);
    vk::raii::ShaderModule fragShaderModule = rendr::createShaderModule(device, fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
        {}, // flags
        vk::ShaderStageFlagBits::eVertex, // stage
        *vertShaderModule, // module
        "main" // pName
    );

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
        {}, // flags
        vk::ShaderStageFlagBits::eFragment, // stage
        *fragShaderModule, // module
        "main" // pName
    );

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = VertexPCT::getBindingDescription();
    auto attributeDescriptions = VertexPCT::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
        {}, // flags
        1, // vertexBindingDescriptionCount
        &bindingDescription, // pVertexBindingDescriptions
        static_cast<uint32_t>(attributeDescriptions.size()), // vertexAttributeDescriptionCount
        attributeDescriptions.data() // pVertexAttributeDescriptions
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, // flags
        vk::PrimitiveTopology::eTriangleList, // topology
        VK_FALSE // primitiveRestartEnable
    );

    vk::Viewport viewport(
        0.0f, // x
        0.0f, // y
        static_cast<float>(swapChainExtent.width), // width
        static_cast<float>(swapChainExtent.height), // height
        0.0f, // minDepth
        1.0f // maxDepth
    );

    vk::Rect2D scissor({ 0, 0 }, swapChainExtent); // offset, extent

    vk::PipelineViewportStateCreateInfo viewportState(
        {}, // flags
        1, // viewportCount
        &viewport, // pViewports
        1, // scissorCount
        &scissor // pScissors
    );

    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, // flags
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill, // polygonMode
        vk::CullModeFlagBits::eBack, // cullMode
        vk::FrontFace::eCounterClockwise, // frontFace
        VK_FALSE, // depthBiasEnable
        0.0f, // depthBiasConstantFactor
        0.0f, // depthBiasClamp
        0.0f, // depthBiasSlopeFactor
        1.0f // lineWidth
    );

    vk::PipelineMultisampleStateCreateInfo multisampling(
        {}, // flags
        vk::SampleCountFlagBits::e1, // rasterizationSamples
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // pSampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
        VK_FALSE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eZero, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // alphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA // colorWriteMask
    );

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, // flags
        VK_FALSE, // logicOpEnable
        vk::LogicOp::eCopy, // logicOp
        1, // attachmentCount
        &colorBlendAttachment, // pAttachments
        { 0.0f, 0.0f, 0.0f, 0.0f } // blendConstants
    );

    vk::StencilOpState stencilOpState(
        vk::StencilOp::eKeep, // failOp
        vk::StencilOp::eKeep, // passOp
        vk::StencilOp::eKeep, // depthFailOp
        vk::CompareOp::eAlways, // compareOp
        0, // compareMask
        0, // writeMask
        0 // reference
    );

    vk::PipelineDepthStencilStateCreateInfo depthStencil(
        {}, // flags
        VK_TRUE, // depthTestEnable
        VK_TRUE, // depthWriteEnable
        vk::CompareOp::eLess, // depthCompareOp
        VK_FALSE, // depthBoundsTestEnable
        VK_FALSE, // stencilTestEnable
        stencilOpState, // front
        stencilOpState, // back
        0.0f, // minDepthBounds
        1.0f // maxDepthBounds
    );

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState(
        {}, // flags
        static_cast<uint32_t>(dynamicStates.size()), // dynamicStateCount
        dynamicStates.data() // pDynamicStates
    );

    return createGraphicsPipeline(
        device,
        pipelineLayout,
        renderPass,
        shaderStages,
        vertexInputInfo,
        inputAssembly,
        viewportState,
        rasterizer,
        multisampling,
        colorBlending,
        depthStencil,
        dynamicState
    );
}


uint32_t findMemoryType(vk::raii::PhysicalDevice const & physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

rendr::Image createImage(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::Device& device,
    vk::MemoryPropertyFlags properties,
    vk::ImageCreateInfo imageInfo,
    vk::ImageViewCreateInfo imageViewInfo) {

    rendr::Image image;
    image.image = device.createImage(imageInfo);
    vk::MemoryRequirements memRequirements = image.image.getMemoryRequirements();
    
    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size, // allocationSize
        findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    );

    image.imageMemory = device.allocateMemory(allocInfo);

    image.image.bindMemory(*image.imageMemory, 0);

    imageViewInfo.image = *image.image;
    image.imageView = device.createImageView(imageViewInfo);

    return image;
}

rendr::Image createDepthImage(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::Device& device,
    uint32_t width,
    uint32_t height){
    
    vk::Format format = findDepthFormat(physicalDevice);
    vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    vk::ImageCreateInfo imageInfo(
        {}, // flags
        vk::ImageType::e2D, // imageType
        format, // format
        vk::Extent3D(width, height, 1), // extent
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1, // samples
        tiling, // tiling
        usage, // usage
        vk::SharingMode::eExclusive, // sharingMode
        0, // queueFamilyIndexCount
        nullptr, // pQueueFamilyIndices
        vk::ImageLayout::eUndefined // initialLayout
    );

    vk::ImageViewCreateInfo viewInfo(
        {}, // flags
        {}, // image
        vk::ImageViewType::e2D, // viewType
        format, // format
        {}, // components
        {   // subresourceRange
            vk::ImageAspectFlagBits::eDepth, //aspectMask
            0, //baseMipLevel
            1, //levelCount
            0, //baseArrayLayer
            1 //layerCount
        } 
    );

    return createImage(physicalDevice, device, properties, imageInfo, viewInfo);
}

std::vector<vk::raii::Framebuffer> createSwapChainFramebuffersWithDepthAtt(
    const vk::raii::Device& device, 
    const vk::raii::RenderPass& renderPass,
    const std::vector<vk::raii::ImageView>& swapChainImageViews,
    const vk::raii::ImageView& depthImageView, 
    uint32_t width, uint32_t height){
    
    std::vector<vk::raii::Framebuffer> framebuffers;
    framebuffers.reserve(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::vector<vk::ImageView> attachments = {
            *swapChainImageViews[i],
            *depthImageView
        };

        vk::FramebufferCreateInfo framebufferInfo(
        {},
        *renderPass,
        attachments,
        width,
        height,
        1
        );

        framebuffers.push_back(vk::raii::Framebuffer(device, framebufferInfo));
    }
    return framebuffers;
}


vk::raii::CommandPool createGraphicsCommandPool( const vk::raii::Device& device,  const rendr::QueueFamilyIndices& queueFamilyIndices)
{
    vk::CommandPoolCreateInfo poolCreateInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient,
        queueFamilyIndices.graphicsFamily.value()
    );
    return vk::raii::CommandPool(device, poolCreateInfo);

}

rendr::Buffer createBuffer(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo(
    {},
    size,
    usage,
    vk::SharingMode::eExclusive);
    rendr::Buffer buffer;
    buffer.buffer = vk::raii::Buffer(device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memAllocInfo(
        size,
        findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    );

    buffer.bufferMemory = vk::raii::DeviceMemory(device, memAllocInfo);
    buffer.buffer.bindMemory(*(buffer.bufferMemory), 0);
    
    return buffer;
}

vk::raii::CommandBuffer beginSingleTimeCommands(const vk::raii::Device &device, const vk::raii::CommandPool& commandPool) {
    vk::CommandBufferAllocateInfo buffAllocInfo(
        *commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    );
    vk::raii::CommandBuffers buffers(device, buffAllocInfo);
    vk::raii::CommandBuffer buffer = std::move(buffers[0]);

    vk::CommandBufferBeginInfo beginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );
    buffer.begin(beginInfo);

    return buffer;
}

void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Queue& quequeToSubmit ) {
    commandBuffer.end();
    vk::SubmitInfo submitInfo({},{}, *commandBuffer);
    quequeToSubmit.submit({submitInfo});
}

void writeTransitionImageLayoutBarrier(const vk::raii::CommandBuffer& singleTimeCommandBuffer, const vk::raii::Image& image, vk::Format format,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    
    vk::ImageMemoryBarrier barrier(
        {}, // srcAccessMask
        {}, // dstAccessMask
        oldLayout, // oldLayout
        newLayout, // newLayout
        VK_QUEUE_FAMILY_IGNORED, // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED, // dstQueueFamilyIndex
        *image, // image
        vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor, // aspectMask
            0, // baseMipLevel
            1, // levelCount
            0, // baseArrayLayer
            1 // layerCount
        )
    );

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setSrcAccessMask({});
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    singleTimeCommandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        {},
        nullptr, nullptr,
        barrier
    );
}
    
void writeCopyBufferToImageCommand(const vk::raii::CommandBuffer& singleTimeCommandBuffer, const vk::raii::Buffer& buffer, const vk::raii::Image& image, uint32_t width, uint32_t height) {
    vk::BufferImageCopy region(
        0, // bufferOffset
        0, // bufferRowLength
        0, // bufferImageHeight
        vk::ImageSubresourceLayers(
            vk::ImageAspectFlagBits::eColor, // aspectMask
            0, // mipLevel
            0, // baseArrayLayer
            1 // layerCount
        ),
        vk::Offset3D(0, 0, 0), // imageOffset
        vk::Extent3D(width, height, 1) // imageExtent
    );

    singleTimeCommandBuffer.copyBufferToImage(
        *buffer,
        *image,
        vk::ImageLayout::eTransferDstOptimal,
        region
    );
}

//writes commands | TransitionImageLayoutBarr | CopyBufferToImage | TransitionImageLayoutBarr | to singleTimeCommandBuffer
Image create2DTextureImage(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, 
    const vk::raii::CommandBuffer& singleTimeCommandBuffer, STBImage ImageData){

    vk::DeviceSize imageSize = ImageData.getWidth() * ImageData.getHeight() * 4;

    rendr::Buffer stagingBuffer = createBuffer(physicalDevice, device, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    void* data = stagingBuffer.bufferMemory.mapMemory(0, imageSize);
        memcpy(data, ImageData.getDataPtr(), static_cast<size_t>(imageSize));
    stagingBuffer.bufferMemory.unmapMemory();

    vk::ImageCreateInfo imageCreateInfo(
        {},
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Srgb,
        vk::Extent3D(ImageData.getWidth(), ImageData.getHeight(),  1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0, // queueFamilyIndexCount
        nullptr, // pQueueFamilyIndices
        vk::ImageLayout::eUndefined // initialLayout
    );

    vk::ImageViewCreateInfo imageViewCreateInfo(
        {}, //flags
        {}, //image
        vk::ImageViewType::e2D, // viewType, 
        vk::Format::eR8G8B8A8Srgb,
        {}, // components
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    );

    rendr::Image textureImage = createImage(physicalDevice, device, vk::MemoryPropertyFlagBits::eDeviceLocal, imageCreateInfo, imageViewCreateInfo);
     
    writeTransitionImageLayoutBarrier(singleTimeCommandBuffer, textureImage.image, vk::Format::eR8G8B8A8Srgb, 
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    writeCopyBufferToImageCommand(singleTimeCommandBuffer, stagingBuffer.buffer, textureImage.image, ImageData.getWidth(), ImageData.getHeight());

    writeTransitionImageLayoutBarrier(singleTimeCommandBuffer, textureImage.image, vk::Format::eR8G8B8A8Srgb, 
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    return textureImage;
}

vk::raii::Sampler createTextureSampler(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice) {
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo(
        {}, // flags
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eLinear, // mipmapMode
        vk::SamplerAddressMode::eRepeat, // addressModeU
        vk::SamplerAddressMode::eRepeat, // addressModeV
        vk::SamplerAddressMode::eRepeat, // addressModeW
        0.0f, // mipLodBias
        VK_TRUE, // anisotropyEnable
        properties.limits.maxSamplerAnisotropy, // maxAnisotropy
        VK_FALSE, // compareEnable
        vk::CompareOp::eAlways, // compareOp
        0.0f, // minLod
        0.0f, // maxLod
        vk::BorderColor::eIntOpaqueBlack, // borderColor
        VK_FALSE // unnormalizedCoordinates
    );

    return vk::raii::Sampler(device, samplerInfo);
}

std::pair<std::vector<VertexPCT>, std::vector<uint32_t>>  loadModel(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::vector<VertexPCT> vertices;
    std::vector<uint32_t> indices;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<VertexPCT, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            VertexPCT vertex{};


            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }        
    }

    return {std::move(vertices), std::move(indices)};
}


void writeCopyBufferCommand(const vk::raii::CommandBuffer& singleTimeCommandBuffer, const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
    std::vector<vk::BufferCopy> copyRegions = {vk::BufferCopy(0,0,size)};
    singleTimeCommandBuffer.copyBuffer(*srcBuffer, *dstBuffer, std::move(copyRegions));
}

//TODO отвязаться от конкретного типа вершин (сделать функцию шаблонной?)
//writes copyBuffer command to singleTimeCommandBuffer
rendr::Buffer createVertexBuffer(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, 
    const vk::raii::CommandBuffer& singleTimeCommandBuffer, std::vector<VertexPCT> vertices){

    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    rendr::Buffer stagingBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eHostVisible |  vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void* data = stagingBuffer.bufferMemory.mapMemory(0,bufferSize);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    stagingBuffer.bufferMemory.unmapMemory();


    rendr::Buffer vertexBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    writeCopyBufferCommand(singleTimeCommandBuffer, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    return vertexBuffer;
}

























}