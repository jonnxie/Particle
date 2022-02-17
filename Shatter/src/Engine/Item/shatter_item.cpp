//
// Created by maybe on 2020/11/24.
//
#include "precompiledhead.h"

#include "shatter_item.h"
#include <sstream>
#include "Engine/Object/device.h"
#include "Engine/Render/shatter_render_include.h"
//#include <ktx.h>
//#include <ktxvulkan.h>
//#include "../../external/ktx/include/ktx.h"
//#include "../../external/ktx/include/ktxvulkan.h"
using namespace shatter::render;

namespace file{
    bool fileExists(const std::string &filename)
    {
        std::ifstream f(filename.c_str());
        return !f.fail();
    }

    std::vector<char> readFile(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open shader file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    ktxResult loadKTXFile(std::string filename, ktxTexture **target) {
        ktxResult result = KTX_SUCCESS;

        if (!fileExists(filename)) {
            throwFile(filename);
        }
        result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

        return result;
    }

    TexDataObject::TexDataObject(int width,int height,unsigned char* data,int dataByteCount,TexType type)
    {
        this->width=width;
        this->height=height;
        this->data=data;
        this->type=type;
        this->dataByteCount=dataByteCount;
    }

    TexDataObject::~TexDataObject()
    {
        delete data;
    }

    TexDataObject* load_DXT5_BC3_TexData(const std::string& _filename){//加载DDS封装格式的BC3(DXT5)格式纹理数据
        //dds文件头长度
#define DDS_HEADER_LENGTH  31
        //纹理宽度偏移量
#define DDS_HEADER_HEIGHT   3
        //纹理高度偏移量
#define DDS_HEADER_WIDTH   4
        //文件头长度偏移量
#define DDS_HEADER_SIZE  1
        //MIPMAP纹理数量标志掩码
#define DDSD_MIPMAPCOUNT 0x20000
        //MIPMAP纹理数量偏移量
#define DDS_HEADER_MIPMAPCOUNT   7
        //dds文件头标记偏移量
#define DDS_HEADER_FLAGS 2

        printf("%s\n", _filename.c_str());
        char    c_file[200];							//文件路径
        strcpy(c_file, _filename.c_str());					//将文件路径转换成字符数组
        FILE * fpPhoto;
        fpPhoto = fopen(c_file, "rb");					//将文件以二进制形式打开
        if (fpPhoto == NULL) {
            printf("打开文件失败\n");
        }
        fseek(fpPhoto, 0, SEEK_END);					//定位到文件末
        int fileBytesCount = ftell(fpPhoto);			//文件长度
        printf("fileBytesCount %d\n", fileBytesCount);
        fseek(fpPhoto, 0, 0);							//回到文件头

        //获取dds文件的数据
        unsigned char* dataTemp = new unsigned char[fileBytesCount];
        fread(dataTemp, fileBytesCount, 1, fpPhoto);
        //以整数数组的视角看文件数据，为文件头服务
        int* headerI = (int*)dataTemp;
        //获取纹理宽度
        int width = headerI[DDS_HEADER_WIDTH];
        printf("width=%d\n", width);
        //获取纹理高度
        int height = headerI[DDS_HEADER_HEIGHT];
        printf("height=%d\n", height);
        //声明纹理层次辅助变量
        int levels = 1;
        //计算出实际的MipMap纹理层次数量
        if (headerI[DDS_HEADER_FLAGS] & DDSD_MIPMAPCOUNT)
        {
            levels = mymax(1, headerI[DDS_HEADER_MIPMAPCOUNT]);
        }
        printf("levels=%d %d\n", levels, headerI[DDS_HEADER_MIPMAPCOUNT]);

        //纹理数据的起始偏移量
        int dataOffset = headerI[DDS_HEADER_SIZE] + 4;

        //获取纹理数据
        unsigned char* dxtData = dataTemp + dataOffset;

        TexDataObject* result = NULL;

        //声明每层纹理的数据字节偏移量
        int offset = 0;
        //对每个mipmap纹理层进行循环
        for (int i = 0; i < levels; ++i)
        {
            //计算本层纹理的数据字节数
            int levelSize = textureLevelSizeS3tcDxt5(width, height);

            printf("levelSize %d offset %d\n", levelSize, offset);

            //获取本层纹理的数据字节序列
            unsigned char* dataLevel = new unsigned char[levelSize];
            memcpy(dataLevel, dxtData, levelSize);

            result = new TexDataObject(width, height, dataLevel, levelSize, RGBA8_BC3_DXT5);

            //计算下一层纹理的宽度
            width = width >> 1;
            //计算下一层纹理的高度
            height = height >> 1;
            //计算新一层纹理的数据字节偏移量
            offset += levelSize;

            break;
        }
        fclose(fpPhoto);

        delete dataTemp;
        return result;
    }

    int textureLevelSizeS3tcDxt5(int width, int height) {
        return ((width + 3) >> 2) * ((height + 3) >> 2) * 16;
    }

    int mymax(int a, int b) {
        return (a > b) ? a : b;
    }

    std::string loadAssetStr(const std::string& fname)
    {
        std::ifstream infile;
        infile.open(fname.data());
        std::cout << "fname>" << fname << std::endl;
        assert(infile.is_open());
        std::string ss;
        std::string s;
        while (getline(infile, s))
        {
            ss += s;
            ss += '\n';
        }
        infile.close();
        /*cout << ss << endl;*/
        return ss;
    }

    int fromBytesToInt(unsigned char* buff)
    {
        int k = 0;
        unsigned char* temp = (unsigned char*)(&k);
        temp[0] = buff[0];
        temp[1] = buff[1];
        temp[2] = buff[2];
        temp[3] = buff[3];
        return k;
    }

    TexDataObject* loadCommonTexData(std::string fname)
    {
        unsigned char* buf = new unsigned char[4];
        unsigned char* buf2 = new unsigned char[4];
        char    c_file[100];
        strcpy(c_file, fname.c_str());
        FILE * fpPhoto;
        fpPhoto = fopen(c_file, "rb");
        if (fpPhoto == NULL) {
            printf("打开文件失败\n");
        }
        fread(buf, 4, 1, fpPhoto);
        int width = fromBytesToInt(buf);
        fread(buf2, 4, 1, fpPhoto);
        int height = fromBytesToInt(buf2);
        unsigned char* data = new unsigned char[width*height * 4];
        fread(data, width*height * 4, 1, fpPhoto);
        TexDataObject* ctdo = new TexDataObject(width, height, data, width*height * 4);
        return ctdo;
    }

    unsigned char* loadBinary(std::string fname)
    {
        char    c_file[100];
        strcpy(c_file, fname.c_str());
        FILE * fpPhoto;
        fpPhoto = fopen(c_file, "rb");
        if (fpPhoto == NULL) {
            printf("打开文件失败\n");
        }
        fseek(fpPhoto, 0, SEEK_END);
        long size = ftell(fpPhoto);
        unsigned char* data = (unsigned char*)malloc(size + 1);
        fseek(fpPhoto, 0, SEEK_SET);
        fread(data, size, 1, fpPhoto);
        fclose(fpPhoto);
        return data;
    }

    int myReadInt(unsigned char* binaryData, int* binaryData_index)
    {
        int readInt = 0;
        unsigned char * readInt_char = (unsigned char*)&readInt;
        for (int i = 0; i < sizeof(int); i++) {
            readInt_char[i] = (unsigned char)binaryData[*binaryData_index];
            (*binaryData_index)++;
        }
        return readInt;
    }

    short myReadUnsignedShort(unsigned char* binaryData, int* binaryData_index)
    {
        short readShort = 0;
        unsigned char * readShort_char = (unsigned char*)&readShort;
        for (int i = 0; i < sizeof(short); i++) {
            readShort_char[i] = (unsigned char)binaryData[*binaryData_index];
            (*binaryData_index)++;
        }
        return readShort;
    }

    int myReadByte(unsigned char* binaryData, int* binaryData_index)
    {
        int readByte = 0;
        unsigned char * readByte_char = (unsigned char*)&readByte;
        for (int i = 0; i < 1; i++) {
            readByte_char[i] = (unsigned char)binaryData[*binaryData_index];
            (*binaryData_index)++;
        }
        return readByte;
    }

    float myReadFloat(unsigned char* binaryData, int* binaryData_index)
    {
        float readFloat = 0.0f;
        unsigned char * readFloat_char = (unsigned char*)&readFloat;
        for (int i = 0; i < sizeof(float); i++) {
            readFloat_char[i] = (unsigned char)binaryData[*binaryData_index];
            (*binaryData_index)++;
        }
        return readFloat;
    }

    std::string myReadString(unsigned char* binaryData, int* binaryData_index,int length)
    {
        int len = length;
        if (len == 0) {
            std::string s;
            return s;
        }
        unsigned char* ansChar = new unsigned char[len + 1];
        for (int i = 0; i < len; i++) {
            ansChar[i] = (unsigned char)binaryData[*binaryData_index];
            (*binaryData_index)++;
        }
        ansChar[len] = '\0';
        std::string str((const char*)ansChar);
        return str;
    }
}

VkShaderModule createShaderModule(const std::vector<char> &code,VkDevice device) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();

    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,unsigned int width,unsigned int heigh){
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {width, heigh};

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats){
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

bool hasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t mat4Size() {
    return mat4_size;
}

void setDeviceFeatures(const VkPhysicalDeviceFeatures& _deviceFeatures){
    deviceFeatures = _deviceFeatures;
    deviceFeatureMap = {
            {"robustBufferAccess" ,deviceFeatures.robustBufferAccess == 1},
            {"fullDrawIndexUint32" ,deviceFeatures.fullDrawIndexUint32 == 1 },
            {"imageCubeArray" ,deviceFeatures.imageCubeArray == 1 },
            {"independentBlend" ,deviceFeatures.independentBlend == 1 },
            {"geometryShader" ,deviceFeatures.geometryShader == 1 },
            {"tessellationShader" ,deviceFeatures.tessellationShader == 1 },
            {"sampleRateShading" ,deviceFeatures.sampleRateShading == 1 },
            {"dualSrcBlend" ,deviceFeatures.dualSrcBlend == 1 },
            {"logicOp" ,deviceFeatures.logicOp == 1},
            {"multiDrawIndirect" ,deviceFeatures.multiDrawIndirect == 1},
            {"drawIndirectFirstInstance" ,deviceFeatures.drawIndirectFirstInstance == 1 },
            {"depthClamp" ,deviceFeatures.depthClamp == 1 },
            {"depthBiasClamp" ,deviceFeatures.depthBiasClamp == 1 },
            {"fillModeNonSolid" ,deviceFeatures.fillModeNonSolid == 1 },
            {"depthBounds" ,deviceFeatures.depthBounds == 1 },
            {"wideLines" ,deviceFeatures.wideLines == 1 },
            {"largePoints" ,deviceFeatures.largePoints == 1 },
            {"alphaToOne" ,deviceFeatures.alphaToOne == 1 },
            {"multiViewport" ,deviceFeatures.multiViewport == 1 },
            {"samplerAnisotropy" ,deviceFeatures.samplerAnisotropy == 1 },
            {"textureCompressionETC2" ,deviceFeatures.textureCompressionETC2 == 1 },
            {"textureCompressionASTC_LDR" ,deviceFeatures.textureCompressionASTC_LDR == 1 },
            {"textureCompressionBC" ,deviceFeatures.textureCompressionBC == 1 },
            {"occlusionQueryPrecise" ,deviceFeatures.occlusionQueryPrecise == 1 },
            {"pipelineStatisticsQuery" ,deviceFeatures.pipelineStatisticsQuery == 1 },
            {"vertexPipelineStoresAndAtomics" ,deviceFeatures.vertexPipelineStoresAndAtomics == 1 },
            {"fragmentStoresAndAtomics" ,deviceFeatures.fragmentStoresAndAtomics == 1 },
            {"shaderTessellationAndGeometryPointSize" ,deviceFeatures.shaderTessellationAndGeometryPointSize == 1 },
            {"shaderImageGatherExtended" ,deviceFeatures.shaderImageGatherExtended == 1 },
            {"shaderStorageImageExtendedFormats" ,deviceFeatures.shaderStorageImageExtendedFormats == 1 },
            {"shaderStorageImageMultisample" ,deviceFeatures.shaderStorageImageMultisample == 1 },
            {"shaderStorageImageReadWithoutFormat" ,deviceFeatures.shaderStorageImageReadWithoutFormat == 1 },
            {"shaderStorageImageWriteWithoutFormat" ,deviceFeatures.shaderStorageImageWriteWithoutFormat == 1 },
            {"shaderUniformBufferArrayDynamicIndexing" ,deviceFeatures.shaderUniformBufferArrayDynamicIndexing == 1 },
            {"shaderSampledImageArrayDynamicIndexing" ,deviceFeatures.shaderSampledImageArrayDynamicIndexing == 1 },
            {"shaderStorageBufferArrayDynamicIndexing" ,deviceFeatures.shaderStorageBufferArrayDynamicIndexing == 1 },
            {"shaderStorageImageArrayDynamicIndexing" ,deviceFeatures.shaderStorageImageArrayDynamicIndexing == 1 },
            {"shaderClipDistance" ,deviceFeatures.shaderClipDistance == 1 },
            {"shaderCullDistance" ,deviceFeatures.shaderCullDistance == 1 },
            {"shaderFloat64" ,deviceFeatures.shaderFloat64 == 1 },
            {"shaderInt64" ,deviceFeatures.shaderInt64 == 1 },
            {"shaderInt16" ,deviceFeatures.shaderInt16 == 1 },
            {"shaderResourceResidency" ,deviceFeatures.shaderResourceResidency == 1 },
            {"shaderResourceMinLod" ,deviceFeatures.shaderResourceMinLod == 1 },
            {"sparseBinding" ,deviceFeatures.sparseBinding == 1 },
            {"sparseResidencyBuffer" ,deviceFeatures.sparseResidencyBuffer == 1 },
            {"sparseResidencyImage2D" ,deviceFeatures.sparseResidencyImage2D == 1 },
            {"sparseResidencyImage3D" ,deviceFeatures.sparseResidencyImage3D == 1 },
            {"sparseResidency2Samples" ,deviceFeatures.sparseResidency2Samples == 1 },
            {"sparseResidency4Samples" ,deviceFeatures.sparseResidency4Samples == 1 },
            {"sparseResidency8Samples" ,deviceFeatures.sparseResidency8Samples == 1 },
            {"sparseResidency16Samples" ,deviceFeatures.sparseResidency16Samples == 1 },
            {"sparseResidencyAliased" ,deviceFeatures.sparseResidencyAliased == 1 },
            {"variableMultisampleRate" ,deviceFeatures.variableMultisampleRate == 1 },
            {"inheritedQueries" ,deviceFeatures.inheritedQueries == 1 },
    };
}

VkPhysicalDeviceFeatures& getDeviceFeatures(){
    return deviceFeatures;
}

bool checkFeatures(const std::string&features)
{
    return deviceFeatureMap[features];
}


void printDeviceFeatures(){
    std::cout<< "robustBufferAccess: "+ std::string(deviceFeatures.robustBufferAccess == 1? "true":"false") << std::endl;
    std::cout<< "fullDrawIndexUint32: "+ std::string(deviceFeatures.fullDrawIndexUint32 == 1? "true":"false") << std::endl;
    std::cout<< "imageCubeArray: "+ std::string(deviceFeatures.imageCubeArray == 1? "true":"false") << std::endl;
    std::cout<< "independentBlend: "+ std::string(deviceFeatures.independentBlend == 1? "true":"false") << std::endl;
    std::cout<< "geometryShader: "+ std::string(deviceFeatures.geometryShader == 1? "true":"false") << std::endl;
    std::cout<< "tessellationShader: "+ std::string(deviceFeatures.tessellationShader == 1? "true":"false") << std::endl;
    std::cout<< "sampleRateShading: "+ std::string(deviceFeatures.sampleRateShading == 1? "true":"false") << std::endl;
    std::cout<< "dualSrcBlend: "+ std::string(deviceFeatures.dualSrcBlend == 1? "true":"false") << std::endl;
    std::cout<< "logicOp: "+ std::string(deviceFeatures.logicOp == 1? "true":"false") << std::endl;
    std::cout<< "multiDrawIndirect: "+ std::string(deviceFeatures.multiDrawIndirect == 1? "true":"false") << std::endl;
    std::cout<< "drawIndirectFirstInstance: "+ std::string(deviceFeatures.drawIndirectFirstInstance == 1? "true":"false") << std::endl;
    std::cout<< "depthClamp: "+ std::string(deviceFeatures.depthClamp == 1? "true":"false") << std::endl;
    std::cout<< "depthBiasClamp: "+ std::string(deviceFeatures.depthBiasClamp == 1? "true":"false") << std::endl;
    std::cout<< "fillModeNonSolid: "+ std::string(deviceFeatures.fillModeNonSolid == 1? "true":"false") << std::endl;
    std::cout<< "depthBounds: "+ std::string(deviceFeatures.depthBounds == 1? "true":"false") << std::endl;
    std::cout<< "wideLines: "+ std::string(deviceFeatures.wideLines == 1? "true":"false") << std::endl;
    std::cout<< "largePoints: "+ std::string(deviceFeatures.largePoints == 1? "true":"false") << std::endl;
    std::cout<< "alphaToOne: "+ std::string(deviceFeatures.alphaToOne == 1? "true":"false") << std::endl;
    std::cout<< "multiViewport: "+ std::string(deviceFeatures.multiViewport == 1? "true":"false") << std::endl;
    std::cout<< "samplerAnisotropy: "+ std::string(deviceFeatures.samplerAnisotropy == 1? "true":"false") << std::endl;
    std::cout<< "textureCompressionETC2: "+ std::string(deviceFeatures.textureCompressionETC2 == 1? "true":"false") << std::endl;
    std::cout<< "textureCompressionASTC_LDR: "+ std::string(deviceFeatures.textureCompressionASTC_LDR == 1? "true":"false") << std::endl;
    std::cout<< "textureCompressionBC: "+ std::string(deviceFeatures.textureCompressionBC == 1? "true":"false") << std::endl;
    std::cout<< "occlusionQueryPrecise: "+ std::string(deviceFeatures.occlusionQueryPrecise == 1? "true":"false") << std::endl;
    std::cout<< "pipelineStatisticsQuery: "+ std::string(deviceFeatures.pipelineStatisticsQuery == 1? "true":"false") << std::endl;
    std::cout<< "vertexPipelineStoresAndAtomics: "+ std::string(deviceFeatures.vertexPipelineStoresAndAtomics == 1? "true":"false") << std::endl;
    std::cout<< "fragmentStoresAndAtomics: "+ std::string(deviceFeatures.fragmentStoresAndAtomics == 1? "true":"false") << std::endl;
    std::cout<< "shaderTessellationAndGeometryPointSize: "+ std::string(deviceFeatures.shaderTessellationAndGeometryPointSize == 1? "true":"false") << std::endl;
    std::cout<< "shaderImageGatherExtended: "+ std::string(deviceFeatures.shaderImageGatherExtended == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageImageExtendedFormats: "+ std::string(deviceFeatures.shaderStorageImageExtendedFormats == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageImageMultisample: "+ std::string(deviceFeatures.shaderStorageImageMultisample == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageImageReadWithoutFormat: "+ std::string(deviceFeatures.shaderStorageImageReadWithoutFormat == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageImageWriteWithoutFormat: "+ std::string(deviceFeatures.shaderStorageImageWriteWithoutFormat == 1? "true":"false") << std::endl;
    std::cout<< "shaderUniformBufferArrayDynamicIndexing: "+ std::string(deviceFeatures.shaderUniformBufferArrayDynamicIndexing == 1? "true":"false") << std::endl;
    std::cout<< "shaderSampledImageArrayDynamicIndexing: "+ std::string(deviceFeatures.shaderSampledImageArrayDynamicIndexing == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageBufferArrayDynamicIndexing: "+ std::string(deviceFeatures.shaderStorageBufferArrayDynamicIndexing == 1? "true":"false") << std::endl;
    std::cout<< "shaderStorageImageArrayDynamicIndexing: "+ std::string(deviceFeatures.shaderStorageImageArrayDynamicIndexing == 1? "true":"false") << std::endl;
    std::cout<< "shaderClipDistance: "+ std::string(deviceFeatures.shaderClipDistance == 1? "true":"false") << std::endl;
    std::cout<< "shaderCullDistance: "+ std::string(deviceFeatures.shaderCullDistance == 1? "true":"false") << std::endl;
    std::cout<< "shaderFloat64: "+ std::string(deviceFeatures.shaderFloat64 == 1? "true":"false") << std::endl;
    std::cout<< "shaderInt64: "+ std::string(deviceFeatures.shaderInt64 == 1? "true":"false") << std::endl;
    std::cout<< "shaderInt16: "+ std::string(deviceFeatures.shaderInt16 == 1? "true":"false") << std::endl;
    std::cout<< "shaderResourceResidency: "+ std::string(deviceFeatures.shaderResourceResidency == 1? "true":"false") << std::endl;
    std::cout<< "shaderResourceMinLod: "+ std::string(deviceFeatures.shaderResourceMinLod == 1? "true":"false") << std::endl;
    std::cout<< "sparseBinding: "+ std::string(deviceFeatures.sparseBinding == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidencyBuffer: "+ std::string(deviceFeatures.sparseResidencyBuffer == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidencyImage2D: "+ std::string(deviceFeatures.sparseResidencyImage2D == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidencyImage3D: "+ std::string(deviceFeatures.sparseResidencyImage3D == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidency2Samples: "+ std::string(deviceFeatures.sparseResidency2Samples == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidency4Samples: "+ std::string(deviceFeatures.sparseResidency4Samples == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidency8Samples: "+ std::string(deviceFeatures.sparseResidency8Samples == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidency16Samples: "+ std::string(deviceFeatures.sparseResidency16Samples == 1? "true":"false") << std::endl;
    std::cout<< "sparseResidencyAliased: "+ std::string(deviceFeatures.sparseResidencyAliased == 1? "true":"false") << std::endl;
    std::cout<< "variableMultisampleRate: "+ std::string(deviceFeatures.variableMultisampleRate == 1? "true":"false") << std::endl;
    std::cout<< "inheritedQueries: "+ std::string(deviceFeatures.inheritedQueries == 1? "true":"false") << std::endl;
}

generatevk(VkPhysicalDeviceMemoryProperties);

void getResult(int _key) {
    std::cout << resultMap[_key] << std::endl;
}

std::string getVkResult(int _key) {
    return resultMap[_key];
}

int patchNum(int _objectNum, int _thread) {
    return (_objectNum + _thread - 1)/_thread;
}

QueueFamilyIndices getIndices() {
    return queueIndices;
}

void setIndices(const QueueFamilyIndices &_indices) {
    queueIndices = _indices;
}

VkViewport getViewPort() {
    return viewPort;
}

void setViewPort(VkViewport _viewport) {
    viewPort = _viewport;
}

VkRect2D getScissor() {
    return scissor;
}

void setScissor(VkRect2D _scissor) {
    scissor = _scissor;
}

std::vector<ThreadObject>* getThreadObjectPool() {
    return &threadObjects;
}

void releaseThreadObjectPool() {
    for(auto &i: threadObjects)
    {
        vkFreeCommandBuffers(Device::getDevice()(),i.commandPool,i.buffers.size(),i.buffers.data());
        vkDestroyCommandPool(Device::getDevice()(),i.commandPool,VK_NULL_HANDLE);
    }
}

std::vector<UnionCommandPool>* getThreadCommandPool() {
    return &threadCommandPool;
}

void releaseThreadCommandPool() {
    for(auto &i: threadCommandPool)
    {
        vkDestroyCommandPool(Device::getDevice()(),i.computePool,VK_NULL_HANDLE);
        vkDestroyCommandPool(Device::getDevice()(),i.graphicsPool,VK_NULL_HANDLE);
    }
}

namespace timer{
    float getTime() {
        return shatter_time;
    }

    void setTime(const float &_time) {
        shatter_time = _time;
    }

    float getDetaTime() {
        return deta_time;
    }

    void setDetaTime(const float &_time) {
        deta_time = _time;
    }

    void setChronoTime(const time_point &_time) {
        chrono_time = _time;
    }

    time_point *getChronoTime() {
        return &chrono_time;
    }
}

void* getDeviceCreateNextChain(){
    return deviceCreatepNextChain;
}

void setDeviceCreateNextChain(void* pNext){
    deviceCreatepNextChain = pNext;
}

void setSwapChainFormat(const VkSurfaceFormatKHR &_format) {
    swapchain_format = _format;
}

VkSurfaceFormatKHR getSwapChainFormat() {
    return swapchain_format;
}

void setSwapChainIndex(int _index) {
    currentSwapChainIndex = _index;
}

void *getIndirectDrawState() {
    return &indirect_draw_state;
}

IndirectDrawState getIndirectState() {
    return indirect_draw_state;
}

VkFormat getDepthFormat() {
    return depth_format;
}

void setDepthFormat(const VkFormat &_format) {
    depth_format = _format;
}

bool memoryTypeFromProperties(VkPhysicalDeviceMemoryProperties &memoryProperties, uint32_t typeBits,
                              VkFlags requirements_mask, uint32_t *typeIndex) {
    //循环确定内存类型索引
    for (uint32_t i = 0; i < 32; i++)
    {
        //若对应类型比特位为1
        if ((typeBits & 1) == 1)
        {
            //此类型与掩码匹配
            if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
            {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    //没有找到所需的类型索引
    return false;
}

Material &getMaterial() {
    return material;
}

namespace tool {

    std::string combine(const std::string& _str, int num) {
        std::stringstream str;
        str << num;
        return _str+ str.str();
    }

    std::string combineShader(const std::string& _shaderName){
        return ShaderFilePath + _shaderName;
    }

    std::string combineTexture(const std::string& _shaderName){
        return TextureFilePath + _shaderName;
    }

    std::string combineModel(const std::string& _shaderName){
        return ModelFilePath + _shaderName;
    }

    VkPipelineShaderStageCreateInfo createShaderStage(const std::string& _filename,ShaderType _type)
    {
        if(!file::fileExists(_filename)){
            throwFile(_filename);
        }
        auto shader_code = file::readFile(_filename);
        VkPipelineShaderStageCreateInfo shader_info;
        shader_info.pNext = VK_NULL_HANDLE;
        shader_info.flags = 0;
        shader_info.pSpecializationInfo = VK_NULL_HANDLE;
        auto m_shader_module = createShaderModule(shader_code, Device::getDevice()());

        shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        switch (_type) {
            case VertexShader:{
                shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            }
            case FragmentShader:{
                shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            }
            case ComputeShader:{
                shader_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                break;
            }
            default: break;
        }
        shader_info.module = m_shader_module;
        shader_info.pName = "main";

        return shader_info;
    }

    VkDescriptorSetLayoutBinding
    getSetLayoutBinding(VkDescriptorType _type, VkShaderStageFlags _flags, uint32_t _binding) {
        return VkDescriptorSetLayoutBinding{_binding, _type, 1, _flags, VK_NULL_HANDLE};
    }

    VkBufferCreateInfo getBufferCreateInfo() {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        return bufferInfo;
    }

    void setLayout(VkCommandBuffer _buffer, VkImage _image, VkImageLayout oldLayout, VkImageLayout newLayout,
                   VkImageSubresourceRange sub) {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _image;

        barrier.subresourceRange = sub;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        switch (oldLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter).
                // Only valid as initial layout. No flags required.
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized.
                // Only valid as initial layout for linear images; preserves memory
                // contents. Make sure host writes have finished.
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment.
                // Make sure writes to the color buffer have finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment.
                // Make sure any writes to the depth/stencil buffer have finished.
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source.
                // Make sure any reads from the image have finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination.
                // Make sure any writes to the image have finished.
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader.
                // Make sure any shader reads from the image have finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;

            default:
                /* Value not used by callers, so not supported. */
                assert(KTX_FALSE);
        }

        // Target layouts (new)
        // The destination access mask controls the dependency for the new image
        // layout.
        switch (newLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination.
                // Make sure any writes to the image have finished.
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source.
                // Make sure any reads from and writes to the image have finished.
                barrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment.
                // Make sure any writes to the color buffer have finished.
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment.
                // Make sure any writes to depth/stencil buffer have finished.
                barrier.dstAccessMask
                        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment).
                // Make sure any writes to the image have finished.
                if (barrier.srcAccessMask == 0) {
                    barrier.srcAccessMask
                            = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                /* Value not used by callers, so not supported. */
                assert(KTX_FALSE);
        }
        vkCmdPipelineBarrier(
                _buffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );
    }

    VkMemoryAllocateInfo memoryAllocateInfo() {
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext = VK_NULL_HANDLE;
        return info;
    }

    VkImageCreateInfo imageCreateInfo() {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = VK_NULL_HANDLE;
        return info;
    }

    VkSamplerCreateInfo samplerCreateInfo() {
        VkSamplerCreateInfo samplerCreateInfo {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        return samplerCreateInfo;
    }

    VkImageViewCreateInfo imageViewCreateInfo()
    {
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.pNext = VK_NULL_HANDLE;
        return viewCreateInfo;
    }

    VkRenderPassCreateInfo renderPassCreateInfo()
    {
        VkRenderPassCreateInfo renderPassCreateInfo {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        return renderPassCreateInfo;
    }

    VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType _type,uint32_t _count)
    {
//        VkDescriptorPoolSize poolSize = {};
//        poolSize.type = _type;
//        poolSize.descriptorCount = _count;
        return VkDescriptorPoolSize{_type,_count};
    }

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType _type,VkShaderStageFlags _flags,uint32_t _binding,uint32_t _descriptorCount)
    {
        VkDescriptorSetLayoutBinding setLayoutBinding {};
        setLayoutBinding.descriptorType = _type;
        setLayoutBinding.stageFlags = _flags;
        setLayoutBinding.binding = _binding;
        setLayoutBinding.descriptorCount = _descriptorCount;
        return setLayoutBinding;
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolSizes,uint32_t _maxSetCount)
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.pNext = VK_NULL_HANDLE;
        descriptorPoolInfo.maxSets = _maxSetCount;
        descriptorPoolInfo.poolSizeCount = poolSizes.size();
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        return descriptorPoolInfo;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding> &_setLayoutBinding) {
        VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pNext = VK_NULL_HANDLE;
        descriptorLayout.bindingCount = _setLayoutBinding.size();
        descriptorLayout.pBindings = _setLayoutBinding.data();
        return descriptorLayout;
    }

    VkDescriptorSetAllocateInfo
    descriptorSetAllocateInfo(VkDescriptorPool _pool, VkDescriptorSetLayout *_layout, uint32_t _count) {
        return VkDescriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                           VK_NULL_HANDLE,
                                           _pool,
                                           _count,
                                           _layout};
    }

    VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) {
        VkDescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = imageLayout;
        return descriptorImageInfo;
    }

    VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding,
                                                  VkDescriptorBufferInfo *bufferInfo, uint32_t descriptorCount) {
        VkWriteDescriptorSet writeDescriptorSet {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = dstSet;
        writeDescriptorSet.descriptorType = type;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.pBufferInfo = bufferInfo;
        writeDescriptorSet.descriptorCount = descriptorCount;
        return writeDescriptorSet;
    }

    VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding,
                                                  VkDescriptorImageInfo *imageInfo, uint32_t descriptorCount) {
        VkWriteDescriptorSet writeDescriptorSet {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = dstSet;
        writeDescriptorSet.descriptorType = type;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.pImageInfo = imageInfo;
        writeDescriptorSet.descriptorCount = descriptorCount;
        return writeDescriptorSet;
    }

    VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset) {
        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = stageFlags;
        pushConstantRange.offset = offset;
        pushConstantRange.size = size;
        return pushConstantRange;
    }

    VkPipelineInputAssemblyStateCreateInfo
    pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags,
                                         VkBool32 primitiveRestartEnable) {
        VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {};
        pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineInputAssemblyStateCreateInfo.topology = topology;
        pipelineInputAssemblyStateCreateInfo.flags = flags;
        pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
        return pipelineInputAssemblyStateCreateInfo;
    }

    VkPipelineLayoutCreateInfo
    pipelineLayoutCreateInfo(const VkDescriptorSetLayout *pSetLayouts, uint32_t setLayoutCount) {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
        pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
        return pipelineLayoutCreateInfo;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(uint32_t setLayoutCount) {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
        return pipelineLayoutCreateInfo;
    }

    VkPipelineRasterizationStateCreateInfo
    pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace,
                                         VkPipelineRasterizationStateCreateFlags flags) {
        VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {};
        pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
        pipelineRasterizationStateCreateInfo.cullMode = cullMode;
        pipelineRasterizationStateCreateInfo.frontFace = frontFace;
        pipelineRasterizationStateCreateInfo.flags = flags;
        pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
        return pipelineRasterizationStateCreateInfo;
    }

    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
            VkColorComponentFlags colorWriteMask,
            VkBool32 blendEnable)
    {
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
        pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
        pipelineColorBlendAttachmentState.blendEnable = blendEnable;
        return pipelineColorBlendAttachmentState;
    }

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(uint32_t attachmentCount,
                                                                          const VkPipelineColorBlendAttachmentState *pAttachments) {
        VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
        pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
        pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
        return pipelineColorBlendStateCreateInfo;
    }

    VkPipelineDepthStencilStateCreateInfo
    pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable,
                                        VkCompareOp depthCompareOp) {
        VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {};
        pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
        pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        return pipelineDepthStencilStateCreateInfo;
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(uint32_t viewportCount, uint32_t scissorCount,
                                                                      VkPipelineViewportStateCreateFlags flags) {
        VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {};
        pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineViewportStateCreateInfo.viewportCount = viewportCount;
        pipelineViewportStateCreateInfo.scissorCount = scissorCount;
        pipelineViewportStateCreateInfo.flags = flags;
        return pipelineViewportStateCreateInfo;
    }

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples,
                                                                            VkPipelineMultisampleStateCreateFlags flags) {
        VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {};
        pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
        pipelineMultisampleStateCreateInfo.flags = flags;
        return pipelineMultisampleStateCreateInfo;
    }

    VkPipelineDynamicStateCreateInfo
    pipelineDynamicStateCreateInfo(const VkDynamicState *pDynamicStates, uint32_t dynamicStateCount,
                                         VkPipelineDynamicStateCreateFlags flags) {
        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {};
        pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
        pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
        pipelineDynamicStateCreateInfo.flags = flags;
        return pipelineDynamicStateCreateInfo;
    }

    VkPipelineDynamicStateCreateInfo
    pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState> &pDynamicStates,
                                         VkPipelineDynamicStateCreateFlags flags) {
        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
        pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
        pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
        pipelineDynamicStateCreateInfo.flags = flags;
        return pipelineDynamicStateCreateInfo;
    }

    VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints) {
        VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo {};
        pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
        return pipelineTessellationStateCreateInfo;
    }

    VkGraphicsPipelineCreateInfo
    pipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags) {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = layout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.flags = flags;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        return pipelineCreateInfo;
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo() {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        return pipelineCreateInfo;
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo(VkPipelineLayout layout, VkPipelineCreateFlags flags) {
        VkComputePipelineCreateInfo computePipelineCreateInfo {};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.layout = layout;
        computePipelineCreateInfo.flags = flags;
        return computePipelineCreateInfo;
    }

    VkVertexInputBindingDescription
    vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
        VkVertexInputBindingDescription vInputBindDescription {};
        vInputBindDescription.binding = binding;
        vInputBindDescription.stride = stride;
        vInputBindDescription.inputRate = inputRate;
        return vInputBindDescription;
    }

    VkVertexInputAttributeDescription
    vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
        VkVertexInputAttributeDescription vInputAttribDescription {};
        vInputAttribDescription.location = location;
        vInputAttribDescription.binding = binding;
        vInputAttribDescription.format = format;
        vInputAttribDescription.offset = offset;
        return vInputAttribDescription;
    }

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() {
        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {};
        pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        return pipelineVertexInputStateCreateInfo;
    }

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
            const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
            const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions) {
        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
        pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data();
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
        return pipelineVertexInputStateCreateInfo;
    }

    VkImageMemoryBarrier imageMemoryBarrier() {
        VkImageMemoryBarrier imageMemoryBarrier {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }


    void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier = tool::imageMemoryBarrier();
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (imageMemoryBarrier.srcAccessMask == 0)
                {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask)
    {
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }

    VkViewport viewport(float width, float height, float minDepth, float maxDepth) {
        VkViewport viewport {};
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;
        return viewport;
    }

    VkRect2D rect2D(
            int32_t width,
            int32_t height,
            int32_t offsetX,
            int32_t offsetY) {
        VkRect2D rect2D {};
        rect2D.extent.width = width;
        rect2D.extent.height = height;
        rect2D.offset.x = offsetX;
        rect2D.offset.y = offsetY;
        return rect2D;
    }

    uint32_t alignedSize(uint32_t value, uint32_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    VkBufferCreateInfo bufferCreateInfo() {
        VkBufferCreateInfo bufCreateInfo {};
        bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        return bufCreateInfo;
    }

    VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags usage, VkDeviceSize size) {
        VkBufferCreateInfo bufCreateInfo {};
        bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufCreateInfo.usage = usage;
        bufCreateInfo.size = size;
        return bufCreateInfo;
    }

    VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR() {
        VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR{};
        accelerationStructureGeometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        return accelerationStructureGeometryKHR;
    }

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKHR() {
        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKHR{};
        rayTracingPipelineCreateInfoKHR.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        return rayTracingPipelineCreateInfoKHR;
    }

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR() {
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR{};
        accelerationStructureBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        return accelerationStructureBuildGeometryInfoKHR;
    }

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR() {
        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR{};
        accelerationStructureBuildSizesInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        return accelerationStructureBuildSizesInfoKHR;
    }

    VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR() {
        VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR{};
        rayTracingShaderGroupCreateInfoKHR.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        return rayTracingShaderGroupCreateInfoKHR;
    }

    VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR() {
        VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR{};
        writeDescriptorSetAccelerationStructureKHR.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        return writeDescriptorSetAccelerationStructureKHR;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo() {
        VkCommandBufferBeginInfo cmdBufferBeginInfo {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        return cmdBufferBeginInfo;
    }

    void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,
                                  VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange) {
        VkImageMemoryBarrier imageMemoryBarrier = tool::imageMemoryBarrier();
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
    }

    VkFramebufferCreateInfo framebufferCreateInfo(){
        VkFramebufferCreateInfo framebufferCreateInfo {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        return framebufferCreateInfo;
    }

    void saveScreenshot(std::basic_string<char> filename) {
//        screenshotSaved = false;
        bool supportsBlit = true;

        // Check blit support for source and destination
        VkFormatProperties formatProps;

        // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
        vkGetPhysicalDeviceFormatProperties(Device::getDevice().physicalDevice, getSwapChainFormat().format, &formatProps);
        if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
            std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
            supportsBlit = false;
        }

        // Check if the device supports blitting to linear images
        vkGetPhysicalDeviceFormatProperties(Device::getDevice().physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
        if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
            std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
            supportsBlit = false;
        }

        // Source for the copy is the last rendered swapchain image
        VkImage srcImage = ShatterRender::getRender().swapchain_images[currentSwapChainIndex];

        // Create the linear tiled destination image to copy to and to read the memory from
        VkImageCreateInfo imageCreateCI(tool::imageCreateInfo());
        imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
        // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
        imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateCI.extent.width = viewPort.width;
        imageCreateCI.extent.height = viewPort.height;
        imageCreateCI.extent.depth = 1;
        imageCreateCI.arrayLayers = 1;
        imageCreateCI.mipLevels = 1;
        imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
        imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        // Create the image
        VkImage dstImage;
        vkCreateImage(Device::getDevice().logicalDevice, &imageCreateCI, nullptr, &dstImage);
        // Create memory to back up the image
        VkMemoryRequirements memRequirements;
        VkMemoryAllocateInfo memAllocInfo(tool::memoryAllocateInfo());
        VkDeviceMemory dstImageMemory;
        vkGetImageMemoryRequirements(Device::getDevice().logicalDevice, dstImage, &memRequirements);
        memAllocInfo.allocationSize = memRequirements.size;
        // Memory must be host visible to copy from
        memAllocInfo.memoryTypeIndex = Device::getDevice().getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkAllocateMemory(Device::getDevice().logicalDevice, &memAllocInfo, nullptr, &dstImageMemory);
        vkBindImageMemory(Device::getDevice().logicalDevice, dstImage, dstImageMemory, 0);

        // Do the actual blit from the swapchain image to our host visible destination image
//        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkCommandBuffer copyCmd = ShatterRender::getRender().beginSingleTimeCommands();

        // Transition destination image to transfer destination layout
        tool::insertImageMemoryBarrier(
                copyCmd,
                dstImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // Transition swapchain image from present to transfer source layout
        tool::insertImageMemoryBarrier(
                copyCmd,
                srcImage,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
        if (supportsBlit)
        {
            // Define the region to blit (we will blit the whole swapchain image)
            VkOffset3D blitSize;
            blitSize.x = viewPort.width;
            blitSize.y = viewPort.height;
            blitSize.z = 1;
            VkImageBlit imageBlitRegion{};
            imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.srcSubresource.layerCount = 1;
            imageBlitRegion.srcOffsets[1] = blitSize;
            imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.dstSubresource.layerCount = 1;
            imageBlitRegion.dstOffsets[1] = blitSize;

            // Issue the blit command
            vkCmdBlitImage(
                    copyCmd,
                    srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlitRegion,
                    VK_FILTER_NEAREST);
        }
        else
        {
            // Otherwise use image copy (requires us to manually flip components)
            VkImageCopy imageCopyRegion{};
            imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.layerCount = 1;
            imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.layerCount = 1;
            imageCopyRegion.extent.width = viewPort.width;
            imageCopyRegion.extent.height = viewPort.height;
            imageCopyRegion.extent.depth = 1;

            // Issue the copy command
            vkCmdCopyImage(
                    copyCmd,
                    srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageCopyRegion);
        }

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on
        tool::insertImageMemoryBarrier(
                copyCmd,
                dstImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // Transition back the swap chain image after the blit is done
        tool::insertImageMemoryBarrier(
                copyCmd,
                srcImage,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        ShatterRender::getRender().endSingleTimeCommands(copyCmd);
//        vulkanDevice->flushCommandBuffer(copyCmd, queue);

        // Get layout of the image (including row pitch)
        VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(Device::getDevice().logicalDevice, dstImage, &subResource, &subResourceLayout);

        // Map image memory so we can start copying from it
        const char* data;
        vkMapMemory(Device::getDevice().logicalDevice, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
        data += subResourceLayout.offset;

        std::ofstream file(filename, std::ios::out | std::ios::binary);

        // ppm header
        file << "P6\n" << viewPort.width << "\n" << viewPort.height << "\n" << 255 << "\n";

        // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
        bool colorSwizzle = false;
        // Check if source is BGR
        // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
        if (!supportsBlit)
        {
            std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
            colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), swapchain_format.format) != formatsBGR.end());
        }

        // ppm binary pixel data
        for (uint32_t y = 0; y < viewPort.height; y++)
        {
            unsigned int *row = (unsigned int*)data;
            for (uint32_t x = 0; x < viewPort.width; x++)
            {
                if (colorSwizzle)
                {
                    file.write((char*)row+2, 1);
                    file.write((char*)row+1, 1);
                    file.write((char*)row, 1);
                }
                else
                {
                    file.write((char*)row, 3);
                }
                row++;
            }
            data += subResourceLayout.rowPitch;
        }
        file.close();

        std::cout << "Screenshot saved to disk" << std::endl;

        // Clean up resources
        vkUnmapMemory(Device::getDevice().logicalDevice, dstImageMemory);
        vkFreeMemory(Device::getDevice().logicalDevice, dstImageMemory, nullptr);
        vkDestroyImage(Device::getDevice().logicalDevice, dstImage, nullptr);

//        screenshotSaved = true;
    }

    VkRenderPassBeginInfo renderPassBeginInfo() {
        VkRenderPassBeginInfo renderPassBeginInfo {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        return renderPassBeginInfo;
    }

    void printMat(const glm::mat4& _in) {
        printf("[%f,%f,%f,%f]\n",_in[0][0],_in[0][1],_in[0][2],_in[0][3]);
        printf("[%f,%f,%f,%f]\n",_in[1][0],_in[1][1],_in[1][2],_in[1][3]);
        printf("[%f,%f,%f,%f]\n",_in[2][0],_in[2][1],_in[2][2],_in[2][3]);
        printf("[%f,%f,%f,%f]\n",_in[3][0],_in[3][1],_in[3][2],_in[3][3]);
    }
}

void transitionImageLayout(
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask){

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    transitionImageLayout(image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

void transitionImageLayout(
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask){
    VkCommandBuffer cmdbuffer = shatter::render::ShatterRender::getRender().beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (imageMemoryBarrier.srcAccessMask == 0)
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

    shatter::render::ShatterRender::getRender().endSingleTimeCommands(cmdbuffer);
}


Vector3f::Vector3f() {
    this->coord = new float[3]{0,0,0};
}
Vector3f::Vector3f(float x, float y, float z) {
    this->coord = new float[3]{x,y,z};
}
Vector3f::~Vector3f()
{
    delete coord;
}
void Vector3f::interpolate(Vector3f* v1, Vector3f* v2, float alpha) {
    this->setX((1 - alpha) * v1->getX() + alpha * v2->getX());
    this->setY((1 - alpha) * v1->getY() + alpha * v2->getY());
    this->setZ((1 - alpha) * v1->getZ() + alpha * v2->getZ());
}
float* Vector3f:: getVector3fArray()
{
    return coord;
}
void Vector3f::setX(float x) {
    this->coord[0] = x;
}
void Vector3f::setY(float y) {
    this->coord[1] = y;
}
void Vector3f::setZ(float z) {
    this->coord[2] = z;
}
float Vector3f::getX()
{
    return this->coord[0];
}
float Vector3f::getY()
{
    return this->coord[1];
}
float Vector3f::getZ()
{
    return this->coord[2];
}

Vector4f::Vector4f() {
    coording=new float[4];
    this->coording[0] = 0;
    this->coording[1] = 0;
    this->coording[2] = 0;
    this->coording[3] = 0;
}
Vector4f::Vector4f(float x1, float x2, float x3, float x4)
{
    coording=new float[4];
    this->coording[0] = x1;
    this->coording[1] = x2;
    this->coording[2] = x3;
    this->coording[3] = x4;
}
Vector4f::~Vector4f()
{
    delete coording;
}
void Vector4f::setFromEulerAngleToQuaternion(float yaw, float pitch, float roll) {
    float angle = 0.0f;
    float sr, sp, sy, cr, cp, cy;
    angle = yaw * 0.5f;
    sr = (float) sin(angle);
    cr = (float) cos(angle);
    angle = pitch * 0.5f;
    sp = (float) sin(angle);
    cp = (float) cos(angle);
    angle = roll * 0.5f;
    sy = (float) sin(angle);
    cy = (float) cos(angle);
    this->setX(sr * cp * cy - cr * sp * sy);
    this->setY(cr * sp * cy + sr * cp * sy);
    this->setZ(cr * cp * sy - sr * sp * cy);
    this->setW(cr * cp * cy + sr * sp * sy);
}
void Vector4f::interpolate(Vector4f* v1, Vector4f* v2, float alpha) {
    double dot = 0, s1, s2, om, sinom;
    for (int i = 0; i < 4; i++) {
        dot += v2->coording[i] * v1->coording[i];
    }
    Vector4f v0;
    if (dot < 0) {
        v0.coording[0] = -v1->coording[0];
        v0.coording[1] = -v1->coording[1];
        v0.coording[2] = -v1->coording[2];
        v0.coording[3] = -v1->coording[3];
        dot = -dot;
    } else {
        v0.coording[0] = v1->coording[0];
        v0.coording[1] = v1->coording[1];
        v0.coording[2] = v1->coording[2];
        v0.coording[3] = v1->coording[3];
    }
    if (dot > 0.999999) {
        s1 = 1.0 - alpha;
        s2 = alpha;
    } else {
        om = acos(dot);
        sinom = sin(om);
        s1 = sin((1.0 - alpha) * om) / sinom;
        s2 = sin(alpha * om) / sinom;
    }
    float tw=(float) (s1 * v0.getW() + s2 * v2->getW());
    float tx=(float) (s1 * v0.getX() + s2 * v2->getX());
    float ty=(float) (s1 * v0.getY() + s2 * v2->getY());
    float tz=(float) (s1 * v0.getZ() + s2 * v2->getZ());
    this->setW(tw);
    this->setX(tx);
    this->setY(ty);
    this->setZ(tz);
}
void Vector4f::setX(float x) {
    this->coording[0] = x;
}
void Vector4f::setY(float y) {
    this->coording[1] = y;
}
void Vector4f::setZ(float z) {
    this->coording[2] = z;
}
void Vector4f::setW(float w) {
    this->coording[3] = w;
}
float Vector4f::getX() {
    return this->coording[0];
}
float Vector4f::getY() {
    return this->coording[1];
}
float Vector4f::getZ() {
    return this->coording[2];
}
float Vector4f::getW() {
    return this->coording[3];
}

Mat4::Mat4(){
    this->matrix = new float[16];
}
Mat4::~Mat4()
{
    delete matrix;
}
void Mat4::loadIdentity()
{
    this->matrix = new float[16];
    Matrix::setIdentityM(matrix, 0);
}
void Mat4::setTranslation(Vector3f* v)
{
    this->matrix[12] = v->getX();
    this->matrix[13] = v->getY();
    this->matrix[14] = v->getZ();
}
void Mat4::genRotationFromEulerAngle(Vector3f* angles)
{
    float yaw=angles->getX();
    float pitch=angles->getY();
    float roll=angles->getZ();
    double cr = cos(yaw);
    double sr = sin(yaw);
    double cp = cos(pitch);
    double sp = sin(pitch);
    double cy = cos(roll);
    double sy = sin(roll);
    double srsp = sr * sp;
    double crsp = cr * sp;
    float* mTemp=new float[16];
    Matrix::setIdentityM(mTemp,0);
    mTemp[0] = (float) (cp * cy);
    mTemp[1] = (float) (cp * sy);
    mTemp[2] = (float) (-sp);
    mTemp[4] = (float) (srsp * cy - cr * sy);
    mTemp[5] = (float) (srsp * sy + cr * cy);
    mTemp[6] = (float) (sr * cp);
    mTemp[8] = (float) (crsp * cy + sr * sy);
    mTemp[9] = (float) (crsp * sy - sr * cy);
    mTemp[10] = (float) (cr * cp);
    float* mTempR=new float[16];
    Matrix::multiplyMM(mTempR, 0, matrix, 0, mTemp, 0);
    this->matrix=mTempR;
}
Mat4* Mat4::mul(Mat4* m1, Mat4* m2)
{
    float* mData = new float[16];
    Matrix::multiplyMM(mData, 0, m1->matrix, 0, m2->matrix, 0);
    delete this->matrix;
    this->matrix = mData;
    return this;
}
void Mat4::copyFrom(Mat4* m)
{
    memcpy(matrix, m->matrix, 16 * sizeof(float));
}
void Mat4::getRotateFromQuaternion(Vector4f* v)
{
    float x=v->getX();
    float y=v->getY();
    float z=v->getZ();
    float w=v->getW();
    this->matrix[0] = (1.0f - 2.0f * y * y - 2.0f * z * z);
    this->matrix[1] = (2.0f * (x * y + w * z));
    this->matrix[2] = (2.0f * (x * z - w * y));
    this->matrix[4] = (2.0f * (x * y - w * z));
    this->matrix[5] = (1.0f - 2.0f * x * x - 2.0f * z * z);
    this->matrix[6] = (2.0f * (y * z + w * x));
    this->matrix[8] = (2.0f * (x * z + w * y));
    this->matrix[9] = (2.0f * (y * z - w * x));
    this->matrix[10] = (1.0f - 2.0f * x * x - 2.0f * y * y);
    this->matrix[12] = (float) 0.0;
    this->matrix[13] = (float) 0.0;
    this->matrix[14] = (float) 0.0;
    this->matrix[3] = (float) 0.0;
    this->matrix[7] = (float) 0.0;
    this->matrix[11] = (float) 0.0;
    this->matrix[15] = (float) 1.0;
}
Vector3f* Mat4::invTransformAndRotate(Vector3f* point)
{
    Vector3f* v=new Vector3f();
    float x = point->getX() - this->matrix[12];
    float y = point->getY() - this->matrix[13];
    float z = point->getZ() - this->matrix[14];
    float tx=this->matrix[0] * x + this->matrix[1] * y + this->matrix[2]* z;
    float ty=this->matrix[4] * x + this->matrix[5] * y + this->matrix[6] * z;
    float tz=this->matrix[8] * x + this->matrix[9] * y + this->matrix[10]* z;
    v->setX(tx);
    v->setY(ty);
    v->setZ(tz);
    return v;
}
Vector3f* Mat4::transform(Vector3f* point)
{
    Vector3f* v=new Vector3f();
    v->setX(
            this->matrix[0] * point->getX()
            + this->matrix[4] * point->getY()
            + this->matrix[8] * point->getZ()
            + this->matrix[12]);
    v->setY(
            this->matrix[1] * point->getX()
            + this->matrix[5] * point->getY()
            + this->matrix[9] * point->getZ()
            + this->matrix[13]);
    v->setZ(
            this->matrix[2] * point->getX()
            + this->matrix[6] * point->getY()
            + this->matrix[10] * point->getZ()
            + this->matrix[14]);
    return v;
}
void Mat4::translate(float x, float y, float z)
{
    Matrix::translateM(matrix, 0, x, y, z);
}
void Mat4::rotate(float angle, float x, float y, float z)
{
    Matrix::rotateM(matrix, 0, angle, x, y, z);
}
void Mat4::scale(float x, float y, float z)
{
    Matrix::scaleM(this->matrix, 0, x, y, z);
}
void Mat4::print_matrix(Mat4 m)
{
    for(int i = 0; i < 4; i++)
    {
        printf("%f,%f,%f,%f\n",m.matrix[i*4],m.matrix[i*4+1],m.matrix[i*4+2],m.matrix[i*4+3]);
    }
}




