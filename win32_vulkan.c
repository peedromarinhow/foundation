#define  FOUNDATION_IMPL
#include "foundation.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

inline void AssertVulkanError(VkResult Result) {
  Assert(Result == VK_SUCCESS);
}

typedef struct _vulkan_state {
  pool  *Pool;
  UINT32 MaxFramesInFlight;
  UINT32 CurrentFrame;
  BOOL   Resized;

  HWND      WindowHandle;
  HINSTANCE WindowInstance;

  VkInstance       Instance;
  VkPhysicalDevice PhysicalDevice;
  VkDevice         LogicalDevice;
  VkSurfaceKHR     Surface;
  VkQueue          GraphicsQueue;
  VkQueue          PresentQueue;
  VkSwapchainKHR   Swapchain;
  VkImage         *SwapchainImages;
  VkFormat         SwapchainImageFormat;
  VkExtent2D       SwapchainImageExtent;
  VkImageView     *SwapchainImageViews;
  VkPipelineLayout PipelineLayout;
  VkRenderPass     RenderPass;
  VkPipeline       Pipeline;
  VkFramebuffer   *SwapchainFramebuffers;
  VkCommandPool    CommandPool;
  VkCommandBuffer  CommandBuffer;
  VkCommandBuffer *CommandBuffers;
  VkSemaphore     *ImageIsAvailableSemaphores;
  VkSemaphore     *RenderIsFinishedSemaphores;
  VkFence         *IsInFlightFences;
} vulkan_state;

typedef struct _vulkan_queue_family_indices {
  UINT32 GraphicsFamily;
  BOOL   GraphicsFamilyIsValid;

  UINT32 PresentFamily;
  BOOL   PresentFamilyIsValid;
} vulkan_queue_family_indices;
function vulkan_queue_family_indices FindDeviceQueueFamilies(pool *Pool, VkSurfaceKHR Surface, VkPhysicalDevice Device) {
  vulkan_queue_family_indices Res;

  UINT32 QueueFamiliesCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamiliesCount, null);
  VkQueueFamilyProperties *QueueFamilies = PoolPutArr(Pool, QueueFamiliesCount, sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamiliesCount, QueueFamilies);

  fornum (QueueFamilyIdx, QueueFamiliesCount) {
    if (QueueFamilies[QueueFamilyIdx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      Res.GraphicsFamily = QueueFamilyIdx;
      Res.GraphicsFamilyIsValid = true;
    }

    VkBool32 PresentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(Device, QueueFamilyIdx, Surface, &PresentSupport);
    if (PresentSupport) {
      Res.PresentFamily = QueueFamilyIdx;
      Res.PresentFamilyIsValid = true;
    }

    if (Res.GraphicsFamilyIsValid && Res.PresentFamilyIsValid)
      break;
  }
  return Res;
}

typedef struct _vulkan_swapchain_support_details {
  VkSurfaceCapabilitiesKHR SurfaceCapabilites;
  VkSurfaceFormatKHR *SurfaceFormats;
  VkPresentModeKHR *SurfacePresentModes;
} vulkan_swapchain_support_details;
function vulkan_swapchain_support_details GetSwapchainSupportDetails(pool *Pool, VkSurfaceKHR Surface, VkPhysicalDevice Device) {
  vulkan_swapchain_support_details Res;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Res.SurfaceCapabilites);

  UINT32 SurfaceFormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &SurfaceFormatCount, null);
  if (SurfaceFormatCount != 0) {
    Res.SurfaceFormats = PoolPutArr(Pool, SurfaceFormatCount, sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &SurfaceFormatCount, Res.SurfaceFormats);
  }

  UINT32 PresentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, null);
  if (PresentModeCount != 0) {
    Res.SurfacePresentModes = PoolPutArr(Pool, PresentModeCount, sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &SurfaceFormatCount, Res.SurfacePresentModes);
  }
  return Res;
}

function VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(VkSurfaceFormatKHR *AvailableFormats) {
  UINT32 AvailableFormatsCount = ArrLen(AvailableFormats);
  fornum (AvailableFormatIdx, AvailableFormatsCount) {
    VkSurfaceFormatKHR Format = AvailableFormats[AvailableFormatIdx];
    if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return Format;
    }
  }
  return AvailableFormats[0];
}

