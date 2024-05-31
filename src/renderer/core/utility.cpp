#include "utility.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace rendr{

bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requiredExtensions) {
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensionsCopy(requiredExtensions.begin(), requiredExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensionsCopy.erase(extension.extensionName);
    }

    return requiredExtensionsCopy.empty();
}


bool isPhysicalDeviceSuitable(vk::raii::PhysicalDevice const & device, vk::raii::SurfaceKHR const & surface, const DeviceConfig& config) {
    
    vk::PhysicalDeviceFeatures features = device.getFeatures();
    vk::PhysicalDeviceProperties properties = device.getProperties();
    bool featuresSupport = config.isDeviceFeaturesSuitable(features);
    bool propertiesSupport = config.isDevicePropertiesSuitable(properties);

    QueueFamilyIndices indices = findQueueFamilies(*device, *surface);
    bool familyIndicesSupport = config.isDeviceFamilyIndicesSuitable(indices);

    bool extensionsSupported = checkDeviceExtensionSupport(*device, config.requiredDeviceExtensions);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return featuresSupport && propertiesSupport && swapChainAdequate && extensionsSupported && familyIndicesSupport;
}

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance const & instance, vk::raii::SurfaceKHR const & surface, const DeviceConfig& config){
    vk::raii::PhysicalDevice physicalDevice(nullptr);
    vk::raii::PhysicalDevices devices( instance );
    
    bool suitableDevicePicked = false;
    for (const auto& device : devices) {
        if (isPhysicalDeviceSuitable(device, surface, config)) {
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

QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
    uint32_t familyIndex = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = familyIndex;
        }
        VkBool32 presentSupport = false;
       
        if(device.getSurfaceSupportKHR(familyIndex, surface)){
            indices.presentFamily = familyIndex;
        }

        //TODO найти все типы очередей
        if (indices.isGraphicsAndPresent()) {
            break;
        }
        familyIndex++;
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

DeviceWithGraphicsAndPresentQueues createDeviceWithGraphicsAndPresentQueues( vk::raii::PhysicalDevice const & physicalDevice,  vk::raii::SurfaceKHR const & surface, const DeviceConfig& config){
    QueueFamilyIndices indices = findQueueFamilies(*physicalDevice, *surface);
    
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
   
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo( vk::DeviceQueueCreateFlags(), indices.graphicsFamily.value(), 1, &queuePriority));
    }

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfos.size()); 
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data()); 
    deviceCreateInfo.setEnabledExtensionCount(config.requiredDeviceExtensions.size()); 
    deviceCreateInfo.setPpEnabledExtensionNames(config.requiredDeviceExtensions.data()); 
    deviceCreateInfo.setPEnabledFeatures(&config.deviceEnableFeatures); 
    deviceCreateInfo.setFlags(vk::DeviceCreateFlags());

    vk::raii::Device device(physicalDevice, deviceCreateInfo);
    vk::raii::Queue graphicsQueue(device, indices.graphicsFamily.value(), 0);
    vk::raii::Queue presentQueue (device, indices.presentFamily.value(), 0);
    
    return DeviceWithGraphicsAndPresentQueues{std::move(device), std::move(graphicsQueue), std::move(presentQueue)};
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats,
    const rendr::SwapChainConfig& config) {
    for (const auto& availableFormat : availableFormats) {
        if (config.isSurfaceFormatSuitable(availableFormat)) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes,
    const rendr::SwapChainConfig& config) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (config.isPresentModeSuitable(availablePresentMode)) {
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

SwapChainData createSwapChain(
    vk::raii::PhysicalDevice const & physicalDevice, 
    vk::raii::SurfaceKHR const & surface, 
    vk::raii::Device const & device, 
    rendr::Window const & win,
    const rendr::SwapChainConfig& config) 
{
    rendr::SwapChainSupportDetails swapChainSupport = rendr::querySwapChainSupport(physicalDevice, surface);
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, config);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, config);
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


std::vector<vk::raii::DescriptorSet> createDescriptorSets(
    const vk::raii::Device& device,  
    const vk::raii::DescriptorPool& descriptorPool,
    const vk::raii::DescriptorSetLayout& descriptorSetLayout, 
    int maxFramesInFlight){ 

    std::vector<vk::raii::DescriptorSet> descriptorSets;
    std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, static_cast<uint32_t>(layouts.size()), layouts.data());
    descriptorSets = device.allocateDescriptorSets(allocInfo);

    return descriptorSets;
}

