//
// Created by maybe on 2021/6/6.
//

#ifndef SHATTER_ENGINE_SHATTER_MACRO_H
#define SHATTER_ENGINE_SHATTER_MACRO_H

#include <map>

#define mapGet(id_type,val_type,type) \
val_type get_##type(id_type _id){       \
    return m_##type##_map[_id];     \
}

#define mapSet(id_type,val_type,type) \
void set##type(id_type _id,val_type _val){\
    m_##type##_map[_id] = _val;           \
}

#define mapCheckSet(id_type,val_type,type) \
void set##type(id_type _id,val_type _val){   \
    check_map(m_##type##_map)                \
    m_##type##_map[_id] = _val;           \
}


#define checkMap(map) \
for(auto& i:map){ \
    if(i.first == _id) throw std::runtime_error(_id + "is already exit");\
}

#define checkMapPrint(map) \
for(auto& i:map){ \
    if(i.first == _id) std::cout << _id + "is already exit" << std::endl;\
}

#define findMap(map) \
int num = 0;          \
for(auto &i:map){     \
    if(i.first != _id) num++;                 \
}                     \
if(num == map.size()){\
    std::cout << _id + "is not exit"<< std::endl;                      \
}

#define checkMapContinue(map) \
int num = 0;          \
for(auto &i:map){     \
    if(i.first != _id) num++;                 \
}                     \
if(num != map.size()){     \
    continue;                \
}

#define printLine(line) \
std::cout << "line`s begin :" << line.m_begin.x  << "," << line.m_begin.y << "," << line.m_begin.z << \
  " line`s end :" << line.m_end.x << "," << line.m_end.y << "," << line.m_end.z                      \
<<std::endl;

#define printPoint(point) \
std::cout << point.x << "," << point.y << "," << point.z << std::endl;

#define printUV(coor) \
std::cout << coor.x << "," << coor.y << std::endl;

#define VK_CHECK_RESULT(f)																				\
{                                                                                                       \
    std::map<int,std::string> vkresultMap = {                                                           \
    {0,std::string("VK_SUCCESS")},                                                                      \
    {1,std::string("VK_NOT_READY")},                                                                    \
    {2,std::string("VK_TIMEOUT")},                                                                      \
    {3,std::string("VK_EVENT_SET")},                                                                    \
    {4,std::string("VK_EVENT_RESET")},                                                                  \
    {5,std::string("VK_INCOMPLETE")},                                                                   \
    {-1,std::string("VK_ERROR_OUT_OF_HOST_MEMORY")},                                                     \
    {-2,std::string("VK_ERROR_OUT_OF_DEVICE_MEMORY")},                                                   \
    {-3,std::string("VK_ERROR_INITIALIZATION_FAILED")},                                                  \
    {-4,std::string("VK_ERROR_DEVICE_LOST")},                                                           \
    {-5,std::string("VK_ERROR_MEMORY_MAP_FAILED")},                                                     \
    {-6,std::string("VK_ERROR_LAYER_NOT_PRESENT")},                                                     \
    {-7,std::string("VK_ERROR_EXTENSION_NOT_PRESENT")},                                                    \
    {-8,std::string("VK_ERROR_FEATURE_NOT_PRESENT")},                                                      \
    {-9,std::string("VK_ERROR_INCOMPATIBLE_DRIVER")},                                                      \
    {-10,std::string("VK_ERROR_TOO_MANY_OBJECTS")},                                                         \
    {-11,std::string("VK_ERROR_FORMAT_NOT_SUPPORTED")},                                                         \
    {-12,std::string("VK_ERROR_FRAGMENTED_POOL")},                                                          \
    {-13,std::string("VK_ERROR_UNKNOWN")},                                                              \
    {-1000069000,std::string("VK_ERROR_OUT_OF_POOL_MEMORY")},           \
    {-1000072003,std::string("VK_ERROR_INVALID_EXTERNAL_HANDLE")},          \
    {-1000161000,std::string("VK_ERROR_FRAGMENTATION")},            \
    {-1000257000,std::string("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS")},           \
    {-1000000000,std::string("VK_ERROR_SURFACE_LOST_KHR")},         \
    {-1000000001,std::string("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR")},         \
    {-1000001003,std::string("VK_SUBOPTIMAL_KHR")},         \
    {-1000001004,std::string("VK_ERROR_OUT_OF_DATE_KHR")},          \
    {-1000003001,std::string("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR")},         \
    {-1000011001,std::string("VK_ERROR_VALIDATION_FAILED_EXT")},            \
    {-1000012000,std::string("VK_ERROR_INVALID_SHADER_NV")},            \
    {-1000158000,std::string("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT")},         \
    {-1000174001,std::string("VK_ERROR_NOT_PERMITTED_EXT")},            \
    {-1000255000,std::string("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT")},          \
    {1000268000,std::string("VK_THREAD_IDLE_KHR")},         \
    {1000268001,std::string("VK_THREAD_DONE_KHR")},         \
    {1000268002,std::string("VK_OPERATION_DEFERRED_KHR")},          \
    {1000268003,std::string("VK_OPERATION_NOT_DEFERRED_KHR")},          \
    {1000297000,std::string("VK_PIPELINE_COMPILE_REQUIRED_EXT")},           \
    {0x7FFFFFFF,std::string("VK_RESULT_MAX_ENUM")}          \
    };\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << vkresultMap[res] << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

#define WARNING(info)                                                                                   \
std::cout << "Warning:"  #info << std::endl;

#define PING(info)  \
printf(#info)

#define COMBINE(str,num)                                                                                \
std::string(str#num)

#define ADVANCE_UNORDERED(map,id,index)                                                                 \
auto iterator = map[id].begin();                                                                        \
std::advance(iterator,index);                                                                           \
map[id].erase(iterator);

#define genInOut(func,type) \
void func(type& _in,bool _inout){ \
    static type val;        \
    static std::mutex func##Mutex;\
    std::lock_guard<std::mutex> guard_lock(func##Mutex);\
    if(_inout){             \
        val = _in;                        \
    }else{                  \
        _in = val;                        \
    }\
}

#define delInOut(func,type) \
void func(type& _in,bool _inout);

#define STATE_IN true

#define STATE_OUT false

#define DebugFloat(f) \
std::cout << std::fixed << f << std::endl

#define Property(type,name) \
type name;\
PropertyGet(type,name)      \
PropertySet(type,name)

#define PropertyGet(type,name) \
type get##name(){               \
    return name;                               \
}

#define PropertySet(type,name) \
void set##name(type _val){               \
     name = _val;                          \
}


#endif //SHATTER_ENGINE_SHATTER_MACRO_H