function VkPresentModeKHR ChooseSwapchainPresentMode(VkPresentModeKHR *AvailablePresentModes) {
  UINT32 AvailablePresentModesCount = ArrLen(AvailablePresentModes);
  fornum (AvailablePresentModeIdx, AvailablePresentModesCount) {
    VkPresentModeKHR PresentMode = AvailablePresentModes[AvailablePresentModeIdx];
    if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return PresentMode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR; 
}

function VkExtent2D ChooseSwapchainExtent(VkSurfaceCapabilitiesKHR *Capabilites, HWND WindowHandle) {
  if (Capabilites->currentExtent.width != UINT32_MAX) {
    return Capabilites->currentExtent;
  }
  else {
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);
    INT32 Width  = ClientRect.right - ClientRect.left;
    INT32 Height = ClientRect.bottom - ClientRect.top;

    VkExtent2D ActualExtent;

    if (Capabilites->minImageExtent.width < Width && Width < Capabilites->maxImageExtent.width)
      ActualExtent.width = Width;
    if (Capabilites->minImageExtent.height < Height && Height < Capabilites->maxImageExtent.height)
      ActualExtent.height = Height;

    return ActualExtent;
  }
}

function void CreateInstance(vulkan_state *Vk) {
  const CHAR *DesiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
  };
  UINT32 AvailableValidationLayersCount;
  vkEnumerateInstanceLayerProperties(&AvailableValidationLayersCount, null);
  VkLayerProperties *AvailableValidationLayers = PoolPutArr(Vk->Pool, AvailableValidationLayersCount, sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&AvailableValidationLayersCount, AvailableValidationLayers);

  BOOL AllDesiredValidationLayersFound = true;
  fornum (DesiredLayerIdx, countof(DesiredValidationLayers)) {
    const CHAR *Desired = DesiredValidationLayers[DesiredLayerIdx];
    BOOL Found = false;
    fornum (AvailableLayerIdx, AvailableValidationLayersCount) {
      CHAR *Available = AvailableValidationLayers[AvailableLayerIdx].layerName;
      if (strcmp(Desired, Available)) {
        Found = true;
        break;
      }
    }
    if (!Found) {
      AllDesiredValidationLayersFound = false;
      break;
    }
  }

  const CHAR *InstanceEnabledExtensionNames[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
  };
  VkInstanceCreateInfo InstanceCreateInfo = {
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .enabledExtensionCount   = countof(InstanceEnabledExtensionNames),
    .ppEnabledExtensionNames = InstanceEnabledExtensionNames,
    .enabledLayerCount       = countof(DesiredValidationLayers),
    .ppEnabledLayerNames     = DesiredValidationLayers,
  };
  AssertVulkanError(vkCreateInstance(&InstanceCreateInfo, null, &Vk->Instance));
}

function void CreateSurface(vulkan_state *Vk) {
  VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {
    .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .hwnd      = Vk->WindowHandle,
    .hinstance = Vk->WindowInstance,
  };
  AssertVulkanError(vkCreateWin32SurfaceKHR(Vk->Instance, &SurfaceCreateInfo, null, &Vk->Surface));
}

