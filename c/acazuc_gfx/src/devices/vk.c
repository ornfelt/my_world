#include "devices/vk_mem_alloc.h"

#include "device_vtable.h"
#include "devices.h"
#include "device.h"
#include "window.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#define VK_DEVICE ((struct gfx_vk_device*)device)

#define FRAMES_COUNT 2

static const VkFormat attribute_types[] =
{
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SNORM,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_SINT,
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R16G16_SNORM,
	VK_FORMAT_R16G16_UINT,
	VK_FORMAT_R16G16_SINT,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SNORM,
	VK_FORMAT_R8G8B8A8_UINT,
	VK_FORMAT_R8G8B8A8_SINT,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8_SNORM,
	VK_FORMAT_R8G8_UINT,
	VK_FORMAT_R8G8_SINT,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_SNORM,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8_SINT,
};

static const VkIndexType index_types[] =
{
	VK_INDEX_TYPE_UINT16,
	VK_INDEX_TYPE_UINT32,
};

static const VkCompareOp compare_functions[] =
{
	VK_COMPARE_OP_NEVER,
	VK_COMPARE_OP_LESS,
	VK_COMPARE_OP_LESS_OR_EQUAL,
	VK_COMPARE_OP_EQUAL,
	VK_COMPARE_OP_GREATER_OR_EQUAL,
	VK_COMPARE_OP_GREATER,
	VK_COMPARE_OP_NOT_EQUAL,
	VK_COMPARE_OP_ALWAYS,
};

static const VkBlendFactor blend_functions[] =
{
	VK_BLEND_FACTOR_ZERO,
	VK_BLEND_FACTOR_ONE,
	VK_BLEND_FACTOR_SRC_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	VK_BLEND_FACTOR_DST_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	VK_BLEND_FACTOR_SRC_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	VK_BLEND_FACTOR_DST_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	VK_BLEND_FACTOR_CONSTANT_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};

static const VkBlendOp blend_equations[] =
{
	VK_BLEND_OP_ADD,
	VK_BLEND_OP_SUBTRACT,
	VK_BLEND_OP_REVERSE_SUBTRACT,
	VK_BLEND_OP_MIN,
	VK_BLEND_OP_MAX,
};

static const VkStencilOp stencil_operations[] =
{
	VK_STENCIL_OP_KEEP,
	VK_STENCIL_OP_ZERO,
	VK_STENCIL_OP_REPLACE,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	VK_STENCIL_OP_INCREMENT_AND_WRAP,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	VK_STENCIL_OP_DECREMENT_AND_WRAP,
	VK_STENCIL_OP_INVERT,
};

static const VkPrimitiveTopology primitive_types[] =
{
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
};

static const VkPolygonMode fill_modes[] =
{
	VK_POLYGON_MODE_POINT,
	VK_POLYGON_MODE_LINE,
	VK_POLYGON_MODE_FILL,
};

static VkCullModeFlagBits cull_modes[] =
{
	VK_CULL_MODE_NONE,
	VK_CULL_MODE_FRONT_BIT,
	VK_CULL_MODE_BACK_BIT,
};

static const VkFrontFace front_faces[] =
{
	VK_FRONT_FACE_CLOCKWISE,
	VK_FRONT_FACE_COUNTER_CLOCKWISE,
};

static const VkColorComponentFlagBits color_masks[] =
{
	0,
	VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_B_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
};

static const VkImageType image_types[] =
{
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_3D,
};

static const VkImageViewType image_view_types[] =
{
	VK_IMAGE_VIEW_TYPE_2D,
	VK_IMAGE_VIEW_TYPE_2D,
	VK_IMAGE_VIEW_TYPE_2D_ARRAY,
	VK_IMAGE_VIEW_TYPE_2D_ARRAY,
	VK_IMAGE_VIEW_TYPE_3D,
};

static const VkFormat formats[] =
{
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	VK_FORMAT_R5G6B5_UNORM_PACK16,
	VK_FORMAT_R8G8_SRGB,
	VK_FORMAT_R8_SRGB,
	VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
};

