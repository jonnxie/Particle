//
// Created by jonnxie on 2022/3/4.
//

#include "aabbVisiter.h"

void AABBVisitor::visitor(std::vector<glm::vec3> &_buffer, Object &_obj) {
    genVertexBufferFromAABB(_obj.m_aabb, _buffer);
}