function void PichPhysicalDevice(vulkan_state *Vk, const CHAR* const *RequiredDeviceExtensions, UINT32 RequiredDeviceExtensionsCount) {
  UINT32 AvailablePhysicalDevicesCount;
  vkEnumeratePhysicalDevices(Vk->Instance, &AvailablePhysicalDevicesCount, null);
  Assert(AvailablePhysicalDevicesCount != 0);
  VkPhysicalDevice *AvailablePhysicalDevices = PoolPutArr(Vk->Pool, AvailablePhysicalDevicesCount, sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(Vk->Instance, &AvailablePhysicalDevicesCount, AvailablePhysicalDevices);

  BOOL AllRequiredDeviceExtensionsFound = true;
  BOOL SwapChainIsAdequate              = false;
  fornum (AvailableDeviceIdx, AvailablePhysicalDevicesCount) {
    VkPhysicalDevice Device = AvailablePhysicalDevices[AvailableDeviceIdx];

    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(Device, &DeviceProperties);

    UINT32 AvailableExtensionsCount;
    vkEnumerateDeviceExtensionProperties(Device, null, &AvailableExtensionsCount, null);
    VkExtensionProperties *AvailableExtensions = PoolPutArr(Vk->Pool, AvailableExtensionsCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(Device, null, &AvailableExtensionsCount, AvailableExtensions);

    fornum (RequiredDeviceExtensionIdx, RequiredDeviceExtensionsCount) {
      const CHAR *Required = RequiredDeviceExtensions[RequiredDeviceExtensionIdx];
      BOOL Found = false;
      fornum (AvailableExtensionIdx, AvailableExtensionsCount) {
        CHAR *Available = AvailableExtensions[AvailableExtensionIdx].extensionName;
        if (strcmp(Required, Available)) {
          Found = true;
          break;
        }
      }
      if (!Found) {
        AllRequiredDeviceExtensionsFound = false;
        break;
      }
    }

    if (AllRequiredDeviceExtensionsFound) {
      vulkan_swapchain_support_details Details = GetSwapchainSupportDetails(Vk->Pool, Vk->Surface, Device);
      SwapChainIsAdequate = (ArrLen(Details.SurfaceFormats) != 0) && (ArrLen(Details.SurfacePresentModes) != 0);
    }

    vulkan_queue_family_indices QueueFamilyIndices = FindDeviceQueueFamilies(Vk->Pool, Vk->Surface, Device);
    if (QueueFamilyIndices.GraphicsFamilyIsValid && QueueFamilyIndices.PresentFamilyIsValid && AllRequiredDeviceExtensionsFound && SwapChainIsAdequate) {
      Vk->PhysicalDevice = Device;
      break;
    }
  }
  Assert(Vk->PhysicalDevice != VK_NULL_HANDLE);
}
function void CreateLogicalDevice(vulkan_state *Vk, const CHAR* const *RequiredDeviceExtensions, UINT32 RequiredDeviceExtensionsCount) {
  vulkan_queue_family_indices QueueFamilyIndices = FindDeviceQueueFamilies(Vk->Pool, Vk->Surface, Vk->PhysicalDevice);
  UINT32 *UniqueQueueFamilies = null;
  ArrAdd(UniqueQueueFamilies, QueueFamilyIndices.GraphicsFamily);
  if (QueueFamilyIndices.GraphicsFamily != QueueFamilyIndices.PresentFamily) {
    ArrAdd(UniqueQueueFamilies, QueueFamilyIndices.PresentFamily);
  }

  VkDeviceQueueCreateInfo *QueueCreateInfos = null;
  FLOAT QueuePriority = 1.0f;
  fornum (QueueFamilyIdx, ArrLen(UniqueQueueFamilies)) {
    VkDeviceQueueCreateInfo QueueCreateInfo = {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = UniqueQueueFamilies[QueueFamilyIdx],
      .queueCount       = 1,
      .pQueuePriorities = &QueuePriority,
    };
    ArrAdd(QueueCreateInfos, QueueCreateInfo);
  }

  VkPhysicalDeviceFeatures DeviceFeatures = {0};
  VkDeviceCreateInfo DeviceCreateInfo = {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pQueueCreateInfos       = QueueCreateInfos,
    .queueCreateInfoCount    = ArrLen(QueueCreateInfos),
    .pEnabledFeatures        = &DeviceFeatures,
    .enabledExtensionCount   = RequiredDeviceExtensionsCount,
    .ppEnabledExtensionNames = RequiredDeviceExtensions,
  };
  AssertVulkanError(vkCreateDevice(Vk->PhysicalDevice, &DeviceCreateInfo, null, &Vk->LogicalDevice));
  vkGetDeviceQueue(Vk->LogicalDevice, QueueFamilyIndices.GraphicsFamily, 0, &Vk->GraphicsQueue);
  vkGetDeviceQueue(Vk->LogicalDevice, QueueFamilyIndices.PresentFamily, 0, &Vk->PresentQueue);
}
function void CreateSwapchain(vulkan_state *Vk) {
  vulkan_swapchain_support_details SwapchainSupportDetails = GetSwapchainSupportDetails(Vk->Pool, Vk->Surface, Vk->PhysicalDevice);
  VkSurfaceFormatKHR               SurfaceFormat           = ChooseSwapchainSurfaceFormat(SwapchainSupportDetails.SurfaceFormats);
  VkPresentModeKHR                 PresentMode             = ChooseSwapchainPresentMode(SwapchainSupportDetails.SurfacePresentModes);
  VkExtent2D                       SurfaceExtent           = ChooseSwapchainExtent(&SwapchainSupportDetails.SurfaceCapabilites, Vk->WindowHandle);
  UINT32                           ImageCount              = SwapchainSupportDetails.SurfaceCapabilites.minImageCount + 1;
  if (SwapchainSupportDetails.SurfaceCapabilites.maxImageCount > 0 &&
      ImageCount > SwapchainSupportDetails.SurfaceCapabilites.maxImageCount) {
    ImageCount = SwapchainSupportDetails.SurfaceCapabilites.maxImageCount;
  }

  VkSwapchainCreateInfoKHR SwapchainCreateInfo = {
    .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface          = Vk->Surface,
    .minImageCount    = ImageCount,
    .imageFormat      = SurfaceFormat.format,
    .imageColorSpace  = SurfaceFormat.colorSpace,
    .imageExtent      = SurfaceExtent,
    .imageArrayLayers = 1,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .preTransform     = SwapchainSupportDetails.SurfaceCapabilites.currentTransform,
    .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode      = PresentMode,
    .clipped          = true,
    .oldSwapchain     = VK_NULL_HANDLE,
  };
  vulkan_queue_family_indices FoundQueueFamilyIndices = FindDeviceQueueFamilies(Vk->Pool, Vk->Surface, Vk->PhysicalDevice);
  UINT32                      QueueFamilyIndices[]    = {FoundQueueFamilyIndices.GraphicsFamily, FoundQueueFamilyIndices.PresentFamily};
  if (FoundQueueFamilyIndices.GraphicsFamily != FoundQueueFamilyIndices.PresentFamily) {
    SwapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    SwapchainCreateInfo.queueFamilyIndexCount = 2;
    SwapchainCreateInfo.pQueueFamilyIndices   = QueueFamilyIndices;
  }
  else {
    SwapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainCreateInfo.queueFamilyIndexCount = 0;
    SwapchainCreateInfo.pQueueFamilyIndices   = 0;
  }
  AssertVulkanError(vkCreateSwapchainKHR(Vk->LogicalDevice, &SwapchainCreateInfo, null, &Vk->Swapchain));

  vkGetSwapchainImagesKHR(Vk->LogicalDevice, Vk->Swapchain, &ImageCount, null);
  Vk->SwapchainImages = PoolPutArr(Vk->Pool, ImageCount, sizeof(VkImage));
  vkGetSwapchainImagesKHR(Vk->LogicalDevice, Vk->Swapchain, &ImageCount, Vk->SwapchainImages);

  Vk->SwapchainImageFormat = SurfaceFormat.format;
  Vk->SwapchainImageExtent = SurfaceExtent;
}

function void CreateImageViews(vulkan_state *Vk) {
  Vk->SwapchainImageViews = PoolPutArr(Vk->Pool, ArrLen(Vk->SwapchainImages), sizeof(VkImageView));
  fornum (SwapchainImageIdx, ArrLen(Vk->SwapchainImageViews)) {
    VkImageViewCreateInfo ImageViewCreateInfo = {
      .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image                           = Vk->SwapchainImages[SwapchainImageIdx],
      .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
      .format                          = Vk->SwapchainImageFormat,
      .components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };
    AssertVulkanError(vkCreateImageView(Vk->LogicalDevice, &ImageViewCreateInfo, null, &Vk->SwapchainImageViews[SwapchainImageIdx]));
  }
}

function void CreateRenderPass(vulkan_state *Vk) {
  VkAttachmentDescription ColorAttachmentDescription = {
    .format         = Vk->SwapchainImageFormat,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference ColorAttachmentReference = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkSubpassDescription SubpassDescription = {
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .pColorAttachments    = &ColorAttachmentReference,
    .colorAttachmentCount = 1,
  };

  VkSubpassDependency SubpassDependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  VkRenderPassCreateInfo RenderPassCreateinfo = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments    = &ColorAttachmentDescription,
    .subpassCount    = 1,
    .pSubpasses      = &SubpassDescription,
    .dependencyCount = 1,
    .pDependencies   = &SubpassDependency,
  };
  AssertVulkanError(vkCreateRenderPass(Vk->LogicalDevice, &RenderPassCreateinfo, null, &Vk->RenderPass));
}

function VkShaderModule CreateShaderModule(vulkan_state *Vk, str SpirvCode) {
  VkShaderModuleCreateInfo CreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = SpirvCode.Len,
    .pCode = (UINT32*)SpirvCode.Str,
  };
  VkShaderModule ShaderModule;
  AssertVulkanError(vkCreateShaderModule(Vk->LogicalDevice, &CreateInfo, null, &ShaderModule));
  return ShaderModule;
}
function void CreatePipeline(vulkan_state *Vk) {
  str VertShaderSpirv = FileOpen("build/vert.spv");
  str FragShaderSpirv = FileOpen("build/frag.spv");

  VkShaderModule VertShaderModule = CreateShaderModule(Vk, VertShaderSpirv);
  VkShaderModule FragShaderModule = CreateShaderModule(Vk, FragShaderSpirv);

  VkPipelineShaderStageCreateInfo VertShaderStageCreateInfo = {
    .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage  = VK_SHADER_STAGE_VERTEX_BIT,
    .module = VertShaderModule,
    .pName  = "main",
  };
  VkPipelineShaderStageCreateInfo FragShaderStageCreateInfo = {
    .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = FragShaderModule,
    .pName  = "main",
  };
  VkPipelineShaderStageCreateInfo ShaderStages[] = {
    VertShaderStageCreateInfo,
    FragShaderStageCreateInfo
  };

  VkDynamicState DynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = countof(DynamicStates),
    .pDynamicStates    = DynamicStates,
  };

  VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };

  VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport Viewport = {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (FLOAT)Vk->SwapchainImageExtent.width,
    .height   = (FLOAT)Vk->SwapchainImageExtent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  VkRect2D Scissor = {
    .offset = {0, 0},
    .extent = Vk->SwapchainImageExtent,
  };
  VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pViewports    = &Viewport,
    .viewportCount = 1,
    .pScissors     = &Scissor,
    .scissorCount  = 1,
  };

  VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {
    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .lineWidth               = 1.0f,
    .cullMode                = VK_CULL_MODE_BACK_BIT,
    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable         = VK_FALSE,
  };

  VkPipelineMultisampleStateCreateInfo MultisamplingCreateInfo = {
    .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable  = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
  };
  VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY, // Optional
    .attachmentCount = 1,
    .pAttachments = &ColorBlendAttachmentState,
    .blendConstants[0] = 0.0f, // Optional
    .blendConstants[1] = 0.0f, // Optional
    .blendConstants[2] = 0.0f, // Optional
    .blendConstants[3] = 0.0f, // Optional
  };

  VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };
  AssertVulkanError(vkCreatePipelineLayout(Vk->LogicalDevice, &PipelineLayoutCreateInfo, null, &Vk->PipelineLayout));

  VkGraphicsPipelineCreateInfo PipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = ShaderStages,
    .pVertexInputState = &VertexInputStateCreateInfo,
    .pInputAssemblyState = &InputAssemblyCreateInfo,
    .pViewportState = &ViewportStateCreateInfo,
    .pRasterizationState = &RasterizerCreateInfo,
    .pMultisampleState = &MultisamplingCreateInfo,
    .pDepthStencilState = null,
    .pColorBlendState = &ColorBlendStateCreateInfo,
    .pDynamicState = &DynamicStateCreateInfo,
    .layout = Vk->PipelineLayout,
    .renderPass = Vk->RenderPass,
    .subpass = 0,
  };
  AssertVulkanError(vkCreateGraphicsPipelines(Vk->LogicalDevice, null, 1, &PipelineCreateInfo, null, &Vk->Pipeline));

  vkDestroyShaderModule(Vk->LogicalDevice, VertShaderModule, null);
  vkDestroyShaderModule(Vk->LogicalDevice, FragShaderModule, null);
}