static const VkSamplerAddressMode address_modes[] =
{
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	VK_SAMPLER_ADDRESS_MODE_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

static const VkFilter min_mag_filterings[] =
{
	VK_FILTER_NEAREST,
	VK_FILTER_NEAREST,
	VK_FILTER_LINEAR,
};

static const VkSamplerMipmapMode mipmap_filterings[] =
{
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

static const VkVertexInputRate input_rates[] =
{
	VK_VERTEX_INPUT_RATE_VERTEX,
	VK_VERTEX_INPUT_RATE_INSTANCE,
};

struct gfx_vk_device
{
	struct gfx_device device;
	VkAllocationCallbacks allocation_callbacks;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkSurfaceFormatKHR *surface_formats;
	uint32_t surface_formats_count;
	bool present_mode_fifo_relaxed;
	bool present_mode_immediate;
	bool present_mode_changed;
	VkFramebuffer *swapchain_framebuffers;
	VkImageView *swapchain_image_views;
	VkImage *swapchain_images;
	uint32_t swapchain_images_count;
	VkFormat swapchain_format;
	VkSampleCountFlagBits msaa_samples;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	VkPhysicalDevice physical_device;
	VkSemaphore swapchain_images_semaphores[FRAMES_COUNT];
	VkSemaphore render_finished_semaphores[FRAMES_COUNT];
	VkFence rendering_fences[FRAMES_COUNT];
	VkCommandBuffer command_buffers[FRAMES_COUNT];
	VkExtent2D swapchain_extent;
	VkSwapchainKHR swapchain;
	VkDescriptorPool descriptor_pool;
	VkCommandPool command_pool;
	VkRenderPass render_pass;
	VkSurfaceKHR surface;
	VkInstance instance;
	VkDevice vk_device;
	VkImage depth_image;
	VmaAllocation depth_image_allocation;
	VkImageView depth_image_view;
	VkImage msaa_image;
	VmaAllocation msaa_image_allocation;
	VkImageView msaa_image_view;
	VkQueue transfer_queue;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSharingMode graphics_present_sharing_mode;
	uint32_t graphics_present_count;
	uint32_t graphics_present_families[2];
	VkSharingMode graphics_transfer_sharing_mode;
	uint32_t graphics_transfer_count;
	uint32_t graphics_transfer_families[2];
	uint32_t transfer_family;
	uint32_t graphics_family;
	uint32_t present_family;
	VkPresentModeKHR present_mode;
	enum gfx_primitive_type primitive;
	uint32_t current_frame;
	uint32_t current_image;
	VmaAllocator vma_allocator;
};

#define ALLOCATION_CALLBACKS NULL //&VK_DEVICE->allocation_callbacks

static const char *vk_err2str(VkResult result)
{
#define TEST_ERR(code) \
	case code: \
		return #code; \

	switch (result)
	{
		TEST_ERR(VK_SUCCESS)
		TEST_ERR(VK_NOT_READY)
		TEST_ERR(VK_TIMEOUT)
		TEST_ERR(VK_EVENT_SET)
		TEST_ERR(VK_EVENT_RESET)
		TEST_ERR(VK_INCOMPLETE)
		TEST_ERR(VK_ERROR_OUT_OF_HOST_MEMORY)
		TEST_ERR(VK_ERROR_OUT_OF_DEVICE_MEMORY)
		TEST_ERR(VK_ERROR_INITIALIZATION_FAILED)
		TEST_ERR(VK_ERROR_DEVICE_LOST)
		TEST_ERR(VK_ERROR_MEMORY_MAP_FAILED)
		TEST_ERR(VK_ERROR_LAYER_NOT_PRESENT)
		TEST_ERR(VK_ERROR_EXTENSION_NOT_PRESENT)
		TEST_ERR(VK_ERROR_FEATURE_NOT_PRESENT)
		TEST_ERR(VK_ERROR_INCOMPATIBLE_DRIVER)
		TEST_ERR(VK_ERROR_TOO_MANY_OBJECTS)
		TEST_ERR(VK_ERROR_FORMAT_NOT_SUPPORTED)
		TEST_ERR(VK_ERROR_FRAGMENTED_POOL)
		TEST_ERR(VK_ERROR_UNKNOWN)
		TEST_ERR(VK_ERROR_OUT_OF_POOL_MEMORY)
		TEST_ERR(VK_ERROR_INVALID_EXTERNAL_HANDLE)
		TEST_ERR(VK_ERROR_FRAGMENTATION)
		TEST_ERR(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
		TEST_ERR(VK_ERROR_SURFACE_LOST_KHR)
		TEST_ERR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
		TEST_ERR(VK_SUBOPTIMAL_KHR)
		TEST_ERR(VK_ERROR_OUT_OF_DATE_KHR)
		TEST_ERR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
		TEST_ERR(VK_ERROR_VALIDATION_FAILED_EXT)
		TEST_ERR(VK_ERROR_INVALID_SHADER_NV)
		TEST_ERR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
		TEST_ERR(VK_ERROR_NOT_PERMITTED_EXT)
		TEST_ERR(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
#if 0
		TEST_ERR(VK_THREAD_IDLE_KHR)
		TEST_ERR(VK_THREAD_DONE_KHR)
		TEST_ERR(VK_OPERATION_DEFERRED_KHR)
		TEST_ERR(VK_OPERATION_NOT_DEFERRED_KHR)
		TEST_ERR(VK_PIPELINE_COMPILE_REQUIRED_EXT)
#endif
		TEST_ERR(VK_RESULT_MAX_ENUM)
	}

	return "unknown error";
#undef TEST_ERR
}

static bool get_queues_id(struct gfx_device *device, VkPhysicalDevice physical_device)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = GFX_MALLOC(sizeof(*queue_families) * queue_family_count);
	if (!queue_families)
	{
		GFX_ERROR_CALLBACK("failed to allocate queue families: %s (%d)", strerror(errno), errno);
		return false;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
	bool transfer_found = false;
	bool graphics_found = false;
	bool present_found = false;
	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		if (!transfer_found && (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		 && !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			VK_DEVICE->transfer_family = i;
			transfer_found = true;
		}
		if (!graphics_found && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VK_DEVICE->graphics_family = i;
			graphics_found = true;
		}
		if (!present_found)
		{
			VkBool32 surface_support = VK_FALSE;
			VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, VK_DEVICE->surface, &surface_support);
			if (result != VK_SUCCESS)
			{
				GFX_ERROR_CALLBACK("failed to find physical device surface support: %s (%d)", vk_err2str(result), result);
				GFX_FREE(queue_families);
				return false;
			}
			if (surface_support)
			{
				VK_DEVICE->present_family = i;
				present_found = true;
			}
		}
	}
	GFX_FREE(queue_families);
	if (!graphics_found || !present_found)
		return false;
	if (!transfer_found)
		VK_DEVICE->transfer_family = VK_DEVICE->graphics_family;
	if (VK_DEVICE->graphics_family != VK_DEVICE->present_family)
	{
		VK_DEVICE->graphics_present_sharing_mode = VK_SHARING_MODE_CONCURRENT;
		VK_DEVICE->graphics_present_families[0] = VK_DEVICE->graphics_family;
		VK_DEVICE->graphics_present_families[1] = VK_DEVICE->present_family;
		VK_DEVICE->graphics_present_count = 2;
	}
	else
	{
		VK_DEVICE->graphics_present_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		VK_DEVICE->graphics_present_count = 0;
	}
	if (VK_DEVICE->graphics_family != VK_DEVICE->transfer_family)
	{
		VK_DEVICE->graphics_transfer_sharing_mode = VK_SHARING_MODE_CONCURRENT;
		VK_DEVICE->graphics_transfer_families[0] = VK_DEVICE->graphics_family;
		VK_DEVICE->graphics_transfer_families[1] = VK_DEVICE->transfer_family;
		VK_DEVICE->graphics_transfer_count = 2;
	}
	else
	{
		VK_DEVICE->graphics_transfer_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		VK_DEVICE->graphics_transfer_count = 0;
	}
	return true;
}

static bool support_extensions(VkPhysicalDevice physical_device)
{
	uint32_t extensions_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, NULL);
	VkExtensionProperties *extensions = GFX_MALLOC(sizeof(*extensions) * extensions_count);
	if (!extensions)
	{
		GFX_ERROR_CALLBACK("device extensions allocation failed: %s (%d)", strerror(errno), errno);
		return false;
	}
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, extensions);
	bool support_swapchain = false;
	for (uint32_t i = 0; i < extensions_count; ++i)
	{
		if (!strcmp(extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			support_swapchain = true;
	}
	GFX_FREE(extensions);
	return support_swapchain;
}

static uint32_t get_max_msaa_samples(struct gfx_device *device)
{
	VkSampleCountFlags counts = VK_DEVICE->physical_device_properties.limits.framebufferColorSampleCounts
	                          & VK_DEVICE->physical_device_properties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return 64;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return 32;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return 16;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return 8;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return 4;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return 2;
	return 1;
}

static bool get_physical_device(struct gfx_device *device)
{
	VkResult result;
	uint32_t device_count = 0;
	result = vkEnumeratePhysicalDevices(VK_DEVICE->instance, &device_count, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to enumerate physical devices: %s (%d)", vk_err2str(result), result);
		return false;
	}
	if (device_count == 0)
	{
		GFX_ERROR_CALLBACK("no physical devices available");
		return false;
	}
	VkPhysicalDevice *devices = GFX_MALLOC(sizeof(*devices) * device_count);
	if (devices == NULL)
	{
		GFX_ERROR_CALLBACK("failed to allocate physical devices: %s (%d)", strerror(errno), errno);
		return false;
	}
	result = vkEnumeratePhysicalDevices(VK_DEVICE->instance, &device_count, devices);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to enumerate physical devices: %s (%d)", vk_err2str(result), result);
		GFX_FREE(devices);
		return false;
	}
	for (uint32_t i = 0; i < device_count; ++i)
	{
		vkGetPhysicalDeviceProperties(devices[i], &VK_DEVICE->physical_device_properties);
		vkGetPhysicalDeviceFeatures(devices[i], &VK_DEVICE->physical_device_features);
		if (!VK_DEVICE->physical_device_features.geometryShader)
			continue;
		if (!VK_DEVICE->physical_device_features.samplerAnisotropy)
			continue;
		//if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		//	continue;
		if (!get_queues_id(device, devices[i]))
			continue;
		if (!support_extensions(devices[i]))
			continue;
		uint32_t formats_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(devices[i], VK_DEVICE->surface, &formats_count, NULL);
		if (!formats_count)
			continue;
		uint32_t present_modes_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(devices[i], VK_DEVICE->surface, &present_modes_count, NULL);
		if (!present_modes_count)
			continue;
		VkSurfaceFormatKHR *surface_formats = GFX_MALLOC(sizeof(*surface_formats) * formats_count);
		if (!surface_formats)
		{
			GFX_ERROR_CALLBACK("surface formats allocation failed: %s (%d)", strerror(errno), errno);
			GFX_FREE(devices);
			return false;
		}
		vkGetPhysicalDeviceSurfaceFormatsKHR(devices[i], VK_DEVICE->surface, &formats_count, surface_formats);
		VkPresentModeKHR *present_modes = GFX_MALLOC(sizeof(*present_modes) * present_modes_count);
		if (!present_modes)
		{
			GFX_ERROR_CALLBACK("present modes allocation failed: %s (%d)", strerror(errno), errno);
			GFX_FREE(surface_formats);
			GFX_FREE(devices);
			return false;
		}
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(devices[i], VK_DEVICE->surface, &present_modes_count, present_modes);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to query surface present modes: %s (%d)", vk_err2str(result), result);
			GFX_FREE(present_modes);
			GFX_FREE(surface_formats);
			GFX_FREE(devices);
			return false;
		}
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[i], VK_DEVICE->surface, &VK_DEVICE->surface_capabilities);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to query surface capabilities: %s (%d)", vk_err2str(result), result);
			GFX_FREE(present_modes);
			GFX_FREE(surface_formats);
			GFX_FREE(devices);
			return false;
		}
		VK_DEVICE->present_mode_fifo_relaxed = false;
		VK_DEVICE->present_mode_immediate = false;
		for (uint32_t j = 0; j < present_modes_count; ++j)
		{
			if (present_modes[j] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
				VK_DEVICE->present_mode_fifo_relaxed = true;
			else if (present_modes[j] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				VK_DEVICE->present_mode_immediate = true;
		}
		GFX_FREE(present_modes);
		VK_DEVICE->surface_formats = surface_formats;
		VK_DEVICE->surface_formats_count = formats_count;
		VK_DEVICE->physical_device = devices[i];
		GFX_FREE(devices);
		return true;
	}
	GFX_ERROR_CALLBACK("can't find suitable physical device");
	GFX_FREE(devices);
	return false;
}

static bool create_device(struct gfx_device *device)
{
	static const char *extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	float queue_priority = 1;
	VkDeviceQueueCreateInfo queues_create_info[3];
	queues_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues_create_info[0].pNext = NULL;
	queues_create_info[0].flags = 0;
	queues_create_info[0].queueFamilyIndex = VK_DEVICE->transfer_family;
	queues_create_info[0].queueCount = 1;
	queues_create_info[0].pQueuePriorities = &queue_priority;
	queues_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues_create_info[1].pNext = NULL;
	queues_create_info[1].flags = 0;
	queues_create_info[1].queueFamilyIndex = VK_DEVICE->graphics_family;
	queues_create_info[1].queueCount = 1;
	queues_create_info[1].pQueuePriorities = &queue_priority;
	queues_create_info[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues_create_info[2].pNext = NULL;
	queues_create_info[2].flags = 0;
	queues_create_info[2].queueFamilyIndex = VK_DEVICE->present_family;
	queues_create_info[2].queueCount = 1;
	queues_create_info[2].pQueuePriorities = &queue_priority;
	VkDeviceCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.queueCreateInfoCount = sizeof(queues_create_info) / sizeof(*queues_create_info);
	create_info.pQueueCreateInfos = queues_create_info;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
	create_info.ppEnabledExtensionNames = extensions;
	create_info.pEnabledFeatures = NULL;
	VkResult result = vkCreateDevice(VK_DEVICE->physical_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->vk_device);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create device: %s (%d)", vk_err2str(result), result);
		return false;
	}
	vkGetDeviceQueue(VK_DEVICE->vk_device, VK_DEVICE->transfer_family, 0, &VK_DEVICE->transfer_queue);
	vkGetDeviceQueue(VK_DEVICE->vk_device, VK_DEVICE->graphics_family, 0, &VK_DEVICE->graphics_queue);
	vkGetDeviceQueue(VK_DEVICE->vk_device, VK_DEVICE->present_family, 0, &VK_DEVICE->present_queue);
	return true;
}

static bool create_vma_allocator(struct gfx_device *device)
{
	VmaAllocatorCreateInfo create_info;
	create_info.flags = 0;
	create_info.physicalDevice = VK_DEVICE->physical_device;
	create_info.device = VK_DEVICE->vk_device;
	create_info.preferredLargeHeapBlockSize = 0;
	create_info.pAllocationCallbacks = ALLOCATION_CALLBACKS;
	create_info.pDeviceMemoryCallbacks = NULL;
	create_info.pHeapSizeLimit = NULL;
	create_info.pVulkanFunctions = NULL;
	create_info.instance = VK_DEVICE->instance;
	create_info.vulkanApiVersion = VK_API_VERSION_1_0;
	create_info.pTypeExternalMemoryHandleTypes = NULL;
	VkResult result = vmaCreateAllocator(&create_info, &VK_DEVICE->vma_allocator);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create vma allocator: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static VkSurfaceFormatKHR *get_surface_format(struct gfx_device *device)
{
	for (uint32_t i = 0; i < VK_DEVICE->surface_formats_count; ++i)
	{
		VkSurfaceFormatKHR *format = &VK_DEVICE->surface_formats[i];
		if (format->format == VK_FORMAT_B8G8R8A8_SRGB
		 && format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}
	return &VK_DEVICE->surface_formats[0];
}

static VkExtent2D get_extent(struct gfx_device *device)
{
	if (VK_DEVICE->surface_capabilities.currentExtent.width != UINT32_MAX)
		return VK_DEVICE->surface_capabilities.currentExtent;
	VkExtent2D extent;
	extent.width = device->window->width;
	extent.height = device->window->height;
	if (extent.width < VK_DEVICE->surface_capabilities.minImageExtent.width)
		extent.width = VK_DEVICE->surface_capabilities.minImageExtent.width;
	else if (extent.height > VK_DEVICE->surface_capabilities.maxImageExtent.width)
		extent.height = VK_DEVICE->surface_capabilities.maxImageExtent.width;
	if (extent.height < VK_DEVICE->surface_capabilities.minImageExtent.height)
		extent.height = VK_DEVICE->surface_capabilities.minImageExtent.height;
	else if (extent.height > VK_DEVICE->surface_capabilities.maxImageExtent.height)
		extent.height = VK_DEVICE->surface_capabilities.maxImageExtent.height;
	return extent;
}

static bool create_swapchain(struct gfx_device *device)
{
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_DEVICE->physical_device, VK_DEVICE->surface, &VK_DEVICE->surface_capabilities);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to query surface capabilities: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkSurfaceFormatKHR *surface_format = get_surface_format(device);
	VkExtent2D extent = get_extent(device);
	uint32_t image_count = VK_DEVICE->surface_capabilities.minImageCount + 1;
	if (VK_DEVICE->surface_capabilities.maxImageCount && image_count > VK_DEVICE->surface_capabilities.maxImageCount)
		image_count = VK_DEVICE->surface_capabilities.maxImageCount;
	VK_DEVICE->swapchain_format = surface_format->format;
	VkSwapchainCreateInfoKHR create_info;
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.surface = VK_DEVICE->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format->format;
	create_info.imageColorSpace = surface_format->colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_DEVICE->graphics_present_sharing_mode;
	create_info.queueFamilyIndexCount = VK_DEVICE->graphics_present_count;
	create_info.pQueueFamilyIndices = VK_DEVICE->graphics_present_families;
	create_info.preTransform = VK_DEVICE->surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = VK_DEVICE->present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	result = vkCreateSwapchainKHR(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->swapchain);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create swapchain: %s (%d)", vk_err2str(result), result);
		return false;
	}
	uint32_t images_count = 0;
	result = vkGetSwapchainImagesKHR(VK_DEVICE->vk_device, VK_DEVICE->swapchain, &images_count, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to get swapchain images count: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkImage *images = GFX_MALLOC(sizeof(*images) * images_count);
	if (!images)
	{
		GFX_ERROR_CALLBACK("failed to allocate images: %s (%d)", strerror(errno), errno);
		return false;
	}
	result = vkGetSwapchainImagesKHR(VK_DEVICE->vk_device, VK_DEVICE->swapchain, &images_count, images);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to get swapchain images: %s (%d)", vk_err2str(result), result);
		GFX_FREE(images);
		return false;
	}
	GFX_FREE(VK_DEVICE->swapchain_images);
	VK_DEVICE->swapchain_images = images;
	VK_DEVICE->swapchain_images_count = images_count;
	VK_DEVICE->swapchain_extent = extent;
	return true;
}

static bool create_image_views(struct gfx_device *device)
{
	uint32_t image_views_count = VK_DEVICE->swapchain_images_count;
	VkImageView *image_views = GFX_MALLOC(sizeof(*image_views) * image_views_count);
	if (!image_views)
	{
		GFX_ERROR_CALLBACK("failed to allocate image views: %s (%d)", strerror(errno), errno);
		return false;
	}
	for (uint32_t i = 0; i < image_views_count; ++i)
	{
		VkImageViewCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.flags = 0;
		create_info.image = VK_DEVICE->swapchain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = VK_DEVICE->swapchain_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		VkResult result = vkCreateImageView(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &image_views[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to allocate image views: %s (%d)", vk_err2str(result), result);
			GFX_FREE(image_views);
			return false;
		}
	}
	VK_DEVICE->swapchain_image_views = image_views;
	return true;
}

static bool create_descriptor_pool(struct gfx_device *device)
{
	VkDescriptorPoolSize pool_sizes[2];
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = 1024 * 1024;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = 1024 * 1024;

	VkDescriptorPoolCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.maxSets = 1024 * 1024; /* XXX oh boy... */
	create_info.poolSizeCount = sizeof(pool_sizes) / sizeof(*pool_sizes);
	create_info.pPoolSizes = pool_sizes;
	VkResult result = vkCreateDescriptorPool(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->descriptor_pool);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create vulkan descriptor pool: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_command_pool(struct gfx_device *device)
{
	VkCommandPoolCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = VK_DEVICE->graphics_family;
	VkResult result = vkCreateCommandPool(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->command_pool);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create vulkan command pool: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_command_buffers(struct gfx_device *device)
{
	VkCommandBufferAllocateInfo allocate_info;
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.commandPool = VK_DEVICE->command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = sizeof(VK_DEVICE->command_buffers) / sizeof(*VK_DEVICE->command_buffers);
	VkResult result = vkAllocateCommandBuffers(VK_DEVICE->vk_device, &allocate_info, VK_DEVICE->command_buffers);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create vulkan command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_sync_objects(struct gfx_device *device)
{
	VkSemaphoreCreateInfo semaphore_info;
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = NULL;
	semaphore_info.flags = 0;
	VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = NULL;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < FRAMES_COUNT; ++i)
	{
		VkResult result = vkCreateSemaphore(VK_DEVICE->vk_device, &semaphore_info, ALLOCATION_CALLBACKS, &VK_DEVICE->swapchain_images_semaphores[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create semaphore: %s (%d)", vk_err2str(result), result);
			return false;
		}
		result = vkCreateSemaphore(VK_DEVICE->vk_device, &semaphore_info, ALLOCATION_CALLBACKS, &VK_DEVICE->render_finished_semaphores[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create semaphore: %s (%d)", vk_err2str(result), result);
			return false;
		}
		result = vkCreateFence(VK_DEVICE->vk_device, &fence_info, ALLOCATION_CALLBACKS, &VK_DEVICE->rendering_fences[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create fence: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}
	return true;
}

static bool create_buffer(struct gfx_device *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaMemoryUsage vma_usage, VkBuffer *buffer, VmaAllocation *allocation)
{
	VkBufferCreateInfo buffer_info;
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.pNext = NULL;
	buffer_info.flags = 0;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_DEVICE->graphics_transfer_sharing_mode;
	buffer_info.queueFamilyIndexCount = VK_DEVICE->graphics_transfer_count;
	buffer_info.pQueueFamilyIndices = VK_DEVICE->graphics_transfer_families;

	VmaAllocationCreateInfo alloc_info;
	alloc_info.flags = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ? (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT) : 0;
	alloc_info.usage = vma_usage;
	alloc_info.requiredFlags = 0;
	alloc_info.preferredFlags = 0;
	alloc_info.memoryTypeBits = 0;
	alloc_info.pool = VK_NULL_HANDLE;
	alloc_info.pUserData = NULL;
	alloc_info.priority = 1;
	VkResult result = vmaCreateBuffer(VK_DEVICE->vma_allocator, &buffer_info, &alloc_info, buffer, allocation, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("buffer allocation of %zu bytes failed: %s (%d)", size, vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool begin_single_time_commands(struct gfx_device *device, VkCommandBuffer *command_buffer)
{
	VkCommandBufferAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.commandPool = VK_DEVICE->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(VK_DEVICE->vk_device, &alloc_info, command_buffer);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to allocate command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}

	VkCommandBufferBeginInfo begin_info;
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = NULL;
	result = vkBeginCommandBuffer(*command_buffer, &begin_info);
	if (result != VK_SUCCESS)
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, command_buffer);
		GFX_ERROR_CALLBACK("failed to begin command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool end_single_time_commands(struct gfx_device *device, VkQueue queue, VkCommandBuffer command_buffer)
{
	VkResult result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS)
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
		GFX_ERROR_CALLBACK("failed to end command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}

	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = NULL;
	submit_info.pWaitDstStageMask = NULL;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;
	result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
		GFX_ERROR_CALLBACK("failed to submit command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	result = vkQueueWaitIdle(queue);
	if (result != VK_SUCCESS)
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
		GFX_ERROR_CALLBACK("failed to wait idle queue: %s (%d)", vk_err2str(result), result);
		return false;
	}
	vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
	return true;
}

static bool create_staging_buffer(struct gfx_device *device, VkBuffer *buffer, VmaAllocation *allocation, const void *data, size_t size)
{
	void *map;
	if (!create_buffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, buffer, allocation))
		return false;
	VkResult result = vmaMapMemory(VK_DEVICE->vma_allocator, *allocation, &map);
	if (result != VK_SUCCESS)
	{
		vmaDestroyBuffer(VK_DEVICE->vma_allocator, *buffer, *allocation);
		GFX_ERROR_CALLBACK("failed to map staging buffer memory: %s (%d)", vk_err2str(result), result);
		return false;
	}
	memcpy(map, data, size);
	vmaUnmapMemory(VK_DEVICE->vma_allocator, *allocation);
	return true;
}

static bool set_buffer_memory_staging(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset, VkCommandBuffer command_buffer)
{
	VkBuffer staging_buffer;
	VmaAllocation staging_allocation;
	if (!create_staging_buffer(device, &staging_buffer, &staging_allocation, data, size))
		return false;
	VkBufferCopy copy_region;
	copy_region.srcOffset = 0;
	copy_region.dstOffset = offset;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, staging_buffer, (VkBuffer)buffer->handle.ptr, 1, &copy_region);
	vmaDestroyBuffer(VK_DEVICE->vma_allocator, staging_buffer, staging_allocation);
	return true;
}

static bool copy_buffer_data_by_staging(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	VkCommandBuffer command_buffer;
	if (!begin_single_time_commands(device, &command_buffer))
		return false;
	if (!set_buffer_memory_staging(device, buffer, data, size, 0, command_buffer))
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
		return false;
	}
	if (!end_single_time_commands(device, VK_DEVICE->transfer_queue, command_buffer))
		return false;
	return true;
}

static void copy_buffer_to_image(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

static void transition_image_layout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels)
{
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		assert(!"invalid layout transition");
	}
	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static bool create_render_pass(struct gfx_device *device)
{
	size_t attachments_count = 2;
	VkAttachmentDescription attachments[3];

	VkAttachmentDescription *color_attachment = &attachments[0];
	color_attachment->format = VK_DEVICE->swapchain_format;
	color_attachment->samples = VK_DEVICE->msaa_samples;
	color_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (VK_DEVICE->msaa_samples != VK_SAMPLE_COUNT_1_BIT)
		color_attachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	else
		color_attachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref;
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription *depth_attachment = &attachments[1];
	depth_attachment->format = VK_FORMAT_D24_UNORM_S8_UINT;
	depth_attachment->samples = VK_DEVICE->msaa_samples;
	depth_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref;
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (VK_DEVICE->msaa_samples != VK_SAMPLE_COUNT_1_BIT)
	{
		attachments_count++;
		VkAttachmentDescription *resolve_attachment = &attachments[2];
		resolve_attachment->format = VK_DEVICE->swapchain_format;
		resolve_attachment->samples = VK_SAMPLE_COUNT_1_BIT;
		resolve_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolve_attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolve_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolve_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolve_attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		resolve_attachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	VkAttachmentReference resolve_attachment_ref;
	resolve_attachment_ref.attachment = 2;
	resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	if (VK_DEVICE->msaa_samples != VK_SAMPLE_COUNT_1_BIT)
		subpass.pResolveAttachments = &resolve_attachment_ref;
	else
		subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkSubpassDependency dependencies[1];
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = 0;

	VkRenderPassCreateInfo render_pass_info;
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = NULL;
	render_pass_info.flags = 0;
	render_pass_info.attachmentCount = attachments_count;
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = sizeof(dependencies) / sizeof(*dependencies);
	render_pass_info.pDependencies = dependencies;
	VkResult result = vkCreateRenderPass(VK_DEVICE->vk_device, &render_pass_info, ALLOCATION_CALLBACKS, &VK_DEVICE->render_pass);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create render pass: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_depth_buffer(struct gfx_device *device)
{
	VkImageCreateInfo image_info;
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
	image_info.extent.width = VK_DEVICE->swapchain_extent.width;
	image_info.extent.height = VK_DEVICE->swapchain_extent.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_DEVICE->msaa_samples;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo alloc_info;
	alloc_info.flags = 0;
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_info.requiredFlags = 0;
	alloc_info.preferredFlags = 0;
	alloc_info.memoryTypeBits = 0;
	alloc_info.pool = VK_NULL_HANDLE;
	alloc_info.pUserData = NULL;
	alloc_info.priority = 1;
	VkResult result = vmaCreateImage(VK_DEVICE->vma_allocator, &image_info, &alloc_info, &VK_DEVICE->depth_image, &VK_DEVICE->depth_image_allocation, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create depth image: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.flags = 0;
	view_info.image = VK_DEVICE->depth_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	result = vkCreateImageView(VK_DEVICE->vk_device, &view_info, ALLOCATION_CALLBACKS, &VK_DEVICE->depth_image_view);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create depth image view: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkCommandBuffer command_buffer;
	if (!begin_single_time_commands(device, &command_buffer))
		return false;
	transition_image_layout(command_buffer, VK_DEVICE->depth_image, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	if (!end_single_time_commands(device, VK_DEVICE->graphics_queue, command_buffer))
		return false;
	return true;
}

static bool create_msaa_buffer(struct gfx_device *device)
{
	VkImageCreateInfo image_info;
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_DEVICE->swapchain_format;
	image_info.extent.width = VK_DEVICE->swapchain_extent.width;
	image_info.extent.height = VK_DEVICE->swapchain_extent.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_DEVICE->msaa_samples;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo alloc_info;
	alloc_info.flags = 0;
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_info.requiredFlags = 0;
	alloc_info.preferredFlags = 0;
	alloc_info.memoryTypeBits = 0;
	alloc_info.pool = VK_NULL_HANDLE;
	alloc_info.pUserData = NULL;
	alloc_info.priority = 1;
	VkResult result = vmaCreateImage(VK_DEVICE->vma_allocator, &image_info, &alloc_info, &VK_DEVICE->msaa_image, &VK_DEVICE->msaa_image_allocation, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create msaa image: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.flags = 0;
	view_info.image = VK_DEVICE->msaa_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = VK_DEVICE->swapchain_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	result = vkCreateImageView(VK_DEVICE->vk_device, &view_info, ALLOCATION_CALLBACKS, &VK_DEVICE->msaa_image_view);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create msaa image view: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_framebuffers(struct gfx_device *device)
{
	VkFramebuffer *framebuffers = malloc(sizeof(*VK_DEVICE->swapchain_framebuffers) * VK_DEVICE->swapchain_images_count);
	if (!framebuffers)
	{
		GFX_ERROR_CALLBACK("failed to malloc framebuffers: %s (%d)", strerror(errno), errno);
		return false;
	}
	for (size_t i = 0; i < VK_DEVICE->swapchain_images_count; ++i)
	{
		size_t attachments_count = 2;
		VkImageView attachments[3];
		attachments[0] = VK_DEVICE->swapchain_image_views[i];
		attachments[1] = VK_DEVICE->depth_image_view;
		if (VK_DEVICE->msaa_samples != VK_SAMPLE_COUNT_1_BIT)
		{
				attachments[2] = VK_DEVICE->msaa_image_view;
				attachments_count++;
		}
		VkFramebufferCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.flags = 0;
		create_info.renderPass = VK_DEVICE->render_pass;
		create_info.attachmentCount = attachments_count;
		create_info.pAttachments = attachments;
		create_info.width = VK_DEVICE->swapchain_extent.width;
		create_info.height = VK_DEVICE->swapchain_extent.height;
		create_info.layers = 1;
		VkResult result = vkCreateFramebuffer(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &framebuffers[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create framebuffer: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}
	VK_DEVICE->swapchain_framebuffers = framebuffers;
	return true;
}

static void cleanup_swapchain(struct gfx_device *device)
{
	vkDestroyImageView(VK_DEVICE->vk_device, VK_DEVICE->msaa_image_view, ALLOCATION_CALLBACKS);
	vmaDestroyImage(VK_DEVICE->vma_allocator, VK_DEVICE->msaa_image, VK_DEVICE->msaa_image_allocation);
	vkDestroyImageView(VK_DEVICE->vk_device, VK_DEVICE->depth_image_view, ALLOCATION_CALLBACKS);
	vmaDestroyImage(VK_DEVICE->vma_allocator, VK_DEVICE->depth_image, VK_DEVICE->depth_image_allocation);
	for (size_t i = 0; i < VK_DEVICE->swapchain_images_count; ++i)
	{
		vkDestroyFramebuffer(VK_DEVICE->vk_device, VK_DEVICE->swapchain_framebuffers[i], ALLOCATION_CALLBACKS);
		vkDestroyImageView(VK_DEVICE->vk_device, VK_DEVICE->swapchain_image_views[i], ALLOCATION_CALLBACKS);
	}
	vkDestroySwapchainKHR(VK_DEVICE->vk_device, VK_DEVICE->swapchain, ALLOCATION_CALLBACKS);
}

static bool recreate_swapchain(struct gfx_device *device)
{
	vkDeviceWaitIdle(VK_DEVICE->vk_device);
	cleanup_swapchain(device);
	create_swapchain(device);
	create_image_views(device);
	create_depth_buffer(device);
	create_msaa_buffer(device);
	create_framebuffers(device);
}

static bool prepare_frame(struct gfx_device *device)
{
again:;
	VkResult result = vkWaitForFences(VK_DEVICE->vk_device, 1, &VK_DEVICE->rendering_fences[VK_DEVICE->current_frame], VK_TRUE, UINT64_MAX);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to wait for rendering fence: %s (%d)", vk_err2str(result), result);
		return false;
	}
	result = vkAcquireNextImageKHR(VK_DEVICE->vk_device, VK_DEVICE->swapchain, UINT64_MAX, VK_DEVICE->swapchain_images_semaphores[VK_DEVICE->current_frame], VK_NULL_HANDLE, &VK_DEVICE->current_image);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain(device);
		goto again;
	}
	if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to acquire next image: %s (%d)", vk_err2str(result), result);
		return false;
	}
	result = vkResetFences(VK_DEVICE->vk_device, 1, &VK_DEVICE->rendering_fences[VK_DEVICE->current_frame]);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to reset rendering fence: %s (%d)", vk_err2str(result), result);
		return false;
	}
	result = vkResetCommandBuffer(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], 0);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to reset command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkCommandBufferBeginInfo begin_info;
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = NULL;
	result = vkBeginCommandBuffer(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], &begin_info);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to begin command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 1;
	clear_values[0].color.float32[1] = 1;
	clear_values[0].color.float32[2] = 1;
	clear_values[0].color.float32[3] = 1;
	clear_values[1].depthStencil.depth = 1;
	clear_values[1].depthStencil.stencil = 0;
	VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = NULL;
	render_pass_begin_info.renderPass = VK_DEVICE->render_pass;
	render_pass_begin_info.framebuffer = VK_DEVICE->swapchain_framebuffers[VK_DEVICE->current_image];
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = VK_DEVICE->swapchain_extent;
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;
	vkCmdBeginRenderPass(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	return true;
}

static bool vk_ctr(struct gfx_device *device, struct gfx_window *window)
{
	VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_KHR;
	VK_DEVICE->msaa_samples = VK_SAMPLE_COUNT_1_BIT;
	if (!gfx_device_vtable.ctr(device, window))
		return false;
	if (!get_physical_device(device))
		return false;
	if (!create_device(device))
		return false;
	if (!create_vma_allocator(device))
		return false;
	if (!create_swapchain(device))
		return false;
	if (!create_image_views(device))
		return false;
	if (!create_descriptor_pool(device))
		return false;
	if (!create_command_pool(device))
		return false;
	if (!create_command_buffers(device))
		return false;
	if (!create_sync_objects(device))
		return false;
	if (!create_render_pass(device))
		return false;
	if (!create_depth_buffer(device))
		return false;
	if (!create_msaa_buffer(device))
		return false;
	if (!create_framebuffers(device))
		return false;
	if (!prepare_frame(device))
		return false;
	device->capabilities.constant_alignment = VK_DEVICE->physical_device_properties.limits.minUniformBufferOffsetAlignment;
	device->capabilities.max_msaa = get_max_msaa_samples(device);
	device->capabilities.max_anisotropy = VK_DEVICE->physical_device_properties.limits.maxSamplerAnisotropy;
	device->capabilities.max_samplers = VK_DEVICE->physical_device_properties.limits.maxPerStageDescriptorSampledImages;
	return true;
}

static void vk_dtr(struct gfx_device *device)
{
	vkDeviceWaitIdle(VK_DEVICE->vk_device);
	cleanup_swapchain(device);
	for (size_t i = 0; i < FRAMES_COUNT; ++i)
	{
		vkDestroySemaphore(VK_DEVICE->vk_device, VK_DEVICE->swapchain_images_semaphores[i], ALLOCATION_CALLBACKS);
		vkDestroySemaphore(VK_DEVICE->vk_device, VK_DEVICE->render_finished_semaphores[i], ALLOCATION_CALLBACKS);
		vkDestroyFence(VK_DEVICE->vk_device, VK_DEVICE->rendering_fences[i], ALLOCATION_CALLBACKS);
	}
	vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, sizeof(VK_DEVICE->command_buffers) / sizeof(*VK_DEVICE->command_buffers), VK_DEVICE->command_buffers);
	vkDestroyCommandPool(VK_DEVICE->vk_device, VK_DEVICE->command_pool, ALLOCATION_CALLBACKS);
	vkDestroyDescriptorPool(VK_DEVICE->vk_device, VK_DEVICE->descriptor_pool, ALLOCATION_CALLBACKS);
	vkDestroyRenderPass(VK_DEVICE->vk_device, VK_DEVICE->render_pass, ALLOCATION_CALLBACKS);
	vkDestroySurfaceKHR(VK_DEVICE->instance, VK_DEVICE->surface, ALLOCATION_CALLBACKS);
	vkDestroyInstance(VK_DEVICE->instance, ALLOCATION_CALLBACKS);
	vkDestroyDevice(VK_DEVICE->vk_device, ALLOCATION_CALLBACKS);
	GFX_FREE(VK_DEVICE->swapchain_image_views);
	GFX_FREE(VK_DEVICE->surface_formats);
	GFX_FREE(VK_DEVICE->swapchain_images);
	gfx_device_vtable.dtr(device);
}

static void vk_tick(struct gfx_device *device)
{
	gfx_device_vtable.tick(device);
}

static void vk_clear_color(struct gfx_device *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	VkClearColorValue clear_value;
	clear_value.float32[0] = color.x;
	clear_value.float32[1] = color.y;
	clear_value.float32[2] = color.z;
	clear_value.float32[3] = color.w;
	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	vkCmdClearColorImage(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], VK_DEVICE->swapchain_images[VK_DEVICE->current_frame], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
}

static void vk_clear_depth_stencil(struct gfx_device *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	VkClearDepthStencilValue clear_value;
	clear_value.depth = depth;
	clear_value.stencil = stencil;
	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	vkCmdClearDepthStencilImage(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], VK_DEVICE->depth_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
}

static void vk_draw_indexed_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndexedIndirect(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], (VkBuffer)buffer->handle.ptr, offset, count, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, count);
}

static void vk_draw_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndirect(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], (VkBuffer)buffer->handle.ptr, offset, count, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, count);
}

static void vk_draw_indexed_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], count, prim_count, offset, 0, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, prim_count);
}

static void vk_draw_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDraw(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], count, prim_count, offset, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, prim_count);
}

static void vk_draw_indexed(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], count, 1, offset, 0, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, 1);
}

