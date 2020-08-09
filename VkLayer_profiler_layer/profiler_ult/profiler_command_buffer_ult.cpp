#include "profiler/profiler.h"

// Undefine names conflicting in gtest
#undef None
#undef Bool

#include <gtest/gtest.h>

#include "profiler_vulkan_state.h"
#include "profiler_vulkan_simple_triangle.h"

namespace Profiler
{
    class ProfilerCommandBufferULT : public testing::Test
    {
    protected:
        VulkanState* Vk;

        VkLayerDispatchTable DT;
        VkLayerInstanceDispatchTable IDT;

        DeviceProfiler* Prof;

        void SetUp() override
        {
            Vk = new VulkanState;
            DT = Vk->GetLayerDispatchTable();
            IDT = Vk->GetLayerInstanceDispatchTable();

            auto& dd = VkDevice_Functions::DeviceDispatch.Get( Vk->Device );
            Prof = &dd.Profiler;
        }

        void TearDown() override
        {
            delete Vk;
        }
    };

    TEST_F( ProfilerCommandBufferULT, AllocateCommandBuffer )
    {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.commandPool = Vk->CommandPool;
        ASSERT_EQ( VK_SUCCESS, DT.AllocateCommandBuffers( Vk->Device, &allocateInfo, &commandBuffer ) );

        ASSERT_EQ( 1, Prof->m_CommandBuffers.size() );
        const auto it = Prof->m_CommandBuffers.cbegin();
        EXPECT_EQ( commandBuffer, it->first );
        EXPECT_EQ( commandBuffer, it->second.GetCommandBuffer() );
        EXPECT_EQ( Vk->CommandPool, it->second.GetCommandPool() );
    }