function void CreateFrameBuffers(vulkan_state *Vk) {
  Vk->SwapchainFramebuffers = PoolPutArr(Vk->Pool, ArrLen(Vk->SwapchainImageViews), sizeof(VkFramebuffer));
  fornum (ImageViewIdx, ArrLen(Vk->SwapchainImageViews)) {
    VkImageView Attachments[] = {Vk->SwapchainImageViews[ImageViewIdx]};
    VkFramebufferCreateInfo FramebufferCreateInfo = {
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass      = Vk->RenderPass,
      .pAttachments    = Attachments,
      .attachmentCount = 1,
      .width           = Vk->SwapchainImageExtent.width,
      .height          = Vk->SwapchainImageExtent.height,
      .layers          = 1,
    };
    AssertVulkanError(vkCreateFramebuffer(Vk->LogicalDevice, &FramebufferCreateInfo, null, &Vk->SwapchainFramebuffers[ImageViewIdx]));
  }
}

function void CreateCommandPool(vulkan_state *Vk) {
  vulkan_queue_family_indices QueueFamilyIndices = FindDeviceQueueFamilies(Vk->Pool, Vk->Surface, Vk->PhysicalDevice);
  VkCommandPoolCreateInfo CommandPoolCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = QueueFamilyIndices.GraphicsFamily,
  };
  AssertVulkanError(vkCreateCommandPool(Vk->LogicalDevice, &CommandPoolCreateInfo, null, &Vk->CommandPool));
}