static void vk_draw(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	vkCmdDraw(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], count, 1, offset, 0);
	gfx_add_draw_stats(device, VK_DEVICE->primitive, count, 1);
}

static bool vk_create_blend_state(struct gfx_device *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a, enum gfx_color_mask color_mask)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->enabled = enabled;
	state->src_c = src_c;
	state->dst_c = dst_c;
	state->src_a = src_a;
	state->dst_a = dst_a;
	state->equation_c = equation_c;
	state->equation_a = equation_a;
	state->color_mask = color_mask;
	return true;
}

static void vk_delete_blend_state(struct gfx_device *device, gfx_blend_state_t *state)
{
	(void)device;
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->depth_write = depth_write;
	state->depth_test = depth_test;
	state->depth_compare = depth_compare;
	state->stencil_enabled = stencil_enabled;
	state->stencil_write_mask = stencil_write_mask;
	state->stencil_compare = stencil_compare;
	state->stencil_reference = stencil_reference;
	state->stencil_compare_mask = stencil_compare_mask;
	state->stencil_fail = stencil_fail;
	state->stencil_zfail = stencil_zfail;
	state->stencil_pass = stencil_pass;
	return true;
}

static void vk_delete_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->fill_mode = fill_mode;
	state->cull_mode = cull_mode;
	state->front_face = front_face;
	state->scissor = scissor;
	return true;
}

