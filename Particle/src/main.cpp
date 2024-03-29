#include <iostream>
#include <vector>
#include <thread>
#include "tiny_gltf.h"

#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/App/shatterappinclude.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Pool/slbpool.h"
#include "Engine/Pool/shaderpool.h"
#include "Engine/Pool/setpool.h"
#include "Engine/Pool/ppool.h"
#include "Engine/Pool/bpool.h"
#include "Engine/Pool/mpool.h"
#include "Engine/Pool/TexturePool.h"
#include "Engine/Object/line3d.h"
#include "Engine/Object/cobject.h"
#include "Engine/Event/threadpool.h"
#include "Engine/Pool/modelsetpool.h"
#include "Engine/Object/camera.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Object/VulkanglTFModels.h"
#include "Engine/Pool/shadergrouppool.h"
#include "Engine/Object/offscreen.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Object/listener.h"
#include "Engine/Pool/LightManager.h"
#include "Engine/Base/Skybox.h"
#include "Engine/Base/PBRBasic.h"
#include "Engine/Base/basic.h"
#include "Engine/Base/tbasic.h"
#include "Engine/Animation/abasic.h"
#include "Engine/Animation/SkinBasic.h"
#include "Engine/Animation/AnimationHandle.h"
#include "Engine/Base/gcoor.h"
#include "Engine/Base/lines.h"
#include "Engine/Base/tris.h"
#include "Engine/Base/B3DM.h"
#include "Engine/Base/I3DM.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "Engine/Mesh/line.h"
#include "Engine/Mesh/plane.h"
#include "Engine/Mesh/aabbVisiter.h"
#include "Engine/Event/delayevent.h"
#include "Engine/Planets/Planet.h"
#include "Engine/Base/CrossTree.h"
#include "Engine/Base/earth.h"
#include "Engine/Animation/animation.h"

void releasePool()
{
    MPool<Line3d>::getPool()->release();
    MPool<Plane3d>::getPool()->release();
    MPool<DObject>::getPool()->release();
    MPool<GObject>::getPool()->release();
    MPool<CObject>::getPool()->release();
    MPool<VkDescriptorSet>::getPool()->release();
    MPool<glm::mat4>::getPool()->release();
    MPool<ObjectBox>::getPool()->release();
    MPool<AABB>::getPool()->release();
    MPool<Target>::getPool()->release();
}

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
    std::array<VkDescriptorImageInfo, 3> descriptorImageInfos = {
            tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[1].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
            tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[2].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
            tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[3].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
    };
    std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
    for (size_t i = 0; i < descriptorImageInfos.size(); i++) {
        writeDescriptorSets[i] = (tool::writeDescriptorSet(SingleSetPool["gBuffer"], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, i, &descriptorImageInfos[i]));
    }
    vkUpdateDescriptorSets(SingleDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = SingleBPool.getBuffer("ViewPort", Buffer_Type::Uniform_Buffer)->m_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = 8;

    VkWriteDescriptorSet writeSet = tool::writeDescriptorSet(SingleSetPool["ViewPort"], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeSet, 0, nullptr);


    SingleSetPool.AllocateDescriptorSets({"BaseTexture"}, &SingleRender.m_colorSet);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[0].imageView;
    imageInfo.sampler = ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[0].sampler;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    writeSet = tool::writeDescriptorSet(SingleRender.m_colorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &imageInfo);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeSet, 0, nullptr);
}

void test()
{
    glm::vec3 min(0.0f, .0f, 0.0f);
    glm::vec3 max(1.0f, 1.f, 1.0f);
    NPlane p1;
    time_point begin1 = std::chrono::system_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        genPlane(min, max, p1);
    }
    auto end1 = std::chrono::system_clock::now();
    int64_t abs_time_f = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - begin1).count();
    std::cout << abs_time_f / 1000.0f << std::endl;
}