function void CreateCommandBuffers(vulkan_state *Vk) {
  Vk->CommandBuffers = PoolPutArr(Vk->Pool, Vk->MaxFramesInFlight, sizeof(VkCommandBuffer));
  VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = Vk->CommandPool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = ArrLen(Vk->CommandBuffers),
  };
  AssertVulkanError(vkAllocateCommandBuffers(Vk->LogicalDevice, &CommandBufferAllocateInfo, Vk->CommandBuffers));
}

function void CreateSynchronizers(vulkan_state *Vk) {
  Vk->ImageIsAvailableSemaphores = PoolPutArr(Vk->Pool, Vk->MaxFramesInFlight, sizeof(VkSemaphore));
  Vk->RenderIsFinishedSemaphores = PoolPutArr(Vk->Pool, Vk->MaxFramesInFlight, sizeof(VkSemaphore));
  Vk->IsInFlightFences           = PoolPutArr(Vk->Pool, Vk->MaxFramesInFlight, sizeof(VkFence));

  VkSemaphoreCreateInfo SemaphoreCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo FenceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  fornum (FrameInFlightIdx, Vk->MaxFramesInFlight) {
    AssertVulkanError(vkCreateSemaphore(Vk->LogicalDevice, &SemaphoreCreateInfo, null, &Vk->ImageIsAvailableSemaphores[FrameInFlightIdx]));
    AssertVulkanError(vkCreateSemaphore(Vk->LogicalDevice, &SemaphoreCreateInfo, null, &Vk->RenderIsFinishedSemaphores[FrameInFlightIdx]));
    AssertVulkanError(vkCreateFence(Vk->LogicalDevice, &FenceCreateInfo, null, &Vk->IsInFlightFences[FrameInFlightIdx]));
  }
}