static void vk_delete_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_buffer(struct gfx_device *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	assert(!buffer->handle.u64);
	buffer->device = device;
	buffer->usage = usage;
	buffer->type = type;
	buffer->size = size;
	VkBufferUsageFlags vk_usage;
	VkMemoryPropertyFlags vk_properties;
	VmaMemoryUsage vma_usage;
	switch (type)
	{
		case GFX_BUFFER_VERTEXES:
			vk_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case GFX_BUFFER_INDICES:
			vk_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		case GFX_BUFFER_UNIFORM:
			vk_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
		default:
			GFX_ERROR_CALLBACK("invalid buffer type: %d", type);
			return false;
	}
	switch (usage)
	{
		case GFX_BUFFER_IMMUTABLE:
		case GFX_BUFFER_STATIC:
		case GFX_BUFFER_DYNAMIC: /* XXX keep staging buffer active ? */
			vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vma_usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			vk_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case GFX_BUFFER_STREAM:
			vk_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			vma_usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			break;
		default:
			GFX_ERROR_CALLBACK("invalid buffer usage: %d", usage);
			return false;
	}
	if (!create_buffer(device, size, vk_usage, vk_properties, vma_usage, (VkBuffer*)&buffer->handle.ptr, (VmaAllocation*)&buffer->allocation.ptr))
		return false;
	if (usage == GFX_BUFFER_STREAM)
	{
		VkResult result = vmaMapMemory(VK_DEVICE->vma_allocator, (VmaAllocation)buffer->allocation.ptr, &buffer->map);
		if (result != VK_SUCCESS)
		{
			vmaDestroyBuffer(VK_DEVICE->vma_allocator, (VkBuffer)buffer->handle.ptr, (VmaAllocation)buffer->allocation.ptr);
			buffer->handle.ptr = NULL;
			GFX_ERROR_CALLBACK("failed to map memory: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}
	if (data)
	{
		switch (usage)
		{
			case GFX_BUFFER_IMMUTABLE:
			case GFX_BUFFER_STATIC:
			case GFX_BUFFER_DYNAMIC:
				if (!copy_buffer_data_by_staging(device, buffer, data, size, 0))
				{
					vmaDestroyBuffer(VK_DEVICE->vma_allocator, (VkBuffer)buffer->handle.ptr, (VmaAllocation)buffer->allocation.ptr);
					buffer->handle.ptr = NULL;
					return false;
				}
				break;
			case GFX_BUFFER_STREAM:
				memcpy(buffer->map, data, size);
				break;
		}
	}
	return true;
}

static bool vk_set_buffer_data(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	assert(buffer->handle.ptr);
	switch (buffer->usage)
	{
		case GFX_BUFFER_IMMUTABLE:
		case GFX_BUFFER_STATIC:
		case GFX_BUFFER_DYNAMIC:
			if (!copy_buffer_data_by_staging(device, buffer, data, size, offset))
				return false;
			break;
		case GFX_BUFFER_STREAM:
			memcpy(&((uint8_t*)buffer->map)[offset], data, size);
			break;
	}
	return true;
}

static void vk_delete_buffer(struct gfx_device *device, gfx_buffer_t *buffer)
{
	if (!buffer || !buffer->handle.ptr)
		return;
	if (buffer->usage == GFX_BUFFER_STREAM)
		vmaUnmapMemory(VK_DEVICE->vma_allocator, (VmaAllocation)buffer->allocation.ptr);
	vmaDestroyBuffer(VK_DEVICE->vma_allocator, (VkBuffer)buffer->handle.ptr, (VmaAllocation)buffer->allocation.ptr);
	buffer->handle.ptr = NULL;
}

static bool vk_create_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state, const struct gfx_attribute_bind *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	assert(!state->handle.ptr);
	state->handle.ptr = (void*)1;
	memcpy(state->binds, binds, sizeof(*binds) * count);
	state->count = count;
	state->index_type = index_type;
	state->index_buffer = index_buffer;
	return true;
}

static void vk_bind_attributes_state(struct gfx_device *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	assert(state->handle.ptr);
	VkBuffer buffers[GFX_MAX_ATTRIBUTES_COUNT];
	VkDeviceSize offsets[GFX_MAX_ATTRIBUTES_COUNT];
	for (uint32_t i = 0; i < input_layout->count; ++i)
	{
		buffers[i] = state->binds[input_layout->binds[i].buffer].buffer->handle.ptr;
		offsets[i] = 0;
	}
	vkCmdBindVertexBuffers(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], 0, state->count, buffers, offsets);
	if (state->index_buffer)
		vkCmdBindIndexBuffer(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], state->index_buffer->handle.ptr, 0, index_types[state->index_type]);
}

