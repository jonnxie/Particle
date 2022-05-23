//
// Created by jonnxie on 2022/5/23.
//

#include "earth.h"

Earth::Earth(glm::vec3 _pos, uint32_t _longitudeResolution, uint32_t _latitudeResolution, float _radius,
             std::string _pipeline, std::vector<std::string> _sets, DrawObjectType _type) {

}

Earth::~Earth() {

}

void Earth::constructG() {
    Object::constructG();
}

void Earth::constructC() {
    Object::constructC();
}

void Earth::constructD() {
    Object::constructD();
}