function void InitializeVulkan(vulkan_state *Vk, pool *Pool, UINT32 MaxFramesInFlight, HWND WindowHandle, HINSTANCE WindowInstance) {
  Vk->Pool               = Pool;
  Vk->MaxFramesInFlight = MaxFramesInFlight;
  Vk->WindowHandle      = WindowHandle;
  Vk->WindowInstance    = WindowInstance;

  CreateInstance(Vk);
  CreateSurface(Vk);
  const CHAR *RequiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  PichPhysicalDevice(Vk, RequiredDeviceExtensions, countof(RequiredDeviceExtensions));
  CreateLogicalDevice(Vk, RequiredDeviceExtensions, countof(RequiredDeviceExtensions));
  CreateSwapchain(Vk);
  CreateImageViews(Vk);
  CreateRenderPass(Vk);
  CreatePipeline(Vk);
  CreateFrameBuffers(Vk);
  CreateCommandPool(Vk);
  CreateCommandBuffers(Vk);
  CreateSynchronizers(Vk);
}

function void ReleaseSwapchain(vulkan_state *Vk) {
  fornum (FrameBufferIdx, ArrLen(Vk->SwapchainFramebuffers)) {
    vkDestroyFramebuffer(Vk->LogicalDevice, Vk->SwapchainFramebuffers[FrameBufferIdx], null);
  }

  fornum (SwapchainImageIdx, ArrLen(Vk->SwapchainImageViews)) {
    vkDestroyImageView(Vk->LogicalDevice, Vk->SwapchainImageViews[SwapchainImageIdx], null);
  }

  vkDestroySwapchainKHR(Vk->LogicalDevice, Vk->Swapchain, null);
}

