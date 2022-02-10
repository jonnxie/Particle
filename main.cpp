#include<iostream>
#include <vector>
#include <thread>
#include "tiny_gltf.h"

#include "Shatter_Render/shatter_render_include.h"
#include "Shatter_App/shatterappinclude.h"
#include "Shatter_Item/shatter_item.h"

#include "Shatter_Object/slbpool.h"
#include "Shatter_Object/shaderpool.h"
#include "Shatter_Object/setpool.h"
#include "Shatter_Object/ppool.h"
#include "Shatter_Object/bpool.h"
#include "Shatter_Object/mpool.h"
#include "Shatter_Object/line3d.h"
#include "Shatter_Object/threadpool.h"
#include "Shatter_Object/modelsetpool.h"
#include "Shatter_Object/camera.h"
#include "Shatter_Object/taskpool.h"
#include "Shatter_Object/VulkanglTFModels.h"
#include "Shatter_Object/shadergrouppool.h"
#include "Shatter_Object/offscreen.h"
#include "Shatter_Object/inputaction.h"
#include "Shatter_Object/listener.h"
#include "Shatter_Object/LightManager.h"
#include "Shatter_Base/Skybox.h"
#include "Shatter_Base/PBRBasic.h"
#include "Shatter_Base/basic.h"
#include "Shatter_Base/tbasic.h"
#include "Shatter_Base/gcoor.h"
#include "Shatter_Base/lines.h"
#include "Shatter_Base/tris.h"
#include "Shatter_Item/shatter_macro.h"
#include "Shatter_Buffer/shatterbufferinclude.h"

#include "Shatter_Mesh/line.h"
#include "Shatter_Mesh/plane.h"
#include "Shatter_Animation/animation.h"