vk::raii::DescriptorSetLayout createUboDescriptorSetLayout(const vk::raii::Device& device) {
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
        0, // binding
        vk::DescriptorType::eUniformBuffer,
        1, // descriptorCount
        vk::ShaderStageFlagBits::eVertex,
        nullptr
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {uboLayoutBinding};

    return createDescriptorSetLayout(device, bindings);
}

vk::raii::DescriptorSetLayout createSamplerDescriptorSetLayout(const vk::raii::Device& device) {
    vk::DescriptorSetLayoutBinding samplerLayoutBinding(
        0,  // binding
        vk::DescriptorType::eCombinedImageSampler,
        1, // descriptorCount
        vk::ShaderStageFlagBits::eFragment,
        nullptr
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {samplerLayoutBinding};

    return createDescriptorSetLayout(device, bindings);
}


vk::raii::DescriptorSetLayout createUboAndSamplerDescriptorSetLayout(const vk::raii::Device& device) {
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
        0, // binding
        vk::DescriptorType::eUniformBuffer,
        1, // descriptorCount
        vk::ShaderStageFlagBits::eVertex,
        nullptr
    );

    vk::DescriptorSetLayoutBinding samplerLayoutBinding(
        1,  // binding
        vk::DescriptorType::eCombinedImageSampler,
        1, // descriptorCount
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
        std::max(size, memRequirements.size),
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

    std::vector<vk::raii::CommandBuffer> buffers = device.allocateCommandBuffers(buffAllocInfo);
    vk::raii::CommandBuffer buffer = std::move(buffers[0]);

    vk::CommandBufferBeginInfo beginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );
    buffer.begin(beginInfo);

    return buffer;
}

void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Queue& queueToSubmit ) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo({}, {}, *commandBuffer);
    queueToSubmit.submit({ submitInfo });
    queueToSubmit.waitIdle();
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
    
void writeCopyBufferToImageCommand(const vk::raii::CommandBuffer& singleTimeCommandBuffer, vk::raii::Buffer& buffer, 
    const vk::raii::Image& image, uint32_t width, uint32_t height) {
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

Image create2DTextureImage(
    const vk::raii::PhysicalDevice &physicalDevice, 
    const vk::raii::Device &device, 
    const vk::raii::CommandPool& commandPool,
    const vk::raii::Queue& graphicsQueue,
    STBImageRaii ImageData){
    
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
     
    vk::raii::CommandBuffer singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device, commandPool);
        writeTransitionImageLayoutBarrier(singleTimeCommandBuffer, textureImage.image, vk::Format::eR8G8B8A8Srgb, 
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue);

    singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device, commandPool);
        writeCopyBufferToImageCommand(singleTimeCommandBuffer, stagingBuffer.buffer, textureImage.image, ImageData.getWidth(), ImageData.getHeight());
    rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue);

    singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device, commandPool);
        writeTransitionImageLayoutBarrier(singleTimeCommandBuffer, textureImage.image, vk::Format::eR8G8B8A8Srgb, 
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue);
    
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


rendr::Mesh<VertexPTN> convertUfbxMeshPart(ufbx_mesh *mesh, ufbx_mesh_part *part, ufbx_matrix* transformMat) {
    std::vector<VertexPTN> vertices;
    std::vector<uint32_t> tri_indices;
    tri_indices.resize(mesh->max_face_triangles * 3);

    for (uint32_t face_index : part->face_indices) {
        ufbx_face face = mesh->faces[face_index];

        uint32_t num_tris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);

        for (size_t i = 0; i < num_tris * 3; i++) {
            uint32_t index = tri_indices[i];

            VertexPTN vertex;
            ufbx_vec3 pos = mesh->vertex_position[index];
            ufbx_vec3 transformedPos = ufbx_transform_position(transformMat, pos);


            vertex.pos = glm::vec3(transformedPos.x, transformedPos.y, transformedPos.z);
            
            if (mesh->vertex_normal.exists) {
                vertex.normal = glm::vec3(pos.x, pos.y, pos.z);
            } else {
                vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f); 
            }

            if (mesh->vertex_uv.exists) {
                vertex.texCoord = glm::vec2(mesh->vertex_uv[index].x, 1.0f - mesh->vertex_uv[index].y);
            } else {
                vertex.texCoord = glm::vec2(0.0f, 0.0f); 
            }
            vertices.push_back(std::move(vertex));
        }
    }

    assert(vertices.size() == part->num_triangles * 3);

    ufbx_vertex_stream streams[1] = {
        { vertices.data(), vertices.size(), sizeof(VertexPTN) },
    };

    std::vector<uint32_t> indices;
    indices.resize(part->num_triangles * 3);

    size_t num_vertices = ufbx_generate_indices(streams, 1, indices.data(), indices.size(), nullptr, nullptr);

    vertices.resize(num_vertices);


    return {std::move(vertices), std::move(indices)};
}