function void RecreateSwapchain(vulkan_state *Vk) {
  RECT ClientRect;
  GetClientRect(Vk->WindowHandle, &ClientRect);
  INT32 Width  = ClientRect.right - ClientRect.left;
  INT32 Height = ClientRect.bottom - ClientRect.top;

  while (Width == 0 || Height == 0) {
    GetClientRect(Vk->WindowHandle, &ClientRect);
    Width  = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
    WaitMessage();
  }

  vkDeviceWaitIdle(Vk->LogicalDevice);

  ReleaseSwapchain(Vk);
  CreateSwapchain(Vk);
  CreateImageViews(Vk);
  CreateFrameBuffers(Vk);
}

function void RecordCommandBuffer(vulkan_state *Vk, VkCommandBuffer CommandBuffer, UINT32 ImageIdx) {
  VkCommandBufferBeginInfo CommandBufferBeginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
  AssertVulkanError(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo));

  VkClearValue ClearColor = {.color.float32 = {0.0f, 0.0f, 0.0f, 1.0f}};
  VkRenderPassBeginInfo RenderPassBeginInfo = {
    .sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass        = Vk->RenderPass,
    .framebuffer       = Vk->SwapchainFramebuffers[ImageIdx],
    .renderArea.offset = {0, 0},
    .renderArea.extent = Vk->SwapchainImageExtent,
    .clearValueCount   = 1,
    .pClearValues      = &ClearColor,
  };

  vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vk->Pipeline);

  VkViewport Viewport = {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (FLOAT)Vk->SwapchainImageExtent.width,
    .height   = (FLOAT)Vk->SwapchainImageExtent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

  VkRect2D Scissor = {
    .offset = {0, 0},
    .extent = Vk->SwapchainImageExtent,
  };
  vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);            

  vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(CommandBuffer);

  AssertVulkanError(vkEndCommandBuffer(CommandBuffer));
}

function void DrawFrame(vulkan_state *Vk) {
  vkWaitForFences(Vk->LogicalDevice, 1, &Vk->IsInFlightFences[Vk->CurrentFrame], VK_TRUE, UINT64_MAX);

  UINT32 ImageIdx;
  VkResult Result = vkAcquireNextImageKHR(Vk->LogicalDevice, Vk->Swapchain, UINT64_MAX, Vk->ImageIsAvailableSemaphores[Vk->CurrentFrame], VK_NULL_HANDLE, &ImageIdx);
  if (Result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain(Vk);
    return;
  }
  else if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR) {
    Assert(0);
  };

  vkResetFences(Vk->LogicalDevice, 1, &Vk->IsInFlightFences[Vk->CurrentFrame]);

  vkResetCommandBuffer(Vk->CommandBuffers[Vk->CurrentFrame], 0);
  RecordCommandBuffer(Vk, Vk->CommandBuffers[Vk->CurrentFrame], ImageIdx);

  VkSemaphore          WaitSemaphores[]      = {Vk->ImageIsAvailableSemaphores[Vk->CurrentFrame]};
  VkSemaphore          SignalSemaphores[]    = {Vk->RenderIsFinishedSemaphores[Vk->CurrentFrame]};
  VkPipelineStageFlags WaitStages[]          = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo SubmitInfo = {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pWaitSemaphores      = WaitSemaphores,
    .waitSemaphoreCount   = countof(WaitSemaphores),
    .pSignalSemaphores    = SignalSemaphores,
    .signalSemaphoreCount = countof(SignalSemaphores),
    .pWaitDstStageMask    = WaitStages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &Vk->CommandBuffers[Vk->CurrentFrame],
  };
  AssertVulkanError(vkQueueSubmit(Vk->GraphicsQueue, 1, &SubmitInfo, Vk->IsInFlightFences[Vk->CurrentFrame]));

  VkSwapchainKHR Swapchains[] = {Vk->Swapchain};
  VkPresentInfoKHR PresentInfo = {
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,  
    .pWaitSemaphores    = SignalSemaphores,
    .swapchainCount     = 1,
    .pSwapchains        = Swapchains,
    .pImageIndices      = &ImageIdx,
  };

  Result = vkQueuePresentKHR(Vk->PresentQueue, &PresentInfo);
  if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || Vk->Resized) {
    Vk->Resized = false;
    RecreateSwapchain(Vk);
  }
  else if (Result != VK_SUCCESS) {
    Assert(0);
  };

  Vk->CurrentFrame = (Vk->CurrentFrame + 1) % Vk->MaxFramesInFlight;
}