void initSet()
{
    TaskPool::pushTask("UpdateOffScreenSets",[](){
        std::array<VkWriteDescriptorSet,5> mc_write = {};

        mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[0].pNext = VK_NULL_HANDLE;
        mc_write[0].dstSet =  SetPool::getPool()["OffScreen"];
        mc_write[0].dstBinding = 0;
        mc_write[0].dstArrayElement = 0;
        mc_write[0].descriptorCount = 1;
        mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mc_write[0].pImageInfo = &SingleOffScreen.m_color_descriptor;
        mc_write[0].pBufferInfo = VK_NULL_HANDLE;

        mc_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[1].pNext = VK_NULL_HANDLE;
        mc_write[1].dstSet =  SetPool::getPool()["VolumeRendering"];
        mc_write[1].dstBinding = 0;
        mc_write[1].dstArrayElement = 0;
        mc_write[1].descriptorCount = 1;
        mc_write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mc_write[1].pImageInfo = &SingleOffScreen.m_color_descriptor;
        mc_write[1].pBufferInfo = VK_NULL_HANDLE;

        VkDescriptorImageInfo volumeInfo;
        volumeInfo.sampler = SingleBPool.getTexture("VolumeTexture")->getSampler();
        volumeInfo.imageView = SingleBPool.getTexture("VolumeTexture")->getImageView();
        volumeInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        mc_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[2].pNext = VK_NULL_HANDLE;
        mc_write[2].dstSet =  SetPool::getPool()["VolumeRendering"];
        mc_write[2].dstBinding = 1;
        mc_write[2].dstArrayElement = 0;
        mc_write[2].descriptorCount = 1;
        mc_write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mc_write[2].pImageInfo = &volumeInfo;
        mc_write[2].pBufferInfo = VK_NULL_HANDLE;

        VkDescriptorImageInfo transInfo;
        transInfo.sampler = SingleBPool.getTexture("TransFer")->getSampler();
        transInfo.imageView = SingleBPool.getTexture("TransFer")->getImageView();
        transInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mc_write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[3].pNext = VK_NULL_HANDLE;
        mc_write[3].dstSet =  SetPool::getPool()["VolumeRendering"];
        mc_write[3].dstBinding = 2;
        mc_write[3].dstArrayElement = 0;
        mc_write[3].descriptorCount = 1;
        mc_write[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mc_write[3].pImageInfo = &transInfo;
        mc_write[3].pBufferInfo = VK_NULL_HANDLE;

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = (*SingleBPool.getBuffer("VolumeInfo",Buffer_Type::Uniform_Buffer))();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VolumeInfo);
        mc_write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[4].pNext = VK_NULL_HANDLE;
        mc_write[4].dstSet =  SetPool::getPool()["VolumeRendering"];
        mc_write[4].dstBinding = 3;
        mc_write[4].dstArrayElement = 0;
        mc_write[4].descriptorCount = 1;
        mc_write[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mc_write[4].pImageInfo = VK_NULL_HANDLE;
        mc_write[4].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(Device::getDevice()(),
                               5,
                               mc_write.data(),
                               0,
                               nullptr);
    });
}

void initTransparentSet()
{
    TaskPool::pushTask("UpdateGSet",[](){
        std::vector< VkDescriptorImageInfo> descriptorImageInfos = {
                tool::descriptorImageInfo(VK_NULL_HANDLE, SingleRender.positionAttachment->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                tool::descriptorImageInfo(VK_NULL_HANDLE, SingleRender.normalAttachment->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                tool::descriptorImageInfo(VK_NULL_HANDLE, SingleRender.albedoAttachment->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        };
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        for (size_t i = 0; i < descriptorImageInfos.size(); i++) {
            writeDescriptorSets.push_back(tool::writeDescriptorSet(SingleSetPool["gBuffer"], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, i, &descriptorImageInfos[i]));
        }
        vkUpdateDescriptorSets(SingleDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    });
}

int main() {
    shatter::app::ShatterApp& app = shatter::app::ShatterApp::getApp();
    /*
     * render
     */
    shatter::render::ShatterRender& render = shatter::render::ShatterRender::getRender();

    auto& slb_pool = SingleSLBPool;
    auto& shader_pool = SingleShaderPool;
    auto& shader_group_pool = SingleShaderGroupPool;
    auto& set_pool = SingleSetPool;
    auto& pipeline_pool = SinglePPool;
    auto& buffer_pool = SingleBPool;
    auto& model_pool = SingleModelSetPool;
    auto& light_manager = SingleLM;

    auto line_pool = MPool<Line3d>::getPool();
    auto& camera = Camera::getCamera();
    auto thread_pool = ThreadPool::pool();
    initTransparentSet();

    std::vector<int> coor_line;
    {
        line_pool->malloc(3,coor_line);
        auto line = (*line_pool)[coor_line[0]];
        line->setLine(glm::vec3(0,0,0),glm::vec3(1,0,0));
        line = (*line_pool)[coor_line[1]];
        line->setLine(glm::vec3(0,0,0),glm::vec3(0,1,0));
        line = (*line_pool)[coor_line[2]];
        line->setLine(glm::vec3(0,0,0),glm::vec3(0,0,1));
    }
    auto coordinate = GCoor::createGCoor(coor_line);
    app.getNObjects()->push_back(coordinate->m_dobjs[0]);

    {
//        auto gline = new GLine({0,0,0},{5,5,5});
//        gline->draw();
//        Tri t = {
//                Point{glm::vec3(0.0f,0.0f,0.0f),
//                      PURPLE_COLOR},
//                Point {glm::vec3(1.0f,0.0f,0.0f),
//                       RED_COLOR},
//                Point{glm::vec3(0.0f,1.0f,0.0f),
//                      CYAN_COLOR}
//        };
//
//        std::vector<Tri> tris{
//                t
//        };
//        auto tri = new Tris(tris,MeshDrawType::Face);
//        tri->init();
    }


    /*
     * Plane
     */

    auto plane = std::make_unique<Plane>(dvec3{0,0,1},dvec3{5,5,5});
    plane->draw();



    /*
     * target plane
     */
    std::vector<Line> lines{
            {
                    {glm::vec3(0.0f,0.0f,0.0f),
                            PURPLE_COLOR},
                    {glm::vec3(1.0f,0.0f,0.0f),
                            RED_COLOR}
            },
            {
                    {glm::vec3(0.0f,0.0f,0.0f),
                            PURPLE_COLOR},
                    {glm::vec3(0.0f,1.0f,0.0f),
                            CYAN_COLOR}
            },
            {
                    {glm::vec3(0.0f,0.0f,0.0f),
                            PURPLE_COLOR},
                    {glm::vec3(0.0f,0.0f,1.0f),
                            GREEN_COLOR}
            },
    };
    auto line = std::make_unique<Lines>(lines);
//    auto line = new Lines(lines);
    line->init();

    TaskPool::pushUpdateTask("CameraTargetPlane",[&](float _abs_time) {
        auto buffer = SingleBPool.getBuffer(tool::combine("Lines",line->id),Buffer_Type::Vertex_Host_Buffer);
        auto target = SingleCamera.m_targetPlane;
        auto center = SingleCamera.center;
        std::array<Line,3> lines{
                Line{
                        Point{center,
                              PURPLE_COLOR},
                        Point{center+target.x_coordinate,
                              RED_COLOR}
                },
                Line{
                        Point{center,
                              PURPLE_COLOR},
                        Point{center+target.y_coordinate,
                              CYAN_COLOR}
                },
                Line {
                        Point{center,
                              PURPLE_COLOR},
                        Point{center+target.z_coordinate,
                              GREEN_COLOR}
                },
        };
        memcpy(buffer->mapped,lines.data(),TargetPlaneDoubleCoordinateSize);
    });

    std::vector<std::string> sky_vec{tool::combineTexture("Skybox_right1.png"),
                                     tool::combineTexture("Skybox_left2.png"),
                                     tool::combineTexture("Skybox_top3.png"),
                                     tool::combineTexture("Skybox_bottom4.png"),
                                     tool::combineTexture("Skybox_front5.png"),
                                     tool::combineTexture("Skybox_back6.png")};

    auto skybox = new Skybox(sky_vec);
    app.getNObjects()->insert(app.getNObjects()->end(),skybox->m_dobjs.begin(),skybox->m_dobjs.end());

    auto a = new animation::Animation(tool::combineModel("ninja.ms3d"),
                                      glm::vec3(-10.0f,-10.0f,0.0f),
                                      glm::vec3(1.0f,0.0f,0.0f),
                                      half_pai,
                                      glm::vec3(0.25f),
                                      0);
    app.getNObjects()->insert(app.getNObjects()->end(), a->m_dobjs.begin(), a->m_dobjs.end());
//
    auto* build = new Basic(std::string(ModelFilePath) + std::string("samplebuilding.gltf"),
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0);
    app.getDObjects()->insert(app.getDObjects()->end(), build->m_dobjs[0]);

    auto* glass = new TBasic(std::string(ModelFilePath) + std::string("samplebuilding_glass.gltf"),
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0);
    app.getTObjects()->insert(app.getTObjects()->end(), glass->m_dobjs[0]);

//    initSet();

    render.getCObjects()->insert(render.getCObjects()->begin(),app.getCObjects()->begin(),app.getCObjects()->end());
    render.getDObjects()->insert(render.getDObjects()->begin(),app.getDObjects()->begin(),app.getDObjects()->end());
    render.getOffDObjects()->insert(render.getOffDObjects()->begin(), app.getOffDObjects()->begin(),app.getOffDObjects()->end());
    render.getTObjects()->insert(render.getTObjects()->begin(), app.getTObjects()->begin(),app.getTObjects()->end());
    render.getNObjects()->insert(render.getNObjects()->begin(), app.getNObjects()->begin(),app.getNObjects()->end());

    TaskPool::executeMultiple();
    render.createCommandBuffer();
    thread_pool->wait();
    app.listener(new OutputPoint);

    try {
        app.update();
        line_pool->release();
        slb_pool.release();
        shader_pool.release();
        set_pool.release();
        pipeline_pool.release();
        buffer_pool.release();
        delete skybox;
        SingleOffScreen.release();
        SingleCascade.release();
        delete build;
        delete glass;
        delete a;
        shatter::render::ShatterRender::getRender().cleanup();
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
