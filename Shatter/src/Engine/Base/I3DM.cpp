//
// Created by jonnxie on 2022/3/28.
//

#include "I3DM.h"
#include PPoolCatalog
#include PipelineCatalog

std::string workingPathFromURL(const std::string& _url){
    size_t pos = _url.find_last_of('/');
    return _url.substr(0, pos);
}

I3DMLoaderBase::I3DMLoaderBase(const std::string &_file, bool _url) {
    if(!_url)
    {
        binaryData = file::loadBinary(_file);
    }else{
        binaryData = nullptr;
        workingPath = workingPathFromURL(_file);
    }
    init();
}

I3DMLoaderBase::~I3DMLoaderBase() {
    free(binaryData);
}

std::string I3DMLoaderBase::resolveExternalURL(const std::string& _url) {
//        if ( /^[^\\/]/.test( url ) ) {
        if(_url.find('/') != 0 || _url.find('\\') != 0){
            //return this.workingPath + '/' + url;
            return workingPath + "/" + _url;
        } else {
            return _url;
        }
}

void I3DMLoaderBase::init() {
    char magic{};
    int  byteLength{};
    int  featureTableJSONByteLength{};
    int  featureTableBinaryByteLength{};
    int  batchTableJSONByteLength{};
    int  batchTableBinaryByteLength{};

    //32 byte head

    // 4 bytes
    magic = file::myReadByte(binaryData, &binaryData_index);
    assert(magic == 'i');
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

    int gltfFormat = file::myReadInt(binaryData, &binaryData_index);

    int feature_table_start = 32;
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
    int batch_length = featureTable.getData("INSTANCES_LENGTH");
    batchTable = BatchTable(batch_table_buffer, batch_length, 0, batchTableJSONByteLength, batchTableBinaryByteLength);

    glbStart = batch_table_start + batchTableJSONByteLength + batchTableBinaryByteLength;
    auto bodyBytes = std::vector<unsigned char>(byteLength - glbStart);
    memcpy(bodyBytes.data(), &binaryData[glbStart], byteLength - glbStart);

//    std::string tmp(bodyBytes.data(), bodyBytes.size());
    if( gltfFormat ) {
        glbBytes = bodyBytes;
    }else{
        std::vector<char> url(byteLength - glbStart);
        memcpy(url.data(), bodyBytes.data(), byteLength - glbStart);
        std::string externalURL = resolveExternalURL(std::string(url.data(), url.size()));
    }
}

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;

    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

I3DMLoader::I3DMLoader(const std::string &_file) : I3DMLoaderBase(_file) {
    m_filePath = _file;
}

void I3DMLoader::init(const std::string &_file) {
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromBinary(glbBytes, _file, &SingleDevice, VkQueue{}, glTFLoadingFlags);
    m_basic = std::make_unique<I3DMBasic>(m_model,
                                          m_pos,
                                          m_rotationAxis,
                                          m_angle,
                                          m_scale,
                                          mallocId(),
                                          m_pipeline,
                                          m_sets);
    SingleRender.normalChanged = true;
}

void I3DMLoader::loadI3DMFile(const std::string &_file) {
    delete m_model;
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromBinary(glbBytes, _file, &SingleDevice, VkQueue{}, glTFLoadingFlags);
    m_basic = std::make_unique<I3DMBasic>(m_model,
                                          m_pos,
                                          m_rotationAxis,
                                          m_angle,
                                          m_scale,
                                          mallocId(),
                                          m_pipeline,
                                          m_sets);
    SingleRender.normalChanged = true;
}

void I3DMBasic::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = ModelSetPool::getPool().malloc();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_world;
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_gGraphics = [&, modelIndex](VkCommandBuffer _cb){
        UnionViewPort& tmp = getViewPort();
        vkCmdSetViewport(_cb, 0, 1, &tmp.view);

        VkRect2D& scissor = getScissor();
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
    SingleRender.getDObjects()->insert(SingleRender.getDObjects()->end(), m_dobjs.begin(), m_dobjs.end());
    TaskPool::pushUpdateTask(tool::combine("I3DMBasic", m_id),[&, modelIndex, d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
}

I3DMBasic::I3DMBasic(vkglTF::Model *_model, glm::vec3 _pos, glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale,
                     int _id, std::string _pipeline, std::vector<std::string> _sets) :
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

I3DMBasic::~I3DMBasic() {
    TaskPool::popUpdateTask(tool::combine("I3DMBasic", m_id));
}
