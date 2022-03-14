#include <iostream>
#include <vector>
#include <thread>
#include "tiny_gltf.h"

#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/App/shatterappinclude.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/pool/slbpool.h"
#include "Engine/pool/shaderpool.h"
#include "Engine/pool/setpool.h"
#include "Engine/pool/ppool.h"
#include "Engine/pool/bpool.h"
#include "Engine/pool/mpool.h"
#include "Engine/Object/line3d.h"
#include "Engine/Event/threadpool.h"
#include "Engine/pool/modelsetpool.h"
#include "Engine/Object/camera.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Object/VulkanglTFModels.h"
#include "Engine/pool/shadergrouppool.h"
#include "Engine/Object/offscreen.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Object/listener.h"
#include "Engine/pool/LightManager.h"
#include "Engine/Base/Skybox.h"
#include "Engine/Base/PBRBasic.h"
#include "Engine/Base/basic.h"
#include "Engine/Base/tbasic.h"
#include "Engine/Animation/abasic.h"
#include "Engine/Animation/AnimationHandle.h"
#include "Engine/Base/gcoor.h"
#include "Engine/Base/lines.h"
#include "Engine/Base/tris.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Buffer/shatterbufferinclude.h"

#include "Engine/Mesh/line.h"
#include "Engine/Mesh/plane.h"
#include "Engine/Mesh/aabbVisiter.h"
//#include "Engine/Animation/animation.h"
#include "Engine/Event/delayevent.h"

#include "Engine/Planets/Planet.h"
#include "Engine/Base/CrossTree.h"

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
//    TaskPool::pushTask("UpdateGSet",[](){
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
//    });
}

int main() {
    Shatter::app::ShatterApp& app = Shatter::app::ShatterApp::getApp();
    /*
     * render
     */
    Shatter::render::ShatterRender& render = Shatter::render::ShatterRender::getRender();

    auto& slb_pool = SingleSLBPool;
    auto& shader_pool = SingleShaderPool;
    auto& shader_group_pool = SingleShaderGroupPool;
    auto& set_pool = SingleSetPool;
    auto& pipeline_pool = SinglePPool;
    auto& buffer_pool = SingleBPool;
    auto& model_pool = SingleModelSetPool;
    auto& light_manager = SingleLM;
    auto& offScreen = SingleOffScreen;

    auto line_pool = MPool<Line3d>::getPool();
    auto& camera = Camera::getCamera();
    auto thread_pool = ThreadPool::pool();
    initTransparentSet();

    glm::vec3 color = GOLD_COLOR;
    input::LineColor(color, STATE_IN);

    auto p =  ParticleGroup(5000,
                            {0, 0, 0},
                            {25, 25, 25});

//    auto ah = new AnimationHandle();

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
    std::vector<glm::vec3> aabbBuffer{};
    auto coordinate = GCoor::createGCoor(coor_line);
    AABBVisitor::visitor(aabbBuffer, *coordinate);
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
//        auto tri = new DTris(tris,MeshDrawType::Face);
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
    auto line = std::make_unique<DLines>(lines);
    line->init();

    auto planet = new Planet(20,
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f));

    TaskPool::pushUpdateTask("CameraTargetPlane",[&](float _abs_time) {
        auto buffer = SingleBPool.getBuffer(tool::combine("DLines",line->id),Buffer_Type::Vertex_Host_Buffer);
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
//    auto a = new animation::Animation(tool::combineModel("ninja.ms3d"),
//                                      glm::vec3(-10.0f,-10.0f,0.0f),
//                                      glm::vec3(1.0f,0.0f,0.0f),
//                                      half_pai,
//                                      glm::vec3(0.25f),
//                                      0);
    auto abasic = new ABasic(tool::combineModel("BoxAnimated.gltf"),
                             glm::vec3(-10.0f,-10.0f,0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0,
                             "ABasic");
    abasic->setAnimationIndex(0);

    auto* build = new Basic(std::string(ModelFilePath) + std::string("samplebuilding.gltf"),
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0);
    auto* glass = new TBasic(std::string(ModelFilePath) + std::string("samplebuilding_glass.gltf"),
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0);

//    initSet();
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
        delete coordinate;
        delete skybox;
//        delete ah;
        SingleOffScreen.release();
        SingleCascade.release();
        delete abasic;
        delete build;
        delete glass;
//        delete a;
        delete planet;
        Shatter::render::ShatterRender::getRender().cleanup();
        SingleThreadPool->release();
        SingleDelaySystem.release();
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