static void vk_delete_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout, const struct gfx_input_layout_bind *binds, uint32_t count, const gfx_shader_state_t *shader_state)
{
	(void)shader_state;
	assert(!input_layout->handle.ptr);
	input_layout->handle.ptr = (void*)1;
	input_layout->device = device;
	memcpy(input_layout->binds, binds, sizeof(*binds) * count);
	input_layout->count = count;
	return true;
}

static void vk_delete_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout)
{
	(void)device;
	if (!input_layout || !input_layout->handle.u64)
		return;
	input_layout->handle.u64 = 0;
}

static bool vk_create_texture(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!texture->handle.u64);
	texture->device = device;
	texture->format = format;
	texture->type = type;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->lod = lod;
	texture->addressing_s = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_t = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_r = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->min_filtering = GFX_FILTERING_NEAREST;
	texture->mag_filtering = GFX_FILTERING_LINEAR;
	texture->mip_filtering = GFX_FILTERING_LINEAR;
	texture->anisotropy = 1;
	texture->min_level = 0;
	texture->max_level = lod - 1;
	VkImageCreateInfo image_info;
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.imageType = image_types[type];
	image_info.format = formats[format];
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = type == GFX_TEXTURE_3D ? depth : 1;
	image_info.mipLevels = lod;
	image_info.arrayLayers = (type == GFX_TEXTURE_2D_ARRAY || type == GFX_TEXTURE_2D_ARRAY_MS) ? depth : 1;
	image_info.samples = 1;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.sharingMode = VK_DEVICE->graphics_transfer_sharing_mode;
	image_info.queueFamilyIndexCount = VK_DEVICE->graphics_transfer_count;
	image_info.pQueueFamilyIndices = VK_DEVICE->graphics_transfer_families;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo alloc_info;
	alloc_info.flags = 0;
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_info.requiredFlags = 0;
	alloc_info.preferredFlags = 0;
	alloc_info.memoryTypeBits = 0;
	alloc_info.pool = VK_NULL_HANDLE;
	alloc_info.pUserData = NULL;
	alloc_info.priority = 1;
	VkResult result = vmaCreateImage(VK_DEVICE->vma_allocator, &image_info, &alloc_info, (VkImage*)&texture->handle.ptr, (VmaAllocation*)&texture->allocation.ptr, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create image: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.flags = 0;
	view_info.image = texture->handle.ptr;
	view_info.viewType = image_view_types[type];
	view_info.format = formats[format];
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask = format == GFX_DEPTH24_STENCIL8 ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT: VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = lod;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = (type == GFX_TEXTURE_2D_ARRAY || type == GFX_TEXTURE_2D_ARRAY_MS) ? depth : 1;
	result = vkCreateImageView(VK_DEVICE->vk_device, &view_info, ALLOCATION_CALLBACKS, (VkImageView*)&texture->view.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create image view: %s (%d)", vk_err2str(result), result);
		vmaDestroyImage(VK_DEVICE->vma_allocator, (VkImage)texture->handle.ptr, (VmaAllocation)texture->allocation.ptr);
		return false;
	}
	texture->sampler.ptr = NULL;
	return true;
}

