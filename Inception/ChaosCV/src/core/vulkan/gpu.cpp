#include "core/vulkan/gpu.hpp"
#include "core/vulkan/vk_allocator.hpp"

namespace chaos
{
	static std::mutex g_instance_lock;

	class VulkanInstanceHolder
	{
	public:
		VulkanInstanceHolder() { instance = 0; }
		~VulkanInstanceHolder() { DestroyGPUInstance(); }

		operator VkInstance() { return instance; }
		VkInstance instance;
	};

	static VulkanInstanceHolder g_instance;

	static int g_gpu_count = 0;
	static int g_default_gpu_index = -1;

	static constexpr int MAX_GPU_COUNT = 8;
	static GPUInfo g_gpu_infos[MAX_GPU_COUNT];



	int support_VK_KHR_external_memory_capabilities = 0;
	int support_VK_KHR_get_physical_device_properties2 = 0;
	int support_VK_KHR_get_surface_capabilities2 = 0;
	int support_VK_KHR_surface = 0;
	int support_VK_EXT_debug_utils = 0;


	// VK_KHR_external_memory_capabilities
	PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR vkGetPhysicalDeviceExternalBufferPropertiesKHR = 0;

	// VK_KHR_get_physical_device_properties2
	PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR = 0;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = 0;
	PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR = 0;
	PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR = 0;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR vkGetPhysicalDeviceQueueFamilyProperties2KHR = 0;
	PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR = 0;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR vkGetPhysicalDeviceSparseImageFormatProperties2KHR = 0;

	// VK_KHR_get_surface_capabilities2
	PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR = 0;
	PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR = 0;

	// VK_KHR_surface
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = 0;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = 0;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = 0;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = 0;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = 0;


	static void InitInstanceExtension()
	{
		if (support_VK_KHR_external_memory_capabilities)
		{
			vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
		}

		if (support_VK_KHR_get_physical_device_properties2)
		{
			vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceFeatures2KHR");
			vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceProperties2KHR");
			vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceFormatProperties2KHR");
			vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
			vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
			vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
			vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
		}

		if (support_VK_KHR_get_surface_capabilities2)
		{
			vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
			vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
		}

		if (support_VK_KHR_surface)
		{
			vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(g_instance, "vkDestroySurfaceKHR");
			vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
			vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
			vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(g_instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
		}
	}



	static uint32_t FindDeviceComputeQueue(const std::vector<VkQueueFamilyProperties>& queueFamilyProperties)
	{
		// first try, compute only queue
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				&& !(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				return i;
			}
		}

		// second try, any queue with compute and graphics
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				&& (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				return i;
			}
		}