ufbx_scene* ufbxOpenScene(const std::string& filepath, bool blender_flag = true) {

    ufbx_load_opts opts = { 0 };
    ufbx_error error; 
    ufbx_scene *scene = ufbx_load_file(filepath.data(), &opts, &error);
    if (!scene) {
        throw(std::runtime_error(error.info));
    }
    return scene;
}

void ufbxCloseScene(ufbx_scene* scene_ptr){
    ufbx_free_scene(scene_ptr);
}

std::vector<std::pair<rendr::Mesh<VertexPTN>, uint32_t>> ufbxLoadMeshesPartsSepByMaterial(ufbx_scene* scene) {

    std::vector<std::pair<rendr::Mesh<VertexPTN>, uint32_t>> meshesParts;
    for (size_t i = 0; i < scene->nodes.count; i++) {
        ufbx_node* node = scene->nodes.data[i];

        if (node->mesh) {
            ufbx_mesh* mesh = node->mesh;
            ufbx_matrix meshTransform = node->geometry_to_world;
            
            for (size_t j = 0; j < mesh->material_parts.count; j++) {
                ufbx_mesh_part* part = &mesh->material_parts.data[j];
                
                if(part->num_faces == 0) continue;   

                uint32_t sceneMatIndex;
                uint32_t partFaceIndex = part->face_indices[0];
                uint32_t matInMeshIndex = mesh->face_material[partFaceIndex];
                
                for(size_t k = 0; k < scene->materials.count; k++){
                    if(!strcmp(mesh->materials[matInMeshIndex]->name.data, scene->materials[k]->name.data)){
                        sceneMatIndex = k;
                    }
                }                                     
                meshesParts.push_back({convertUfbxMeshPart(mesh, part, &meshTransform), sceneMatIndex});
            }
        }
    } 

    return meshesParts;
}

void writeCopyBufferCommand(const vk::raii::CommandBuffer& singleTimeCommandBuffer, const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
    std::vector<vk::BufferCopy> copyRegions = {vk::BufferCopy(0,0,size)};
    singleTimeCommandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegions);
}

rendr::Buffer createIndexBuffer(const vk::raii::PhysicalDevice &physicalDevice, 
    const vk::raii::Device &device,
    const vk::raii::CommandPool& commandPool,
    const vk::raii::Queue& graphicsQueue,
    const std::vector<uint32_t>& indices){

    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    rendr::Buffer stagingBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eHostVisible |  vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void* data = stagingBuffer.bufferMemory.mapMemory(0, bufferSize);
        memcpy(data, indices.data(), (size_t) bufferSize);
    stagingBuffer.bufferMemory.unmapMemory();

    rendr::Buffer indexBuffer = createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    vk::raii::CommandBuffer singleTimeCommandBuffer = rendr::beginSingleTimeCommands(device, commandPool);
        writeCopyBufferCommand(singleTimeCommandBuffer, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);
    rendr::endSingleTimeCommands(singleTimeCommandBuffer, graphicsQueue);
    
    return indexBuffer;
}

//TODO отвязаться от конкретного типа юниформ буфера (сделать функцию шаблонной?)
std::vector<rendr::Buffer> createAndMapUniformBuffers(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::Device &device, 
    std::vector<void*>& uniformBuffersMappedData, size_t numOfBuffers, MVPUniformBufferObject ubo) {
    
    vk::DeviceSize bufferSize = sizeof(ubo);
    std::vector<rendr::Buffer> uniformBuffers;
    uniformBuffers.reserve(numOfBuffers);
    uniformBuffersMappedData.clear();
    uniformBuffersMappedData.reserve(numOfBuffers);

    for (size_t i = 0; i < numOfBuffers; i++) {
        uniformBuffers.push_back(createBuffer(physicalDevice, device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, 
                                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
        
        uniformBuffersMappedData.push_back(uniformBuffers.back().bufferMemory.mapMemory(0, bufferSize));
    }

    return uniformBuffers;
}


vk::raii::DescriptorPool createDescriptorPool(const vk::raii::Device& device, uint32_t maxFramesInFlight) {
    std::array<vk::DescriptorPoolSize, 2> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, maxFramesInFlight),
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxFramesInFlight)
    };

    vk::DescriptorPoolCreateInfo poolInfo(
        {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet}, // flags
        maxFramesInFlight, // maxSets
        static_cast<uint32_t>(poolSizes.size()), // poolSizeCount
        poolSizes.data() // pPoolSizes
    );

    return vk::raii::DescriptorPool(device, poolInfo);
}