int main() {
    Shatter::App::ShatterApp& app = Shatter::App::ShatterApp::getApp();
    app.setMainWindow();
    UnionViewPort viewPort;
    viewPort.view.x = 0;
    viewPort.view.y = 0;
    viewPort.view.width = Config::getConfig("presentWidth");
    viewPort.view.height = Config::getConfig("presentHeight");
    viewPort.inverseWidth = 1.0f / float(Config::getConfig("presentWidth"));
    viewPort.inverseHeight = 1.0f / float(Config::getConfig("presentHeight"));
    viewPort.view.minDepth = 0;
    viewPort.view.maxDepth = 1;
    viewPort.scissor.offset = { 0, 0};
    viewPort.scissor.extent = { (uint32_t)Config::getConfig("presentWidth"), (uint32_t)Config::getConfig("presentHeight")};
    SingleAPP.setPresentViewPort(viewPort);
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
    auto& texPool = SingleTexturePool;

//    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
//    auto model = new vkglTF::Model;
//    model->loadFromFile(tool::combineModel("Box.gltf"),
//                        &SingleDevice,
//                        VkQueue{},
//                        glTFLoadingFlags,
//                        glm::mat4(1.0f),
//                        1.0f,
//                        false);
//
//    delete model;
    texPool.addTexture("test", tool::combineTexture("Skybox_top3.png"), TextureType::Texture2DDefault);

    texPool.addTexture("world_color_8k", tool::combineTexture("world_color_8k.jpg"), TextureType::Texture2DDefault);
    texPool.addTexture("world_color_16k", tool::combineTexture("world_color_16k.jpg"), TextureType::Texture2DDefault);

    texPool.addTexture("world_height",
                       tool::combineTexture("world_height.png"),
                       TextureType::Texture2DHeight);

    auto line_pool = MPool<Line3d>::getPool();
    {
        auto cPool = MPool<CObject>::getPool();
        auto c = cPool->malloc();
        (*cPool)[c]->compute(VkCommandBuffer{});
    }
    auto& camera = Camera::getCamera();
    auto thread_pool = ThreadPool::pool();
    initTransparentSet();

    glm::vec3 color = GOLD_COLOR;
    input::LineColor(color, STATE_IN);

    auto b3dm = new B3DMLoader(tool::combineB3DM("lr.b3dm"));

    auto i = new I3DMLoader(tool::combineI3DM("tree.i3dm"));
    i->setPos(glm::vec3(-10.0f, -9.0f, 0.0f));
    i->setPipeline("I3DMInstanceBasic");
    i->setTextured(true);
    i->loadI3DMFileInstance();
//    i->loadI3DMFile();

    auto p = new ParticleGroup(5000,
                            {0, 0, 0},
                            {25, 25, 25});

    auto ah = new AnimationHandle();
    ah->setPos(glm::vec3(-10.0f,-10.0f,4.0f));
    ah->setRotationAxis(glm::vec3(1.0f,0.0f,0.0f));
    ah->setAngle(-half_pai);
    ah->setScale(glm::vec3(1.0f));
    ah->setId(0);
    ah->setPipeline("AGBasic");
    ah->setSets(std::vector<std::string>{"Camera", "Planet"});
    ah->setDrawType(DrawObjectType::Default);
//    ah->loadAnimation(tool::combineModel("BoxAnimated.gltf"));
//    ah->loadAnimation(tool::combineModel("InterpolationTest.glb"), true);
    ah->loadAnimation(tool::combineModel("Cesium_Air.glb"), true);
//    ABasic(tool::combineModel("BoxAnimated.gltf"),
//           glm::vec3(-10.0f,-10.0f,0.0f),
//           glm::vec3(1.0f,0.0f,0.0f),
//           -half_pai,
//           glm::vec3(1.0f),
//           0,
//           "ABasic")

    auto skin = new SkinBasic(tool::combineModel("CesiumMan.gltf"),
                              glm::vec3(-8.0f,-8.0f,0.0f),
                              glm::vec3(0.0f,1.0f,0.0f),
                              -half_pai,
                              glm::vec3(1.0f));

    auto skinInstance = new SkinBasicInstance(tool::combineModel("CesiumMan.gltf"),
                                              std::vector<glm::vec3>{
                                                      glm::vec3(-10,-4,0),
                                                      glm::vec3(-10,-6,0),
                                                      glm::vec3(-10,-8,0)
                                              },
                                              glm::vec3(0.0f,1.0f,0.0f),
                                              -half_pai,
                                              glm::vec3(1.0f)
    );

    std::vector<int> coor_line;
    {
        line_pool->malloc(3,coor_line);
        auto line = (*line_pool)[coor_line[0]];
        line->setLine(glm::vec3(0, 0, 0),glm::vec3(1, 0, 0));
        line = (*line_pool)[coor_line[1]];
        line->setLine(glm::vec3(0, 0, 0),glm::vec3(0, 1, 0));
        line = (*line_pool)[coor_line[2]];
        line->setLine(glm::vec3(0, 0, 0),glm::vec3(0, 0, 1));
    }
//    std::vector<glm::vec3> aabbBuffer{};
    std::vector<glm::vec3> coor_lines{ glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 0),glm::vec3(0, 1, 0), glm::vec3(0, 0, 0),glm::vec3(0, 0, 1) };
    auto coordinate = GCoor::createGCoor(coor_lines);
//    AABBVisitor::visitor(aabbBuffer, *coordinate);
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
    auto plane = new Plane(dvec3{ 0, 0, 1},dvec3{ 5, 5, 5});
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
    auto line = new DLines(lines);
    line->init();

    SingleCamera.setLineId(line->id);

    auto planet = new Planet(10,
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             5.0f,
                             BLUE_COLOR,
                             "GPlanet",
                             {"Camera", "Planet"},
                             DrawObjectType::Default);

    auto earth = new Earth(glm::vec3(-10.0f,-10.0f,20.0f),
                           50,
                           50,
                           10,
                           "EarthHeight",
                            true
                           );

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

    auto* build = new Basic(tool::combineModel("samplebuilding.gltf"),
                             glm::vec3(0.0f),
                             glm::vec3(1.0f,0.0f,0.0f),
                             -half_pai,
                             glm::vec3(1.0f),
                             0);
    auto* glass = new TBasic(tool::combineModel("samplebuilding_glass.gltf"),
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

    TargetPlane targetPlane;
    SingleAPP.generateWorkPlane(targetPlane, glm::vec3(0.0f, 0.0f, 0.0f));

    try {
        app.update();
        slb_pool.release();
        shader_pool.release();
        set_pool.release();
        pipeline_pool.release();
        buffer_pool.release();
        delete coordinate;
        delete skybox;
        delete p;
        delete b3dm;
        delete i;
        delete ah;
        delete skin;
        delete skinInstance;
        SingleOffScreen.release();
        texPool.release();
        SingleCascade.release();
//        delete abasic;
        delete build;
        delete glass;
//        delete a;
        delete plane;
        delete line;
        delete planet;
        delete earth;
        Shatter::render::ShatterRender::getRender().cleanup();
        SingleThreadPool->release();
        SingleDelaySystem.release();
        releasePool();
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
