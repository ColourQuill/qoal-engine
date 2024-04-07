#pragma once

#include <fstream>
#include <vector>
#include <string>

namespace vkr {
    class Pipeline {
        public:
            Pipeline(Device& d, Swapchain& s, VkPrimitiveTopology t, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, uint32_t typeIndex = 0) : device{d}, swapchain{s} {
                CreatePipelineLayout();
                auto configInfo = DefaultPipelineConfig(t);
                CreatePipeline(typeIndex, configInfo, vertexShaderPath, fragmentShaderPath);
            }

            ~Pipeline() {
                vkDestroyPipeline(device.GetDevice(), pipeline, nullptr);
                vkDestroyPipelineLayout(device.GetDevice(), pipelineLayout, nullptr);
            }

            struct PipelineConfigInfo {
                VkPipelineViewportStateCreateInfo viewportInfo;
                VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
                VkPipelineRasterizationStateCreateInfo rasterizationInfo;
                VkPipelineMultisampleStateCreateInfo multisampleInfo;
                VkPipelineColorBlendAttachmentState colorBlendAttachment;
                VkPipelineColorBlendStateCreateInfo colorBlendInfo;
                VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
                std::vector<VkDynamicState> dynamicStateEnables;
                VkPipelineDynamicStateCreateInfo dynamicStateInfo;
                VkPipelineLayout pipelineLayout = nullptr;
                VkRenderPass renderPass = nullptr;
                uint32_t subpass = 0;
            };

            PipelineConfigInfo DefaultPipelineConfig(VkPrimitiveTopology topology) {
                PipelineConfigInfo configInfo{};

                configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                configInfo.inputAssemblyInfo.topology = topology;
                configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

                configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                configInfo.viewportInfo.viewportCount = 1;
                configInfo.viewportInfo.pViewports = nullptr;
                configInfo.viewportInfo.scissorCount = 1;
                configInfo.viewportInfo.pScissors = nullptr;

                configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
                configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
                configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
                configInfo.rasterizationInfo.lineWidth = 1.0f;
                configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
                configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
                configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
                configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
                configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
                configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

                configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
                configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                configInfo.multisampleInfo.minSampleShading = 1.0f;
                configInfo.multisampleInfo.pSampleMask = nullptr;
                configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
                configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

                configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
                configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

                configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
                configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
                configInfo.colorBlendInfo.attachmentCount = 1;
                configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
                configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
                configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
                configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
                configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

                configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
                configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
                configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
                configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
                configInfo.depthStencilInfo.minDepthBounds = 0.0f;
                configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
                configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
                configInfo.depthStencilInfo.front = {};
                configInfo.depthStencilInfo.back = {};

                configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
                configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
                configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
                configInfo.dynamicStateInfo.flags = 0;

                return configInfo;
            }

            void CreatePipeline(uint32_t typeIndex, PipelineConfigInfo& configInfo, const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
                auto vertexShaderCode = ReadFile(vertexShaderPath);
                auto fragmentShaderCode = ReadFile(fragmentShaderPath);

                VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
                VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);

                VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vertexShaderModule;
                vertShaderStageInfo.pName = "main";
                VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = fragmentShaderModule;
                fragShaderStageInfo.pName = "main";
                VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

                std::vector<VkVertexInputBindingDescription> bindingDescriptions;
                std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
                if (typeIndex == 0) {
                    bindingDescriptions = Vertex3D::GetBindingDescriptions();
                    attributeDescriptions = Vertex3D::GetAttributeDescributions();
                }
                else if (typeIndex == 1) {
                    bindingDescriptions = Vertex2D::GetBindingDescriptions();
                    attributeDescriptions = Vertex2D::GetAttributeDescributions();
                }
                else {
                    throw std::runtime_error("Invalid pipeline type index. (0 = 3D, 1 = 2D)");
                }

                VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
                vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
                vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

                VkGraphicsPipelineCreateInfo pipelineInfo{};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount = 2;
                pipelineInfo.pStages = shaderStages;

                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
                pipelineInfo.pViewportState = &configInfo.viewportInfo;
                pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
                pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
                pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
                pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
                pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

                pipelineInfo.layout = pipelineLayout;
                pipelineInfo.renderPass = swapchain.GetRenderPass();
                pipelineInfo.subpass = configInfo.subpass;

                pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
                pipelineInfo.basePipelineIndex = -1;

                if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create pipeline.");
                }

                vkDestroyShaderModule(device.GetDevice(), vertexShaderModule, nullptr);
                vkDestroyShaderModule(device.GetDevice(), fragmentShaderModule, nullptr);
            }

            void CreatePipelineLayout() {
                VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 0;
                pipelineLayoutInfo.pSetLayouts = nullptr;
                pipelineLayoutInfo.pushConstantRangeCount = 0;
                pipelineLayoutInfo.pPushConstantRanges = nullptr;

                vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
            }

            std::vector<char> ReadFile(const std::string& filename) {
                std::ifstream file(filename, std::ios::ate | std::ios::binary);

                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open file.");
                }

                size_t fileSize = (size_t) file.tellg();
                std::vector<char> buffer(fileSize);

                file.seekg(0);
                file.read(buffer.data(), fileSize);

                file.close();
                
                return buffer;
            }

            VkShaderModule CreateShaderModule(const std::vector<char>& code) {
                VkShaderModuleCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = code.size();
                createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

                VkShaderModule shaderModule;
                if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create shader module.");
                }

                return shaderModule;
            }

            VkPipeline& GetPipeline() {
                return pipeline;
            }

            VkPipelineLayout& GetPipelineLayout() {
                return pipelineLayout;
            }

        private:
            Device& device;
            Swapchain& swapchain;

            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
    };
}