static bool vk_set_texture_data(struct gfx_device *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
	assert(texture->handle.u64);
	VkCommandBuffer command_buffer;
	if (!begin_single_time_commands(device, &command_buffer))
		return false;
	VkBuffer staging_buffer;
	VmaAllocation staging_allocation;
	if (!create_staging_buffer(device, &staging_buffer, &staging_allocation, data, size))
	{
		vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &command_buffer);
		return false;
	}
	/* XXX transition should be made more carefuly, based on real current layout */
	transition_image_layout(command_buffer, texture->handle.ptr, formats[texture->format], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
	copy_buffer_to_image(command_buffer, staging_buffer, texture->handle.ptr, width, height);
	transition_image_layout(command_buffer, texture->handle.ptr, formats[texture->format], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
	vmaDestroyBuffer(VK_DEVICE->vma_allocator, staging_buffer, staging_allocation);
	/* XXX should be transfer queue, but it does generate a VK_ERROR_DEVICE_LOST */
	if (!end_single_time_commands(device, VK_DEVICE->graphics_queue, command_buffer))
		return false;
	return true;
}

static void vk_set_texture_addressing(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->addressing_s = addressing_s;
	texture->addressing_t = addressing_t;
	texture->addressing_r = addressing_r;
}

static void vk_set_texture_filtering(struct gfx_device *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->min_filtering = min_filtering;
	texture->mag_filtering = mag_filtering;
	texture->mip_filtering = mip_filtering;
}

static void vk_set_texture_anisotropy(struct gfx_device *device, gfx_texture_t *texture, uint32_t anisotropy)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->anisotropy = anisotropy;
}

static void vk_set_texture_levels(struct gfx_device *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->min_level = min_level;
	texture->max_level = max_level;
}

static bool vk_finalize_texture(struct gfx_device *device, gfx_texture_t *texture)
{
	assert(texture->handle.ptr);
	VkSamplerCreateInfo sampler_info;
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.pNext = NULL;
	sampler_info.flags = 0;
	sampler_info.magFilter = min_mag_filterings[texture->min_filtering];
	sampler_info.minFilter = min_mag_filterings[texture->mag_filtering];
	sampler_info.mipmapMode = mipmap_filterings[texture->mip_filtering];
	sampler_info.addressModeU = address_modes[texture->addressing_s];
	sampler_info.addressModeV = address_modes[texture->addressing_t];
	sampler_info.addressModeW = address_modes[texture->addressing_r];
	sampler_info.mipLodBias = 0;
	sampler_info.anisotropyEnable = texture->anisotropy > 1;
	sampler_info.maxAnisotropy = texture->anisotropy;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.minLod = 0;
	sampler_info.maxLod = texture->mip_filtering != GFX_FILTERING_NONE ? texture->lod - 1 : 0;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	VkResult result = vkCreateSampler(VK_DEVICE->vk_device, &sampler_info, ALLOCATION_CALLBACKS, (VkSampler*)&texture->sampler.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create image sampler: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_texture(struct gfx_device *device, gfx_texture_t *texture)
{
	if (!texture || !texture->handle.ptr)
		return;
	vkDestroySampler(VK_DEVICE->vk_device, (VkSampler)texture->sampler.ptr, ALLOCATION_CALLBACKS);
	vkDestroyImageView(VK_DEVICE->vk_device, (VkImageView)texture->view.ptr, ALLOCATION_CALLBACKS);
	vmaDestroyImage(VK_DEVICE->vma_allocator, (VkImage)texture->handle.ptr, (VmaAllocation)texture->allocation.ptr);
	texture->handle.ptr = NULL;
}

static bool vk_create_shader(struct gfx_device *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t size)
{
	assert(!shader->handle.u64);
	shader->device = device;
	shader->type = type;
	VkShaderModuleCreateInfo shader_create_info;
	shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_create_info.pNext = NULL;
	shader_create_info.flags = 0;
	shader_create_info.codeSize = size;
	shader_create_info.pCode = (const uint32_t*)data;
	VkResult result = vkCreateShaderModule(VK_DEVICE->vk_device, &shader_create_info, ALLOCATION_CALLBACKS, (VkShaderModule*)&shader->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create shader module: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_shader(struct gfx_device *device, gfx_shader_t *shader)
{
	if (!shader || !shader->handle.ptr)
		return;
	vkDestroyShaderModule(VK_DEVICE->vk_device, (VkShaderModule)shader->handle.ptr, ALLOCATION_CALLBACKS);
	shader->handle.ptr = NULL;
}

static bool vk_create_shader_state(struct gfx_device *device, gfx_shader_state_t *shader_state, const gfx_shader_t **shaders, uint32_t shaders_count, const struct gfx_shader_attribute *attributes, const struct gfx_shader_constant *constants, const struct gfx_shader_sampler *samplers)
{
	assert(!shader_state->handle.ptr);
	const gfx_shader_t *vertex_shader = NULL;
	const gfx_shader_t *fragment_shader = NULL;
	const gfx_shader_t *geometry_shader = NULL;
	for (uint32_t i = 0; i < shaders_count; ++i)
	{
		if (!shaders[i])
			continue;
		switch (shaders[i]->type)
		{
			case GFX_SHADER_VERTEX:
				if (vertex_shader)
				{
					GFX_ERROR_CALLBACK("multiple vertex shaders given");
					return false;
				}
				vertex_shader = shaders[i];
				break;
			case GFX_SHADER_FRAGMENT:
				if (fragment_shader)
				{
					GFX_ERROR_CALLBACK("multiple fragment shaders given");
					return false;
				}
				fragment_shader = shaders[i];
				break;
			case GFX_SHADER_GEOMETRY:
				if (geometry_shader)
				{
					GFX_ERROR_CALLBACK("multiple geometry shaders given");
					return false;
				}
				geometry_shader = shaders[i];
				break;
			default:
				GFX_ERROR_CALLBACK("unknown shader type");
				return false;
		}
	}
	if (!vertex_shader)
	{
		GFX_ERROR_CALLBACK("no vertex shader given");
		return false;
	}
	if (!fragment_shader)
	{
		GFX_ERROR_CALLBACK("no fragment shader given");
		return false;
	}

	assert(vertex_shader->handle.u32[0]);
	assert(fragment_shader->handle.u32[0]);
	if (geometry_shader)
		assert(geometry_shader->handle.u32[0]);

	shader_state->device = device;
	{
		VkDescriptorSetLayoutBinding bindings[GFX_MAX_CONSTANTS_COUNT];
		shader_state->constants_count = 0;
		while (constants[shader_state->constants_count].name)
		{
			VkDescriptorSetLayoutBinding *binding = &bindings[shader_state->constants_count];
			binding->binding = constants[shader_state->constants_count].bind;
			binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding->descriptorCount = 1;
			binding->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			binding->pImmutableSamplers = NULL;
			shader_state->constants_count++;
		}

		VkDescriptorSetLayoutCreateInfo layout_info;
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = NULL;
		layout_info.flags = 0;
		layout_info.bindingCount = shader_state->constants_count;
		layout_info.pBindings = bindings;

		VkResult result = vkCreateDescriptorSetLayout(VK_DEVICE->vk_device, &layout_info, ALLOCATION_CALLBACKS, (VkDescriptorSetLayout*)&shader_state->descriptors[0].ptr);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create descriptor set layout: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}
	{
		VkDescriptorSetLayoutBinding bindings[GFX_MAX_SAMPLERS_COUNT];
		shader_state->samplers_count = 0;
		while (samplers[shader_state->samplers_count].name)
		{
			VkDescriptorSetLayoutBinding *binding = &bindings[shader_state->samplers_count];
			binding->binding = samplers[shader_state->samplers_count].bind;
			binding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding->descriptorCount = 1;
			binding->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			binding->pImmutableSamplers = NULL;
			shader_state->samplers_count++;
		}

		VkDescriptorSetLayoutCreateInfo layout_info;
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = NULL;
		layout_info.flags = 0;
		layout_info.bindingCount = shader_state->samplers_count;
		layout_info.pBindings = bindings;

		VkResult result = vkCreateDescriptorSetLayout(VK_DEVICE->vk_device, &layout_info, ALLOCATION_CALLBACKS, (VkDescriptorSetLayout*)&shader_state->descriptors[1].ptr);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("failed to create descriptor set layout: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}

	VkDescriptorSetLayout descriptors[2];
	descriptors[0] = (VkDescriptorSetLayout)shader_state->descriptors[0].ptr;
	descriptors[1] = (VkDescriptorSetLayout)shader_state->descriptors[1].ptr;
	VkPipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = NULL;
	pipeline_layout_info.flags = 0;
	pipeline_layout_info.setLayoutCount = sizeof(descriptors) / sizeof(*descriptors);
	pipeline_layout_info.pSetLayouts = descriptors;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = NULL;
	VkResult result = vkCreatePipelineLayout(VK_DEVICE->vk_device, &pipeline_layout_info, ALLOCATION_CALLBACKS, (VkPipelineLayout*)&shader_state->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create pipeline layout: %s (%d)", vk_err2str(result), result);
		return false;
	}

	if (vertex_shader)
		shader_state->vertex_shader = vertex_shader->handle;
	if (fragment_shader)
		shader_state->fragment_shader = fragment_shader->handle;
	if (geometry_shader)
		shader_state->geometry_shader = geometry_shader->handle;

	return true;
}

static void vk_delete_shader_state(struct gfx_device *device, gfx_shader_state_t *shader_state)
{
	if (!shader_state || !shader_state->handle.ptr)
		return;
	vkDestroyDescriptorSetLayout(VK_DEVICE->vk_device, (VkDescriptorSetLayout)shader_state->descriptors[0].ptr, ALLOCATION_CALLBACKS);
	vkDestroyDescriptorSetLayout(VK_DEVICE->vk_device, (VkDescriptorSetLayout)shader_state->descriptors[1].ptr, ALLOCATION_CALLBACKS);
	vkDestroyPipelineLayout(VK_DEVICE->vk_device, (VkPipelineLayout)shader_state->handle.ptr, ALLOCATION_CALLBACKS);
	shader_state->handle.ptr = NULL;
}

static void vk_bind_constant(struct gfx_device *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	//vkCmdBindDescriptorSets(VK_DEVICE->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, bind, 1, &descriptor_sets[i], 1, &offset);
}

static void vk_bind_samplers(struct gfx_device *device, uint32_t start, uint32_t count, const gfx_texture_t **texture)
{
	//vkCmdBindDescriptorSets
}

static bool vk_create_constant_state(struct gfx_device *device, gfx_constant_state_t *state, const gfx_shader_state_t *shader_state)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->shader_state = shader_state;
	VkDescriptorSetAllocateInfo allocate_info;
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.descriptorPool = VK_DEVICE->descriptor_pool;
	allocate_info.descriptorSetCount = 1;
	allocate_info.pSetLayouts = (VkDescriptorSetLayout*)&shader_state->descriptors[0].ptr;
	VkResult result = vkAllocateDescriptorSets(VK_DEVICE->vk_device, &allocate_info, (VkDescriptorSet*)&state->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create constant descriptor set: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_set_constant(struct gfx_device *device, gfx_constant_state_t *state, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = buffer->handle.ptr;
	buffer_info.offset = offset;
	buffer_info.range = size;

	VkWriteDescriptorSet write_descriptor;
	write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor.pNext = NULL;
	write_descriptor.dstSet = state->handle.ptr;
	write_descriptor.dstBinding = bind;
	write_descriptor.dstArrayElement = 0;
	write_descriptor.descriptorCount = 1;
	write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor.pImageInfo = NULL;
	write_descriptor.pBufferInfo = &buffer_info;
	write_descriptor.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(VK_DEVICE->vk_device, 1, &write_descriptor, 0, NULL);
}

static void vk_bind_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	vkCmdBindDescriptorSets(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)state->shader_state->handle.ptr, 0, 1, (VkDescriptorSet*)&state->handle.ptr, 0, NULL);
}

static void vk_delete_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	vkFreeDescriptorSets(VK_DEVICE->vk_device, VK_DEVICE->descriptor_pool, 1, (VkDescriptorSet*)&state->handle.ptr);
	state->handle.ptr = NULL;
}

static bool vk_create_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state, const gfx_shader_state_t *shader_state)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->shader_state = shader_state;
	VkDescriptorSetAllocateInfo allocate_info;
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.descriptorPool = VK_DEVICE->descriptor_pool;
	allocate_info.descriptorSetCount = 1;
	allocate_info.pSetLayouts = (VkDescriptorSetLayout*)&shader_state->descriptors[1].ptr;
	VkResult result = vkAllocateDescriptorSets(VK_DEVICE->vk_device, &allocate_info, (VkDescriptorSet*)&state->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create samplers descriptor set: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_set_sampler(struct gfx_device *device, gfx_sampler_state_t *state, uint32_t bind, const gfx_texture_t *texture)
{
	VkDescriptorImageInfo image_info;
	image_info.sampler = texture->sampler.ptr;
	image_info.imageView = texture->view.ptr;
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write_descriptor;
	write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor.pNext = NULL;
	write_descriptor.dstSet = state->handle.ptr;
	write_descriptor.dstBinding = 0;
	write_descriptor.dstArrayElement = 0;
	write_descriptor.descriptorCount = 1;
	write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor.pImageInfo = &image_info;
	write_descriptor.pBufferInfo = NULL;
	write_descriptor.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(VK_DEVICE->vk_device, 1, &write_descriptor, 0, NULL);
}

static void vk_bind_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	vkCmdBindDescriptorSets(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)state->shader_state->handle.ptr, 1, 1, (VkDescriptorSet*)&state->handle.ptr, 0, NULL);
}

static void vk_delete_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	vkFreeDescriptorSets(VK_DEVICE->vk_device, VK_DEVICE->descriptor_pool, 1, (VkDescriptorSet*)&state->handle.ptr);
	state->handle.ptr = NULL;
}

static bool vk_create_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
	render_target->device = device;
	return true;
}