		// third try, any queue with compute
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				return i;
			}
		}

		return -1;
	}

	static uint32_t FindDeviceGraphicsQueue(const std::vector<VkQueueFamilyProperties>& queueFamilyProperties)
	{
		// first try, graphics only queue
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if ((queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				&& !(queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				return i;
			}
		}

		// second try, any queue with graphics and compute
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if ((queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				&& (queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				return i;
			}
		}

		// third try, any queue with graphics
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				return i;
			}
		}

		return -1;
	}

	static uint32_t FindDeviceTransferQueue(const std::vector<VkQueueFamilyProperties>& queueFamilyProperties)
	{
		// first try, transfer only queue
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if ((queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
				&& !(queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
				&& !(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				return i;
			}
		}

		// second try, any queue with transfer
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

			if (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				return i;
			}
		}

		// third try, use compute queue
		uint32_t compute_queue_index = FindDeviceComputeQueue(queueFamilyProperties);
		if (compute_queue_index != (uint32_t)-1)
		{
			return compute_queue_index;
		}

		// fourth try, use graphics queue
		uint32_t graphics_queue_index = FindDeviceGraphicsQueue(queueFamilyProperties);
		if (graphics_queue_index != (uint32_t)-1)
		{
			return graphics_queue_index;
		}

		//     NCNN_LOGE("no transfer queue");
		return -1;
	}

	static int FindDefaultVulkanDeviceIndex()
	{
		// first try, discrete gpu
		for (int i = 0; i < g_gpu_count; i++)
		{
			if (g_gpu_infos[i].type == 0)
				return i;
		}

		// second try, integrated gpu
		for (int i = 0; i < g_gpu_count; i++)
		{
			if (g_gpu_infos[i].type == 1)
				return i;
		}

		// third try, any probed device
		if (g_gpu_count > 0)
			return 0;

		LOG(INFO) << "no vulkan device";
		return -1;
	}

	void CreateGPUInstance()
	{
		std::lock_guard lock(g_instance_lock);

		if ((VkInstance)g_instance != 0) return;

		VkResult ret;
		
		std::vector<const char*> enabledExtensions;

		uint32_t instanceExtensionPropertyCount;
		ret = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionPropertyCount, NULL);
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumerateInstanceExtensionProperties failed %d", ret);

		std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionPropertyCount);
		ret = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionPropertyCount, instanceExtensionProperties.data());
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumerateInstanceExtensionProperties failed %d", ret);

		support_VK_KHR_get_physical_device_properties2 = 0;
		support_VK_KHR_get_surface_capabilities2 = 0;
		support_VK_KHR_surface = 0;
		support_VK_EXT_debug_utils = 0;

		for (uint32_t j = 0; j < instanceExtensionPropertyCount; j++)
		{
			const VkExtensionProperties& exp = instanceExtensionProperties[j];

			if (strcmp(exp.extensionName, "VK_KHR_external_memory_capabilities") == 0)
				support_VK_KHR_external_memory_capabilities = exp.specVersion;
			else if (strcmp(exp.extensionName, "VK_KHR_get_physical_device_properties2") == 0)
				support_VK_KHR_get_physical_device_properties2 = exp.specVersion;
			else if (strcmp(exp.extensionName, "VK_KHR_get_surface_capabilities2") == 0)
				support_VK_KHR_get_surface_capabilities2 = exp.specVersion;
			else if (strcmp(exp.extensionName, "VK_KHR_surface") == 0)
				support_VK_KHR_surface = exp.specVersion;
			else if (strcmp(exp.extensionName, "VK_EXT_debug_utils") == 0)
				support_VK_EXT_debug_utils = exp.specVersion;
		}

		if (support_VK_KHR_external_memory_capabilities)
			enabledExtensions.push_back("VK_KHR_external_memory_capabilities");
		if (support_VK_KHR_get_physical_device_properties2)
			enabledExtensions.push_back("VK_KHR_get_physical_device_properties2");
		if (support_VK_KHR_get_surface_capabilities2)
			enabledExtensions.push_back("VK_KHR_get_surface_capabilities2");
		if (support_VK_KHR_surface)
			enabledExtensions.push_back("VK_KHR_surface");

		VkApplicationInfo applicationInfo;
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = 0;
		applicationInfo.pApplicationName = "ChaosCV";
		applicationInfo.applicationVersion = 0;
		applicationInfo.pEngineName = "ChaosCV";
		applicationInfo.engineVersion = 20200727;
		applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

		VkInstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = 0;
		instanceCreateInfo.flags = 0;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledLayerCount = 0; // enabledLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = nullptr; // enabledLayers.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

		VkInstance instance = 0;
		ret = vkCreateInstance(&instanceCreateInfo, 0, &instance);
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkCreateInstance failed %d", ret);

		g_instance.instance = instance;

		InitInstanceExtension();

		uint32_t physicalDeviceCount = 0;
		ret = vkEnumeratePhysicalDevices(g_instance, &physicalDeviceCount, 0);
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumeratePhysicalDevices failed %d", ret);

		if (physicalDeviceCount > MAX_GPU_COUNT)
			physicalDeviceCount = MAX_GPU_COUNT;

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

		ret = vkEnumeratePhysicalDevices(g_instance, &physicalDeviceCount, physicalDevices.data());
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumeratePhysicalDevices failed %d", ret);

		// find proper device and queue
		int gpu_info_index = 0;
		for (uint32_t i = 0; i < physicalDeviceCount; i++)
		{
			const VkPhysicalDevice& physicalDevice = physicalDevices[i];
			GPUInfo& gpu_info = g_gpu_infos[gpu_info_index];

			// device type
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

			gpu_info.bug_storage_buffer_no_l1 = false;
			gpu_info.bug_layout_binding_id_alias = false;
			gpu_info.bug_corrupted_online_pipeline_cache = false;
			gpu_info.bug_implicit_fp16_arithmetic = false;

			if (physicalDeviceProperties.vendorID == 0x5143 
				&& physicalDeviceProperties.apiVersion < VK_MAKE_VERSION(1, 0, 49))
			{
				// qcom adreno with old buggy driver cannot handle binding id alias
				gpu_info.bug_layout_binding_id_alias = true;
			}

			if (physicalDeviceProperties.vendorID == 0x5143 
				&& physicalDeviceProperties.apiVersion < VK_MAKE_VERSION(1, 0, 66))
			{
				// qcom adreno with old buggy driver cannot share created pipeline properly
				gpu_info.bug_corrupted_online_pipeline_cache = true;
			}

			if (physicalDeviceProperties.vendorID == 0x5143 
				&& !(physicalDeviceProperties.deviceID == 0x6040001 
					|| physicalDeviceProperties.deviceID == 0x6050002))
			{
				// NOTE but qcom855/qcom855plus/qcom865 are known exceptions
				// qcom adreno storage buffer without L1 cache
				gpu_info.bug_storage_buffer_no_l1 = true;
			}

			if (physicalDeviceProperties.vendorID == 0x13b5
				&& (physicalDeviceProperties.deviceID == 0x7500001
					|| physicalDeviceProperties.deviceID == 0x8602000
					|| physicalDeviceProperties.deviceID == 0x8800020))
			{
				// these arm mali midgard era driver cannot handle binding id alias
				gpu_info.bug_layout_binding_id_alias = true;
			}

			if (physicalDeviceProperties.vendorID == 0x13b5
				&& (physicalDeviceProperties.deviceID == 0x7500001
					|| physicalDeviceProperties.deviceID == 0x8602000
					|| physicalDeviceProperties.deviceID == 0x8800020
					|| physicalDeviceProperties.deviceID == 0x70901010
					|| physicalDeviceProperties.deviceID == 0x74021000
					|| physicalDeviceProperties.deviceID == 0x60a00002
					|| physicalDeviceProperties.deviceID == 0x62210001))
			{
				// NOTE rk3288/rk3399/t880/g51/g52/g71/g72
				// however, g76/g77 has explicit fp16 arithmetic
				// arm mali driver accept spirv with fp16 arithmetic
				gpu_info.bug_implicit_fp16_arithmetic = true;
			}

			if (physicalDeviceProperties.vendorID == 0x5143
				&& (physicalDeviceProperties.deviceID == 0x6030001
					|| physicalDeviceProperties.deviceID == 0x6040001
					|| physicalDeviceProperties.deviceID == 0x6050002))
			{
				// TODO enable devices other than qcom845/qcom855/qcom855plus/qcom865
				// qcom adreno driver accept spirv with fp16 arithmetic
				gpu_info.bug_implicit_fp16_arithmetic = true;
			}

			gpu_info.physical_device = physicalDevice;

			// info
			gpu_info.api_version = physicalDeviceProperties.apiVersion;
			gpu_info.driver_version = physicalDeviceProperties.driverVersion;
			gpu_info.vendor_id = physicalDeviceProperties.vendorID;
			gpu_info.device_id = physicalDeviceProperties.deviceID;
			memcpy(gpu_info.pipeline_cache_uuid, physicalDeviceProperties.pipelineCacheUUID, VK_UUID_SIZE);

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				gpu_info.type = 0;
			else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				gpu_info.type = 1;
			else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
				gpu_info.type = 2;
			else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
				gpu_info.type = 3;
			else
				gpu_info.type = -1;

			// device capability
			gpu_info.max_shared_memory_size = physicalDeviceProperties.limits.maxComputeSharedMemorySize;

			gpu_info.max_workgroup_count[0] = physicalDeviceProperties.limits.maxComputeWorkGroupCount[0];
			gpu_info.max_workgroup_count[1] = physicalDeviceProperties.limits.maxComputeWorkGroupCount[1];
			gpu_info.max_workgroup_count[2] = physicalDeviceProperties.limits.maxComputeWorkGroupCount[2];

			gpu_info.max_workgroup_invocations = physicalDeviceProperties.limits.maxComputeWorkGroupInvocations;

			gpu_info.max_workgroup_size[0] = physicalDeviceProperties.limits.maxComputeWorkGroupSize[0];
			gpu_info.max_workgroup_size[1] = physicalDeviceProperties.limits.maxComputeWorkGroupSize[1];
			gpu_info.max_workgroup_size[2] = physicalDeviceProperties.limits.maxComputeWorkGroupSize[2];

			gpu_info.memory_map_alignment = physicalDeviceProperties.limits.minMemoryMapAlignment;
			gpu_info.buffer_offset_alignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
			gpu_info.non_coherent_atom_size = physicalDeviceProperties.limits.nonCoherentAtomSize;
			gpu_info.buffer_image_granularity = physicalDeviceProperties.limits.bufferImageGranularity;
			gpu_info.max_image_dimension_1d = physicalDeviceProperties.limits.maxImageDimension1D;
			gpu_info.max_image_dimension_2d = physicalDeviceProperties.limits.maxImageDimension2D;
			gpu_info.max_image_dimension_3d = physicalDeviceProperties.limits.maxImageDimension3D;

			gpu_info.timestamp_period = physicalDeviceProperties.limits.timestampPeriod;

			// find compute queue
			uint32_t queueFamilyPropertiesCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, 0);

			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

			gpu_info.compute_queue_family_index = FindDeviceComputeQueue(queueFamilyProperties);
			gpu_info.graphics_queue_family_index = FindDeviceGraphicsQueue(queueFamilyProperties);
			gpu_info.transfer_queue_family_index = FindDeviceTransferQueue(queueFamilyProperties);

			gpu_info.compute_queue_count = queueFamilyProperties[gpu_info.compute_queue_family_index].queueCount;
			gpu_info.graphics_queue_count = queueFamilyProperties[gpu_info.graphics_queue_family_index].queueCount;
			gpu_info.transfer_queue_count = queueFamilyProperties[gpu_info.transfer_queue_family_index].queueCount;

			gpu_info.unified_compute_transfer_queue = gpu_info.compute_queue_family_index == gpu_info.transfer_queue_family_index;

			// cache memory properties
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &gpu_info.physicalDeviceMemoryProperties);

			// get device extension
			uint32_t deviceExtensionPropertyCount = 0;
			ret = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, NULL);
			CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumerateDeviceExtensionProperties failed %d", ret);

			std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionPropertyCount);
			ret = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, deviceExtensionProperties.data());
			CHECK_EQ(ret, VK_SUCCESS) << Format("vkEnumerateDeviceExtensionProperties failed %d", ret);

			// extension capability
			gpu_info.support_VK_KHR_8bit_storage = 0;
			gpu_info.support_VK_KHR_16bit_storage = 0;
			gpu_info.support_VK_KHR_bind_memory2 = 0;
			gpu_info.support_VK_KHR_dedicated_allocation = 0;
			gpu_info.support_VK_KHR_descriptor_update_template = 0;
			gpu_info.support_VK_KHR_external_memory = 0;
			gpu_info.support_VK_KHR_get_memory_requirements2 = 0;
			gpu_info.support_VK_KHR_maintenance1 = 0;
			gpu_info.support_VK_KHR_push_descriptor = 0;
			gpu_info.support_VK_KHR_sampler_ycbcr_conversion = 0;
			gpu_info.support_VK_KHR_shader_float16_int8 = 0;
			gpu_info.support_VK_KHR_shader_float_controls = 0;
			gpu_info.support_VK_KHR_storage_buffer_storage_class = 0;
			gpu_info.support_VK_KHR_swapchain = 0;
			gpu_info.support_VK_EXT_queue_family_foreign = 0;

			for (uint32_t j = 0; j < deviceExtensionPropertyCount; j++)
			{
				const VkExtensionProperties& exp = deviceExtensionProperties[j];

				if (strcmp(exp.extensionName, "VK_KHR_8bit_storage") == 0)
					gpu_info.support_VK_KHR_8bit_storage = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_16bit_storage") == 0)
					gpu_info.support_VK_KHR_16bit_storage = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_bind_memory2") == 0)
					gpu_info.support_VK_KHR_bind_memory2 = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_dedicated_allocation") == 0)
					gpu_info.support_VK_KHR_dedicated_allocation = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_descriptor_update_template") == 0)
					gpu_info.support_VK_KHR_descriptor_update_template = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_external_memory") == 0)
					gpu_info.support_VK_KHR_external_memory = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_get_memory_requirements2") == 0)
					gpu_info.support_VK_KHR_get_memory_requirements2 = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_maintenance1") == 0)
					gpu_info.support_VK_KHR_maintenance1 = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_push_descriptor") == 0)
					gpu_info.support_VK_KHR_push_descriptor = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_sampler_ycbcr_conversion") == 0)
					gpu_info.support_VK_KHR_sampler_ycbcr_conversion = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_shader_float16_int8") == 0)
					gpu_info.support_VK_KHR_shader_float16_int8 = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_shader_float_controls") == 0)
					gpu_info.support_VK_KHR_shader_float_controls = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_storage_buffer_storage_class") == 0)
					gpu_info.support_VK_KHR_storage_buffer_storage_class = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_KHR_swapchain") == 0)
					gpu_info.support_VK_KHR_swapchain = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_EXT_memory_budget") == 0)
					gpu_info.support_VK_EXT_memory_budget = exp.specVersion;
				else if (strcmp(exp.extensionName, "VK_EXT_queue_family_foreign") == 0)
					gpu_info.support_VK_EXT_queue_family_foreign = exp.specVersion;
			}

			// check features
			gpu_info.support_fp16_packed = true;
			gpu_info.support_fp16_storage = false;
			gpu_info.support_fp16_arithmetic = false;
			gpu_info.support_int8_storage = false;
			gpu_info.support_int8_arithmetic = false;
			gpu_info.support_ycbcr_conversion = false;
			if (support_VK_KHR_get_physical_device_properties2)
			{
				void* queryExtensionFeatures = 0;

				// query int8 storage
				VkPhysicalDevice8BitStorageFeaturesKHR query8BitStorageFeatures;
				query8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
				query8BitStorageFeatures.pNext = 0;
				if (gpu_info.support_VK_KHR_8bit_storage)
				{
					query8BitStorageFeatures.pNext = queryExtensionFeatures;
					queryExtensionFeatures = &query8BitStorageFeatures;
				}

				// query fp16/int16 storage
				VkPhysicalDevice16BitStorageFeaturesKHR query16BitStorageFeatures;
				query16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
				query16BitStorageFeatures.pNext = 0;
				if (gpu_info.support_VK_KHR_16bit_storage)
				{
					query16BitStorageFeatures.pNext = queryExtensionFeatures;
					queryExtensionFeatures = &query16BitStorageFeatures;
				}

				// query fp16/int8 arithmetic
				VkPhysicalDeviceFloat16Int8FeaturesKHR queryFloat16Int8Features;
				queryFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
				queryFloat16Int8Features.pNext = 0;
				if (gpu_info.support_VK_KHR_shader_float16_int8)
				{
					queryFloat16Int8Features.pNext = queryExtensionFeatures;
					queryExtensionFeatures = &queryFloat16Int8Features;
				}

				// query ycbcr_conversion
				VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR querySamplerYcbcrConversionFeatures;
				querySamplerYcbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR;
				querySamplerYcbcrConversionFeatures.pNext = 0;
				if (gpu_info.support_VK_KHR_sampler_ycbcr_conversion)
				{
					querySamplerYcbcrConversionFeatures.pNext = queryExtensionFeatures;
					queryExtensionFeatures = &querySamplerYcbcrConversionFeatures;
				}

				VkPhysicalDeviceFeatures2KHR queryFeatures;
				queryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
					queryFeatures.pNext = queryExtensionFeatures;

				vkGetPhysicalDeviceFeatures2KHR(physicalDevice, &queryFeatures);

				if (gpu_info.support_VK_KHR_8bit_storage)
				{
					gpu_info.support_int8_storage = query8BitStorageFeatures.storageBuffer8BitAccess && query8BitStorageFeatures.uniformAndStorageBuffer8BitAccess;
				}
				if (gpu_info.support_VK_KHR_16bit_storage && queryFeatures.features.shaderStorageImageExtendedFormats)
				{
					// shaderStorageImageExtendedFormats enables r16f format in storage image
					gpu_info.support_fp16_storage = query16BitStorageFeatures.storageBuffer16BitAccess && query16BitStorageFeatures.uniformAndStorageBuffer16BitAccess;
				}
				if (gpu_info.support_VK_KHR_shader_float16_int8)
				{
					gpu_info.support_fp16_arithmetic = queryFloat16Int8Features.shaderFloat16;
					gpu_info.support_int8_arithmetic = queryFloat16Int8Features.shaderInt8;
				}
				if (gpu_info.support_VK_KHR_sampler_ycbcr_conversion)
				{
					gpu_info.support_ycbcr_conversion = querySamplerYcbcrConversionFeatures.samplerYcbcrConversion;
				}
			}
			else
			{
				// // TODO
				// VkPhysicalDeviceFeatures features;
				// vkGetPhysicalDeviceFeatures(physicalDevice, &features);
			}

			if (physicalDeviceProperties.vendorID == 0x13b5)
			{
				// the 16bit_storage implementation of arm mali driver is buggy :[
				gpu_info.support_fp16_storage = false;
			}

			if (physicalDeviceProperties.vendorID == 0x10002 && physicalDeviceProperties.deviceID == 0x70006214 && physicalDeviceProperties.apiVersion == VK_MAKE_VERSION(1, 1, 82))
			{
				// the 16bit_storage implementation of vivante gc1700 driver is buggy :[
				gpu_info.support_fp16_storage = false;
			}

			if (gpu_info.bug_implicit_fp16_arithmetic)
			{
				// force capability on as long as the driver accept spirv with fp16 arithmetic :D
				gpu_info.support_fp16_arithmetic = true;
			}

			LOG(INFO) << Format("[%u %s]  queueC=%u[%u]  queueG=%u[%u]  queueT=%u[%u]", i, physicalDeviceProperties.deviceName,
				gpu_info.compute_queue_family_index, gpu_info.compute_queue_count,
				gpu_info.graphics_queue_family_index, gpu_info.graphics_queue_count,
				gpu_info.transfer_queue_family_index, gpu_info.transfer_queue_count);

			LOG(INFO) << Format("[%u %s]  bugsbn1=%d  buglbia=%d  bugcopc=%d  bugihfa=%d", i, physicalDeviceProperties.deviceName,
				gpu_info.bug_storage_buffer_no_l1, gpu_info.bug_layout_binding_id_alias, gpu_info.bug_corrupted_online_pipeline_cache, gpu_info.bug_implicit_fp16_arithmetic);

			LOG(INFO) << Format("[%u %s]  fp16p=%d  fp16s=%d  fp16a=%d  int8s=%d  int8a=%d", i, physicalDeviceProperties.deviceName,
				gpu_info.support_fp16_packed, gpu_info.support_fp16_storage, gpu_info.support_fp16_arithmetic,
				gpu_info.support_int8_storage, gpu_info.support_int8_arithmetic);

			gpu_info_index++;
		}

		g_gpu_count = gpu_info_index;

		// the default gpu device
		g_default_gpu_index = FindDefaultVulkanDeviceIndex();
	}

	void DestroyGPUInstance()
	{
		std::lock_guard lock(g_instance_lock);

		if ((VkInstance)g_instance == 0) return;

		for (int i = 0; i < MAX_GPU_COUNT; i++)
		{
			//delete g_default_vkdev[i];
			//g_default_vkdev[i] = 0;
		}

		vkDestroyInstance(g_instance, 0);
		g_instance.instance = 0;
	}


	static bool IsGPUInstanceReady()
	{
		std::lock_guard lock(g_instance_lock);
		return (VkInstance)g_instance != 0;
	}

	static void TryCreateGPUInstance()
	{
		if (not IsGPUInstanceReady()) CreateGPUInstance();
	}

	int GetGPUCount()
	{
		TryCreateGPUInstance();
		return g_gpu_count;
	}

	int GetDefaultGPUIndex()
	{
		TryCreateGPUInstance();
		return g_default_gpu_index;
	}

	const GPUInfo& GetGPUInfo(int device_index)
	{
		TryCreateGPUInstance();
		return g_gpu_infos[device_index];
	}



	VulkanDevice::VulkanDevice(int device_index) : info(g_gpu_infos[device_index])
	{
		TryCreateGPUInstance();

		std::vector<const char*> enabledExtensions;
		if (info.support_VK_KHR_8bit_storage)
			enabledExtensions.push_back("VK_KHR_8bit_storage");
		if (info.support_VK_KHR_16bit_storage)
			enabledExtensions.push_back("VK_KHR_16bit_storage");
		if (info.support_VK_KHR_bind_memory2)
			enabledExtensions.push_back("VK_KHR_bind_memory2");
		if (info.support_VK_KHR_dedicated_allocation)
			enabledExtensions.push_back("VK_KHR_dedicated_allocation");
		if (info.support_VK_KHR_descriptor_update_template)
			enabledExtensions.push_back("VK_KHR_descriptor_update_template");
		if (info.support_VK_KHR_external_memory)
			enabledExtensions.push_back("VK_KHR_external_memory");
		if (info.support_VK_KHR_get_memory_requirements2)
			enabledExtensions.push_back("VK_KHR_get_memory_requirements2");
		if (info.support_VK_KHR_maintenance1)
			enabledExtensions.push_back("VK_KHR_maintenance1");
		if (info.support_VK_KHR_push_descriptor)
			enabledExtensions.push_back("VK_KHR_push_descriptor");
		if (info.support_VK_KHR_sampler_ycbcr_conversion)
			enabledExtensions.push_back("VK_KHR_sampler_ycbcr_conversion");
		if (info.support_VK_KHR_shader_float16_int8)
			enabledExtensions.push_back("VK_KHR_shader_float16_int8");
		if (info.support_VK_KHR_shader_float_controls)
			enabledExtensions.push_back("VK_KHR_shader_float_controls");
		if (info.support_VK_KHR_storage_buffer_storage_class)
			enabledExtensions.push_back("VK_KHR_storage_buffer_storage_class");
		if (info.support_VK_KHR_swapchain)
			enabledExtensions.push_back("VK_KHR_swapchain");
		if (info.support_VK_EXT_memory_budget)
			enabledExtensions.push_back("VK_EXT_memory_budget");
		if (info.support_VK_EXT_queue_family_foreign)
			enabledExtensions.push_back("VK_EXT_queue_family_foreign");


		void* enabledExtensionFeatures = 0;

		// enable int8 storage
		VkPhysicalDevice8BitStorageFeaturesKHR enabled8BitStorageFeatures;
		enabled8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
		enabled8BitStorageFeatures.pNext = 0;
		enabled8BitStorageFeatures.storageBuffer8BitAccess = info.support_int8_storage;
		enabled8BitStorageFeatures.uniformAndStorageBuffer8BitAccess = info.support_int8_storage;
		enabled8BitStorageFeatures.storagePushConstant8 = VK_FALSE;
		if (support_VK_KHR_get_physical_device_properties2 && info.support_VK_KHR_8bit_storage)
		{
			enabled8BitStorageFeatures.pNext = enabledExtensionFeatures;
			enabledExtensionFeatures = &enabled8BitStorageFeatures;
		}

		// enable fp16/int16 storage
		VkPhysicalDevice16BitStorageFeaturesKHR enabled16BitStorageFeatures;
		enabled16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
		enabled16BitStorageFeatures.pNext = 0;
		enabled16BitStorageFeatures.storageBuffer16BitAccess = info.support_fp16_storage;
		enabled16BitStorageFeatures.uniformAndStorageBuffer16BitAccess = info.support_fp16_storage;
		enabled16BitStorageFeatures.storagePushConstant16 = VK_FALSE;
		enabled16BitStorageFeatures.storageInputOutput16 = VK_FALSE;
		if (support_VK_KHR_get_physical_device_properties2 && info.support_VK_KHR_16bit_storage)
		{
			enabled16BitStorageFeatures.pNext = enabledExtensionFeatures;
			enabledExtensionFeatures = &enabled16BitStorageFeatures;
		}

		// enable fp16/int8 arithmetic
		VkPhysicalDeviceFloat16Int8FeaturesKHR enabledFloat16Int8Features;
		enabledFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
		enabledFloat16Int8Features.pNext = 0;
		enabledFloat16Int8Features.shaderFloat16 = info.support_fp16_arithmetic;
		enabledFloat16Int8Features.shaderInt8 = info.support_int8_arithmetic;
		if (support_VK_KHR_get_physical_device_properties2 && info.support_VK_KHR_shader_float16_int8)
		{
			enabledFloat16Int8Features.pNext = enabledExtensionFeatures;
			enabledExtensionFeatures = &enabledFloat16Int8Features;
		}

		// enable ycbcr conversion
		VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR querySamplerYcbcrConversionFeatures;
		querySamplerYcbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR;
		querySamplerYcbcrConversionFeatures.pNext = 0;
		querySamplerYcbcrConversionFeatures.samplerYcbcrConversion = info.support_ycbcr_conversion;
		if (support_VK_KHR_get_physical_device_properties2 && info.support_ycbcr_conversion)
		{
			querySamplerYcbcrConversionFeatures.pNext = enabledExtensionFeatures;
			enabledExtensionFeatures = &querySamplerYcbcrConversionFeatures;
		}

		std::vector<float> compute_queue_priorities(info.compute_queue_count, 1.f);   // 0.f ~ 1.f
		std::vector<float> graphics_queue_priorities(info.graphics_queue_count, 1.f); // 0.f ~ 1.f
		std::vector<float> transfer_queue_priorities(info.transfer_queue_count, 1.f); // 0.f ~ 1.f

		VkDeviceQueueCreateInfo deviceQueueCreateInfos[3];

		VkDeviceQueueCreateInfo deviceComputeQueueCreateInfo;
		deviceComputeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceComputeQueueCreateInfo.pNext = 0;
		deviceComputeQueueCreateInfo.flags = 0;
		deviceComputeQueueCreateInfo.queueFamilyIndex = info.compute_queue_family_index;
		deviceComputeQueueCreateInfo.queueCount = info.compute_queue_count;
		deviceComputeQueueCreateInfo.pQueuePriorities = compute_queue_priorities.data();

		VkDeviceQueueCreateInfo deviceGraphicsQueueCreateInfo;
		deviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceGraphicsQueueCreateInfo.pNext = 0;
		deviceGraphicsQueueCreateInfo.flags = 0;
		deviceGraphicsQueueCreateInfo.queueFamilyIndex = info.graphics_queue_family_index;
		deviceGraphicsQueueCreateInfo.queueCount = info.graphics_queue_count;
		deviceGraphicsQueueCreateInfo.pQueuePriorities = graphics_queue_priorities.data();

		VkDeviceQueueCreateInfo deviceTransferQueueCreateInfo;
		deviceTransferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceTransferQueueCreateInfo.pNext = 0;
		deviceTransferQueueCreateInfo.flags = 0;
		deviceTransferQueueCreateInfo.queueFamilyIndex = info.transfer_queue_family_index;
		deviceTransferQueueCreateInfo.queueCount = info.transfer_queue_count;
		deviceTransferQueueCreateInfo.pQueuePriorities = transfer_queue_priorities.data();

		VkDeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = enabledExtensionFeatures;
		deviceCreateInfo.flags = 0;
		if (info.compute_queue_family_index == info.graphics_queue_family_index && info.compute_queue_family_index == info.transfer_queue_family_index)
		{
			deviceQueueCreateInfos[0] = deviceComputeQueueCreateInfo;
			deviceCreateInfo.queueCreateInfoCount = 1;
		}
		else if (info.compute_queue_family_index == info.graphics_queue_family_index && info.compute_queue_family_index != info.transfer_queue_family_index)
		{
			deviceQueueCreateInfos[0] = deviceComputeQueueCreateInfo;
			deviceQueueCreateInfos[1] = deviceTransferQueueCreateInfo;
			deviceCreateInfo.queueCreateInfoCount = 2;
		}
		else if (info.compute_queue_family_index != info.graphics_queue_family_index && info.graphics_queue_family_index == info.transfer_queue_family_index)
		{
			deviceQueueCreateInfos[0] = deviceComputeQueueCreateInfo;
			deviceQueueCreateInfos[1] = deviceGraphicsQueueCreateInfo;
			deviceCreateInfo.queueCreateInfoCount = 2;
		}
		else // if (info.compute_queue_family_index != info.graphics_queue_family_index && info.graphics_queue_family_index != info.transfer_queue_family_index)
		{
			deviceQueueCreateInfos[0] = deviceComputeQueueCreateInfo;
			deviceQueueCreateInfos[1] = deviceGraphicsQueueCreateInfo;
			deviceQueueCreateInfos[2] = deviceTransferQueueCreateInfo;
			deviceCreateInfo.queueCreateInfoCount = 3;
		}
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = 0;
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
		deviceCreateInfo.pEnabledFeatures = 0; // VkPhysicalDeviceFeatures pointer

		VkResult ret = vkCreateDevice(info.physical_device, &deviceCreateInfo, 0, &device);
		CHECK_EQ(ret, VK_SUCCESS) << Format("vkCreateDevice failed %d", ret);

		InitDeviceExtension();


		compute_queues.resize(info.compute_queue_count);
		blob_allocators.resize(info.compute_queue_count);
		staging_allocators.resize(info.compute_queue_count);
		for (uint32_t i = 0; i < info.compute_queue_count; i++)
		{
			vkGetDeviceQueue(device, info.compute_queue_family_index, i, &compute_queues[i]);
			blob_allocators[i] = new VkBlobAllocator(this);
			staging_allocators[i] = new VkStagingAllocator(this);
		}
		if (info.compute_queue_family_index != info.graphics_queue_family_index)
		{
			graphics_queues.resize(info.graphics_queue_count);
			for (uint32_t i = 0; i < info.graphics_queue_count; i++)
			{
				vkGetDeviceQueue(device, info.graphics_queue_family_index, i, &graphics_queues[i]);
			}
		}
		if (info.compute_queue_family_index != info.transfer_queue_family_index && info.graphics_queue_family_index != info.transfer_queue_family_index)
		{
			transfer_queues.resize(info.transfer_queue_count);
			for (uint32_t i = 0; i < info.transfer_queue_count; i++)
			{
				vkGetDeviceQueue(device, info.transfer_queue_family_index, i, &transfer_queues[i]);
			}
		}

		// prepare immutable texelfetch sampler
		{
			VkSamplerCreateInfo samplerCreateInfo;
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.pNext = 0;
			samplerCreateInfo.flags = 0;
			samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			samplerCreateInfo.maxAnisotropy = 1;
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 0.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			samplerCreateInfo.unnormalizedCoordinates = VK_TRUE;

			texelfetch_sampler = 0;
			ret = vkCreateSampler(device, &samplerCreateInfo, 0, &texelfetch_sampler);
			CHECK_EQ(ret, VK_SUCCESS) << Format("vkCreateSampler failed %d", ret);
		}

		//CreateDummyBufferImage();
		//pipeline_cache = new PipelineCache(this);
		//memset(uop_packing, 0, sizeof(uop_packing));
	}




	void VulkanDevice::InitDeviceExtension()
	{
		if (info.support_VK_KHR_bind_memory2)
		{
			vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
			//vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
		}

		if (info.support_VK_KHR_descriptor_update_template)
		{
			vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplateKHR");
			vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplateKHR");
			vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplateKHR");
		}

		if (info.support_VK_KHR_get_memory_requirements2)
		{
			vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2KHR");
			//vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2KHR");
			//vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2KHR");
		}

		if (info.support_VK_KHR_maintenance1)
		{
			vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
		}

		if (info.support_VK_KHR_push_descriptor)
		{
			if (info.support_VK_KHR_descriptor_update_template)
			{
				vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetWithTemplateKHR");
			}

			vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
		}

		if (info.support_VK_KHR_sampler_ycbcr_conversion)
		{
			vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkCreateSamplerYcbcrConversionKHR");
			vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkDestroySamplerYcbcrConversionKHR");
		}

		if (info.support_VK_KHR_swapchain)
		{
			vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
			vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
			vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
			//vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
			//vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
		}
	}

	uint32_t VulkanDevice::FindMemoryIndex(uint32_t memory_type_bits, VkFlags required, VkFlags preferred, VkFlags preferred_not) const
	{
		// first try, find required and with preferred and without preferred_not
		for (uint32_t i = 0; i < info.physicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			bool is_required = (1 << i) & memory_type_bits;
			if (is_required)
			{
				const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[i];
				if ((memoryType.propertyFlags & required) == required
					&& (preferred && (memoryType.propertyFlags & preferred))
					&& (preferred_not && !(memoryType.propertyFlags & preferred_not)))
				{
					return i;
				}
			}
		}

		// second try, find required and with preferred
		for (uint32_t i = 0; i < info.physicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			bool is_required = (1 << i) & memory_type_bits;
			if (is_required)
			{
				const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[i];
				if ((memoryType.propertyFlags & required) == required
					&& (preferred && (memoryType.propertyFlags & preferred)))
				{
					return i;
				}
			}
		}

		// third try, find required and without preferred_not
		for (uint32_t i = 0; i < info.physicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			bool is_required = (1 << i) & memory_type_bits;
			if (is_required)
			{
				const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[i];
				if ((memoryType.propertyFlags & required) == required
					&& (preferred_not && !(memoryType.propertyFlags & preferred_not)))
				{
					return i;
				}
			}
		}

		// fourth try, find any required
		for (uint32_t i = 0; i < info.physicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			bool is_required = (1 << i) & memory_type_bits;
			if (is_required)
			{
				const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[i];
				if ((memoryType.propertyFlags & required) == required)
				{
					return i;
				}
			}
		}

		LOG(ERROR) << Format("no such memory type %u %u %u %u", memory_type_bits, required, preferred, preferred_not);
		return -1;
	}

	bool VulkanDevice::IsMappable(uint32_t memory_type_index) const
	{
		const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[memory_type_index];

		return memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}

	bool VulkanDevice::IsCoherent(uint32_t memory_type_index) const
	{
		const VkMemoryType& memoryType = info.physicalDeviceMemoryProperties.memoryTypes[memory_type_index];

		return memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

}