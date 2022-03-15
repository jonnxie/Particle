//
// Created by maybe on 2021/7/8.
//
#include "precompiledhead.h"

#include <thread>
#include "mpool.h"
#include "Engine/Object/line3d.h"
#include "Engine/Object/plane3d.h"
#include "Engine/Object/gobject.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/cobject.h"
#include "Engine/Object/aabb.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/configs.h"


template<>MPool<Line3d>* MPool<Line3d>::m_line3d_pool = new MPool<Line3d>(100);

template<>MPool<Plane3d>* MPool<Plane3d>::m_plane3d_pool = new MPool<Plane3d>(20);

template<>MPool<DObject>* MPool<DObject>::m_dobject_pool = new MPool<DObject>(100);

template<>MPool<GObject>* MPool<GObject>::m_gobject_pool = new MPool<GObject>(100);

template<>MPool<CObject>* MPool<CObject>::m_cobject_pool = new MPool<CObject>(100);

template<>MPool<VkDescriptorSet>* MPool<VkDescriptorSet>::m_set_pool = new MPool<VkDescriptorSet>(Config::getConfig("DefaultModelCount"));

template<>MPool<glm::mat4>* MPool<glm::mat4>::m_model_matrix_pool = new MPool<glm::mat4>(100);

template<>MPool<ObjectBox>* MPool<ObjectBox>::m_object_pool = new MPool<ObjectBox>(100);

template<>MPool<AABB>* MPool<AABB>::m_aabb_pool = new MPool<AABB>(100);

//template<class Object_Type>
//MPool<Object_Type> &MPool<Object_Type>::getNPool() {
//    if constexpr (std::is_convertible_v<Object_Type,Line3d>)
//    {
//        static MPool<Line3d> val(100);
//        return val;
//    }
//}