static void vk_delete_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
}

static void vk_bind_render_target(struct gfx_device *device, const gfx_render_target_t *render_target)
{
}

static void vk_set_render_target_texture(struct gfx_device *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
}

static void vk_set_render_target_draw_buffers(struct gfx_device *device, gfx_render_target_t *render_target, uint32_t *render_buffers, uint32_t render_buffers_count)
{
}

static void vk_resolve_render_target(struct gfx_device *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
}

static bool vk_create_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state, const gfx_shader_state_t *shader_state, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout, enum gfx_primitive_type primitive)
{
	assert(state && !state->handle.ptr);
	state->primitive = primitive;

	uint32_t shader_stages_count = 2;
	VkPipelineShaderStageCreateInfo shader_stages[3];
	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].pNext = NULL;
	shader_stages[0].flags = 0;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = (VkShaderModule)shader_state->vertex_shader.ptr;
	shader_stages[0].pName = "main";
	shader_stages[0].pSpecializationInfo = NULL;
	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].pNext = NULL;
	shader_stages[1].flags = 0;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = (VkShaderModule)shader_state->fragment_shader.ptr;
	shader_stages[1].pName = "main";
	shader_stages[1].pSpecializationInfo = NULL;
	if (shader_state->geometry_shader.ptr)
	{
		shader_stages_count++;
		shader_stages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[2].pNext = NULL;
		shader_stages[2].flags = 0;
		shader_stages[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[2].module = (VkShaderModule)shader_state->geometry_shader.ptr;
		shader_stages[2].pName = "main";
		shader_stages[2].pSpecializationInfo = NULL;
	}

	VkVertexInputAttributeDescription input_attribute_descriptions[GFX_MAX_ATTRIBUTES_COUNT];
	VkVertexInputBindingDescription input_binding_descriptions[GFX_MAX_ATTRIBUTES_COUNT];
	uint32_t attrib_count = 0;
	for (uint32_t i = 0; i < input_layout->count; ++i)
	{
		if (input_layout->binds[i].type == GFX_ATTR_DISABLED)
			continue;
		input_attribute_descriptions[attrib_count].location = i;
		input_attribute_descriptions[attrib_count].binding = i;
		input_attribute_descriptions[attrib_count].format = attribute_types[input_layout->binds[i].type];
		input_attribute_descriptions[attrib_count].offset = input_layout->binds[i].offset;
		input_binding_descriptions[attrib_count].binding = i;
		input_binding_descriptions[attrib_count].stride = input_layout->binds[i].stride;
		input_binding_descriptions[attrib_count].inputRate = input_rates[input_layout->binds[i].step_mode];
		attrib_count++;
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_create_info;
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.pNext = NULL;
	vertex_input_create_info.flags = 0;
	vertex_input_create_info.vertexBindingDescriptionCount = attrib_count;
	vertex_input_create_info.pVertexBindingDescriptions = input_binding_descriptions;
	vertex_input_create_info.vertexAttributeDescriptionCount = attrib_count;
	vertex_input_create_info.pVertexAttributeDescriptions = input_attribute_descriptions;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
	input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.pNext = NULL;
	input_assembly_create_info.flags = 0;
	input_assembly_create_info.topology = primitive_types[primitive];
	input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterization_create_info;
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.pNext = NULL;
	rasterization_create_info.flags = 0;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = fill_modes[rasterizer->fill_mode];
	rasterization_create_info.cullMode = cull_modes[rasterizer->cull_mode];
	rasterization_create_info.frontFace = front_faces[rasterizer->front_face];
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasConstantFactor = 0;
	rasterization_create_info.depthBiasClamp = 0;
	rasterization_create_info.depthBiasSlopeFactor = 0;
	rasterization_create_info.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisample_create_info;
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.pNext = NULL;
	multisample_create_info.flags = 0;
	multisample_create_info.rasterizationSamples = VK_DEVICE->msaa_samples;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.minSampleShading = 1;
	multisample_create_info.pSampleMask = NULL;
	multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_create_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.pNext = NULL;
	depth_stencil_create_info.flags = 0;
	depth_stencil_create_info.depthTestEnable = depth_stencil->depth_test;
	depth_stencil_create_info.depthWriteEnable = depth_stencil->depth_write;
	depth_stencil_create_info.depthCompareOp = compare_functions[depth_stencil->depth_compare];
	depth_stencil_create_info.depthBoundsTestEnable = VK_TRUE;
	depth_stencil_create_info.stencilTestEnable = depth_stencil->stencil_enabled;
	depth_stencil_create_info.front.failOp = stencil_operations[depth_stencil->stencil_fail];
	depth_stencil_create_info.front.passOp = stencil_operations[depth_stencil->stencil_pass];
	depth_stencil_create_info.front.depthFailOp = stencil_operations[depth_stencil->stencil_zfail];
	depth_stencil_create_info.front.compareOp = compare_functions[depth_stencil->stencil_compare];
	depth_stencil_create_info.front.compareMask = depth_stencil->stencil_compare_mask;
	depth_stencil_create_info.front.writeMask = depth_stencil->stencil_write_mask;
	depth_stencil_create_info.front.reference = depth_stencil->stencil_reference;
	depth_stencil_create_info.back.failOp = stencil_operations[depth_stencil->stencil_fail];
	depth_stencil_create_info.back.passOp = stencil_operations[depth_stencil->stencil_pass];
	depth_stencil_create_info.back.depthFailOp = stencil_operations[depth_stencil->stencil_zfail];
	depth_stencil_create_info.back.compareOp = compare_functions[depth_stencil->stencil_compare];
	depth_stencil_create_info.back.compareMask = depth_stencil->stencil_compare_mask;
	depth_stencil_create_info.back.writeMask = depth_stencil->stencil_write_mask;
	depth_stencil_create_info.back.reference = depth_stencil->stencil_reference;
	depth_stencil_create_info.minDepthBounds = 0;
	depth_stencil_create_info.maxDepthBounds = 1;

	VkPipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.blendEnable = blend->enabled;
	color_blend_attachment.srcColorBlendFactor = blend_functions[blend->src_c];
	color_blend_attachment.dstColorBlendFactor = blend_functions[blend->dst_c];
	color_blend_attachment.colorBlendOp = blend_equations[blend->equation_c];
	color_blend_attachment.srcAlphaBlendFactor = blend_functions[blend->src_a];
	color_blend_attachment.dstAlphaBlendFactor = blend_functions[blend->dst_a];
	color_blend_attachment.alphaBlendOp = blend_equations[blend->equation_a];
	color_blend_attachment.colorWriteMask = color_masks[blend->color_mask];

	VkPipelineColorBlendStateCreateInfo color_blend_create_info;
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.pNext = NULL;
	color_blend_create_info.flags = 0;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = 0;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment;
	color_blend_create_info.blendConstants[0] = 1;
	color_blend_create_info.blendConstants[1] = 1;
	color_blend_create_info.blendConstants[2] = 1;
	color_blend_create_info.blendConstants[3] = 1;

	VkViewport viewport;
	viewport.x = 0;
	viewport.y = device->window->height;
	viewport.width = device->window->width;
	viewport.height = -(float)device->window->height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = device->window->width;
	scissor.extent.height = device->window->height;

	VkPipelineViewportStateCreateInfo viewport_state_create_info;
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.pNext = NULL;
	viewport_state_create_info.flags = 0;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	VkDynamicState dynamic_states[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pNext = NULL;
	dynamic_state_create_info.flags = 0;
	dynamic_state_create_info.dynamicStateCount = sizeof(dynamic_states) / sizeof(*dynamic_states);
	dynamic_state_create_info.pDynamicStates = dynamic_states;

	VkGraphicsPipelineCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.stageCount = shader_stages_count;
	create_info.pStages = shader_stages;
	create_info.pVertexInputState = &vertex_input_create_info;
	create_info.pInputAssemblyState = &input_assembly_create_info;
	create_info.pTessellationState = NULL;
	create_info.pViewportState = &viewport_state_create_info;
	create_info.pRasterizationState = &rasterization_create_info;
	create_info.pMultisampleState = &multisample_create_info;
	create_info.pDepthStencilState = &depth_stencil_create_info;
	create_info.pColorBlendState = &color_blend_create_info;
	create_info.pDynamicState = &dynamic_state_create_info;
	create_info.layout = (VkPipelineLayout)shader_state->handle.ptr;
	create_info.renderPass = VK_DEVICE->render_pass;
	create_info.subpass = 0;
	create_info.basePipelineHandle = VK_NULL_HANDLE;
	create_info.basePipelineIndex = -1;
	VkResult result = vkCreateGraphicsPipelines(VK_DEVICE->vk_device, VK_NULL_HANDLE, 1, &create_info, ALLOCATION_CALLBACKS, (VkPipeline*)&state->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to create graphics pipeline: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	vkDestroyPipeline(VK_DEVICE->vk_device, (VkPipeline)state->handle.ptr, ALLOCATION_CALLBACKS);
	state->handle.ptr = NULL;
}

static void vk_bind_pipeline_state(struct gfx_device *device, const gfx_pipeline_state_t *state)
{
	assert(state);
	VK_DEVICE->primitive = state->primitive;
	vkCmdBindPipeline(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)state->handle.ptr);
}

static void vk_set_viewport(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkViewport viewport;
	viewport.x = x;
	viewport.y = height - y;
	viewport.width = width;
	viewport.height = -(float)height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], 0, 1, &viewport);
}