std::vector<vk::raii::CommandBuffer> createCommandBuffers(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool, uint32_t framesInFlight) {
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::CommandBufferAllocateInfo allocInfo(
        *commandPool, // commandPool
        vk::CommandBufferLevel::ePrimary, // level
        framesInFlight // commandBufferCount
    );

    commandBuffers = device.allocateCommandBuffers(allocInfo);

    return commandBuffers;
}

std::vector<rendr::PerFrameSync> createSyncObjects(const vk::raii::Device& device, uint32_t framesInFlight) {
    
    std::vector<rendr::PerFrameSync> syncObjects(framesInFlight);
    syncObjects.resize(framesInFlight);

    vk::SemaphoreCreateInfo semaphoreInfo(
        {}, // flags
        nullptr
    );

    vk::FenceCreateInfo fenceInfo(
        vk::FenceCreateFlagBits::eSignaled // flags
    );

    for (size_t i = 0; i < framesInFlight; i++) {
        syncObjects[i].imageAvailableSemaphore = vk::raii::Semaphore(device, semaphoreInfo);
        syncObjects[i].renderFinishedSemaphore = vk::raii::Semaphore(device, semaphoreInfo);
        syncObjects[i].inFlightFence = vk::raii::Fence(device, fenceInfo);
    }

    return syncObjects;
}


RendererSetup::RendererSetup():
descriptorSetLayout_(nullptr), 
renderPass_(nullptr),
pipelineLayout_(nullptr),
graphicsPipeline_(nullptr),
swapChainFramebuffers_(),
descriptorPool_(nullptr)
{}


SwapChain::SwapChain()
: swapChain_(nullptr){}

void SwapChain::create(const rendr::Device &renderDevice, const rendr::Window &win, const rendr::SwapChainConfig& config){
    rendr::SwapChainData swapChainData = rendr::createSwapChain(renderDevice.physicalDevice_, renderDevice.surface_, renderDevice.device_, win, config);
    swapChain_ = std::move(swapChainData.swapChain);
    swapChainExtent_ = std::move(swapChainData.swapChainExtent);
    swapChainImageFormat_ = std::move(swapChainData.swapChainImageFormat);
    swapChainImages_ = swapChain_.getImages();
    swapChainImageViews_ = rendr::createImageViews(swapChainImages_, renderDevice.device_, swapChainImageFormat_, vk::ImageAspectFlagBits::eColor);
}

void SwapChain::clear(){
    for (auto& imageView : swapChainImageViews_) {
        imageView.clear();
    }
    swapChain_.clear();
}


Device::Device():
instance_(),
surface_(nullptr),
physicalDevice_(nullptr),
device_(nullptr),
graphicsQueue_(nullptr),
presentQueue_(nullptr),
commandPool_(nullptr)
{}

void Device::create(DeviceConfig config, const rendr::Window& win){
    instance_ = rendr::Instance(win);
    surface_ = win.createSurface(*instance_);
    physicalDevice_ = pickPhysicalDevice(*instance_, surface_, config);
    rendr::DeviceWithGraphicsAndPresentQueues deviceAndQueues = rendr::createDeviceWithGraphicsAndPresentQueues(physicalDevice_, surface_, config);
    device_ = std::move(deviceAndQueues.device);
    graphicsQueue_ = std::move(deviceAndQueues.graphicsQueue);
    presentQueue_ = std::move(deviceAndQueues.presentQueue);
    commandPool_ =  rendr::createGraphicsCommandPool(device_, rendr::findQueueFamilies(*physicalDevice_, *surface_));
}



const std::string AppInfo::name = "My Application";
const std::string AppInfo::engineName = "My Engine";
const int AppInfo::version = 1; 
const int AppInfo::engineVersion = 1; 
const uint32_t AppInfo::apiVersion = VK_API_VERSION_1_3; 

const std::vector<const char*> DebugConfig::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

bool checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : DebugConfig::validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; 
}

Instance::Instance() : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr) {}

