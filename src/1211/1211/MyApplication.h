
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

// Debug フラグ
#ifdef NDEBUG
// Vulkan は、高速化のために常にエラーチェックをするわけではない。
// といっても、何も表示されないとデバッグが困難なので、
// 開発中に使うものとして、エラーチェックを行う層(Layer)に置き換えられるようにしている
// この、エラーチェック用のシステムを挟むのが、validation Layer
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class MyApplication
{
private:
	constexpr static char APP_NAME[] = "Vulkan Application";

	GLFWwindow* window_;
	VkInstance instance_;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger_;// デバッグメッセージを伝えるオブジェクト

public:
	MyApplication() : window_(nullptr) {}
	~MyApplication() {}

	void run()
	{
		// 初期化
		initializeWindow();
		initializeVulkan();

		// 通常処理
		mainloop();

		// 後片付け
		finalizeVulkan();
		finalizeWindow();
	}

private:
	// 表示ウィンドウの設定
	void initializeWindow()
	{
		const int WIDTH = 800;
		const int HEIGHT = 600;

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);// OpenGL の種類の設定
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);// ユーザーはウィンドウサイズを変更できない

		window_ = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
	}

	void finalizeWindow()
	{
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	// 通常の処理
	void mainloop()
	{
		while (!glfwWindowShouldClose(window_))
		{
			glfwPollEvents();
		}
	}

	// Vulkanの設定
	void initializeVulkan()
	{
		createInstance(&instance_);
		initializeDebugMessenger(instance_, debugMessenger_);
		physicalDevice_ = pickPhysicalDevice(instance_);
	}

	void finalizeVulkan()
	{
		finalizeDebugMessenger(instance_, debugMessenger_);
		vkDestroyInstance(instance_, nullptr);
	}

	static void createInstance(VkInstance* dest)
	{
		// アプケーション情報を定めるための構造体
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// 構造体の種類
		appInfo.pApplicationName = APP_NAME;						// アプリケーション名
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);		// 開発者が決めるバージョン番号
		appInfo.pEngineName = "My Engine";							// ゲームエンジン名
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);			// ゲームエンジンのバージョン
		appInfo.apiVersion = VK_API_VERSION_1_0;					// 使用するAPIのバージョン

		// 新しく作られるインスタンスの設定の構造体
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// 構造体の種類
		createInfo.pApplicationInfo = &appInfo;						// VkApplicationInfoの情報

		// valkanの拡張機能を取得して、初期化データに追加
		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			static const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};

			// 検証レイヤーの確認
			if (!checkValidationLayerSupport(validationLayers)) {
				throw std::runtime_error("validation layers requested, but not available!");
			}

			// インスタンスへの設定
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// デバッグメッセンジャーもその後に引き続いて作成する
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = populateDebugMessengerCreateInfo();
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		// インスタンスの生成
		if (vkCreateInstance(&createInfo, nullptr, dest) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	static std::vector<const char*> getRequiredExtensions()
	{
		// 拡張の個数を検出
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

#ifdef _DEBUG
		// 有効なエクステンションの表示
		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension << std::endl;
		}
#endif

		return extensions;
	}

	// 検証レイヤーに対応しているか確認
	static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		// レイヤーのプロパティを取得
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);				// レイヤー数の取得
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());// プロパティ自体の取得

		// 全てのレイヤーが検証レイヤーに対応しているか確認
		for (const char* layerName : validationLayers) {
			if (![](const char* layerName, const auto& availableLayers) {
				// レイヤーが検証レイヤーのどれかを持っているか確認
				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) { return true; }
				}
				return false;
				}(layerName, availableLayers)) {
				return false;
			}// どこかのレイヤーがvalidationLayersのレイヤーに対応していないのはダメ
		}

		return true;
	}

	/*** デバイスの選択 ***/
	static VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance)
	{
		// デバイス数の取得
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

		// デバイスの取得
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// 適切なデバイスを選出(最高得点のデバイスを使用する)
		VkPhysicalDevice best_device = VK_NULL_HANDLE;
		int best_score = 0;
		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			if (best_score < score) {
				best_device = device;
				best_score = score;
			}
		}

		// 使える物理デバイスがなければ大問題
		if (best_device == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU!");

		return best_device;
	}

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;

		bool isComplete() {
			return graphicsFamily.has_value();// 今回は、登録されていればok
		}
	};

	static int rateDeviceSuitability(const VkPhysicalDevice device)
	{
		// デバイスに関する情報を取得
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// 外付けGPUなら高評価
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

		// 最大テクスチャ解像度を性能の評価値に加える
		score += deviceProperties.limits.maxImageDimension2D;

		// テッセレーションシェーダに対応していないと問題外(テッセセレーションを使う場合)
//		if (!deviceFeatures.tessellationShader) return 0;

		// Queue Familyの確認
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isComplete()) return 0;

#ifdef _DEBUG
		// デバイス名の表示
		std::cout << "Physical Device: " << deviceProperties.deviceName
			<< " (score: " << score << ")" << std::endl;
#endif // _DEBUG
		return score;
	}

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		// キューファミリーの数を取得
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		// キューファミリーを取得
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef _DEBUG
		std::cout << std::endl;
#endif // _DEBUG

		int i = 0;
		QueueFamilyIndices indices;
		for (const auto& queueFamily : queueFamilies) {

#ifdef _DEBUG
			std::cout << "queueFamily: " << queueFamily.queueCount << "quque(s)" << std::endl;
#endif // _DEBUG

			// キューファミリーにキューがあり、グラフィックスキューとして使えるか調べる
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	/*** debugMessenger の処理 ***/
	// 初期化
	static void initializeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();// 生成情報の構築

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {// 生成
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// debugMessenger の生成情報の作成
	static VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |	// 診断メッセージ
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |								// リソースの作成などの情報メッセージ(かなり表示される)
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |							// 必ずしもエラーではないが、アプリケーションのバグである可能性が高い動作に関するメッセージ
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;								// 無効であり、クラッシュを引き起こす可能性のある動作に関するメッセージ
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |			// 仕様またはパフォーマンスとは無関係のイベントが発生
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |							// 仕様に違反する、または間違いの可能性を示すものが発生
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;							// Vulkanの最適でない使用の可能性
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

	// debugMessenger の生成
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		// vkCreateDebugUtilsMessengerEXTに対応しているか確認して実行
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr) return VK_ERROR_EXTENSION_NOT_PRESENT;
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	// 片付け
	static void finalizeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		// vkCreateDebugUtilsMessengerEXTに対応しているか確認して実行
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func == nullptr) return;
		func(instance, debugMessenger, nullptr);
	}
};