#include "device.hpp"


namespace rendr{
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

}