static void vk_set_scissor(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkRect2D rect;
	rect.offset.x = x;
	rect.offset.y = y;
	rect.extent.width = width;
	rect.extent.height = height;
	vkCmdSetScissor(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], 0, 1, &rect);
}

static void vk_set_line_width(struct gfx_device *device, float line_width)
{
	vkCmdSetLineWidth(VK_DEVICE->command_buffers[VK_DEVICE->current_frame], line_width);
}

static void vk_set_point_size(struct gfx_device *device, float point_size)
{
	(void)device;
	(void)point_size;
}

static const struct gfx_device_vtable vk_vtable =
{
	GFX_DEVICE_VTABLE_DEF(vk)
};

void gfx_vk_set_swap_interval(struct gfx_device *device, int interval)
{
	if (interval < 0 && VK_DEVICE->present_mode_fifo_relaxed)
		VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	else if (!interval && VK_DEVICE->present_mode_immediate)
		VK_DEVICE->present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	else
		VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_KHR;
	VK_DEVICE->present_mode_changed = true;
}

void gfx_vk_swap_buffers(struct gfx_device *device)
{
	vkCmdEndRenderPass(VK_DEVICE->command_buffers[VK_DEVICE->current_frame]);
	VkResult result = vkEndCommandBuffer(VK_DEVICE->command_buffers[VK_DEVICE->current_frame]);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to end command buffer: %s (%d)", vk_err2str(result), result);
		return;
	}
	VkSemaphore signal_semaphores[] = {VK_DEVICE->render_finished_semaphores[VK_DEVICE->current_frame]};
	VkSemaphore wait_semaphores[] = {VK_DEVICE->swapchain_images_semaphores[VK_DEVICE->current_frame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = sizeof(wait_semaphores) / sizeof(*wait_semaphores);
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &VK_DEVICE->command_buffers[VK_DEVICE->current_frame];
	submit_info.signalSemaphoreCount = sizeof(signal_semaphores) / sizeof(*signal_semaphores);
	submit_info.pSignalSemaphores = signal_semaphores;
	result = vkQueueSubmit(VK_DEVICE->graphics_queue, 1, &submit_info, VK_DEVICE->rendering_fences[VK_DEVICE->current_frame]);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to submit queue: %s (%d)", vk_err2str(result), result);
	}

	VkSwapchainKHR swapchains[] = {VK_DEVICE->swapchain};
	VkPresentInfoKHR present_info;
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = NULL;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = sizeof(swapchains) / sizeof(*swapchains);
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &VK_DEVICE->current_image;
	present_info.pResults = NULL;
	result = vkQueuePresentKHR(VK_DEVICE->present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR
	 || result == VK_SUBOPTIMAL_KHR
	 || VK_DEVICE->swapchain_extent.width != device->window->width
	 || VK_DEVICE->swapchain_extent.height != device->window->height
	 || VK_DEVICE->present_mode_changed)
	{
		VK_DEVICE->present_mode_changed = false;
		recreate_swapchain(device);
	}
	else if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("failed to swap buffers: %s (%d)", vk_err2str(result), result);
	}
	VK_DEVICE->current_frame = (VK_DEVICE->current_frame + 1) % FRAMES_COUNT;
	prepare_frame(device);
}

static void *vk_allocation(void *userdata, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	uint8_t *addr = GFX_MALLOC(size + alignment);
	size_t delta = (size_t)addr % alignment;
	if (!delta)
		return addr;
	return addr - delta + alignment;
}

static void *vk_realloaction(void *userdata, void *original, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	uint8_t *addr = GFX_REALLOC(original, size + alignment);
	size_t delta = (size_t)addr % alignment;
	if (!delta)
		return addr;
	return addr - delta + alignment;
}

static void vk_free(void *userdata, void *memory)
{
	GFX_FREE(memory);
}

static void vk_internal_allocation(void *userdata, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
{
}

static void vk_internal_free(void *userdata, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
{
}

struct gfx_device *gfx_vk_device_new(struct gfx_window *window, VkInstance instance, VkSurfaceKHR surface)
{
	struct gfx_vk_device *device = GFX_MALLOC(sizeof(*device));
	if (!device)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(device, 0, sizeof(*device));
	device->allocation_callbacks.pUserData = NULL;
	device->allocation_callbacks.pfnAllocation = vk_allocation;
	device->allocation_callbacks.pfnReallocation = vk_realloaction;
	device->allocation_callbacks.pfnFree = vk_free;
	device->allocation_callbacks.pfnInternalAllocation = vk_internal_allocation;
	device->allocation_callbacks.pfnInternalFree = vk_internal_free;
	struct gfx_device *dev = &device->device;
	dev->vtable = &vk_vtable;
	device->instance = instance;
	device->surface = surface;
	if (!dev->vtable->ctr(dev, window))
	{
		dev->vtable->dtr(dev);
		GFX_FREE(device);
		return NULL;
	}
	return dev;
}
