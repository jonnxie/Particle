//
// Created by AnWell on 2022/3/26.
//

#include "B3DM.h"

#include <utility>
#include PPoolCatalog
#include PipelineCatalog

B3DMLoaderBase::B3DMLoaderBase(const std::string &_file) {
    binaryData = file::loadBinary(_file);
    init();
}

void B3DMLoaderBase::init(){
    char magic{};
    int  byteLength{};
    int  featureTableJSONByteLength{};
    int  featureTableBinaryByteLength{};
    int  batchTableJSONByteLength{};
    int  batchTableBinaryByteLength{};

    // 4 bytes
    magic = file::myReadByte(binaryData, &binaryData_index);
    assert(magic == 'b');
    magic = file::myReadByte(binaryData, &binaryData_index);
    assert(magic == '3');
    magic = file::myReadByte(binaryData, &binaryData_index);
    assert(magic == 'd');
    magic = file::myReadByte(binaryData, &binaryData_index);
    assert(magic == 'm');

    // 4 bytes
    version = file::myReadInt(binaryData, &binaryData_index);
    assert(version == 1);

    // 4 bytes
    byteLength = file::myReadInt(binaryData, &binaryData_index);
    // 4 bytes
    featureTableJSONByteLength =  file::myReadInt(binaryData, &binaryData_index);
    // 4 bytes
    featureTableBinaryByteLength =  file::myReadInt(binaryData, &binaryData_index);
    // 4 bytes
    batchTableJSONByteLength =  file::myReadInt(binaryData, &binaryData_index);
    // 4 bytes
    batchTableBinaryByteLength =  file::myReadInt(binaryData, &binaryData_index);


    int feature_table_start = 28;
    std::vector<unsigned char> feature_table_buffer(featureTableJSONByteLength + featureTableBinaryByteLength);
    memcpy(feature_table_buffer.data(),
           &binaryData[feature_table_start],
           featureTableJSONByteLength + featureTableBinaryByteLength);
    featureTable = FeatureTable(feature_table_buffer, 0, featureTableJSONByteLength, featureTableBinaryByteLength);

    int batch_table_start = feature_table_start + featureTableJSONByteLength + featureTableBinaryByteLength;
    std::vector<unsigned char> batch_table_buffer(batchTableJSONByteLength + batchTableBinaryByteLength);
    memcpy(batch_table_buffer.data(),
           &binaryData[batch_table_start],
           batchTableJSONByteLength + batchTableBinaryByteLength);
    int batch_length = featureTable.getData("BATCH_LENGTH");
    batchTable = BatchTable(batch_table_buffer, batch_length, 0, batchTableJSONByteLength, batchTableBinaryByteLength);

    glbStart = batch_table_start + batchTableJSONByteLength + batchTableBinaryByteLength;
    glbBytes = std::vector<unsigned char>(byteLength - glbStart);
    memcpy(glbBytes.data(), &binaryData[glbStart], byteLength - glbStart);
}

B3DMLoaderBase::~B3DMLoaderBase() {
    free(binaryData);
}

B3DMLoader::B3DMLoader(const std::string &_file) : B3DMLoaderBase(_file)
{
    init(_file);
}

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;

    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

void B3DMLoader::init(const std::string &_file) {
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromBinary(glbBytes, _file, &SingleDevice, VkQueue{}, glTFLoadingFlags);
    m_basic = std::make_unique<B3DMBasic>(m_model,
                                          m_pos,
                                          m_rotationAxis,
                                          m_angle,
                                          m_scale,
                                          mallocId(),
                                          m_pipeline,
                                          m_sets);
    SingleRender.normalChanged = true;
}

void B3DMLoader::loadB3DMFile(const std::string& _file) {
    delete m_model;
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromBinary(glbBytes, _file, &SingleDevice, VkQueue{}, glTFLoadingFlags);
    m_basic = std::make_unique<B3DMBasic>(m_model,
                                          m_pos,
                                          m_rotationAxis,
                                          m_angle,
                                          m_scale,
                                          mallocId(),
                                          m_pipeline,
                                          m_sets);
    SingleRender.normalChanged = true;
}

B3DMBasic::B3DMBasic(vkglTF::Model* _model, glm::vec3 _pos, glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale,
                     int _id, std::string  _pipeline, std::vector<std::string>  _sets):
                     m_id(_id),
                     m_pipeline(std::move(_pipeline)),
                     m_sets(std::move(_sets))
{
    m_model = _model;
    m_scale = glm::scale(glm::mat4(1.0f), _scale);
    m_rotate = glm::rotate(glm::mat4(1.0f), _angle, _rotationAxis);
    m_translation = glm::translate(glm::mat4(1.0f), _pos);
    m_world = m_translation * m_scale * m_rotate;
    init();
}

void B3DMBasic::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = ModelSetPool::getPool().malloc();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_world;
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_gGraphics = [&, modelIndex](VkCommandBuffer _cb){
        UnionViewPort& tmp = SingleAPP.getPresentViewPort();
        vkCmdSetViewport(_cb, 0, 1, &tmp.view);
        VkRect2D& scissor = tmp.scissor;
        vkCmdSetScissor(_cb,0,1,&scissor);

        auto set_pool = MPool<VkDescriptorSet>::getPool();
        std::vector<VkDescriptorSet> sets{(*(*set_pool)[modelIndex])};
        for(auto & s: m_sets)
        {
            sets.emplace_back(SingleSetPool[s]);
        }
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_pipeline]->getPipelineLayout(),
                                0,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()[m_pipeline]->getPipeline());
        m_model->draw(_cb);
    };
    insertDObject(d);
    for (int i : m_dobjs)
    {
        SingleRender.pushDObjects(i);
    }
    TaskPool::pushUpdateTask(tool::combine("B3DMBasic", m_id),[&, modelIndex, d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
}

B3DMBasic::~B3DMBasic() {
    TaskPool::popUpdateTask(tool::combine("B3DMBasic", m_id));
}
