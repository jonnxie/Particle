//
// Created by maybe on 2021/6/5.
//
#include "precompiledhead.h"

#include "inputtype.h"
#include "tiny_gltf.h"
#include "VulkanglTFModels.h"
#include "Engine/Item/shatter_item.h"

VkVertexInputBindingDescription getBindingDescription(Input_Type _type){
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    switch (_type) {
        case Input_Type::Point3d:{
            bindingDescription.stride = sizeof(Point3d);
            break;
        }
        case Input_Type::Point2d:{
            bindingDescription.stride = sizeof(Point2d);
            break;
        }
        case Input_Type::Point2dColor:{
            bindingDescription.stride = sizeof(Point_UNDER_PC);
            break;
        }
        case Input_Type::Point3dNormal:{
            bindingDescription.stride = sizeof(Point3d_Normal);
            break;
        }
        case Input_Type::Point3dColor:{
            bindingDescription.stride = sizeof(Point_PC);
            break;
        }
        case Input_Type::Point3dColorUV:{
            bindingDescription.stride = sizeof(Point3d_Color_UV);
            break;
        }
        case Input_Type::Point3dUV:{
            bindingDescription.stride = sizeof(Point_UV);
            break;
        }
        case Input_Type::Point2dUV3d:{
            bindingDescription.stride = sizeof(Point2dUV3d);
            break;
        }
        case Input_Type::Point3dUV1d:{
            bindingDescription.stride = sizeof(Point3dUV1d);
            break;
        }
        case Input_Type::Point3dNormalUV:{
            bindingDescription.stride = sizeof(GltfPoint);
            break;
        }
        case Input_Type::Point3dColorSize:{
            bindingDescription.stride = Point3dColorSizeSize;
            break;
        }
        case Input_Type::FrustumCullSample:{
            bindingDescription.stride = sizeof(vkglTF::Vertex);
            break;
        }
        case Input_Type::FrustumCullSampleInstance:{
            bindingDescription.binding = 1;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            bindingDescription.stride = sizeof(InstanceData);
            break;
        }
        case Input_Type::Basic:{
            bindingDescription.stride = sizeof(vkglTF::Vertex);
            break;
        }
        case Input_Type::TransparentGlass:{
            auto ptr = vkglTF::Vertex::getPipelineVertexInputState({vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Color, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV});
            bindingDescription = *(ptr->pVertexBindingDescriptions);
            break;
        }
        case Input_Type::Building:{
            auto ptr = vkglTF::Vertex::getPipelineVertexInputState({vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Color, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV});
            bindingDescription = *(ptr->pVertexBindingDescriptions);
            break;
        }
        case Input_Type::NONE:{
            break;
        }
    }
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(Input_Type _type){
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    switch (_type) {
        case Input_Type::Point3d:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3d, pos)});
            break;
        }
        case Input_Type::Point2d:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Point2d, pos)});
            break;
        }
        case Input_Type::Point2dColor:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Point_UNDER_PC, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point_UNDER_PC, color)});
            break;
        }
        case Input_Type::Point3dNormal:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3d_Normal, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3d_Normal, nor)});
            break;
        }
        case Input_Type::Point3dColor:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point_PC, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point_PC, color)});
            break;
        }
        case Input_Type::Point3dColorUV:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3d_Color_UV, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3d_Color_UV, color)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{2,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Point3d_Color_UV, uv)});
            break;
        }
        case Input_Type::Point3dUV:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point_UV, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Point_UV, uv)});
            break;
        }
        case Input_Type::Point2dUV3d:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Point2dUV3d, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point2dUV3d, uv)});
            break;
        }
        case Input_Type::Point3dUV1d:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3dUV1d, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32_SFLOAT,offsetof(Point3dUV1d, coordinate)});
            break;
        }
        case Input_Type::Point3dNormalUV:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GltfPoint, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GltfPoint, normal)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{2,0,VK_FORMAT_R32G32_SFLOAT,   offsetof(GltfPoint, uv)});
            break;
        }
        case Input_Type::Point3dColorSize:{
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3dColorSize, pos)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Point3dColorSize, color)});
            attributeDescriptions.push_back(    VkVertexInputAttributeDescription{2,0,VK_FORMAT_R32_SFLOAT,   offsetof(Point3dColorSize, size)});
            break;
        }
        case Input_Type::FrustumCullSample:{
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, pos)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, normal)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, color)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, pos)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(1, 5, VK_FORMAT_R32_SFLOAT, offsetof(InstanceData, scale)));
            break;
        }
        case Input_Type::Basic:{
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, pos)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkglTF::Vertex, normal)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(vkglTF::Vertex, uv)));
            attributeDescriptions.push_back(tool::vertexInputAttributeDescription(0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vkglTF::Vertex, color)));
            break;
        }
        case Input_Type::Building:
        case Input_Type::TransparentGlass:{
            auto ptr = vkglTF::Vertex::getPipelineVertexInputState({vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Color, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV});
            attributeDescriptions.resize(ptr->vertexAttributeDescriptionCount);
            for(size_t index = 0; index < ptr->vertexAttributeDescriptionCount; index++)
            {
                attributeDescriptions[index] = ptr->pVertexAttributeDescriptions[index];
            }
            break;
        }
        case Input_Type::NONE:{
            break;
        }
    }
    return attributeDescriptions;
}