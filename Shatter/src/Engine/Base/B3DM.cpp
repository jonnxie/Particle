//
// Created by AnWell on 2022/3/26.
//

#include "B3DM.h"

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
    int* batch_length = reinterpret_cast<int *>(featureTable.getData("BATCH_LENGTH", 0));
    batchTable = BatchTable(batch_table_buffer, *batch_length, 0, batchTableJSONByteLength, batchTableBinaryByteLength);

    glbStart = batch_table_start + batchTableJSONByteLength + batchTableBinaryByteLength;
    glbBytes = std::vector<char>(byteLength - glbStart);
    memcpy(glbBytes.data(), &binaryData[glbStart], byteLength - glbStart);
}

B3DMLoaderBase::~B3DMLoaderBase() {
    free(binaryData);
}