    TEST_F( ProfilerCommandBufferULT, ProfileSecondaryCommandBuffer )
    {
        // Create simple triangle app
        VulkanSimpleTriangle simpleTriangle( Vk, IDT, DT );
        VkCommandBuffer commandBuffers[ 2 ] = {};

        { // Allocate command buffers
            VkCommandBufferAllocateInfo allocateInfo = {};
            allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;
            allocateInfo.commandPool = Vk->CommandPool;
            ASSERT_EQ( VK_SUCCESS, DT.AllocateCommandBuffers( Vk->Device, &allocateInfo, &commandBuffers[ 0 ] ) );
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            ASSERT_EQ( VK_SUCCESS, DT.AllocateCommandBuffers( Vk->Device, &allocateInfo, &commandBuffers[ 1 ] ) );
        }
        { // Begin secondary command buffer
            VkCommandBufferInheritanceInfo inheritanceInfo = {};
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = simpleTriangle.RenderPass;
            inheritanceInfo.subpass = 0;
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            ASSERT_EQ( VK_SUCCESS, DT.BeginCommandBuffer( commandBuffers[ 1 ], &beginInfo ) );
        }
        { // Record commands
            DT.CmdBindPipeline( commandBuffers[ 1 ], VK_PIPELINE_BIND_POINT_GRAPHICS, simpleTriangle.Pipeline );
            DT.CmdDraw( commandBuffers[ 1 ], 3, 1, 0, 0 );
        }
        { // End secondary command buffer
            DT.EndCommandBuffer( commandBuffers[ 1 ] );
        }
        { // Begin primary command buffer
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            ASSERT_EQ( VK_SUCCESS, DT.BeginCommandBuffer( commandBuffers[ 0 ], &beginInfo ) );
        }
        { // Image layout transitions
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = Vk->QueueFamilyIndex;
            barrier.dstQueueFamilyIndex = Vk->QueueFamilyIndex;
            barrier.image = simpleTriangle.FramebufferImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

            DT.CmdPipelineBarrier( commandBuffers[ 0 ],
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_DEPENDENCY_BY_REGION_BIT,
                0, nullptr,
                0, nullptr,
                1, &barrier );
        }
        { // Begin render pass
            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = simpleTriangle.RenderPass;
            beginInfo.renderArea = simpleTriangle.RenderArea;
            beginInfo.framebuffer = simpleTriangle.Framebuffer;
            DT.CmdBeginRenderPass( commandBuffers[ 0 ], &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS );
        }
        { // Submit secondary command buffer
            DT.CmdExecuteCommands( commandBuffers[ 0 ], 1, &commandBuffers[ 1 ] );
        }
        { // End render pass
            DT.CmdEndRenderPass( commandBuffers[ 0 ] );
        }
        { // End primary command buffer
            DT.EndCommandBuffer( commandBuffers[ 0 ] );
        }
        { // Submit primary command buffer
            VkSubmitInfo submitInfo = {};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[ 0 ];
            ASSERT_EQ( VK_SUCCESS, DT.QueueSubmit( Vk->Queue, 1, &submitInfo, VK_NULL_HANDLE ) );
        }
        { // Collect data
            Prof->Present( {}, {} );

            const auto& data = Prof->GetData();
            ASSERT_EQ( 1, data.m_Submits.size() );
            ASSERT_EQ( 1, data.m_Submits[ 0 ].m_CommandBuffers.size() );

            const auto& cmdBufferData = data.m_Submits[ 0 ].m_CommandBuffers[ 0 ];
            EXPECT_EQ( commandBuffers[ 0 ], cmdBufferData.m_Handle );
            EXPECT_FALSE( cmdBufferData.m_Subregions.empty() );

            const auto& renderPassData = cmdBufferData.m_Subregions[ 1 ];
            EXPECT_EQ( simpleTriangle.RenderPass, renderPassData.m_Handle );
            EXPECT_FALSE( renderPassData.m_Subregions.empty() );

            const auto& subpassData = renderPassData.m_Subregions[ 1 ];
            EXPECT_EQ( 0, subpassData.m_Index );
            EXPECT_EQ( VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, subpassData.m_Contents );
            EXPECT_EQ( 0, subpassData.m_Pipelines.size() );
            EXPECT_FALSE( subpassData.m_SecondaryCommandBuffers.empty() );

            const auto& secondaryCmdBufferData = subpassData.m_SecondaryCommandBuffers[ 0 ];
            EXPECT_EQ( commandBuffers[ 1 ], secondaryCmdBufferData.m_Handle );
            EXPECT_FALSE( secondaryCmdBufferData.m_Subregions.empty() );

            const auto& inheritedRenderPassData = secondaryCmdBufferData.m_Subregions[ 0 ];
            EXPECT_EQ( VK_NULL_HANDLE, inheritedRenderPassData.m_Handle );
            EXPECT_FALSE( inheritedRenderPassData.m_Subregions.empty() );

            const auto& inheritedSubpassData = inheritedRenderPassData.m_Subregions[ 0 ];
            EXPECT_EQ( -1, inheritedSubpassData.m_Index );
            EXPECT_EQ( VK_SUBPASS_CONTENTS_INLINE, inheritedSubpassData.m_Contents );
            EXPECT_EQ( 0, inheritedSubpassData.m_SecondaryCommandBuffers.size() );
            EXPECT_FALSE( inheritedSubpassData.m_Pipelines.empty() );

            const auto& pipelineData = inheritedSubpassData.m_Pipelines.at( 1 );
            EXPECT_EQ( simpleTriangle.Pipeline, pipelineData.m_Handle );
            EXPECT_EQ( VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineData.m_BindPoint );
            EXPECT_FALSE( pipelineData.m_Subregions.empty() );

            const auto& drawcallData = pipelineData.m_Subregions.at( 0 );
            EXPECT_EQ( ProfilerDrawcallType::eDraw, drawcallData.m_Type );
            EXPECT_LT( 0, drawcallData.m_Ticks );

            // Validate that drawcall data propagates to pipeline stats
            EXPECT_EQ( drawcallData.m_Ticks, pipelineData.m_Stats.m_TotalTicks );
            EXPECT_EQ( 1, pipelineData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( 1, pipelineData.m_Stats.m_TotalDrawcallCount );

            // Validate that pipeline stats propagate to subpass stats
            EXPECT_EQ( pipelineData.m_Stats.m_TotalTicks, inheritedSubpassData.m_Stats.m_TotalTicks );
            EXPECT_EQ( pipelineData.m_Stats.m_TotalDrawCount, inheritedSubpassData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( pipelineData.m_Stats.m_TotalDrawcallCount, inheritedSubpassData.m_Stats.m_TotalDrawcallCount );

            // Validate that subpass stats propagate to renderpass stats
            EXPECT_EQ( inheritedSubpassData.m_Stats.m_TotalTicks, inheritedRenderPassData.m_Stats.m_TotalTicks );
            EXPECT_EQ( inheritedSubpassData.m_Stats.m_TotalDrawCount, inheritedRenderPassData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( inheritedSubpassData.m_Stats.m_TotalDrawcallCount, inheritedRenderPassData.m_Stats.m_TotalDrawcallCount );

            // Validate that renderpass stats propagate to secondary command buffer stats
            EXPECT_EQ( inheritedRenderPassData.m_Stats.m_TotalTicks, secondaryCmdBufferData.m_Stats.m_TotalTicks );
            EXPECT_EQ( inheritedRenderPassData.m_Stats.m_TotalDrawCount, secondaryCmdBufferData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( inheritedRenderPassData.m_Stats.m_TotalDrawcallCount, secondaryCmdBufferData.m_Stats.m_TotalDrawcallCount );

            // Validate that secondary command buffer stats propagate to subpass stats
            EXPECT_EQ( secondaryCmdBufferData.m_Stats.m_TotalTicks, subpassData.m_Stats.m_TotalTicks );
            EXPECT_EQ( secondaryCmdBufferData.m_Stats.m_TotalDrawCount, subpassData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( secondaryCmdBufferData.m_Stats.m_TotalDrawcallCount, subpassData.m_Stats.m_TotalDrawcallCount );

            // Validate that subpass stats propagate to renderpass stats
            // Render pass data includes begin/end ops, so expected time is greater than subpass time
            EXPECT_LE( subpassData.m_Stats.m_TotalTicks, renderPassData.m_Stats.m_TotalTicks );
            EXPECT_LE( subpassData.m_Stats.m_TotalDrawCount, renderPassData.m_Stats.m_TotalDrawCount );
            EXPECT_LE( subpassData.m_Stats.m_TotalDrawcallCount, renderPassData.m_Stats.m_TotalDrawcallCount );

            // Validate that renderpass stats propagate to primary command buffer stats
            EXPECT_EQ( renderPassData.m_Stats.m_TotalTicks, cmdBufferData.m_Stats.m_TotalTicks );
            EXPECT_EQ( renderPassData.m_Stats.m_TotalDrawCount, cmdBufferData.m_Stats.m_TotalDrawCount );
            EXPECT_EQ( renderPassData.m_Stats.m_TotalDrawcallCount, cmdBufferData.m_Stats.m_TotalDrawcallCount );
        }
    }
}