Instance::Instance(Window const &window) : context_(), instance_(nullptr), debugUtilsMessenger_(nullptr)
{
    if (DebugConfig::enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo applicationInfo(
        AppInfo::name.data(),
        AppInfo::version,
        AppInfo::engineName.data(),
        AppInfo::engineVersion,
        AppInfo::apiVersion);

    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoOutsideInstance{};
    if (DebugConfig::enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(DebugConfig::validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = DebugConfig::validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfoOutsideInstance);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfoOutsideInstance;
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    std::vector<const char *> extensions = window.getRequiredExtensions();

    if (DebugConfig::enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    instance_ = vk::raii::Instance(context_, instanceCreateInfo);

    if (DebugConfig::enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoInsideInstance{};
        populateDebugMessengerCreateInfo(debugCreateInfoInsideInstance);
        debugUtilsMessenger_ = vk::raii::DebugUtilsMessengerEXT(instance_, debugCreateInfoInsideInstance);
    }
}
Instance::Instance(Instance &&other) noexcept
: context_(std::move(other.context_)),
    instance_(std::move(other.instance_)),
    debugUtilsMessenger_(std::move(other.debugUtilsMessenger_)) 
{}

Instance &Instance::operator=(Instance &&other) noexcept{
    if (this != &other) {
        context_ = std::move(other.context_);
        instance_ = std::move(other.instance_);
        debugUtilsMessenger_ = std::move(other.debugUtilsMessenger_);
    }
    return *this;
}
vk::raii::Instance const &Instance::operator*() const{
    return instance_;
}


void Renderer::cleanupSwapChain(){
    for(auto& setup : rendrSetups_){
        setup.second.swapChainFramebuffers_.clear();
    }

    depthImage_.imageView.clear();
    depthImage_.image.clear();
    depthImage_.imageMemory.clear();
    swapChain_.clear();
}

void Renderer::updateUniformBuffer(rendr::MVPUniformBufferObject ubo){
    memcpy(uniformBuffersMapped_[currentFrame_], &ubo, sizeof(ubo));
}

Renderer::Renderer()
: descriptorSetLayout_(nullptr), descriptorPool_(nullptr){}

void Renderer::drawFrame()
{
    vk::Result waitFanceRes = device_.device_.waitForFences({*framesSyncObjs_[currentFrame_].inFlightFence}, VK_TRUE, UINT64_MAX);

    std::pair<vk::Result, uint32_t> imageAcqRes = swapChain_.swapChain_.acquireNextImage(UINT64_MAX, 
        *framesSyncObjs_[currentFrame_].imageAvailableSemaphore, nullptr);

    uint32_t imageIndex = imageAcqRes.second;

    device_.device_.resetFences({*framesSyncObjs_[currentFrame_].inFlightFence});

    commandBuffers_[currentFrame_].reset();
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submitInfo(
        1, // waitSemaphoreCount
        &(*framesSyncObjs_[currentFrame_].imageAvailableSemaphore), // pWaitSemaphores
        &waitStages, // pWaitDstStageMask
        1, // commandBufferCount
        &(*commandBuffers_[currentFrame_]), // pCommandBuffers
        1, // signalSemaphoreCount
        &(*framesSyncObjs_[currentFrame_].renderFinishedSemaphore) // pSignalSemaphores
    );

    device_.graphicsQueue_.submit({submitInfo}, *framesSyncObjs_[currentFrame_].inFlightFence);

    vk::PresentInfoKHR presentInfo(
        1, // waitSemaphoreCount
        &(*framesSyncObjs_[currentFrame_].renderFinishedSemaphore), // pWaitSemaphores
        1, // swapchainCount
        &(*swapChain_.swapChain_), // pSwapchains
        &imageIndex // pImageIndices
    );
    
    vk::Result presenrRes = device_.presentQueue_.presentKHR(presentInfo);

    currentFrame_ = (currentFrame_ + 1) % framesInFlight_;
}

void Renderer::setDrawableObjects(std::vector<IDrawableObj*> objs){
    for(auto& objs : setupIndexToDrawableObjs){
        objs.second.clear();
    }

    for(auto& obj : objs){
        int setupInd = obj->renderMaterial->renderSetupIndex;
        setupIndexToDrawableObjs[setupInd].push_back(obj);
    }
}

void Renderer::initMaterial(Material& material){
    material.renderSetupIndex = matIndCount_;
    rendrSetups_[matIndCount_] = material.createRendererSetup(*this, descriptorSetLayout_, framesInFlight_);
    matIndCount_++;
}

void Renderer::recreateSwapChain(const rendr::Window& window){
    auto size = window.getFramebufferSize();
    int width = size.first;
    int height = size.second;
    while (width == 0 || height == 0) {
        size = window.getFramebufferSize();
        width = size.first;
        height = size.second;
        window.waitEvents();
    }
    device_.device_.waitIdle();
    cleanupSwapChain();
    swapChain_.create(device_, window, swapChainConfig_);

    depthImage_ = rendr::createDepthImage(device_.physicalDevice_, device_.device_, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);

    for(auto& setup : rendrSetups_){
        setup.second.swapChainFramebuffersRecreationFunc_(*this, setup.second);
    }
}

void Renderer::waitIdle(){
    device_.device_.waitIdle();
}

float Renderer::getSwapChainAspect(){
    return swapChain_.swapChainExtent_.width / (float) swapChain_.swapChainExtent_.height;
}

void Renderer::init(const RendererConfig& config, rendr::Window& window){
    framesInFlight_ = config.framesInFlight;
    device_.create(config.deviceConfig, window);
    swapChain_.create(device_, window, config.swapChainConfig);
    swapChainConfig_ = config.swapChainConfig;
    depthImage_ = rendr::createDepthImage(device_.physicalDevice_, device_.device_, swapChain_.swapChainExtent_.width, swapChain_.swapChainExtent_.height);
    uniformBuffers_ = rendr::createAndMapUniformBuffers(device_.physicalDevice_, device_.device_, uniformBuffersMapped_, framesInFlight_, rendr::MVPUniformBufferObject());
    commandBuffers_ = rendr::createCommandBuffers(device_.device_, device_.commandPool_, framesInFlight_);
    framesSyncObjs_ = rendr::createSyncObjects(device_.device_, framesInFlight_);

    descriptorSetLayout_ = rendr::createUboDescriptorSetLayout(device_.device_);
    descriptorPool_ = rendr::createDescriptorPool(device_.device_, framesInFlight_);
    descriptorSets_ = rendr::createDescriptorSets(device_.device_, descriptorPool_, descriptorSetLayout_, framesInFlight_);

    for(int i = 0; i < framesInFlight_; i++){
        vk::DescriptorBufferInfo bufferInfo(
            *uniformBuffers_[i].buffer, // buffer
            0, // offset
            sizeof(rendr::MVPUniformBufferObject) // range
        );

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {
            vk::WriteDescriptorSet(
                *descriptorSets_[i], // dstSet
                0, // dstBinding
                0, // dstArrayElement
                1, // descriptorCount
                vk::DescriptorType::eUniformBuffer, // descriptorType
                nullptr, // pImageInfo  
                &bufferInfo, // pBufferInfo
                nullptr // pTexelBufferView
            )};
        device_.device_.updateDescriptorSets(descriptorWrites, nullptr);
    }
    
    window.callbacks.winResized = [this, &window](int,int){
        this->recreateSwapChain(window);
    };
}

void Renderer::recordCommandBuffer(uint32_t imageIndex){
    vk::raii::CommandBuffer& commandBuffer = commandBuffers_[currentFrame_];
    
    vk::CommandBufferBeginInfo beginInfo(
        {}, // flags
        nullptr // pInheritanceInfo
    );
    commandBuffer.begin(beginInfo);

    for (auto& objs : setupIndexToDrawableObjs ) {
    
        rendr::RendererSetup& setup = rendrSetups_[objs.first];
        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].color = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

        vk::RenderPassBeginInfo renderPassInfo(
            *setup.renderPass_, // renderPass
            *setup.swapChainFramebuffers_[imageIndex], // framebuffer
            vk::Rect2D({0, 0}, swapChain_.swapChainExtent_), // renderArea
            clearValues // clearValues
        );

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *setup.graphicsPipeline_);

        vk::Viewport viewport(
            0.0f, 0.0f,
            static_cast<float>(swapChain_.swapChainExtent_.width),
            static_cast<float>(swapChain_.swapChainExtent_.height),
            0.0f, 1.0f
        );
        commandBuffer.setViewport(0, viewport);

        vk::Rect2D scissor({0, 0}, swapChain_.swapChainExtent_);
        commandBuffer.setScissor(0, scissor);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *setup.pipelineLayout_, 0, *descriptorSets_[currentFrame_],{});
        
        for(auto& obj : objs.second){
            obj->bindResources(device_.device_, commandBuffer, setup.pipelineLayout_, currentFrame_);  

            commandBuffer.drawIndexed(obj->getNumOfDrawIndices(), 1, 0, 0, 0);
        }
        
        commandBuffer.endRenderPass();
    }        
    commandBuffer.end();
}

}

