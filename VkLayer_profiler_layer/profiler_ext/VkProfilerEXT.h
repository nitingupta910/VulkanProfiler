#pragma once
#include <vulkan/vulkan.h>

#ifndef VK_EXT_profiler
#define VK_EXT_profiler 1
#define VK_EXT_PROFILER_SPEC_VERSION 1
#define VK_EXT_PROFILER_EXTENSION_NAME "VK_EXT_profiler"

enum VkProfilerStructureTypeEXT
{
    VK_STRUCTURE_TYPE_PROFILER_CREATE_INFO_EXT = 1000999000,
    VK_PROFILER_STRUCTURE_TYPE_MAX_ENUM_EXT = 0x7FFFFFFF
};

enum VkProfilerCreateFlagBitsEXT
{
    VK_PROFILER_CREATE_DISABLE_OVERLAY_BIT_EXT = 1,
    VK_PROFILER_CREATE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
};

typedef VkFlags VkProfilerCreateFlagsEXT;

enum VkProfilerModeEXT
{
    VK_PROFILER_MODE_PER_DRAWCALL_EXT,
    VK_PROFILER_MODE_PER_PIPELINE_EXT,
    VK_PROFILER_MODE_PER_RENDER_PASS_EXT,
    VK_PROFILER_MODE_PER_COMMAND_BUFFER_EXT,
    VK_PROFILER_MODE_PER_SUBMIT_EXT,
    VK_PROFILER_MODE_PER_FRAME_EXT,
    VK_PROFILER_MODE_MAX_ENUM_EXT = 0x7FFFFFFF
};

enum VkProfilerRegionTypeEXT
{
    VK_PROFILER_REGION_TYPE_FRAME_EXT,
    VK_PROFILER_REGION_TYPE_SUBMIT_EXT,
    VK_PROFILER_REGION_TYPE_COMMAND_BUFFER_EXT,
    VK_PROFILER_REGION_TYPE_DEBUG_MARKER_EXT,
    VK_PROFILER_REGION_TYPE_RENDER_PASS_EXT,
    VK_PROFILER_REGION_TYPE_PIPELINE_EXT,
    VK_PROFILER_REGION_TYPE_MAX_ENUM_EXT = 0x7FFFFFFF
};

enum VkProfilerSyncModeEXT
{
    VK_PROFILER_SYNC_MODE_PRESENT_EXT,
    VK_PROFILER_SYNC_MODE_SUBMIT_EXT,
    VK_PROFILER_SYNC_MODE_MAX_ENUM_EXT = 0x7FFFFFFF
};

enum VkProfilerPerformanceCounterUnitEXT
{
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_GENERIC_EXT = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_PERCENTAGE_EXT = VK_PERFORMANCE_COUNTER_UNIT_PERCENTAGE_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_NANOSECONDS_EXT = VK_PERFORMANCE_COUNTER_UNIT_NANOSECONDS_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_BYTES_EXT = VK_PERFORMANCE_COUNTER_UNIT_BYTES_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_BYTES_PER_SECOND_EXT = VK_PERFORMANCE_COUNTER_UNIT_BYTES_PER_SECOND_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_KELVIN_EXT = VK_PERFORMANCE_COUNTER_UNIT_KELVIN_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_WATTS_EXT = VK_PERFORMANCE_COUNTER_UNIT_WATTS_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_VOLTS_EXT = VK_PERFORMANCE_COUNTER_UNIT_VOLTS_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_AMPS_EXT = VK_PERFORMANCE_COUNTER_UNIT_AMPS_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_HERTZ_EXT = VK_PERFORMANCE_COUNTER_UNIT_HERTZ_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_CYCLES_EXT = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_UNIT_MAX_ENUM_EXT = VK_PERFORMANCE_COUNTER_UNIT_MAX_ENUM_KHR
};

enum VkProfilerPerformanceCounterStorageEXT
{
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_INT32_EXT = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_INT64_EXT = VK_PERFORMANCE_COUNTER_STORAGE_INT64_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_UINT32_EXT = VK_PERFORMANCE_COUNTER_STORAGE_UINT32_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_UINT64_EXT = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_FLOAT32_EXT = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT32_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_FLOAT64_EXT = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR,
    VK_PROFILER_PERFORMANCE_COUNTER_STORAGE_MAX_ENUM_EXT = VK_PERFORMANCE_COUNTER_STORAGE_MAX_ENUM_KHR
};

typedef struct VkProfilerCreateInfoEXT
{
    VkProfilerStructureTypeEXT sType;
    const void* pNext;
    VkProfilerCreateFlagsEXT flags;
} VkProfilerCreateInfoEXT;

typedef struct VkProfilerRegionDataEXT
{
    VkProfilerRegionTypeEXT regionType;
    char regionName[ 256 ];
    float duration;
} VkProfilerRegionDataEXT;

typedef struct VkProfilerMemoryDataEXT
{
    uint64_t deviceLocalMemoryAllocated;
} VkProfilerMemoryDataEXT;

typedef struct VkProfilerDataEXT
{
    VkProfilerRegionDataEXT frame;
    VkProfilerMemoryDataEXT memory;
} VkProfilerDataEXT;

typedef struct VkProfilerPerformanceCounterPropertiesEXT
{
    char shortName[ 64 ];
    char description[ 256 ];
    VkProfilerPerformanceCounterUnitEXT unit;
    VkProfilerPerformanceCounterStorageEXT storage;
} VkProfilerPerformanceCounterPropertiesEXT;

typedef VkPerformanceCounterResultKHR VkProfilerPerformanceCounterResultEXT;

typedef VKAPI_ATTR VkResult( VKAPI_CALL* PFN_vkSetProfilerModeEXT )(VkDevice, VkProfilerModeEXT);
typedef VKAPI_ATTR VkResult( VKAPI_CALL* PFN_vkSetProfilerSyncModeEXT )(VkDevice, VkProfilerSyncModeEXT);
typedef VKAPI_ATTR VkResult( VKAPI_CALL* PFN_vkGetProfilerFrameDataEXT )(VkDevice, VkProfilerRegionDataEXT*);
typedef VKAPI_ATTR VkResult( VKAPI_CALL* PFN_vkGetProfilerCommandBufferDataEXT )(VkDevice, VkCommandBuffer, VkProfilerRegionDataEXT*);
typedef VKAPI_ATTR VkResult( VKAPI_CALL* PFN_vkEnumerateProfilerMetricPropertiesEXT )(VkDevice, uint32_t*, VkProfilerPerformanceCounterPropertiesEXT*);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkSetProfilerModeEXT(
    VkDevice device,
    VkProfilerModeEXT mode );

VKAPI_ATTR VkResult VKAPI_CALL vkSetProfilerSyncModeEXT(
    VkDevice device,
    VkProfilerSyncModeEXT syncMode );

VKAPI_ATTR VkResult VKAPI_CALL vkGetProfilerFrameDataEXT(
    VkDevice device,
    VkProfilerRegionDataEXT* pData );

VKAPI_ATTR VkResult VKAPI_CALL vkGetProfilerCommandBufferDataEXT(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    VkProfilerRegionDataEXT* pData );

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateProfilerPerformanceCounterPropertiesEXT(
    VkDevice device,
    uint32_t* pProfilerMetricCount,
    VkProfilerPerformanceCounterPropertiesEXT* pProfilerMetricProperties );
#endif // VK_NO_PROTOTYPES
#endif // VK_EXT_profiler
