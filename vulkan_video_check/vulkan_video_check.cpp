#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cstring>

// Function to check if a specific extension is supported by a physical device
bool isExtensionSupported(VkPhysicalDevice physicalDevice, const char* extensionName) {
    uint32_t extensionCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate device extension properties!" << std::endl;
        return false;
    }
    
    if (extensionCount == 0) {
        return false;
    }
    
    std::vector<VkExtensionProperties> extensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
    
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to get device extension properties!" << std::endl;
        return false;
    }
    
    // Check if the requested extension is in the list
    return std::any_of(extensions.begin(), extensions.end(),
        [extensionName](const VkExtensionProperties& ext) {
            return strcmp(ext.extensionName, extensionName) == 0;
        });
}

// Function to get device type as string
std::string getDeviceTypeString(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown";
    }
}

int main() {
    // Initialize Vulkan instance
    VkInstance instance;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Video Encode Checker";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance! Error code: " << result << std::endl;
        std::cerr << "Make sure Vulkan is properly installed on your system." << std::endl;
        return 1;
    }

    // Enumerate physical devices
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0) {
        std::cerr << "Failed to find any Vulkan physical devices!" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to enumerate physical devices!" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::cout << "Found " << deviceCount << " Vulkan physical device(s)" << std::endl;
    std::cout << "=============================================" << std::endl;

    bool anyDeviceSupportsExtension = false;

    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDevice device = devices[i];
        
        // Get device properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        
        // Get device features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        
        std::cout << "\nDevice #" << (i + 1) << ":" << std::endl;
        std::cout << "  Name: " << deviceProperties.deviceName << std::endl;
        std::cout << "  Type: " << getDeviceTypeString(deviceProperties.deviceType) << std::endl;
        std::cout << "  API Version: " 
                  << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                  << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
                  << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
        std::cout << "  Driver Version: " << deviceProperties.driverVersion << std::endl;
        std::cout << "  Vendor ID: 0x" << std::hex << deviceProperties.vendorID << std::dec << std::endl;
        
        // Check for video related extensions
        const std::vector<const char*> videoExtensions = {
            "VK_KHR_video_queue",
            "VK_KHR_video_decode_queue",
            "VK_KHR_video_encode_queue",
            "VK_KHR_video_decode_h264",
            "VK_KHR_video_decode_h265",
            "VK_KHR_video_encode_h264",
            "VK_KHR_video_encode_h265"
        };

        for (const char* extensionName : videoExtensions) {
            bool supportsExtension = isExtensionSupported(device, extensionName);
            
            std::cout << "  " << std::left << std::setw(35) << extensionName << " support: " 
                      << (supportsExtension ? "\033[1;32mYES\033[0m" : "\033[1;31mNO\033[0m") << std::endl;
            
            if (supportsExtension) {
                if (strcmp(extensionName, "VK_KHR_video_encode_queue") == 0) {
                    anyDeviceSupportsExtension = true;
                }

                // Get extension properties to show version information
                uint32_t extensionCount = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
                
                for (const auto& ext : extensions) {
                    if (strcmp(ext.extensionName, extensionName) == 0) {
                        std::cout << "    Extension Version: " << ext.specVersion << std::endl;
                        break;
                    }
                }
            }
        }
    }

    std::cout << "\n=============================================" << std::endl;
    if (anyDeviceSupportsExtension) {
        std::cout << "\033[1;32mAt least one device supports VK_KHR_video_encode_queue!\033[0m" << std::endl;
        std::cout << "You can use hardware-accelerated video encoding with Vulkan." << std::endl;
    } else {
        std::cout << "\033[1;31mNo devices support VK_KHR_video_encode_queue.\033[0m" << std::endl;
        std::cout << "Hardware-accelerated video encoding is not available on this system." << std::endl;
        
        // Suggest checking for the base video queue extension
        std::cout << "\nNote: You might want to check if VK_KHR_video_queue is supported instead," << std::endl;
        std::cout << "as VK_KHR_video_encode_queue builds upon it." << std::endl;
    }

    // Clean up
    vkDestroyInstance(instance, nullptr);
    
    return 0;
}