function void ReleaseVulkan(vulkan_state *Vk) {
  vkDeviceWaitIdle(Vk->LogicalDevice);

  ReleaseSwapchain(Vk);

  vkDestroyPipeline(Vk->LogicalDevice, Vk->Pipeline, null);
  vkDestroyPipelineLayout(Vk->LogicalDevice, Vk->PipelineLayout, null);
  vkDestroyRenderPass(Vk->LogicalDevice, Vk->RenderPass, null);
  fornum (FrameInFlightIdx, Vk->MaxFramesInFlight) {
    vkDestroySemaphore(Vk->LogicalDevice, Vk->ImageIsAvailableSemaphores[FrameInFlightIdx], null);
    vkDestroySemaphore(Vk->LogicalDevice, Vk->RenderIsFinishedSemaphores[FrameInFlightIdx], null);
    vkDestroyFence(Vk->LogicalDevice, Vk->IsInFlightFences[FrameInFlightIdx], null);
  }
  vkDestroyCommandPool(Vk->LogicalDevice, Vk->CommandPool, null);
  vkDestroyDevice(Vk->LogicalDevice, null);
  vkDestroySurfaceKHR(Vk->Instance, Vk->Surface, null);
  vkDestroyInstance(Vk->Instance, null);
}

static DWORD WINAPI MainThread(LPVOID Param) {
  HWND      Window   = (HWND)Param;
  HINSTANCE Instance = GetModuleHandleW(null);
  HDC       DevCtx   = GetDC(Window);
  HGLRC     GlCtx    = null;

  vulkan_state Vk = {0};
  InitializeVulkan(&Vk, ReservePool(), 2, Window, Instance);

  ShowWindow(Window, SW_SHOW);

  BOOL Running = true;
  while (Running) {
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {  
      switch(Message.message) {
        case WM_CLOSE:
        case WM_DESTROY:
          Running = false;
          break;
        case WM_SIZING:
          Vk.Resized = true;
      }
    }

    DrawFrame(&Vk);
  }

  ReleaseVulkan(&Vk);

  ReleaseDC(Window, DevCtx);
  DestroyWindow(Window);
  ExitProcess(0);
}

static DWORD MainThreadID = 0;
static LRESULT CALLBACK ActualWndProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
  switch (Message) {
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_ENTERSIZEMOVE:
    case WM_SIZE:
      PostThreadMessageW(MainThreadID, Message, WParam, LParam);
      break;
    default:
      return DefWindowProcW(Window, Message, WParam, LParam);
  }
  return 0;
}

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {
  WNDCLASSEXW ServiceClass = {
    .cbSize        = sizeof(ServiceClass),
    .lpfnWndProc   = DefWindowProcW,
    .hInstance     = Instance,
    .hIcon         = LoadIconA(null, IDI_APPLICATION),
    .hCursor       = LoadCursorA(null, IDC_ARROW),
    .lpszClassName = L"Hidden Dangerous ServiceClass"
  };
  RegisterClassExW(&ServiceClass);
  HWND ServiceWindow = CreateWindowExW(
    0, ServiceClass.lpszClassName, L"Hidden Dangerous Window", 0,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    0, 0, Instance, 0
  );

  WNDCLASSW ActualClass = {
    .style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
    .lpfnWndProc   = ActualWndProc,
    .hInstance     = Instance,
    .hCursor       = LoadCursor(0, IDC_ARROW),
    .lpszClassName = L"Actual Dangerous Class"
  };
  RegisterClassW(&ActualClass);
  HWND ActualWindow = CreateWindowW(
    L"Actual Dangerous Class", L"title", WS_TILEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    0, 0, Instance, 0
  );

  CreateThread(0, 0, MainThread, ActualWindow, 0, &MainThreadID);

  while (true) {
    MSG Message;
    GetMessageW(&Message, 0, 0, 0);
    TranslateMessage(&Message);
    if ((Message.message == WM_CHAR) || (Message.message == WM_KEYDOWN) ||
        (Message.message == WM_QUIT) || (Message.message == WM_SIZE))
      PostThreadMessageW(MainThreadID, Message.message, Message.wParam, Message.lParam);
    else
      DispatchMessageW(&Message);
  }
}