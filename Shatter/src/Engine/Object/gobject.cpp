//
// Created by maybe on 2021/5/26.
//

#include "gobject.h"
#include "aabb.h"
#include "bpool.h"

void GObject::genBuffer(const B_id& _id) {
    BPool::getPool().createVertexBuffer(_id,m_vertex_size,m_vertex);
    BPool::getPool().createIndexBuffer(_id,m_index_size,m_index);
}

void GObject::genVBuffer(const B_id & _id) {
    BPool::getPool().createVertexBuffer(_id,m_vertex_size,m_vertex);
}

void GObject::genIBuffer(const B_id & _id) {
    BPool::getPool().createIndexBuffer(_id,m_index_size,m_index);
}

