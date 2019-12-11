#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class MyApplication
{
private:
	GLFWwindow* window_;
	constexpr static char APP_NAME[] = "Vulkan Application";


public:
	MyApplication(): window_(nullptr) {}
	~MyApplication() {}


	VkInstance instance_;
	void run()
	{
		initializeWindow();
		initializeVulkan();

		mainloop();

		finalizeWindow();
		finalizeWindow();
	}


	void initializeWindow()
	{
		const int WIDTH = 800;
		const int HEIGHT = 600;

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window_ = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
	}

	void finalizeWindow()
	{
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	void mainloop()
	{
		while (!glfwWindowShouldClose(window_))
		{
			glfwPollEvents();
		}
	}

	void initializeVulkan()
	{
		createInstance(&instance_);
	}

	void finalizeVulkan()
	{
		vkDestroyInstance(instance_, nullptr);
	}

	static void createInstance(VkInstance* dest)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    
		appInfo.pApplicationName = APP_NAME;                  
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "My Engine";         
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);    
		appInfo.apiVersion = VK_API_VERSION_1_0;   

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
		createInfo.pApplicationInfo = &appInfo;                    

		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		if (vkCreateInstance(&createInfo, nullptr, dest) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}


	static std::vector<const char*> getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
#ifdef _DEBUG
		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension << std::endl;
		}
#endif
		return extensions;
	}

	static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);	
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			if (![](const char* layerName, const auto& availableLayers)
				{
				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0) { return true; }
				}
				return false;
				}(layerName, availableLayers)) {
				return false;
			}
		}

		return true;
	}


	static void initializeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	static VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |	
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |								// リソースの作成などの情報メッセージ(かなり表示される)
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |	
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;		
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;							
		createInfo.pfnUserCallback = [](
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) -> VKAPI_ATTR VkBool32
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		};

		return createInfo;
	}

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr) return VK_ERROR_EXTENSION_NOT_PRESENT;
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	static void finalizeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func == nullptr) return;
		func(instance, debugMessenger, nullptr);
	}
};

